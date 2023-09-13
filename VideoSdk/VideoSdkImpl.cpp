
#if defined(_WIN32) || defined(_WIN64)
#include "framework.h"
#endif

#include "VideoSdkImpl.h"
#include "sys_log.h"
#include "pubutil.h"

#include <math.h>
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"

/**
* @brief sdk
*	私有协议
*  1 初始化 加载库
*  2  init 
*  3  
* @author 
*/

VideoSdkImpl * VideoSdkImpl::m_pInstance = nullptr;

VideoSdkImpl::VideoSdkImpl()
{
	m_nLastError = 0;
	m_blInit = false;

	m_blRunning = false;

	m_nMaxChannel = 0;

	log_init("videochk.log");
	log_set_level(MHC_LOG_ERROR);

	m_blImm = true;
	m_nMaxThread = 64;
	m_nDuration = 10;
	m_tmThStart = 0;

	RtspInterface::Instance();

	start();
}

VideoSdkImpl::~VideoSdkImpl()
{
}

VideoSdkImpl * VideoSdkImpl::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new VideoSdkImpl();
	}

	return m_pInstance;
}

void VideoSdkImpl::start()
{
	if (m_blRunning)
		return;

	m_blRunning = true;

	m_stChkParam.delayVideo = 2;
	m_stChkParam.frameRate = 25;
	m_stChkParam.qualityThreshold = 0.7;

	m_thread = std::thread(&VideoSdkImpl::process, this);
}

void VideoSdkImpl::stop()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_blRunning)
		return;

	m_blRunning = false;
}

void VideoSdkImpl::process()
{
	m_nFailTime = 0;
	while (m_blRunning)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_waits.size() > 0)
		{
			time_t time_now;
			time_now = time(NULL);
			struct tm * st = localtime(&(time_now));
			char sztm[256] = { 0 };
			sprintf(sztm, "%02d%02d", st->tm_hour, st->tm_min);

			if (m_blImm || sztm > m_szTime)
			{
				uint64_t tmn = MHC_STLNOW_SECOND;
				if (tmn - m_tmThStart > m_nDuration * 2)
				{
					int nm = 0;
					std::lock_guard<std::mutex> lock(m_mutexWait);
					for (std::map<std::string, std::string>::const_iterator iter = m_waits.begin(); iter != m_waits.end(); )
					{
						std::lock_guard<std::mutex> lock(m_mutexMap);
						std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(iter->first);
						if (channel != m_channels.end())
						{
							channel->second->SetPollingParam(m_blImm, m_szTime, m_nMaxThread);
							channel->second->SetWorkMode(3, m_nDuration);
						}
						else
						{
							VideoChannel* vc = new VideoChannel();
							vc->DeviceID = iter->first;
							vc->RtspUrl = iter->second;
							vc->SetPollingParam(m_blImm, m_szTime, m_nMaxThread);
							vc->SetWorkMode(3, m_nDuration);
							m_channels[iter->first] = vc;
						}
						//delete node
						iter = m_waits.erase(iter);

						nm++;
						if (nm >= m_nMaxThread)
						{
							break;
						}

						//sleep for next thread
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
					}

					//reset time start
					m_tmThStart = MHC_STLNOW_SECOND;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

bool VideoSdkImpl::Init(int delayVideo, int frameRate, double qualityThreshold)
{
	m_blInit = true;

	m_stChkParam.delayVideo = delayVideo;
	m_stChkParam.frameRate = frameRate;
	m_stChkParam.qualityThreshold = qualityThreshold;
	return true;
}

bool VideoSdkImpl::UnInit()
{
	m_blInit = false; 
	return true;
}

#include "document.h"

std::vector<std::string> splitstr(std::string strtem, char a)
{
	std::vector<std::string> strvec;

	std::string::size_type pos1, pos2;
	pos2 = strtem.find(a);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		strvec.push_back(strtem.substr(pos1, pos2 - pos1));

		pos1 = pos2 + 1;
		pos2 = strtem.find(a, pos1);
	}
	strvec.push_back(strtem.substr(pos1));
	return strvec;
}

#include "MD5.hpp"
#include "sole.hpp"
std::string GenMsgId(std::string r1)
{
	MD5 md5;
	std::string str = r1 + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	md5.read(str);
	str = md5.Encryption();
	return str.substr(8, 16);
}

int VideoSdkImpl::SetStreamStatusQuest(const char* quest)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	std::map<std::string, std::string> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId") || !arr.HasMember("rtspUrl"))
			continue;
		devices[arr["DeviceId"].GetString()] = (arr["rtspUrl"].GetString());
	}

	////lock map  
	{
		std::lock_guard<std::mutex> lock(m_mutexMap);
		for (std::map<std::string, std::string>::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
		{
			//重新设置工作模式
			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(iter->first);
			if (channel != m_channels.end())
				channel->second->SetWorkMode(2, m_stChkParam.delayVideo);
			else
			{
				VideoChannel* vc = new VideoChannel();
				vc->DeviceID = iter->first;
				vc->RtspUrl = iter->second;
				vc->SetWorkMode(2, m_stChkParam.delayVideo);
				m_channels[iter->first] = vc;
			}
		}
	}

	return 0;
}

void VideoSdkImpl::InterMakeStreamStatus(std::list<stru_VideoStatus> resStatus, std::string& res)
{
	rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

	writer.StartObject();

	writer.Key("StreamStatusList");

	writer.StartArray();

	for (stru_VideoStatus &iter : resStatus)
	{
		writer.StartObject();
		writer.Key("DeviceId");
		writer.String(iter.DeviceID.c_str());

		writer.Key("Online");
		writer.String(iter.Online.c_str());
		writer.Key("VideoQuality");
		writer.String(iter.VideoQuality.c_str());
		writer.Key("FrameRate");
		writer.Int(iter.FrameRate);

		writer.EndObject();
	}
	writer.EndArray();
	writer.EndObject();
	res = strBuf.GetString();
}

/**
* @brief 
* @param quest
	json格式
* @return 返回值无
*/
int VideoSdkImpl::GetStreamStatusRespond(const char* quest, char* resBody, int& reslen)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	std::vector<std::string> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId"))
			continue;
		devices.push_back(arr["DeviceId"].GetString());
	}
	std::list<stru_VideoStatus> resStatus;
	{
		reslen = 0;
		
		std::lock_guard<std::mutex> lock(m_mutexMap);
		uint64_t tm = MHC_STLNOW_MILLISEC;
		for (std::string device : devices)
		{
			int nKeyFrameCount; 
			int nFrameRate;
			
			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(device);
			if (channel != m_channels.end())
			{
				channel->second->GetWorkResult(nKeyFrameCount, nFrameRate);

				stru_VideoStatus status;
				status.DeviceID = channel->second->DeviceID;
				status.ChannelID = channel->second->RtspUrl;
				status.FrameRate = nFrameRate;
				status.KeyFrameCount = nKeyFrameCount;

				if (nKeyFrameCount > 0)
				{
					status.Online = "Online";
				}
				else
					status.Online = "Offline";

				float quality = nFrameRate * 1.0f / m_stChkParam.frameRate;
				if (quality >= 0.95)
					status.VideoQuality = "High";
				else if (quality < 0.95 && quality >= 0.75)
					status.VideoQuality = "Normal";
				else if (quality < 0.75 && quality >= 0.5)
					status.VideoQuality = "Low";
				else
					status.VideoQuality = "Poor";

				resStatus.push_back(status);

				reslen++;
			}
			else
			{
				log_print(MHC_LOG_WARN, "data error deviceid =%s \n", device.c_str());
			}
		}
	}
	
	if (resBody == nullptr)
		return -2;

	std::string szres;
	InterMakeStreamStatus(resStatus, szres);
	strncpy(resBody, szres.c_str(), szres.length());

	return 0;
}

int VideoSdkImpl::SetForceKeyFrameQuest(const char* quest)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	std::map<std::string, std::string> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId") || !arr.HasMember("rtspUrl"))
			continue;
		devices[arr["DeviceId"].GetString()] = (arr["rtspUrl"].GetString());
	}

	////lock map  
	{
		std::lock_guard<std::mutex> lock(m_mutexMap);
		for (std::map<std::string, std::string>::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
		{
			//重新设置工作模式
			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(iter->first);
			if (channel != m_channels.end())
				channel->second->SetWorkMode(1, 3);
			else
			{
				VideoChannel* vc = new VideoChannel();
				vc->DeviceID = iter->first;
				vc->RtspUrl = iter->second;
				vc->SetWorkMode(1, 3);
				m_channels[iter->first] = vc;
			}
		}
	}

	return 0;
}

void VideoSdkImpl::InterMakeForceKeyFrameStatus(std::list<stru_VideoStatus> resStatus, std::string& res)
{
	rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

	writer.StartObject();

	writer.Key("ForceKeyFrameStatusList");

	writer.StartArray();

	for (stru_VideoStatus &iter : resStatus)
	{
		writer.StartObject();
		writer.Key("DeviceId");
		writer.String(iter.DeviceID.c_str());

		writer.Key("Status");
		writer.String(iter.ForceKeyStatus.c_str());

		writer.EndObject();
	}

	writer.EndArray();
	writer.EndObject();
	res = strBuf.GetString();
}

int VideoSdkImpl::GetForceKeyFrameRespond(const char* quest, char* resBody, int& reslen)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	std::vector<std::string> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId"))
			continue;
		devices.push_back(arr["DeviceId"].GetString());
	}
	std::list<stru_VideoStatus> resStatus;
	{
		reslen = 0;

		std::lock_guard<std::mutex> lock(m_mutexMap);
		for (std::string device : devices)
		{
			int nKeyFrameCount;
			int nFrameRate;

			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(device);
			if (channel != m_channels.end())
			{
				channel->second->GetWorkResult(nKeyFrameCount, nFrameRate);

				stru_VideoStatus status;
				status.DeviceID = channel->second->DeviceID;
				status.ChannelID = channel->second->RtspUrl;
				status.FrameRate = nFrameRate;
				status.KeyFrameCount = nKeyFrameCount;

				status.Duration = channel->second->GetDuration();
				uint64_t tm = MHC_STLNOW_SECOND;

				printf("%s %d status.Duration %d \n", channel->first.c_str(), nKeyFrameCount, status.Duration);

				if (nKeyFrameCount > 0)
					status.ForceKeyStatus = "CHECKED";
				else
					status.ForceKeyStatus = "UNCKECKED";
				resStatus.push_back(status);

				reslen++;
			}
			else
			{
				log_print(MHC_LOG_WARN, "data error deviceid =%s \n", device.c_str());
			}
		}
	}

	if (resBody == nullptr)
		return -2;

	std::string szres;
	InterMakeForceKeyFrameStatus(resStatus, szres);
	strncpy(resBody, szres.c_str(), szres.length());

	return 0;
}

int VideoSdkImpl::SetKeyFrameQuest(const char* quest)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;
	
	if (!doc.HasMember("Duration") || !doc["Duration"].IsNumber())
		return -1; 
	std::map<std::string, std::string> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId") || !arr.HasMember("rtspUrl"))
			continue;
		devices[arr["DeviceId"].GetString()] = (arr["rtspUrl"].GetString());
	}
	int duration = doc["Duration"].GetInt();
	////lock map  
	{
		std::lock_guard<std::mutex> lock(m_mutexMap);
		for (std::map<std::string, std::string>::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
		{
			//重新设置工作模式
			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(iter->first);
			if (channel != m_channels.end())
				channel->second->SetWorkMode(2, duration);
			else
			{
				VideoChannel* vc = new VideoChannel();
				vc->DeviceID = iter->first;
				vc->RtspUrl = iter->second;
				vc->SetWorkMode(2, duration);
				m_channels[iter->first] = vc;
			}
		}
	}

	return 0;
}

void VideoSdkImpl::InterMakeKeyFrameStatus(std::list<stru_VideoStatus> resStatus, std::string& res)
{
	rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

	writer.StartObject();

	writer.Key("KeyFrameStatusList");

	writer.StartArray();

	for (stru_VideoStatus &iter : resStatus)
	{
		writer.StartObject();
		writer.Key("DeviceId");
		writer.String(iter.DeviceID.c_str());

		writer.Key("Status");
		writer.String(iter.status.c_str());
		writer.Key("Duration");
		writer.Int(iter.Duration);
		writer.Key("KeyFrameCount");
		writer.Int(iter.KeyFrameCount);
		writer.Key("FrameRate");
		writer.Int(iter.FrameRate);
		writer.EndObject();
	}
	writer.EndArray();
	writer.EndObject();

	res = strBuf.GetString();
}

int VideoSdkImpl::GetKeyFrameRespond(const char* quest, char* resBody, int& reslen)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	std::vector<std::string> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId"))
			continue;
		devices.push_back(arr["DeviceId"].GetString());
	}
	std::list<stru_VideoStatus> resStatus;
	{
		reslen = 0;

		std::lock_guard<std::mutex> lock(m_mutexMap);
		for (std::string device : devices)
		{
			int nKeyFrameCount;
			int nFrameRate;

			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(device);
			if (channel != m_channels.end())
			{
				channel->second->GetWorkResult(nKeyFrameCount, nFrameRate);

				stru_VideoStatus status;
				status.DeviceID = channel->second->DeviceID;
				status.ChannelID = channel->second->RtspUrl;
				status.FrameRate = nFrameRate;
				status.KeyFrameCount = nKeyFrameCount;

				status.Duration = channel->second->GetDuration();
				uint64_t tm = MHC_STLNOW_SECOND;
				if (nKeyFrameCount > 0)
				{
					status.Online = "Online";
				}
				else
					status.Online = "Offline";
				//printf("channel %s nKeyFrameCount %d videoStatus.Duration %d \n", channel->first.c_str(), nKeyFrameCount, status.Duration);

				if (nKeyFrameCount >= status.Duration - 1)
					status.status = "CHECKPASS";
				else if (channel->second->IsCheckIng())
					status.status = "CHECKING";
				else
					status.status = "CHECKFAILED";
				resStatus.push_back(status);

				reslen++;
			}
			else
			{
				log_print(MHC_LOG_WARN, "data error deviceid =%s \n", device.c_str());
			}
		}
	}

	if (resBody == nullptr)
		return -2;

	std::string szres;
	InterMakeKeyFrameStatus(resStatus, szres);
	strncpy(resBody, szres.c_str(), szres.length());

	return 0;
}

int VideoSdkImpl::SetPollingQuest(const char* quest)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	if (!doc.HasMember("Duration") || !doc["Duration"].IsNumber())
		return -1;

	std::lock_guard<std::mutex> lock(m_mutexWait);
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId") || !arr.HasMember("rtspUrl"))
			continue;
		m_waits[arr["DeviceId"].GetString()] = (arr["rtspUrl"].GetString());
	}
	int duration = doc["Duration"].GetInt();
	if (!doc.HasMember("Immediately") || !doc["Immediately"].IsBool())
		return -1;
	if (!doc.HasMember("ScheduleTime") || !doc["ScheduleTime"].IsString())
		return -1;
	if (!doc.HasMember("MaxThread") || !doc["MaxThread"].IsNumber())
		return -1;

	bool bImmediate = doc["Immediately"].GetBool();
	std::string scheduleTime = doc["ScheduleTime"].GetString();
	int nMaxThread = doc["MaxThread"].GetInt();
	m_nDuration = duration;

	////lock map   move to main loop
	//{
	//	m_tmThStart = MHC_STLNOW_SECOND;
	//	m_blImm = bImmediate;
	//	m_szTime = scheduleTime;
	//	m_nMaxThread = nMaxThread;

	//	time_t time_now;
	//	time_now = time(NULL);
	//	struct tm * st = localtime(&(time_now));
	//	char sztm[256] = { 0 };
	//	sprintf(sztm, "02d%02d", st->tm_hour, st->tm_min);

	//	//立即执行的 还有就是
	//	if (m_blImm || sztm > m_szTime)
	//	{
	//		int nm = 0;
	//		std::lock_guard<std::mutex> lock(m_mutexMap);
	//		for (std::map<std::string, std::string>::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
	//		{
	//			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(iter->first);
	//			if (channel != m_channels.end())
	//			{
	//				channel->second->SetPollingParam(bImmediate, scheduleTime, nMaxThread);
	//				channel->second->SetWorkMode(3, duration);
	//			}
	//			else
	//			{
	//				VideoChannel* vc = new VideoChannel();
	//				vc->DeviceID = iter->first;
	//				vc->RtspUrl = iter->second;
	//				vc->SetPollingParam(bImmediate, scheduleTime, nMaxThread);
	//				vc->SetWorkMode(3, duration);
	//				m_channels[iter->first] = vc;
	//			}

	//			nm++;
	//			if (nm >= m_nMaxThread)
	//			{
	//				break;
	//			}
	//		}
	//	}
	//}

	return 0;
}

void VideoSdkImpl::InterMakePolling(std::list<stru_VideoStatus> resStatus, std::string& res)
{
	rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

	writer.StartObject();

	writer.Key("PollingStatusList");

	writer.StartArray();

	for (stru_VideoStatus &iter : resStatus)
	{
		writer.StartObject();
		writer.Key("DeviceId");
		writer.String(iter.DeviceID.c_str());
		//DeviceStatus
		writer.Key("DeviceStatus");
		writer.StartObject();
		writer.Key("Online");
		writer.String(iter.Online.c_str());
		writer.Key("VideoQuality");
		writer.String(iter.VideoQuality.c_str());
		writer.Key("FrameRate");
		writer.Int(iter.FrameRate);
		writer.EndObject();

		writer.Key("ForceKeyFrameStatus");
		writer.StartObject();
		writer.Key("Status");
		writer.String(iter.ForceKeyStatus.c_str());
		writer.EndObject();

		writer.Key("KeyFrameStatus");
		writer.StartObject();
		writer.Key("Status");
		writer.String(iter.status.c_str());
		writer.Key("Duration");
		writer.Int(iter.Duration);
		writer.Key("KeyFrameCount");
		writer.Int(iter.KeyFrameCount);
		writer.EndObject();

		writer.EndObject();
	}

	writer.EndArray();
	writer.EndObject();

	res = strBuf.GetString();
}

int VideoSdkImpl::GetPollingRespond(const char* quest, char* resBody, int& reslen)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	std::vector<std::string> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId"))
			continue;
		devices.push_back(arr["DeviceId"].GetString());
	}
	std::list<stru_VideoStatus> resStatus;
	{
		reslen = 0;

		std::lock_guard<std::mutex> lock(m_mutexMap);
		for (std::string device : devices)
		{
			int nKeyFrameCount;
			int nFrameRate;

			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(device);
			if (channel != m_channels.end())
			{
				channel->second->GetWorkResult(nKeyFrameCount, nFrameRate);

				stru_VideoStatus status;
				status.DeviceID = channel->second->DeviceID;
				status.ChannelID = channel->second->RtspUrl;
				status.FrameRate = nFrameRate;
				status.KeyFrameCount = nKeyFrameCount;

				status.Duration = channel->second->GetDuration();
				uint64_t tm = MHC_STLNOW_SECOND;

				printf("channel %s nKeyFrameCount %d videoStatus.Duration %d \n", channel->first.c_str(), nKeyFrameCount, status.Duration);
				if (nKeyFrameCount > 0)
				{
					status.Online = "Online";
				}
				else
					status.Online = "Offline";

				float quality = nFrameRate * 1.0f / m_stChkParam.frameRate;
				if (quality >= 0.95)
					status.VideoQuality = "High";
				else if (quality < 0.95 && quality >= 0.75)
					status.VideoQuality = "Normal";
				else if (quality < 0.75 && quality >= 0.5)
					status.VideoQuality = "Low";
				else
					status.VideoQuality = "Poor";

				if (nKeyFrameCount > 0)
					status.ForceKeyStatus = "CHECKED";
				else
					status.ForceKeyStatus = "UNCKECKED";

				if (nKeyFrameCount >= status.Duration - 1)
					status.status = "CHECKPASS";
				else if (channel->second->IsCheckIng())
					status.status = "CHECKING";
				else
					status.status = "CHECKFAILED";
				resStatus.push_back(status);

				reslen++;
			}
			else
			{
				log_print(MHC_LOG_WARN, "data error deviceid =%s \n", device.c_str());
			}
		}
	}

	if (resBody == nullptr)
		return -2;

	std::string szres;
	InterMakePolling(resStatus, szres);
	strncpy(resBody, szres.c_str(), szres.length());

	return 0;
}

int VideoSdkImpl::RecordIntegrityQuest(const char* quest)
{
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(quest);

	if (doc.HasParseError())
		return -1;
	if (!doc.IsObject() || !doc.HasMember("DeviceList"))
		return -1;
	if (!doc["DeviceList"].IsArray())
		return -1;

	std::map<std::string, VIDEORECORDCHECKPARAMS> devices;
	for (int i = 0; i < doc["DeviceList"].Size(); i++)
	{
		rapidjson::Value arr = doc["DeviceList"][i].GetObject();

		if (!arr.HasMember("DeviceId") || !arr.HasMember("rtspUrl"))
			continue;
		//devices[arr["DeviceId"].GetString()] = (arr["rtspUrl"].GetString());

		if (!arr.HasMember("Interval") || !arr.HasMember("Duration")
			|| !arr["Interval"].IsNumber() || !arr["Duration"].IsNumber())
			continue;
		if (!arr.HasMember("SegmentTimes") || !arr["SegmentTimes"].IsArray())
			continue;

		VIDEORECORDCHECKPARAMS dev;
		dev.DeviceId = arr["DeviceId"].GetString();
		dev.rtspUrl = arr["rtspUrl"].GetString();
		dev.Interval = arr["Interval"].GetInt();
		dev.Duration = arr["Duration"].GetInt();
		//SegmentTimes
		for (int is = 0; is < arr["SegmentTimes"].Size(); is++)
		{

		}

	}
	////lock map  
	{
		std::lock_guard<std::mutex> lock(m_mutexMap);
		for (std::map<std::string, VIDEORECORDCHECKPARAMS>::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
		{
			//重新设置工作模式
			std::map<std::string, VideoChannel*>::iterator channel = m_channels.find(iter->first);
			if (channel != m_channels.end())
				channel->second->SetWorkMode(4, 1);
			else
			{
				VideoChannel* vc = new VideoChannel();
				vc->DeviceID = iter->first;
				//VIDEORECORDCHECKPARAMS dev;
				vc->RtspUrl = (iter->second).rtspUrl;
				vc->SetWorkMode(5, 1);
				m_channels[iter->first] = vc;
			}
		}
	}

	return 0;
}

int VideoSdkImpl::RecordIntegrityRespond(const char* quest, char* resBody, int& reslen)
{
	return 0;
}

int VideoSdkImpl::RecordIntegrityKeyQuest(const char* quest)
{
	return 0;
}

int VideoSdkImpl::RecordIntegrityKeyRespond(const char* quest, char* resBody, int& reslen)
{
	return 0;
}

int VideoSdkImpl::GetLastError()
{
	return m_nLastError;
}

void VideoSdkImpl::SetLogLevel(int level)
{
	if (level < MHC_LOG_INFO)
		log_set_level(level);
	else if (level == 27182818) //e 2.7182818284590452
		log_set_level(MHC_LOG_DEBUG); 
	else if (level == 618033989) //fi 0.6180339887
		log_set_level(MHC_LOG_TRACE);
}

//https://blog.csdn.net/fkbiubiubiu/article/details/123530127
//bool VideoSdkImpl::init_mdecode()
//{
//	int ret = 0;
//	pAVFormatContext = avformat_alloc_context();//分配全局上下文空间
//	pAVpacket = av_packet_alloc();              //分配数据包空间
//	pAVFrame = av_frame_alloc();               //分配单帧空间
//	pAVFrameRgb = av_frame_alloc();           //分配rgb单帧空间
//	if (!pAVFormatContext || !pAVpacket || !pAVFrame || !pAVFrameRgb)
//	{
//		qDebug() << "init_mdecode failed";
//		return false;
//	}
//	AVDictionary *optionsDict = NULL;
//	av_dict_set(&optionsDict, "buffer_size", "10240000", 0); //设置缓存大小，1080p可将值调大
//	av_dict_set(&optionsDict, "rtsp_transport", "tcp", 0); //以udp方式打开，如果以tcp方式打开将udp替换为tcp
//	av_dict_set(&optionsDict, "stimeout", "20000000", 0); //设置超时断开连接时间，单位微秒
//	av_dict_set(&optionsDict, "max_delay", "30000000", 0);
//
//	ret = avformat_open_input(&pAVFormatContext, filename.toUtf8().data(), 0, 0);
//	if (ret)
//	{
//		qDebug() << "Failed to avformat_open_input(&pAVFormatContext, filename.toUtf8().data(), 0, 0)";
//		return false;
//	}
//	/*
//	 * 探测流媒体信息。
//	*/
//	ret = avformat_find_stream_info(pAVFormatContext, 0);
//	if (ret < 0)
//	{
//		qDebug() << "Failed to avformat_find_stream_info(pAVCodecContext, 0)";
//		return false;
//	}
//
//	av_dump_format(pAVFormatContext, 0, filename.toUtf8().data(), 0);//打印文件中包含的格式信息
//
//	for (int index = 0; index < pAVFormatContext->nb_streams; index++) //遍历寻找视频流
//	{
//		pAVCodecContext = pAVFormatContext->streams[index]->codec;
//		if (pAVCodecContext->codec_type == AVMEDIA_TYPE_VIDEO)
//		{
//			videoIndex = index;//此处只找视频流,不找音频和其他
//			break;
//		}
//	}
//	if (videoIndex == -1 || !pAVCodecContext)
//	{
//		qDebug() << "Failed to find video stream";
//		return false;
//	}
//	/*
//		查找解码器并打开。
//	*/
//	pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
//	if (!pAVCodec)
//	{
//		qDebug() << "Fialed to avcodec_find_decoder(pAVCodecContext->codec_id):" << pAVCodecContext->codec_id;
//		return false;
//	}
//	QString url_type = filename.mid(0, 4);
//
//	if (url_type == "rtsp")
//	{
//		if (avcodec_open2(pAVCodecContext, pAVCodec, &optionsDict))
//		{
//			qDebug() << "Failed to avcodec_open2(pAVCodecContext, pAVCodec, pAVDictionary)";
//			return false;
//		}
//	}
//	else
//	{
//		if (avcodec_open2(pAVCodecContext, pAVCodec, NULL))
//		{
//			qDebug() << "Failed to avcodec_open2(pAVCodecContext, pAVCodec, pAVDictionary)";
//			return false;
//		}
//	}
//	qDebug() << "video W x H:" << pAVCodecContext->width << "x" << pAVCodecContext->height << pAVCodecContext->pix_fmt;
//	numBytes = avpicture_get_size(mpix_fmt, pAVCodecContext->width, pAVCodecContext->height);       //计算转换后的内存大小
//	outbuffer = (uchar*)av_malloc(numBytes);//申请转换后图片存放的内存
//
//	/*
//	 * int avpicture_fill(AVPicture *picture, const uint8_t *ptr, enum AVPixelFormat pix_fmt, int width, int height)
//	 * 上面的pAVFrameRgba只是malloc了一段结构体内存，结构体中的数据部分是没有分配的，使用此函数将pAVFrameRgba的data和outbuffer关联起来
//	 * pFrameRGB里面使用的是outbuffer所指向的内存空间.
//	 * 此函数在ffmpeg4.2后变为av_image_fill_arrays;
//	 * 我理解的是这个函数主要是给buffer添加pix_fmt width height linesize等属性.
//	 */
//	avpicture_fill((AVPicture *)pAVFrameRgb, outbuffer, mpix_fmt, pAVCodecContext->width, pAVCodecContext->height);
//
//	/*
//	 * sws_getContext函数
//	 * 相当于初始化转换函数，当后面使用sws_scale执行转换的时候不在写入格式等信息
//	*/
//	pSwsContext = sws_getContext(pAVCodecContext->width, pAVCodecContext->height, pAVCodecContext->pix_fmt,//转换前的长、宽、像素格式
//		pAVCodecContext->width, pAVCodecContext->height, mpix_fmt,         //转换后的长、宽、像素格式
//		SWS_BICUBIC,                                                      //转换方法  libswscale/swscale.h
//		0, 0, 0                                                                   //其他参数默认为空
//	);
//	return true;
//}


//https ://blog.csdn.net/fkbiubiubiu/article/details/123530127
