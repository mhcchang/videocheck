/*****************************************************************************
* projectName: tracking sdk													 *
* file  pubutil.h															 *
* brief 跟踪SDK 定义 															 *
* date: 2019/11/11 13:00													 *
* copyright(c) 2019 mhc													 *
*****************************************************************************/
#pragma once
#include <stdint.h>
//#include "defs.h"
#include "sys_inc.h"
#include <string>

#define C_MAX_RTSP_CLIENT 128

//double 精度判断
#ifndef EPSILON
#define EPSILON   (1e-10)
#endif 
//float
#ifndef EPSILON_FLOAT
#define EPSILON_FLOAT   (1e-6)
#endif

const int C_MAXTRACKCOUNT = 20;

/**
* @brief 错误码枚举类型
*	SDK内部错误码的值为 ERROR_CODE + ERROR_CODE_SEGMENT
*	跟踪处理线程使用popObj获取信息并处理 队列为先进先出对列
*/

/**SDK内部错误码 0 无错误，-1 未知错误，-2 文件未找到， -3 CUDA错误，-4 CUDA设备未找到**/
enum ERROR_CODE {
	NONE_ERROR = 0,
	ERROR_UNKNOWN = 0xFFFFFFFF,
	ERROR_FILE_NO_EXISTS = 0xFFFFFFFE,
	ERROR_CUDA_ERROR = 0xFFFFFFFD,
	ERROR_NO_DEVICE = 0xFFFFFFFC,

	ERROR_FRAME_NULL = 0x00001001,
	ERROR_CALLBACK = 0x00001000
};

/**
* @brief 错误码枚举类型
*	SDK内部错误码 在SDK内不涉及算法的错误不需要使用ERROR_CODE_SEGMENT
*	在SDK算法内的错误需要加 ERROR_CODE_SEGMENT1
*	调用第三方SDK或库函数出现的错误需要加 ERROR_CODE_SEGMENT2
*	调用系统函数出现的错误加 ERROR_CODE_SEGMENT3
*	其他类型的错误 需要加 ERROR_CODE_SEGMENT4
**/
enum ERROR_CODE_SEGMENT {
	ERROR_CODE_SEGMENT0 = 0x00000000,  //SDK 最外层
	ERROR_CODE_SEGMENT1 = 0x10000000,  //SDK 内部
	ERROR_CODE_SEGMENT2 = 0x20000000,  //3RDPART
	ERROR_CODE_SEGMENT3 = 0x40000000,  //SYS
	ERROR_CODE_SEGMENT4 = 0x80000000   //OTHER
};

/**
* @brief 跟踪状态枚举类型
*	0 未知或未启动跟踪，1 开始跟踪，2 跟丢 尝试找回目标，3跟丢 无法找回，4 确认目标离开相机视野范围
**/
enum TRACK_STATUS {
	STATUS_UNKNOWN = 0,
	STATUS_NORMAL = 1,
	STATUS_MISS_TRY = 2,
	STATUS_MISS = 3,
	STATUS_OUTVIDEO = 4
};

/**
* @brief  帧数据
*/
typedef struct rtp_buffer {
	//i = 1 sps... = 11 p= 2 b = 2
	uint32_t uFrameType;
	uint32_t uLen;
	uint8_t* buf;
	//float scale;
	//rtp_buffer() : uFrameType(2), uLen(0), buf(nullptr), scale(1.0f) {};
	//rtp_buffer(uint32_t uFrameType, uint32_t uLen, uint8_t* buf, float scale) : uFrameType(uFrameType), uLen(uLen), buf(buf), scale(scale) {};
} RTP_BUFFER;

typedef struct stru_VideoStatus
{
	std::string DeviceID;
	std::string ChannelID;
	std::string Online; // ": "ONLINE",
	std::string VideoQuality; //" : "High",
	int FrameRate;
	std::string ForceKeyStatus;
	std::string status;
	int Duration;
	int KeyFrameCount;
}STRU_VIDEOSTATUS;

#ifndef EPSILON
#define EPSILON   (1e-10)
#endif 
//float
#ifndef EPSILON_FLOAT
#define EPSILON_FLOAT   (1e-6)
#endif 

#if __WINDOWS_OS__
//提升进程权限
bool improvePv();
//关机
bool powerOffProc();
//注销
bool logOffProc();
//重启
bool reBootProc();

void WriteReportEvent(char* szName, char* szFunction);
#elif __LINUX_OS__

#include<unistd.h>
#include<linux/reboot.h>

//关机
bool powerOffProc();
//注销
bool logOffProc();
//重启
bool reBootProc();
void WriteReportEvent(char* szName, char* szFunction);

#endif


/**
* @brief 公共函数接口类
*	设置 和获取错误码
*/
class PubUtils
{
public:
	PubUtils();
	~PubUtils();

	/**设置全局的错误码*/
	static int SetErrorCode(int nErrorCode);
	/**获取全局的错误码 只能获取一次*/
	static int GetErrorCode();
	/**获取全局的最后一次错误码*/
	static int GetLastErrorCode();
private:
	static int m_nErrorCode;
	static int m_nLastErrorCode;
};

