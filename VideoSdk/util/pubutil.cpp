
//#include "defs.h"
#include "pubutil.h"

#include <string.h>


/////////////////!!!!! mhc
#if __WINDOWS_OS__
#include "winbase.h"

/**************************
* C++实现Windows关机，注销，重启
* **************************/
#include <iostream>
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <conio.h>
#include <ctype.h>

using namespace std;

//提升进程权限
bool improvePv()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	//打开当前进程的权限令牌
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
		return false;

	//获得某一特定权限的权限标识LUID，保存在tkp中
	//查看里面的权限
	//第一个参数：NULL表示要查看的是当前本地系统
	//第二个参数：指定特权名称
	//第三个参数：用来接收返回的特权名称信息
	if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
		return false;

	//设置特权数组的个数
	tkp.PrivilegeCount = 1;
	//SE_PRIVILEGE_ENABLE_BY_DEFAULT    特权默认启动
	//SE_PRIVILEGE_ENABLED              特权启动
	//SE_PRIVILEGE_USER_FOR_ACCESS      用来访问一个对象或者服务
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;    //特权启动

	//调用AdjustTokenPrivileges来提升我们需要的系统权限
	//修改令牌的权限
	//第一个参数：调整特权的句柄
	//第二个参数：禁止所有权限表示
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, NULL, NULL, NULL))
		return false;

	return true;
}

//关机
bool powerOffProc()
{
	if (!improvePv() || !ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, SHTDN_REASON_MAJOR_APPLICATION))
		return false;
	return true;
}

//注销
bool logOffProc()
{
	if (!improvePv() || !ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, SHTDN_REASON_MAJOR_APPLICATION))
		return false;
	return true;
}

//重启
bool reBootProc()
{
	if (!improvePv() || !ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_APPLICATION))
	{
		printf("improvePv failed");
		return false;
	}
	printf("reboot now");
	return true;
}

void WriteReportEvent(char* szName, char* szFunction)
{
	HANDLE hEventSource;
	LPCSTR lpszStrings[2];
	unsigned int len = sizeof(szFunction);
	char *Buffer = new char[len];

	hEventSource = RegisterEventSourceA(NULL, szName);

	if (NULL != hEventSource)
	{
		//StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());
		strcpy(Buffer, szFunction);
		lpszStrings[0] = szName;
		lpszStrings[1] = Buffer;
		//详细请参考MSDN
		ReportEventA(hEventSource,        // event log handle
			EVENTLOG_INFORMATION_TYPE, // event type
			0,                   // event category
			100,				 // event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			lpszStrings,         // array of strings
			NULL);               // no binary data    
		DeregisterEventSource(hEventSource);
	}
}
#elif __LINUX_OS__

#include <unistd.h>
#include <sys/reboot.h>

#ifndef LINUX_REBOOT_CMD_RESTART
	#define	LINUX_REBOOT_CMD_RESTART	0x01234567
#endif
#ifndef LINUX_REBOOT_CMD_RESTART
	#define	LINUX_REBOOT_CMD_HALT	0xCDEF0123
#endif
#ifndef LINUX_REBOOT_CMD_CAD_ON
	#define	LINUX_REBOOT_CMD_CAD_ON	0x89ABCDEF
#endif
#ifndef LINUX_REBOOT_CMD_CAD_ON
	#define LINUX_REBOOT_CMD_CAD_OFF	0x00000000
#endif
#ifndef LINUX_REBOOT_CMD_CAD_ON
	#define LINUX_REBOOT_CMD_POWER_OFF	0x4321FEDC
#endif
#ifndef LINUX_REBOOT_CMD_CAD_ON
	#define LINUX_REBOOT_CMD_RESTART2	0xA1B2C3D4
#endif
#ifndef LINUX_REBOOT_CMD_CAD_ON
	#define LINUX_REBOOT_CMD_SW_SUSPEND 0xD000FCE2
#endif
////关机
//bool powerOffProc()
//{
//	sync();
//	return reboot(LINUX_REBOOT_CMD_POWER_OFF, 0) == -1 ? false : true;
//}
////注销
//bool logOffProc()
//{
//	sync();
//	return reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART, 0) == -1 ? false : true;
//}
//
////重启
//bool reBootProc()
//{
//	sync();
//	return reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART, 0) == -1 ? false : true;
//}
//
//void WriteReportEvent(char* szName, char* szFunction)
//{
//	return;
//}

bool powerOffProc()
{
	sync();
	return reboot(LINUX_REBOOT_CMD_POWER_OFF) == -1 ? false : true;
}
//注销
bool logOffProc()
{
	sync();
	return reboot(LINUX_REBOOT_CMD_RESTART) == -1 ? false : true;
}

//重启
bool reBootProc()
{
	sync();
	return reboot(LINUX_REBOOT_CMD_RESTART) == -1 ? false : true;
}

#endif

int PubUtils::m_nErrorCode = 0;
int PubUtils::m_nLastErrorCode = 0;

//static int MhcPubFuns::m_nErrorCode = 0;
PubUtils::PubUtils()
{
}

PubUtils::~PubUtils()
{
}

int PubUtils::SetErrorCode(int nErrorCode)
{
	m_nLastErrorCode = m_nErrorCode;
	m_nErrorCode = nErrorCode;
	return m_nErrorCode;
}

int PubUtils::GetErrorCode()
{
	//只保留最后一次的错误码 取走之后清零
	int nErrorCode = m_nErrorCode;
	m_nLastErrorCode = m_nErrorCode;
	m_nErrorCode = ERROR_CODE::NONE_ERROR;
	return nErrorCode;
}

int PubUtils::GetLastErrorCode()
{
	return m_nLastErrorCode;
}

int TrackSdkGetLastError()
{
	return PubUtils::GetErrorCode();
}

void TrackSdkSetLastError(int nErrorCode)
{
	PubUtils::SetErrorCode(nErrorCode);
}

unsigned char CheckSum(unsigned char *ptz, int nl)
{
	char ch = 0;
	for (int i = 0; i < nl; i++)
	{
		ch += ptz[i];
	}

	return ch;
}

int hex2int(char c)
{
	if ((c >= 'A') && (c <= 'Z'))
	{
		return c - 'A' + 10;
	}
	else if ((c >= 'a') && (c <= 'z'))
	{
		return c - 'a' + 10;
	}
	else if ((c >= '0') && (c <= '9'))
	{
		return c - '0';
	}
	return 0;
}
