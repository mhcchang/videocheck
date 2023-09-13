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
	int delayVideo;				//���ʱ��
	int frameRate;				//֡��
	double qualityThreshold;	//��ֵ
};

struct VIDEORECORDCHECKPARAMS
{
	std::string DeviceId;		//���ʱ��
	std::string rtspUrl;		//
	int Interval;				//֡��
	int Duration;				//��ֵ

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
	* @brief ��Ƶ��״̬��ѯ
	* @param quest
		json��ʽ
	* @return ����ֵ��
	*/
	int GetStreamStatusRespond(const char* quest, char* resBody, int& reslen);
	//3. ǿ�ƹؼ�֡״̬

	int SetForceKeyFrameQuest(const char* quest);
	int GetForceKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//4. �ؼ�֡״̬
	int SetKeyFrameQuest(const char* quest);
	int GetKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//5. Ѳ��
	int SetPollingQuest(const char* quest);
	int GetPollingRespond(const char* quest, char* resBody, int& reslen);


	//6. 
	int RecordIntegrityQuest(const char* quest);
	int RecordIntegrityRespond(const char* quest, char* resBody, int& reslen);

	int RecordIntegrityKeyQuest(const char* quest);
	int RecordIntegrityKeyRespond(const char* quest, char* resBody, int& reslen);

	int GetLastError();
	//����ʹ��ʱ������debug��־����ȡֵ27182818 e
	//trace��־���� ȡֵ 618033989 fi 
	void SetLogLevel(int level);
protected:

	VideoSdkImpl();
	~VideoSdkImpl();

private:
	static VideoSdkImpl* m_pInstance;

	/**�̺߳�����ѭ����*/
	void process();
private:
	//
	int m_nCount;
	void * m_pParam;
	int m_nFailTime;
	std::thread m_thread;
	// �߳�ִ��״̬ 
	volatile std::atomic_bool m_blRunning;
	volatile std::atomic_bool m_blInit;
	// �߳��� 
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

	//	//�ؼ�֡����
	//	unsigned long long nKeySize;
	//	unsigned char * pKeyFrame;

	//	StruChannelParam() : nKeySize(-1), pKeyFrame(nullptr) {};
	//};

	//�ڲ�ʹ��
	mutable std::mutex m_mutexMap;
	std::map<std::string, VideoChannel*> m_channels;

	//�ȴ������
	volatile uint64_t m_tmThStart;
	bool m_blImm;
	std::string m_szTime;
	int m_nMaxThread;
	int m_nDuration;  //����ʱ������
	int m_nInterval;  //���ʱ������
	std::mutex m_mutexWait;
	std::map<std::string, std::string> m_waits;

	int m_nMaxChannel;
private:
	void InterMakeStreamStatus(std::list<stru_VideoStatus> resStatus, std::string& res);
	void InterMakeForceKeyFrameStatus(std::list<stru_VideoStatus> resStatus, std::string& res);
	void InterMakeKeyFrameStatus(std::list<stru_VideoStatus> resStatus, std::string& res);
	void InterMakePolling(std::list<stru_VideoStatus> resStatus, std::string& res);
};
