#ifndef __VIDEOCHANNEL_H__
#define __VIDEOCHANNEL_H__

#include <stdint.h>
#include <stdio.h>
#include <list>
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>
#include "RtspInterface.h"
#include "httpclient.h"

extern "C"
{
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include <libavutil/time.h>
	#include "libavutil/pixfmt.h"
	#include "libswscale/swscale.h"
	#include "libswresample/swresample.h"
	#include "libavutil/imgutils.h"
}

class VideoSdkImpl;

struct RtpDataNode
{
	uint8_t * buffer;
	int size;
	long time;
	bool isLostPacket; //是否丢帧了

	RtpDataNode()
	{
		buffer = NULL;
		size = 0;
		time = 0;
		isLostPacket = false;
	}
};

///通道数据，这个数据需要通过catalog后获取到的，基本上就是相机的一个通道
class VideoChannel
{
public:
	VideoChannel();
	~VideoChannel();

	///塞入rtp数据等待处理
	void InputRtpBuffer(uint8_t *buffer, int size, uint32_t sequenceNumber, bool isLastPacket);

	//设置模式 实时1 录像2
	void SetInviteMode(int playmode) { m_nInviteMode = playmode; }
	int GetInviteMode() { return m_nInviteMode; }

	//设置检测帧率
	void SetFrameRate(int nFrameRate) { m_nFrameRate = nFrameRate; };
	uint64_t GetKeyFrameTm() { return m_tmLastKeyFrame; };

	//设置检测帧率
	void SetPollingParam(bool bImmediate, std::string startTime, int nTerm) { m_bImmediate = bImmediate; m_startTime = startTime; m_nTerm = nTerm; };
	void GetPollingParam(bool &bImmediate, std::string &startTime, int &nTerm) { bImmediate = m_bImmediate; startTime = m_startTime; nTerm = m_nTerm;};
	//设置工作模式 0 无工作. 1. 关键帧模式， 强制关键帧 要求计时时间不超过1秒 关键帧要求n秒内持续关键帧 只是时间不同  2. 视频流监测 1秒内解码成功不低于20帧
	//nSecond 检测持续时间 
	void SetWorkMode(int nWorkMode, int nSecond);
	int GetDuration() { return m_nMilliSecond / 1000.0f; };
	//关键帧的返回关键帧数量 质量检测的返回帧率
	bool GetWorkResult(int &nKeyFrameCount, int &nFrameRate);
	bool IsCheckIng();

protected:
	void threadStart();
	void threadStop();
	void dealwithDataNode(const RtpDataNode &node);
	void process();
public:
	int RtpSSRC; //rtp身份标志，用于invite请求，用来区分多个rtp流。
	std::string DeviceName;
	std::string DeviceID;
	//std::string IPAddress;

	std::string RtspUrl;

	int Port;
	//std::string Status;
	//get时候用
	stru_VideoStatus status;
private:

	VideoSdkImpl* m_pMain;
	int m_nInviteMode; //1=play, 2=playback
	int m_nChannelId; 
	uint32_t m_nReceiveRtpTime;     //接收到rtp数据的时间(用来判断rtp数据接收是否超时了)

	bool m_bIsCurrentFrameLostPacket;
	uint32_t m_nLastSequenceNumber; //上一次的rtp序号

	uint8_t * m_pRtpBuffer; //存放RTP收到的PS流
	int m_nRtpBufferSize;

	///ffmpeg解码相关参数
	AVCodec * m_pCodec;
	AVCodecContext * m_pCodecCtx;
	AVFrame * m_pFrameRGB;
	AVFrame * m_pFrame;
	AVPacket m_packet;

	SwsContext * m_img_convert_ctx;
	uint8_t * m_out_buffer_rgb;
	int m_nBytes_rgb;

	char * m_pH264buf;

	bool m_bIsLastKeyFrameLostPacket; //上一个I帧是否丢包了，是的话，接下来的帧都有可能花屏 接下来所有的帧， 都不传入检测
	bool m_bIsKeyFrameGetted; //用来记录I帧是否获取到了 否则丢弃得到的帧

	RtspInterface * m_rtsp;
	//最后一次关键帧时间
	uint64_t m_tmLastKeyFrame;
	uint64_t m_tmNowKeyFrame;

	volatile int m_nFrameRate;
	//工作模式 设置工作模式 0 无工作. 1. 关键帧模式， 强制关键帧 要求计时时间不超过1秒 关键帧要求n秒内持续关键帧 只是时间不同  2. 视频流监测 1秒内解码成功不低于20帧
	//nMilliSecond 检测持续时间
	volatile int m_nWorkMode;
	//持续时间
	volatile int m_nMilliSecond;
	//工作开始时间 毫秒
	volatile uint64_t m_tmWorkStart;
	//工作时间内帧计数
	volatile int m_nKeyFrameCount;
	volatile int m_nFrameCount;

	volatile bool m_bImmediate;
	std::string m_startTime;
	volatile int m_nTerm;

	std::thread m_thread;
	std::mutex m_mutex;
	volatile std::atomic_bool m_blRunning;

	bool m_blInitThread;
	ObjectLists<RTP_BUFFER> *m_lsObj;
	void dealwithData();

	volatile std::atomic_bool m_blStreamClosed;
	//for record fun
	HttpClient * m_httpclient;

#ifdef DEBUG
	FILE * m_fl;
#endif	
private:
	void DealWithRgb32Frame(const uint8_t *rgb32Buffer, int bufferSize, int width, int height, bool isLostPacket, bool isKeyFrame);

	bool OpenH264Decoder();
	void CloseH264Decoder();

	void DecodeH264Buffer(uint8_t *buffer, int size, bool isLostPacket);
};

#endif // __VIDEOCHANNEL_H__
