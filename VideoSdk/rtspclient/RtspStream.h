// RTSPstream.h: interface for the RTSP class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __RTSP_STREAM__
#define __RTSP_STREAM__

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
//#include "GroupsockHelper.hh"

#include <time.h>
#include <string>
#include <map>

#include "ObjectLists.h"
#include "pubutil.h"

using namespace std;

class RtspSession  
{
public:
	RtspSession();
	virtual ~RtspSession();

	int startRTSPClient(char const* progName, char const* rtspURL, int debugLevel);
	int stopRTSPClient();

	int openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, int debugLevel);

	bool CheckRunning();
	int CheckStatus();
	//多个链接
	//map<string, RTSPClient*> m_rtspList;

	RTSPClient* m_rtspClient;
	char volatile eventLoopWatchVariable;
	
	bool m_running;
	//pthread_t tid;
	uint32_t tid;
	static void *rtsp_thread_fun(void *param);
	void rtsp_fun();
	string m_rtspUrl;
	string m_progName;
	int m_debugLevel;

	int m_nStatus;
	int m_nID;
	float m_fScale;
	ObjectLists<RTP_BUFFER> * m_lsObj;
};

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState {
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;

  int m_nID;

  ObjectLists<RTP_BUFFER> * m_lsObj;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient {
public:
  static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

  int m_nStatus;

protected:
  ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~ourRTSPClient();

public:
    //int m_nID;
    StreamClientState scs;
	int nVideoWidth;
	int nVideoHeight;
	int nVideoFps;

	//ObjectLists<RTP_BUFFER> * m_lsObj;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
  static DummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)

  string sps;
  string url;
  int m_nIndex;
  struct timeval start_time;
  //外部设置数据队列
  void setBufferList(ObjectLists<RTP_BUFFER> *lsObj);
private:
  DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
  virtual ~DummySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  u_int8_t* fReceiveBuffer;
  MediaSubsession& fSubsession;
  char* fStreamId;
  SPropRecord* sPropRecords;
  uint8_t* buff;
  uint8_t* tmpbuf;
  bool m_blFirstFrame;
  bool m_blFirstKeyFrame;
  uint32_t m_nRecvNum;

  ObjectLists<RTP_BUFFER> *m_lsObj;
};


// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

#endif // __RTSP_STREAM__
