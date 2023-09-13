#include "VideoSdk.h"
#include "VideoSdkImpl.h"

int VideoSdk_Init(int delayVideo, int frameRate, double qualityThreshold)
{
	return VideoSdkImpl::GetInstance()->Init(delayVideo, frameRate, qualityThreshold);
}

int VideoSdk_UnInit()
{
	return VideoSdkImpl::GetInstance()->UnInit();
}

int VideoSdk_GetLastError()
{
	return VideoSdkImpl::GetInstance()->GetLastError();
}

void VideoSdk_SetLogLevel(int level)
{
	return VideoSdkImpl::GetInstance()->SetLogLevel(level);
}

//2. StreamStatus
int VideoSdk_SetStreamStatusQuest(const char* quest)
{
	return VideoSdkImpl::GetInstance()->SetStreamStatusQuest(quest);
}

int VideoSdk_GetStreamStatusRespond(const char* quest, char* resBody, int& reslen)
{
	return VideoSdkImpl::GetInstance()->GetStreamStatusRespond(quest, resBody, reslen);
}

//3. ForceKeyFrame
int VideoSdk_SetForceKeyFrameQuest(const char* quest)
{
	return VideoSdkImpl::GetInstance()->SetForceKeyFrameQuest(quest);
}

int VideoSdk_GetForceKeyFrameRespond(const char* quest, char* resBody, int& reslen)
{
	return VideoSdkImpl::GetInstance()->GetForceKeyFrameRespond(quest, resBody, reslen);
}

//4. KeyFrame
int VideoSdk_SetKeyFrameQuest(const char* quest)
{
	return VideoSdkImpl::GetInstance()->SetKeyFrameQuest(quest);
}

int VideoSdk_GetKeyFrameRespond(const char* quest, char* resBody, int& reslen)
{
	return VideoSdkImpl::GetInstance()->GetStreamStatusRespond(quest, resBody, reslen);
}

//5. PollingQuest
int VideoSdk_SetPollingQuest(const char* quest)
{
	return VideoSdkImpl::GetInstance()->SetPollingQuest(quest);
}

int VideoSdk_GetPollingRespond(const char* quest, char* resBody, int& reslen)
{
	return VideoSdkImpl::GetInstance()->GetPollingRespond(quest, resBody, reslen);
}

//6. RecordIntegrity
int VideoSdk_RecordIntegrityQuest(const char* quest)
{
	return VideoSdkImpl::GetInstance()->RecordIntegrityQuest(quest);
}

int VideoSdk_RecordIntegrityRespond(const char* quest, char* resBody, int& reslen)
{
	return VideoSdkImpl::GetInstance()->RecordIntegrityRespond(quest, resBody, reslen);
}

int VideoSdk_RecordIntegrityKeyQuest(const char* quest)
{
	return VideoSdkImpl::GetInstance()->RecordIntegrityKeyQuest(quest);
}

int VideoSdk_RecordIntegrityKeyRespond(const char* quest, char* resBody, int& reslen)
{
	return VideoSdkImpl::GetInstance()->RecordIntegrityKeyRespond(quest, resBody, reslen);
}
