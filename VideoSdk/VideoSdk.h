/**
* @brief video check sdk jni
* @detail 私有协议
*  
* @date 2022-07-20 
* @version 1.0
* @author mhchang 2022
*/

/**
* Change Logs
* @date 2022-8-3
* @version 1.1
* @modify 修改 结构
*/

#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) ||  defined(WIN64) || defined(WINDOWS) || defined(_WINDOWS)
#define __WINDOWS_OS__	1
#define __LINUX_OS__	0
#elif __GNUC__ 
#define __WINDOWS_OS__	0
#define __LINUX_OS__	1
#else
#error "Not Supported OS!"
#endif

#if __WINDOWS_OS__
#pragma warning (push)
#pragma warning (disable: 4251 4305 4996 4267 4244 4838 4305)

#ifdef MHC_VIDEOSDK_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#elif __LINUX_OS__
#define DLL_API
#define _access access

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

//错误码  使用VideoSdk_GetLastError 获取
//无错误
#define C_ERROR_NONE			0x00000000

//无效设备id
#define C_ERROR_INVALIDID		0x00000001
//重复初始化
#define C_ERROR_DUPLICATEINIT	0x00000002
//未初始化
#define C_ERROR_NOINIT			0x00000003
//未点播或已停止
#define C_ERROR_NOPLAY			0x00000004

//参数错误
#define C_ERROR_PARAMS			0x00010000
//ip未找到
#define C_ERROR_PARAMS_IP		0x00010001
//端口错误
#define C_ERROR_PARAMS_PORT		0x00010002
//密码或用户名错误
#define C_ERROR_PARAMS_USER		0x00010003
//profile参数错误
#define C_ERROR_PARAMS_PROFILE	0x00010004
//无效通道号
#define C_ERROR_PARAMS_CHANNEL	0x00010006

//连接错误
#define C_ERROR_CONNECT			0x00020000
//连接错误 PROFILE错误
#define C_ERROR_CONNECT_PROFILE	0x00020001

//连接错误 uri错误
#define C_ERROR_URL				0x00030000

//内部错误
#define C_ERROR_INTER			0x00100000
//内部错误 获取设备信息错误
#define C_ERROR_INTER_GETINFO	0x00100200

//未知错误
#define C_ERROR_UNKNOWN			0xFFFFFFFF

////////////////////////////////////////////
#define CALLBACK __stdcall


/**
* @brief 视频流参数信息
*/
typedef struct stru_StreamParam {
	int id;					//设备id 调用login之后返回 设备id  一个ip/port 对应一个设备id
	int channel;			//设备通道号 调用login之后返回 相同设备id下可以有多个不同的通道号 即id和channel为一对多关系
	int width;				//视频宽度 0~~3840
	int height;				//视频高度 0~~2160
	int format;				//视频格式 0=mjpeg 1=mpeg4 2=h264 3=h265 4=PS 5=TS
	int bitRate;			//视频码率 bps 不大于40m  缺省为4k视频10m，1080p 4m
	int frameRate;			//帧率 缺省为25帧
	char streamUrl[256];	//rtsp流地址
	void* pUser;			//用户保留字段 用户自行使用 sdk不做更改
} STRU_STREAMPARAM, *PSTRU_STREAMPARAM;


#ifdef __cplusplus
extern "C"
{
#endif

	/**
	* @brief 获取最后一次调用函数时的错误值
	* @return 如果调用没有错误 返回0 否则返回非零值  请参考C_ERROR_XXXX定义得具体内容
	*/
	int DLL_API VideoSdk_GetLastError();

	/**
	* @brief 初始化
	* @return 返回值 true 成功; false 失败
	*/
	int DLL_API VideoSdk_Init(int delayVideo, int frameRate, double qualityThreshold);

	/**
	* @brief 反初始化
	* @return 返回值 true 成功; false 失败
	*/
	int DLL_API VideoSdk_UnInit();

	///**
	//* @brief 开启实时流
	//* @param id 设备id
	//* @param channel 设备id下对应的通道号
	//* @return 返回值 true 成功 false 失败
	//*/
	//bool DLL_API ZH_RealPlayEx(int id, int channel);

	///**
	//* @brief 接受实时流数据
	//* @param id 设备id
	//* @param channel 设备id下对应的通道号
	//* @param callback 回调函数指针
	//* @param blKeyData 回调函数的数据是否为完整帧 true为完整帧数据，可能只是sps/pps数据， false为普通rtp数据包，解码前需要自行组织保存数据 包括网络抖动导致rtp包顺序颠倒 丢失等等
	//* @param pUser 此值设置回调时传入 并由回调的参数 STRU_STREAMPARAM 传出
	//* @return 返回值 true 成功 false 失败
	//*/
	//bool DLL_API ZH_SetRealDataCallBackEx(int id, int channel, PRealDataCallBack callback, void* pUser);

	/**
	* @brief 设置日志级别
	* @param level
		TRACE=0; DEBUG=1; INFO=2; WARN=3; ERROR=4 FATAL=5 缺省为error
	* @return 返回值无
	*/
	void DLL_API VideoSdk_SetLogLevel(int level);

	//2. 视频流状态
	/**
	* @brief 视频流状态查询
	* @param quest
		json格式
	* @return 返回值无
	*/
	int DLL_API VideoSdk_SetStreamStatusQuest(const char* quest);
	/**
	* @brief 视频流状态查询
	* @param quest
		json格式
	* @return 返回值无
	*/
	int DLL_API VideoSdk_GetStreamStatusRespond(const char* quest, char* resBody, int& reslen);
	//3. 强制关键帧状态

	int DLL_API VideoSdk_SetForceKeyFrameQuest(const char* quest);
	int DLL_API VideoSdk_GetForceKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//4. 关键帧状态
	int DLL_API VideoSdk_SetKeyFrameQuest(const char* quest);
	int DLL_API VideoSdk_GetKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//5. 巡检
	int DLL_API VideoSdk_SetPollingQuest(const char* quest);
	int DLL_API VideoSdk_GetPollingRespond(const char* quest, char* resBody, int& reslen);

	//6. 录像完整性
	int DLL_API VideoSdk_RecordIntegrityQuest(const char* quest);
	int DLL_API VideoSdk_RecordIntegrityRespond(const char* quest, char* resBody, int& reslen);

	int DLL_API VideoSdk_RecordIntegrityKeyQuest(const char* quest);
	int DLL_API VideoSdk_RecordIntegrityKeyRespond(const char* quest, char* resBody, int& reslen);

#ifdef __cplusplus
}
#endif

#if __WINDOWS_OS__
#pragma warning (pop)
#elif __LINUX_OS__
#pragma GCC diagnostic pop
#else
#error "Not Supported OS!"
#endif
