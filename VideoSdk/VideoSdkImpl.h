#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include <chrono>
#include <ratio>
#include <functional>

#include "VideoSdkImpl.h"

#include "VideoChannel.h"

struct VIDEOCHECKPARAMS
{
	int delayVideo;				//检测时长
	int frameRate;				//帧率
	double qualityThreshold;	//阈值
};

struct VIDEORECORDCHECKPARAMS
{
	std::string DeviceId;		//检测时长
	std::string rtspUrl;		//
	int Interval;				//帧率
	int Duration;				//阈值

	std::map<uint64_t, uint64_t> mapSegmentTimes;
};

#define C_MAX_STREAMCOUNT 1024
class VideoSdkImpl
{
public:
	static VideoSdkImpl* GetInstance();

	bool Init(int delayVideo, int frameRate, double qualityThreshold);
	bool UnInit();

	int SetStreamStatusQuest(const char* quest);
	/**
	* @brief 视频流状态查询
	* @param quest
		json格式
	* @return 返回值无
	*/
	int GetStreamStatusRespond(const char* quest, char* resBody, int& reslen);
	//3. 强制关键帧状态

	int SetForceKeyFrameQuest(const char* quest);
	int GetForceKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//4. 关键帧状态
	int SetKeyFrameQuest(const char* quest);
	int GetKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//5. 巡检
	int SetPollingQuest(const char* quest);
	int GetPollingRespond(const char* quest, char* resBody, int& reslen);


	//6. 
	int RecordIntegrityQuest(const char* quest);
	int RecordIntegrityRespond(const char* quest, char* resBody, int& reslen);

	int RecordIntegrityKeyQuest(const char* quest);
	int RecordIntegrityKeyRespond(const char* quest, char* resBody, int& reslen);

	int GetLastError();
	//对外使用时不开放debug日志级别取值27182818 e
	//trace日志级别 取值 618033989 fi 
	void SetLogLevel(int level);
protected:

	VideoSdkImpl();
	~VideoSdkImpl();

private:
	static VideoSdkImpl* m_pInstance;

	/**线程函数主循环体*/
	void process();
private:
	//
	int m_nCount;
	void * m_pParam;
	int m_nFailTime;
	std::thread m_thread;
	// 线程执行状态 
	volatile std::atomic_bool m_blRunning;
	volatile std::atomic_bool m_blInit;
	// 线程锁 
	mutable std::mutex m_mutex;
	void start();
	void stop();

	void setRunning(bool blRunning) { m_blRunning = blRunning; };

	volatile uint32_t m_nLastError;

	VIDEOCHECKPARAMS m_stChkParam;
	//struct StruChannelParam
	//{
	//	std::string id;
	//	std::string url;

	//	//关键帧数据
	//	unsigned long long nKeySize;
	//	unsigned char * pKeyFrame;

	//	StruChannelParam() : nKeySize(-1), pKeyFrame(nullptr) {};
	//};

	//内部使用
	mutable std::mutex m_mutexMap;
	std::map<std::string, VideoChannel*> m_channels;

	//等待处理的
	volatile uint64_t m_tmThStart;
	bool m_blImm;
	std::string m_szTime;
	int m_nMaxThread;
	int m_nDuration;  //持续时间秒数
	int m_nInterval;  //间隔时间秒数
	std::mutex m_mutexWait;
	std::map<std::string, std::string> m_waits;

	int m_nMaxChannel;
private:
	void InterMakeStreamStatus(std::list<stru_VideoStatus> resStatus, std::string& res);
	void InterMakeForceKeyFrameStatus(std::list<stru_VideoStatus> resStatus, std::string& res);
	void InterMakeKeyFrameStatus(std::list<stru_VideoStatus> resStatus, std::string& res);
	void InterMakePolling(std::list<stru_VideoStatus> resStatus, std::string& res);
};
