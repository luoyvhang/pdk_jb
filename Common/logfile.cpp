#include "logfile.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <fstream>

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static std::string __s_logDir;
static bool __s_init_dir = false;

void	logSetDir(const char* dir)
{
#if defined(WIN32) || defined(WIN64)
	_mkdir(dir);
#else
	mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	__s_logDir = dir;
	__s_init_dir = true;
}

void	logSave(const char* tag, const char* fmt, ...)
{
	if (!__s_init_dir) {
		logSetDir("log");
	}

	time_t	iTime = time(NULL);
	struct tm* pTM = localtime(&iTime);

	char buf[20480];
	sprintf(buf, "%s/%04d-%02d-%02d-%s.log", __s_logDir.c_str(),
		pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, tag);

	std::fstream file(buf, std::ios::app | std::ios::out | std::ios::binary);
	if (!file) return;
	sprintf(buf, "[%02d:%02d:%02d] %s:", pTM->tm_hour, pTM->tm_min, pTM->tm_sec, tag);
	file << buf;

	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	file << buf;
	file << "\n\n";
	file.flush();
	file.close();

#ifdef _DEBUG_CONSOLE
	printf("[%02d:%02d:%02d] %s:", pTM->tm_hour, pTM->tm_min, pTM->tm_sec, tag);
	printf("%s\n", buf);
#endif
}