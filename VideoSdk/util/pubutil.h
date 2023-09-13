/*****************************************************************************
* projectName: tracking sdk													 *
* file  pubutil.h															 *
* brief ����SDK ���� 															 *
* date: 2019/11/11 13:00													 *
* copyright(c) 2019 mhc													 *
*****************************************************************************/
#pragma once
#include <stdint.h>
//#include "defs.h"
#include "sys_inc.h"
#include <string>

#define C_MAX_RTSP_CLIENT 128

//double �����ж�
#ifndef EPSILON
#define EPSILON   (1e-10)
#endif 
//float
#ifndef EPSILON_FLOAT
#define EPSILON_FLOAT   (1e-6)
#endif

const int C_MAXTRACKCOUNT = 20;

/**
* @brief ������ö������
*	SDK�ڲ��������ֵΪ ERROR_CODE + ERROR_CODE_SEGMENT
*	���ٴ����߳�ʹ��popObj��ȡ��Ϣ������ ����Ϊ�Ƚ��ȳ�����
*/

/**SDK�ڲ������� 0 �޴���-1 δ֪����-2 �ļ�δ�ҵ��� -3 CUDA����-4 CUDA�豸δ�ҵ�**/
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
* @brief ������ö������
*	SDK�ڲ������� ��SDK�ڲ��漰�㷨�Ĵ�����Ҫʹ��ERROR_CODE_SEGMENT
*	��SDK�㷨�ڵĴ�����Ҫ�� ERROR_CODE_SEGMENT1
*	���õ�����SDK��⺯�����ֵĴ�����Ҫ�� ERROR_CODE_SEGMENT2
*	����ϵͳ�������ֵĴ���� ERROR_CODE_SEGMENT3
*	�������͵Ĵ��� ��Ҫ�� ERROR_CODE_SEGMENT4
**/
enum ERROR_CODE_SEGMENT {
	ERROR_CODE_SEGMENT0 = 0x00000000,  //SDK �����
	ERROR_CODE_SEGMENT1 = 0x10000000,  //SDK �ڲ�
	ERROR_CODE_SEGMENT2 = 0x20000000,  //3RDPART
	ERROR_CODE_SEGMENT3 = 0x40000000,  //SYS
	ERROR_CODE_SEGMENT4 = 0x80000000   //OTHER
};

/**
* @brief ����״̬ö������
*	0 δ֪��δ�������٣�1 ��ʼ���٣�2 ���� �����һ�Ŀ�꣬3���� �޷��һأ�4 ȷ��Ŀ���뿪�����Ұ��Χ
**/
enum TRACK_STATUS {
	STATUS_UNKNOWN = 0,
	STATUS_NORMAL = 1,
	STATUS_MISS_TRY = 2,
	STATUS_MISS = 3,
	STATUS_OUTVIDEO = 4
};

/**
* @brief  ֡����
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
//��������Ȩ��
bool improvePv();
//�ػ�
bool powerOffProc();
//ע��
bool logOffProc();
//����
bool reBootProc();

void WriteReportEvent(char* szName, char* szFunction);
#elif __LINUX_OS__

#include<unistd.h>
#include<linux/reboot.h>

//�ػ�
bool powerOffProc();
//ע��
bool logOffProc();
//����
bool reBootProc();
void WriteReportEvent(char* szName, char* szFunction);

#endif


/**
* @brief ���������ӿ���
*	���� �ͻ�ȡ������
*/
class PubUtils
{
public:
	PubUtils();
	~PubUtils();

	/**����ȫ�ֵĴ�����*/
	static int SetErrorCode(int nErrorCode);
	/**��ȡȫ�ֵĴ����� ֻ�ܻ�ȡһ��*/
	static int GetErrorCode();
	/**��ȡȫ�ֵ����һ�δ�����*/
	static int GetLastErrorCode();
private:
	static int m_nErrorCode;
	static int m_nLastErrorCode;
};

