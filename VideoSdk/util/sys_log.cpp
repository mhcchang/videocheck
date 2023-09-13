/***************************************************************************************

****************************************************************************************/

#include "sys_inc.h"
#include "sys_log.h"
//
//#if __WINDOWS_OS__
//
//#else
//
//#endif
/***************************************************************************************/
static FILE * g_pLogFile  = NULL;
static void * g_pLogMutex = NULL;
static int    g_log_level = MHC_LOG_INFO;
static int    g_log_toscreen = 1;

char g_szLogFile[256] = { 0 };

static const char * g_log_level_str[] = 
{
	"DETAIL",
	"TRACE",
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"FATAL",
	"OFF"
};

/***************************************************************************************/
int log_init(const char * log_fname)
{
//#define R_OK 4 /* Test for read permission. */
//#define W_OK 2 /* Test for write permission. */
//#define X_OK 1 /* Test for execute permission. */
//#define F_OK 0 /* Test for existence. */
//	具体含义如下：
//		R_OK 只判断是否有读权限
//		W_OK 只判断是否有写权限
//		X_OK 判断是否有执行权限
//		F_OK 只判断是否存在

	log_close();

	if (_access("log", 0) != 0)
	{
#if __WINDOWS_OS__
		_mkdir("log");
#else 
		_mkdir("log", S_IRWXU | S_IRWXG | S_IRWXO);
#endif
	}

	g_pLogFile = fopen(log_fname, "a+");
	if (g_pLogFile == NULL)
	{
		printf("log init fopen[%s] failed[%s]\r\n", log_fname, strerror(errno));
		return -1;
	}

	strcpy(g_szLogFile, log_fname);

	if (g_pLogMutex == nullptr)
		g_pLogMutex = sys_os_create_mutex();
	if (g_pLogMutex == NULL)
	{
		printf("log init mutex failed[%s]\r\n", strerror(errno));
		return -1;
	}
	
	return 0;
}

void log_reinit_newday(char* path)
{
	if (strcmp(path, g_szLogFile) != 0)
	{
		if (log_reinit(path) == 0)
		{
			strcpy(g_szLogFile, path);
		}
	}
}

int log_time_init(const char * fname_prev)
{
	char fpath[256];
	time_t time_now = time(NULL);
	struct tm * st = localtime(&(time_now));
	
	sprintf(fpath, "%s-%04d%02d%02d_%02d%02d%02d.txt", fname_prev, 
		st->tm_year+1900, st->tm_mon+1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
	
	return log_init(fpath);
}

int log_reinit(const char * log_fname)
{
	if (g_pLogMutex == nullptr)
		g_pLogMutex = sys_os_create_mutex();
	if (g_pLogMutex == NULL)
	{
		printf("log init mutex failed[%s]\r\n", strerror(errno));
		return -1;
	}

    sys_os_mutex_enter(g_pLogMutex);
    
	if (g_pLogFile)
	{
		fclose(g_pLogFile);
		g_pLogFile = NULL;
	}
	if (_access("log", 0) != 0)
	{
#if __WINDOWS_OS__
		_mkdir("log");
#else
		_mkdir("log", S_IRWXU | S_IRWXG | S_IRWXO);
#endif
	}

    g_pLogFile = fopen(log_fname, "a+");
	if (g_pLogFile == NULL)
	{
		printf("log init fopen[%s] failed[%s]\r\n", log_fname, strerror(errno));
		return -1;
	}
	
    sys_os_mutex_leave(g_pLogMutex);

    return 0;
}

int log_time_reinit(const char * fname_prev)
{
    char fpath[256];
	time_t time_now = time(NULL);
	struct tm * st = localtime(&(time_now));
	
	sprintf(fpath, "%s-%04d%02d%02d_%02d%02d%02d.log", fname_prev, 
		st->tm_year+1900, st->tm_mon+1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
	
	return log_reinit(fpath);
}

void log_close()
{
    sys_os_mutex_enter(g_pLogMutex);
    
	if (g_pLogFile)
	{
		fclose(g_pLogFile);
		g_pLogFile = NULL;
	}

    sys_os_mutex_leave(g_pLogMutex);
    
	if (g_pLogMutex)
	{
		sys_os_destroy_sig_mutex(g_pLogMutex);
		g_pLogMutex = NULL;
	}
}

//#include <timezoneapi.h>
#include <sys/timeb.h>

int _log_print(int level, const char *fmt, va_list argptr)
{
	int slen = 0;
	time_t time_now;
	struct tm * st;

	if (g_pLogFile == NULL || g_pLogMutex == NULL)
	{
		return 0;
	}

	struct timeb timebuffer;
	unsigned short millitm1;
	ftime(&timebuffer);
	millitm1 = timebuffer.millitm;

	time_now = time(NULL);
	st = localtime(&(time_now));
		
	sys_os_mutex_enter(g_pLogMutex);

	if (g_pLogFile)
    {
    	fprintf(g_pLogFile, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] : [%s] ", 
    		st->tm_year+1900, st->tm_mon+1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec, millitm1,
    		g_log_level_str[level]);
    	
    	slen = vfprintf(g_pLogFile,fmt,argptr);
		fflush(g_pLogFile);
		if (g_log_toscreen)
		{
			printf("[%04d-%02d-%02d %02d:%02d:%02d.%03d] : [%s] ",
				st->tm_year + 1900, st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec, millitm1,
				g_log_level_str[level]);
			vprintf(fmt, argptr);
		}
    }
    
	sys_os_mutex_leave(g_pLogMutex);

	return slen;
}

#ifndef IOS

int log_print(int level, const char * fmt, ...)
{
    if (level < g_log_level || level > MHC_LOG_FATAL)
    {
        return 0;
    }
    else
    {
        int slen;
        va_list argptr;
        va_start(argptr,fmt);
        slen = _log_print(level, fmt, argptr);
        va_end(argptr);
        return slen;
    }
}

#else

int log_printfff(int level, const char * fmt,...)
{
    if (level < g_log_level || level > MHC_LOG_FATAL)
    {
        return 0;
    }
    else
    {
        int slen;
        va_list argptr;
        va_start(argptr,fmt);
        slen = _log_print(level, fmt, argptr);
        va_end(argptr);
        return slen;
    }
}

#endif

static int _log_lock_print(const char *fmt, va_list argptr)
{
	int slen;

	if (g_pLogFile == NULL || g_pLogMutex == NULL)
	{
		return 0;
	}
	
	slen = vfprintf(g_pLogFile, fmt, argptr);
	if (g_log_toscreen)
	{
		vprintf(fmt, argptr);
	}
	fflush(g_pLogFile);
	return slen;
}

int log_lock_start(const char * fmt,...)
{
	int slen = 0;
	va_list argptr;		

	if (g_pLogFile == NULL || g_pLogMutex == NULL)
		return 0;

	va_start(argptr,fmt);
	sys_os_mutex_enter(g_pLogMutex);
	slen = _log_lock_print(fmt,argptr);
	va_end(argptr);
	return slen;
}

int log_lock_print(const char * fmt,...)
{
	int slen;
	va_list argptr;
	va_start(argptr,fmt);
	slen = _log_lock_print(fmt, argptr);
	va_end(argptr);
	return slen;
}

int log_lock_end(const char * fmt,...)
{
	int slen;
	va_list argptr;
	va_start(argptr,fmt);
	slen = _log_lock_print(fmt, argptr);
	va_end(argptr);
	sys_os_mutex_leave(g_pLogMutex);
	return slen;
}

void log_set_toscreen(int log_screen)
{
	g_log_toscreen = log_screen;
}

int log_get_toscreen()
{
	return g_log_toscreen;
}

void log_set_level(int level)
{
	g_log_level = level;
	if (g_log_level == MHC_LOG_DEBUG || g_log_level == MHC_LOG_TRACE)
		g_log_toscreen = 1;
}

int log_get_level()
{
	return g_log_level;
}

void Logging_NewFile(int blOnoff, int flag)
{
	time_t time_now;
	time_now = time(NULL);
	struct tm * st = localtime(&(time_now));
	char szfile[256] = { 0 };

	//0 default yyyymmdd
	//1 month yyyymm
	//2 day yyyymmdd
	//4 hour yyyymmdd-HH
	//8 hour yyyymmdd-HHMM
	switch (flag)
	{
	default:
	case 0:
	case 2:
		sprintf(szfile, "log/zhuohe_SentrySvr_%04d%02d%02d.log",
			st->tm_year + 1900, st->tm_mon + 1, st->tm_mday);
		break;
	case 1:
		sprintf(szfile, "log/zhuohe_SentrySvr_%04d%02d.log",
			st->tm_year + 1900, st->tm_mon + 1);
		break;
	case 4:
		sprintf(szfile, "log/zhuohe_SentrySvr_%04d%02d%02d-%02d.log",
			st->tm_year + 1900, st->tm_mon + 1, st->tm_mday, st->tm_hour);
		break;
	case 8:
		sprintf(szfile, "log/zhuohe_SentrySvr_%04d%02d%02d-%02d%02d.log",
			st->tm_year + 1900, st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min);
		break;
	}

	if (blOnoff == 0)
		log_init(szfile);
	else
		log_reinit(szfile);
}

void log_set_levelstr(const char* levelstr)
{
	g_log_level = 1;
	for (int ii = 0; ii < 6; ii++)
	{
		if (strcmp(levelstr, g_log_level_str[ii]) == 0)
		{
			g_log_level = ii;
			return;
		}
	}
}

void BinToHexStr(unsigned char *Bin, int nlen, unsigned char * Hex)
{
	uint16 i;
	uint8 j;

	for (i = 0; i < nlen; i++)
	{
		j = (Bin[i] >> 4) & 0xf;

		if (j <= 9)
		{
			Hex[i * 2] = (j + '0');
		}
		else
		{
			Hex[i * 2] = (j + 'a' - 10);
		}

		j = Bin[i] & 0xf;

		if (j <= 9)
		{
			Hex[i * 2 + 1] = (j + '0');
		}
		else
		{
			Hex[i * 2 + 1] = (j + 'a' - 10);
		}
	}

	Hex[nlen * 2] = '\0';
}

int log_print_hex(int level, unsigned char * data, int nlen)
{
	unsigned char * Hex = new unsigned char[nlen * 2 + 2];
	BinToHexStr(data, nlen, Hex);
	Hex[nlen * 2] = '\n';
	Hex[nlen * 2 + 1] = '\0';
	int nres = log_print(level, (const char*)Hex);

	delete Hex;
	return nres;
}
