/**
* @brief video check sdk jni
* @detail ˽��Э��
*  
* @date 2022-07-20 
* @version 1.0
* @author mhchang 2022
*/

/**
* Change Logs
* @date 2022-8-3
* @version 1.1
* @modify �޸� �ṹ
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

//������  ʹ��VideoSdk_GetLastError ��ȡ
//�޴���
#define C_ERROR_NONE			0x00000000

//��Ч�豸id
#define C_ERROR_INVALIDID		0x00000001
//�ظ���ʼ��
#define C_ERROR_DUPLICATEINIT	0x00000002
//δ��ʼ��
#define C_ERROR_NOINIT			0x00000003
//δ�㲥����ֹͣ
#define C_ERROR_NOPLAY			0x00000004

//��������
#define C_ERROR_PARAMS			0x00010000
//ipδ�ҵ�
#define C_ERROR_PARAMS_IP		0x00010001
//�˿ڴ���
#define C_ERROR_PARAMS_PORT		0x00010002
//������û�������
#define C_ERROR_PARAMS_USER		0x00010003
//profile��������
#define C_ERROR_PARAMS_PROFILE	0x00010004
//��Чͨ����
#define C_ERROR_PARAMS_CHANNEL	0x00010006

//���Ӵ���
#define C_ERROR_CONNECT			0x00020000
//���Ӵ��� PROFILE����
#define C_ERROR_CONNECT_PROFILE	0x00020001

//���Ӵ��� uri����
#define C_ERROR_URL				0x00030000

//�ڲ�����
#define C_ERROR_INTER			0x00100000
//�ڲ����� ��ȡ�豸��Ϣ����
#define C_ERROR_INTER_GETINFO	0x00100200

//δ֪����
#define C_ERROR_UNKNOWN			0xFFFFFFFF

////////////////////////////////////////////
#define CALLBACK __stdcall


/**
* @brief ��Ƶ��������Ϣ
*/
typedef struct stru_StreamParam {
	int id;					//�豸id ����login֮�󷵻� �豸id  һ��ip/port ��Ӧһ���豸id
	int channel;			//�豸ͨ���� ����login֮�󷵻� ��ͬ�豸id�¿����ж����ͬ��ͨ���� ��id��channelΪһ�Զ��ϵ
	int width;				//��Ƶ��� 0~~3840
	int height;				//��Ƶ�߶� 0~~2160
	int format;				//��Ƶ��ʽ 0=mjpeg 1=mpeg4 2=h264 3=h265 4=PS 5=TS
	int bitRate;			//��Ƶ���� bps ������40m  ȱʡΪ4k��Ƶ10m��1080p 4m
	int frameRate;			//֡�� ȱʡΪ25֡
	char streamUrl[256];	//rtsp����ַ
	void* pUser;			//�û������ֶ� �û�����ʹ�� sdk��������
} STRU_STREAMPARAM, *PSTRU_STREAMPARAM;


#ifdef __cplusplus
extern "C"
{
#endif

	/**
	* @brief ��ȡ���һ�ε��ú���ʱ�Ĵ���ֵ
	* @return �������û�д��� ����0 ���򷵻ط���ֵ  ��ο�C_ERROR_XXXX����þ�������
	*/
	int DLL_API VideoSdk_GetLastError();

	/**
	* @brief ��ʼ��
	* @return ����ֵ true �ɹ�; false ʧ��
	*/
	int DLL_API VideoSdk_Init(int delayVideo, int frameRate, double qualityThreshold);

	/**
	* @brief ����ʼ��
	* @return ����ֵ true �ɹ�; false ʧ��
	*/
	int DLL_API VideoSdk_UnInit();

	///**
	//* @brief ����ʵʱ��
	//* @param id �豸id
	//* @param channel �豸id�¶�Ӧ��ͨ����
	//* @return ����ֵ true �ɹ� false ʧ��
	//*/
	//bool DLL_API ZH_RealPlayEx(int id, int channel);

	///**
	//* @brief ����ʵʱ������
	//* @param id �豸id
	//* @param channel �豸id�¶�Ӧ��ͨ����
	//* @param callback �ص�����ָ��
	//* @param blKeyData �ص������������Ƿ�Ϊ����֡ trueΪ����֡���ݣ�����ֻ��sps/pps���ݣ� falseΪ��ͨrtp���ݰ�������ǰ��Ҫ������֯�������� �������綶������rtp��˳��ߵ� ��ʧ�ȵ�
	//* @param pUser ��ֵ���ûص�ʱ���� ���ɻص��Ĳ��� STRU_STREAMPARAM ����
	//* @return ����ֵ true �ɹ� false ʧ��
	//*/
	//bool DLL_API ZH_SetRealDataCallBackEx(int id, int channel, PRealDataCallBack callback, void* pUser);

	/**
	* @brief ������־����
	* @param level
		TRACE=0; DEBUG=1; INFO=2; WARN=3; ERROR=4 FATAL=5 ȱʡΪerror
	* @return ����ֵ��
	*/
	void DLL_API VideoSdk_SetLogLevel(int level);

	//2. ��Ƶ��״̬
	/**
	* @brief ��Ƶ��״̬��ѯ
	* @param quest
		json��ʽ
	* @return ����ֵ��
	*/
	int DLL_API VideoSdk_SetStreamStatusQuest(const char* quest);
	/**
	* @brief ��Ƶ��״̬��ѯ
	* @param quest
		json��ʽ
	* @return ����ֵ��
	*/
	int DLL_API VideoSdk_GetStreamStatusRespond(const char* quest, char* resBody, int& reslen);
	//3. ǿ�ƹؼ�֡״̬

	int DLL_API VideoSdk_SetForceKeyFrameQuest(const char* quest);
	int DLL_API VideoSdk_GetForceKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//4. �ؼ�֡״̬
	int DLL_API VideoSdk_SetKeyFrameQuest(const char* quest);
	int DLL_API VideoSdk_GetKeyFrameRespond(const char* quest, char* resBody, int& reslen);
	//5. Ѳ��
	int DLL_API VideoSdk_SetPollingQuest(const char* quest);
	int DLL_API VideoSdk_GetPollingRespond(const char* quest, char* resBody, int& reslen);

	//6. ¼��������
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
