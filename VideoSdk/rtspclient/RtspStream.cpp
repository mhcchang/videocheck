// RTSP.cpp: implementation of the CRTSP class.
//
//////////////////////////////////////////////////////////////////////
//#include "stdafx.h"
//#include <stdint.h>

#include <math.h>
#include <string>

#include "sys_inc.h"
#include "sys_log.h"

#include "RtspStream.h"
#include "RtspInterface.h"

using namespace std;


//#define __DEBUG_SAVEFILE__
//for gettimeofday
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RtspSession::RtspSession()
{
	m_rtspClient = NULL;
	m_running = false;
	m_nStatus = 0;
	eventLoopWatchVariable = 0;
	m_nID = 0;
	m_lsObj = nullptr;
	//m_pCB = NULL;
}

RtspSession::~RtspSession()
{

}

bool RtspSession::CheckRunning()
{
	return m_running;
}

int RtspSession::CheckStatus()
{
	return m_nStatus;
}

int RtspSession::startRTSPClient(char const* progName, char const* rtspURL, int debugLevel)
{
	if (m_nStatus != 0)
		return 1;

	m_progName = progName;
	m_rtspUrl = rtspURL;
	m_debugLevel = debugLevel;
	eventLoopWatchVariable = 0;

	log_print(MHC_LOG_DEBUG, "%s %d, strUrl[%s] m_debugLevel[%d] \n", __FUNCTION__, __LINE__, rtspURL, m_debugLevel);

	tid = sys_os_create_thread((void *)&rtsp_thread_fun, (void*)this);
	if (tid == 0)
	{
		perror("pthread_create()");
		//retval = -1;
		//goto terminate;
		return -1;
	}

	return 0;
}

int RtspSession::stopRTSPClient()
{
	eventLoopWatchVariable = 1;
	m_running = false;
	return 0;
}

void *RtspSession::rtsp_thread_fun(void *param)
{
	RtspSession *pThis = (RtspSession*)param;
	pThis->rtsp_fun();
	return NULL;
}

void RtspSession::rtsp_fun()
{
	//::startRTSP(m_progName.c_str(), m_rtspUrl.c_str(), m_ndebugLever);
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	if (openURL(*env, m_progName.c_str(), m_rtspUrl.c_str(), m_debugLevel) == 0)
	{
		m_nStatus = 1;
		env->taskScheduler().doEventLoop(&eventLoopWatchVariable);

		printf("end doEventLoop %d\n ", eventLoopWatchVariable);
		m_running = false;
		eventLoopWatchVariable = 0;
		m_nStatus = 2;
		//20210621 mhc test
		if (m_rtspClient)
		{
			shutdownStream(m_rtspClient, 0);
		}
		m_rtspClient = NULL;

		//log_print(MHC_LOG_DEBUG, "%s %d, openURL[%s] m_nStatus[%d] \n", __FUNCTION__, __LINE__, m_rtspUrl.c_str(), m_nStatus);
	}

	env->reclaim();
	
	env = NULL;
	delete scheduler;
	scheduler = NULL;
	m_nStatus = 2;
}

int RtspSession::openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, int debugLevel)
{
	m_rtspClient = ourRTSPClient::createNew(env, rtspURL, debugLevel, progName);
	if (m_rtspClient == NULL)
	{
		//env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
		log_print(MHC_LOG_ERROR, "%s %d, Failed to create a RTSP client for URL[%s] \n", __FUNCTION__, __LINE__, rtspURL);

		return -1;
	}

	((ourRTSPClient*)(m_rtspClient))->scs.m_nID = m_nID;
	((ourRTSPClient*)(m_rtspClient))->scs.m_lsObj = m_lsObj;

	m_rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
	return 0;
}

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
	return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
	return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName) {
	//env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
	//env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	do
	{
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) 
		{
			env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
			delete[] resultString;
			break;
		}

		char* const sdpDescription = resultString;
		//env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";
		log_print(MHC_LOG_DEBUG, "%s %d, Got a SDP description[%s]\n", __FUNCTION__, __LINE__, sdpDescription);

		//sdpDescription
		//mhc sps parse
		
		// Create a media session object from this SDP description:
		scs.session = MediaSession::createNew(env, sdpDescription);
		delete[] sdpDescription; // because we don't need it anymore
		if (scs.session == NULL)
		{
			//env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
			log_print(MHC_LOG_ERROR, "%s %d, Failed to create a MediaSession object from the SDP description[%s]\n", __FUNCTION__, __LINE__, env.getResultMsg());
			break;
		}
		else if (!scs.session->hasSubsessions()) 
		{
			//env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			log_print(MHC_LOG_ERROR, "%s %d, This session has no media subsessions (i.e., no \"m=\" lines)\n", __FUNCTION__, __LINE__, env.getResultMsg());

			break;
		}

		// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
		// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
		// (Each 'subsession' will have its own data source.)
		scs.iter = new MediaSubsessionIterator(*scs.session);
		setupNextSubsession(rtspClient);
		return;
	} while (0);

	// An unrecoverable error occurred with this stream.
	shutdownStream(rtspClient, 1);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP True

void setupNextSubsession(RTSPClient* rtspClient)
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

	scs.subsession = scs.iter->next();
	if (scs.subsession != NULL)
	{
		//if (!scs.subsession->initiate())
		if (!scs.subsession->initiate())
		{
			//env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
			log_print(MHC_LOG_ERROR, "%s %d, Failed to initiate %p subsession: [%s] \n", __FUNCTION__, __LINE__, scs.subsession, env.getResultMsg());

			setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
		}
		else
		{
			//env << *rtspClient << "Initiated the \"" << *scs.subsession
			//	<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1 << ")\n";
			log_print(MHC_LOG_INFO, "%s %d, Initiated the %p subsession (client ports:[%d -- %d] ) \n", __FUNCTION__, __LINE__, scs.subsession, scs.subsession->clientPortNum(), scs.subsession->clientPortNum() + 1);
			// Continue setting up this subsession, by sending a RTSP "SETUP" command:
			rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
		}
		return;
	}

	// We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
	if (scs.session->absStartTime() != NULL)
	{
		// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
	}
	else
	{
		scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
	}
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	do
	{
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0)
		{
			//env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
			log_print(MHC_LOG_ERROR, "%s %d, Failed to set up the %p subsession (:[%s]) \n", __FUNCTION__, __LINE__, scs.subsession, resultString);

			break;
		}

		//env << *rtspClient << "Set up the \"" << *scs.subsession
		//	<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1 << ")\n";
		log_print(MHC_LOG_DEBUG, "%s %d, Set up the %p subsession (client ports:[%d -- %d] ) \n", __FUNCTION__, __LINE__, scs.subsession, scs.subsession->clientPortNum(), scs.subsession->clientPortNum() + 1);

		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)

		//目前只处理视频
		if (strcmp(scs.subsession->mediumName(), "video") == 0)
		{
			DummySink* pVideoSink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
			//
			pVideoSink->m_nIndex = scs.m_nID;
			pVideoSink->setBufferList(scs.m_lsObj);
			string name = pVideoSink->name();
			pVideoSink->sps = scs.subsession->fmtp_spropparametersets();

			//env <<*rtspClient<<"sps=" << pVideoSink->sps.c_str()<<endl;

			scs.subsession->sink = pVideoSink;
		}

		// perhaps use your own custom "MediaSink" subclass instead
		if (scs.subsession->sink == NULL) {
			//env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
			//	<< "\" subsession: " << env.getResultMsg() << "\n";
			log_print(MHC_LOG_ERROR, "%s %d, Failed to create a data sink for the %p subsession :[%s] \n", __FUNCTION__, __LINE__, scs.subsession, env.getResultMsg());

			break;
		}

		//env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
		log_print(MHC_LOG_DEBUG, "%s %d,Created a data sink for the %p subsession \n", __FUNCTION__, __LINE__, scs.subsession);

		scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
		scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
			subsessionAfterPlaying, scs.subsession);
		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if (scs.subsession->rtcpInstance() != NULL) 
		{
			scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
		}
	} while (0);
	delete[] resultString;

	// Set up the next subsession, if any:
	setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
	Boolean success = False;

	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0)
		{
			log_print(MHC_LOG_WARN, "%s %d, Failed to start playing session: %s \n", __FUNCTION__, __LINE__, resultString);

			env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
			break;
		}

		// Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
		// using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
		// 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
		// (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
		if (scs.duration > 0) 
		{
			unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
			scs.duration += delaySlop;
			unsigned uSecsToDelay = (unsigned)(scs.duration * 1000000);
			scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
		}

		log_print(MHC_LOG_INFO, "%s %d, Started playing session \n", __FUNCTION__, __LINE__);
		//env << *rtspClient << "Started playing session";
		if (scs.duration > 0)
		{
			log_print(MHC_LOG_DEBUG, "%s %d, for up to %10.6f seconds\n", __FUNCTION__, __LINE__, scs.duration);
			//env << " (for up to " << scs.duration << " seconds)";
		}
		//env << "...\n";

		success = True;
	} while (0);
	delete[] resultString;

	if (!success)
	{
		// An unrecoverable error occurred with this stream.
		shutdownStream(rtspClient, 2);
	}
}

// Implementation of the other event handlers:
void subsessionAfterPlaying(void* clientData)
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while ((subsession = iter.next()) != NULL)
	{
		if (subsession->sink != NULL) return; // this subsession is still active
	}

	// All subsessions' streams have now been closed, so shutdown the client:
	shutdownStream(rtspClient, 3);
}

void subsessionByeHandler(void* clientData)
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	UsageEnvironment& env = rtspClient->envir(); // alias

	//env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";
	log_print(MHC_LOG_DEBUG, "%s %d, Received RTCP \"BYE\" on  %p subsession\n", __FUNCTION__, __LINE__, subsession);
	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData)
{
	ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
	StreamClientState& scs = rtspClient->scs; // alias

	scs.streamTimerTask = NULL;

	// Shut down the stream:
	shutdownStream(rtspClient, 4);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode)
{
	ourRTSPClient* rtsp = (ourRTSPClient*)rtspClient;
	
	if (exitCode != 0)
	{
		return;
	}

	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

	// First, check whether any subsessions have still to be closed:
	if (scs.session != NULL) 
	{
		Boolean someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;

		while ((subsession = iter.next()) != NULL) 
		{
			if (subsession->sink != NULL) 
			{
				Medium::close(subsession->sink);
				subsession->sink = NULL;

				if (subsession->rtcpInstance() != NULL) 
				{
					subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
				}

				someSubsessionsWereActive = True;
			}
		}

		if (someSubsessionsWereActive) {
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			rtspClient->sendTeardownCommand(*scs.session, NULL);
		}
	}

	if (rtspClient)
	{
		env << *rtspClient << "Closing the stream.\n";
		Medium::close(rtspClient);
	}
}

// Implementation of "ourRTSPClient":
ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) 
{
	return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
	: RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) 
{
	m_nStatus = 0;
}

ourRTSPClient::~ourRTSPClient() 
{
}

// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
	: iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) 
{
}

StreamClientState::~StreamClientState()
{
	delete iter;
	if (session != NULL) 
	{
		// We also need to delete "session", and unschedule "streamTimerTask" (if set)
		UsageEnvironment& env = session->envir(); // alias

		env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
		Medium::close(session);
	}
}

// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:\\

//3840*2160*3 mhc 2022/01/20 yuv...
//#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 27648000
//#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 24883200
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 12441600
uint8_t* buff = NULL;

int starttime;

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
	return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
	: MediaSink(env),
	fSubsession(subsession) 
{
	fStreamId = strDup(streamId);
	fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];

	memset(fReceiveBuffer, 0, 20480);
	//buff = new uint8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
	buff = new uint8_t[20480];
	sPropRecords = NULL;
	m_nIndex = -1;

	m_blFirstFrame = true;
	m_blFirstKeyFrame = false;
	m_nRecvNum = 0;

	m_lsObj = nullptr;
}

DummySink::~DummySink() 
{
	delete[] fReceiveBuffer;
	delete[] fStreamId;
	delete[] buff;
	if (sPropRecords)
		delete[] sPropRecords;

	//delete[] tmpbuf;
}

void DummySink::setBufferList(ObjectLists<RTP_BUFFER> *lsObj)
{
	m_lsObj = lsObj;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
	struct timeval presentationTime, unsigned durationInMicroseconds) {
	DummySink* sink = (DummySink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

int Width = 0;
int Height = 0;
unsigned numSPropRecords;

////bool h264_decode_seq_parameter_set(uint8_t * buf, uint32_t nLen, int &Width, int &Height);
//void BinToHexStr(uint8_t *Bin, int nlen, uint8_t * Hex)
//{
//	uint16 i;
//	uint8 j;
//
//	for (i = 0; i < nlen; i++)
//	{
//		j = (Bin[i] >> 4) & 0xf;
//
//		if (j <= 9)
//		{
//			Hex[i * 2] = (j + '0');
//		}
//		else
//		{
//			Hex[i * 2] = (j + 'a' - 10);
//		}
//
//		j = Bin[i] & 0xf;
//
//		if (j <= 9)
//		{
//			Hex[i * 2 + 1] = (j + '0');
//		}
//		else
//		{
//			Hex[i * 2 + 1] = (j + 'a' - 10);
//		}
//	};
//
//	Hex[nlen * 2] = '\0';
//};

static int ic = 0;
void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
	struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
	//m_blFirstFrame = false;
	unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	if (!strcmp(fSubsession.mediumName(), "video"))
	{
		if (m_nIndex >= 0)
		{
			//live555的第一帧是 sps pps信息 一般和i帧分开 
			if (m_blFirstFrame)
			{
				if (strcmp(fSubsession.codecName(), "H264") == 0 || strcmp(fSubsession.codecName(), "h264") == 0)
				{
					unsigned int num;
					SPropRecord *sps = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), num);
					// For H.264 video stream, we use a special sink that insert start_codes:
					struct timeval tv = { 0,0 };

					uint8_t* tmpbuf = new uint8_t[20480];

					memcpy(tmpbuf, start_code, 4);
					memcpy(tmpbuf + 4, sps[0].sPropBytes, sps[0].sPropLength);
					memcpy(tmpbuf + 4 + sps[0].sPropLength, tmpbuf, 4);
					memcpy(tmpbuf + 8 + sps[0].sPropLength, sps[1].sPropBytes, sps[1].sPropLength);
					m_nRecvNum = 8 + sps[0].sPropLength + sps[1].sPropLength;
					if (m_lsObj)
					{
						RTP_BUFFER tbuf = { 11, m_nRecvNum, tmpbuf};
						m_lsObj->pushObj(tbuf);

						std::string sRet = "";
						char tmpMsg[4] = { 0 };

						for (int i = 0; i < tbuf.uLen; i++)
						{
							sprintf(tmpMsg, "%02x ", tmpbuf[i]);
							sRet += tmpMsg;
						}
						envir() << "----m_blFirstFrame-push_back 264 head " << tbuf.uLen << " [" << sRet.c_str() << "] m_lsObj=" << m_lsObj << "\r\n";
						log_print(MHC_LOG_INFO, "%s %d, m_blFirstFrame push_back 264 head size[%d] [%s] frame size=[%d] \n", __FUNCTION__, __LINE__, tbuf.uLen, sRet.c_str(), m_lsObj->size());
						//delete tmpMsg;
					}

					log_print(MHC_LOG_DEBUG, "%s %d, -----push_back 264 size[%d] \n", __FUNCTION__, __LINE__, m_nRecvNum);
					envir() << "-----push_back 264 " << m_nRecvNum << "\r\n";
					delete[] sps;
					m_blFirstFrame = false;
				}
				else if (strcmp(fSubsession.codecName(), "H265") == 0 || strcmp(fSubsession.codecName(), "h265") == 0)
				{
					uint8_t* tmpbuf = new uint8_t[20480];
					
					unsigned int npos = 0;
					unsigned numSPropRecords;
					SPropRecord* sPropRecords = parseSPropParameterSets(fSubsession.fmtp_spropvps(), numSPropRecords);
					for (unsigned i = 0; i < numSPropRecords; ++i) 
					{
						memcpy(tmpbuf + npos, start_code, 4);
						npos += 4;
						memcpy(tmpbuf + npos, sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
						npos += sPropRecords[i].sPropLength;
					}
					delete[] sPropRecords;
					sPropRecords = parseSPropParameterSets(fSubsession.fmtp_spropsps(), numSPropRecords);
					for (unsigned i = 0; i < numSPropRecords; ++i)
					{
						memcpy(tmpbuf + npos, start_code, 4);
						npos += 4;
						memcpy(tmpbuf + npos, sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
						npos += sPropRecords[i].sPropLength;
					}
					delete[] sPropRecords;
					sPropRecords = parseSPropParameterSets(fSubsession.fmtp_sproppps(), numSPropRecords);
					for (unsigned i = 0; i < numSPropRecords; ++i)
					{
						memcpy(tmpbuf + npos, start_code, 4);
						npos += 4;
						memcpy(tmpbuf + npos, sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
						npos += sPropRecords[i].sPropLength;
					}
					delete[] sPropRecords;
					//frame data
					if (m_lsObj) 
					{
						RTP_BUFFER tbuf = { 11, npos, tmpbuf};
						
						m_lsObj->pushObj(tbuf);
						envir() << "-----push_back 265 head" << tbuf.uLen << " \n";
						log_print(MHC_LOG_DEBUG, "%s %d, -----push_back 265 size[%d] [%p]\n", __FUNCTION__, __LINE__, tbuf.uLen, tmpbuf);
					}
					m_blFirstFrame = false;
				}
			}
			else 
			{
				if (strcmp(fSubsession.codecName(), "H264") == 0)
				{
					if (m_blFirstKeyFrame == false)
					{
						//还没有i帧  查i帧信息 不是的话跳过
						int keyframe = ((fReceiveBuffer[0] & 0x1F) == 5);

						unsigned int num;
						SPropRecord *sps = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), num);
						// For H.264 video stream, we use a special sink that insert start_codes:
						struct timeval tv = { 0,0 };

						uint8_t* tmpbuf = new uint8_t[20480];

						memcpy(tmpbuf, start_code, 4);
						memcpy(tmpbuf + 4, sps[0].sPropBytes, sps[0].sPropLength);
						memcpy(tmpbuf + 4 + sps[0].sPropLength, tmpbuf, 4);
						memcpy(tmpbuf + 8 + sps[0].sPropLength, sps[1].sPropBytes, sps[1].sPropLength);
						m_nRecvNum = 8 + sps[0].sPropLength + sps[1].sPropLength;
						if (m_lsObj)
						{
							RTP_BUFFER tbuf = { 11, m_nRecvNum, tmpbuf };
							m_lsObj->pushObj(tbuf);

							std::string sRet = "";
							char tmpMsg[4] = { 0 };// = new char[tbuf.uLen * 3 + 1];
													//memset(tmpMsg, 20, tbuf.uLen * 3 + 1);
							for (int i = 0; i < tbuf.uLen; i++)
							{
								//uint8_t tt = *(uint8_t*)(tmpbuf + i);

								sprintf(tmpMsg, "%02x ", tmpbuf[i]);
								sRet += tmpMsg;
							}

							log_print(MHC_LOG_DEBUG, "%s %d, ----keyframe -push_back 264 head size[%d] [ %s ] m_lsObj= %p \n", __FUNCTION__, __LINE__, tbuf.uLen, sRet.c_str(), m_lsObj);

						}
						delete[] sps;

						uint8_t* listbuf = NULL;
						m_nRecvNum = frameSize + 4;
						listbuf = new uint8_t[m_nRecvNum];
						memcpy(listbuf, start_code, 4);
						memcpy(listbuf + 4, fReceiveBuffer, frameSize);

						if (m_lsObj)
						{
							//rtp数据压栈
							RTP_BUFFER tbuf = { 1, m_nRecvNum, listbuf};
							m_lsObj->pushObj(tbuf);
							//envir() << "---264--push_back key data " << tbuf.uLen << " m_lsObj=" << listbuf << "\r\n";
							//printf(" %d NO ----264 -push_back data len[%d] fReceiveBuffer=[%p] \n", __LINE__, tbuf.uLen, fReceiveBuffer);
							
							log_print(MHC_LOG_DEBUG, "%s %d, ---264--push_back key data[%d] m_lsObj[%p] \n", __FUNCTION__, __LINE__, tbuf.uLen, m_lsObj);
							if (keyframe == 1)
								m_blFirstKeyFrame = true;
						}

					}
					else
					{
						//已经发送关键帧
						uint8_t* listbuf = NULL;
						m_nRecvNum = frameSize + 4;
						listbuf = new uint8_t[m_nRecvNum];
						memcpy(listbuf, start_code, 4);
						memcpy(listbuf + 4, fReceiveBuffer, frameSize);

						if (frameSize <= 43)
						{
							//envir() << "---264 push Frame data pps " << frameSize << " m_lsObj=" << m_lsObj << "\r\n";
							log_print(MHC_LOG_DEBUG, "%s %d, ---264 push Frame data pps[%d] m_lsObj[%p] \n", __FUNCTION__, __LINE__, frameSize, m_lsObj);
						}
						int keyframe = ((fReceiveBuffer[0] & 0x1F) == 5);
						if (m_lsObj)
						{
							//rtp数据压栈
							RTP_BUFFER tbuf = { (uint32_t)(keyframe == 1 ? 1 : 2), m_nRecvNum, listbuf };
							m_lsObj->pushObj(tbuf);

							//printf(" %d ----264 -push_back data len[%d] fReceiveBuffer=[%p] \n", __LINE__, tbuf.uLen, fReceiveBuffer);

							if (keyframe == 1)
							{
								//envir() << "---264 push Frame data key " << frameSize << " m_lsObj=" << m_lsObj << "\r\n";
								log_print(MHC_LOG_DEBUG, "%s %d, ---264 push Frame data key[%d] m_lsObj[%p] \n", __FUNCTION__, __LINE__, frameSize, m_lsObj);
							}
							else
							{
								//envir() << "---264 push Frame data " << frameSize << " m_lsObj=" << m_lsObj << "\r\n";
								log_print(MHC_LOG_DEBUG, "%s %d, 264 push Frame key[%d] m_lsObj[%p] \n", __FUNCTION__, __LINE__, frameSize, m_lsObj);
							}
						}
						else
						{
							envir() << "---264 not push Frame data " << frameSize << " m_lsObj=" << m_lsObj << "\r\n";
							log_print(MHC_LOG_DEBUG, "%s %d, ---264 not push Frame data[%d] m_lsObj[%p] \n", __FUNCTION__, __LINE__, frameSize, m_lsObj);
						}
					}
				}
				else if (strcmp(fSubsession.codecName(), "H265") == 0)
				{
					if (m_blFirstKeyFrame == false)
					{
						int type = (fReceiveBuffer[0] & 0x7E) >> 1;
						int keyframe = 0;
						if (type <= 9)
						{
							//p 
						}
						else if (type >= 16 && type <= 21)
						{
							//i
							envir() << "--265 frame type " << type << " i \n";
							log_print(MHC_LOG_DEBUG, "%s %d, --265 key frame type[%d]\n", __FUNCTION__, __LINE__, type);
							keyframe = 1;
						}
						else
						{
							//envir() << "--265 frame type " << type << " etc... \n";
							log_print(MHC_LOG_DEBUG, "%s %d, --265 frame type[%d] etc...\n", __FUNCTION__, __LINE__, type);
						}

						uint8_t* listbuf = NULL;

						m_nRecvNum = frameSize + 4;
						listbuf = new uint8_t[m_nRecvNum];
						memcpy(listbuf, start_code, 4);
						memcpy(listbuf + 4, fReceiveBuffer, frameSize);

						if (m_lsObj)
						{
							//rtp数据压栈
							RTP_BUFFER tbuf = {1, m_nRecvNum, listbuf };
							m_lsObj->pushObj(tbuf);
							//envir() << "-----push_back key data " << tbuf.uLen << " \n";
							//printf("++ %d NOkey ----265 -push_back data listbuf=[%p] [len=%d] [1] [ic=%d]\n", __LINE__, listbuf, tbuf.uLen, ic++);
							log_print(MHC_LOG_DEBUG, "%s %d, -----push_back key data[%d] [%d] \n", __FUNCTION__, __LINE__, tbuf.uLen, ic++);

							if (keyframe == 1)
								m_blFirstKeyFrame = true;
						}
					}
					else
					{
						int type = (fReceiveBuffer[0] & 0x7E) >> 1;
						int keyframe = 2;
						if (type <= 9)
						{
							//p
							//envir() << "--265 frame type p " << type << " \n";
							log_print(MHC_LOG_DEBUG, "%s %d, --265 frame type p [%d] \n", __FUNCTION__, __LINE__, type);
						}
						else if (type >= 16 && type <= 21)
						{
							//envir() << "--265 frame type " << type << " i \n";
							log_print(MHC_LOG_DEBUG, "%s %d, --265 key frame type[%d] \n", __FUNCTION__, __LINE__, type);
							keyframe = 1;
						}
						else
						{
							//envir() << "--265 frame type " << type << " etc... \n";
							keyframe = 3;
							log_print(MHC_LOG_DEBUG, "%s %d, --265 frame type[%d] etc...\n", __FUNCTION__, __LINE__, type);
						}

						if (m_lsObj)
						{
							uint8_t* listbuf = NULL;

							m_nRecvNum = frameSize + 4;
							listbuf = new uint8_t[m_nRecvNum];
							memcpy(listbuf, start_code, 4);
							memcpy(listbuf + 4, fReceiveBuffer, frameSize);

							RTP_BUFFER tbuf = { (uint32_t)keyframe, m_nRecvNum, listbuf };
							m_lsObj->pushObj(tbuf);
							//envir() << "----265 -push_back data " << tbuf.uLen << " fReceiveBuffer=" << fReceiveBuffer << " \n";
							//printf("++ %d ----265 -push_back data  listbuf=[%p] len[%d] [%d] [%d]\n", __LINE__, listbuf, tbuf.uLen, keyframe, ic++);
						}
					}
				}
			}
		}
	}

	continuePlaying();
}

Boolean DummySink::continuePlaying()
{
	if (fSource == NULL) return False; // sanity check (should not happen)
		// Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
	fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE - 1024,
		afterGettingFrame, this,
		onSourceClosure, this);

	//envir() << " DummySink::continuePlaying " << " \n";
	return True;
}
