/***************************************************************************************
 *

 *
****************************************************************************************/

#ifndef	__SYS_LOG_H__
#define	__SYS_LOG_H__

// log level
#define MHC_LOG_DETAIL -1
#define MHC_LOG_TRACE   0
#define MHC_LOG_DEBUG   1
#define MHC_LOG_INFO    2
#define MHC_LOG_WARN    3
#define MHC_LOG_ERROR   4
#define MHC_LOG_FATAL   5
#define MHC_LOG_OFF	   6

#pragma warning (disable: 4305 4996 4267 4244 4838)

#define LOG_NEWFILE_PRINT(flag, level, fmt, ...) { \
		Logging_NewFile(1, flag); \
		log_print(level, fmt, ##__VA_ARGS__); \
	}

#define LOG_NEWMONTH_PRINT(level, fmt, ...) LOG_NEWFILE_PRINT(1, level, fmt, ##__VA_ARGS__)
#define LOG_NEWDAY_PRINT(level, fmt, ...) LOG_NEWFILE_PRINT(2, level, fmt, ##__VA_ARGS__)
#define LOG_NEWHOUR_PRINT(level, fmt, ...) LOG_NEWFILE_PRINT(4, level, fmt, ##__VA_ARGS__)
#define LOG_NEWMINUTE_PRINT(level, fmt, ...) LOG_NEWFILE_PRINT(8, level, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

int 	log_init(const char * log_fname);
int 	log_time_init(const char * fname_prev);
int     log_reinit(const char * log_fname);
int     log_time_reinit(const char * fname_prev);
void 	log_close();
void    log_set_toscreen(int log_screen);
int     log_get_toscreen();

void    log_set_level(int level);
int     log_get_level();
void	log_reinit_newday(char* path);
void	log_set_levelstr(const char* levelstr);

int 	log_print(int level, const char * fmt, ...);
int		log_print_hex(int level, unsigned char * data, int nlen);

int 	log_lock_start(const char * fmt, ...);
int 	log_lock_print(const char * fmt, ...);
int 	log_lock_end(const char * fmt, ...);

void	Logging_NewFile(int blOnoff, int flag = 0);
#ifdef __cplusplus
}
#endif

#endif	//	__SYS_LOG_H__
