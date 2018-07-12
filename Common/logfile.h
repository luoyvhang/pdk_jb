#ifndef _LOG_FILE_H_797DB3D46F7F4F76B1724EFF6259CF74
#define _LOG_FILE_H_797DB3D46F7F4F76B1724EFF6259CF74


void	logSetDir(const char* dir);
void	logSave(const char* tag, const char* fmt, ...);


#define logMsg(fmt,...) logSave("message",fmt,##__VA_ARGS__)
#define logErr(fmt,...) logSave("error",fmt,##__VA_ARGS__)
#define logDebug(fmt,...) logSave("debug",fmt,##__VA_ARGS__)
#define logInfo(fmt,...) logSave("info",fmt,##__VA_ARGS__)
#define logWarning(fmt,...) logSave("warning",fmt,##__VA_ARGS__)

#ifndef _NO_ASSERT
#define logAssert(fmt,...) logSave("assert",fmt,##__VA_ARGS__)
#else
#define logAssert(fmt,...)
#endif

#define ASSERT_B(x) do { if(!(x)){logAssert("ASSERT (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return false;} } while (0);
#define ASSERT_V(x) do { if(!(x)){logAssert("ASSERT (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return;} } while (0);
#define ASSERT_I(x) do { if(!(x)){logAssert("ASSERT (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return 0;} } while (0);
#define ASSERT_P(x) do { if(!(x)){logAssert("ASSERT (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return NULL;} } while(0);

#define CHECK_B(x) do { if(!(x)){logErr("CHECK (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return false;} } while (0);
#define CHECK_V(x) do { if(!(x)){logErr("CHECK (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return;} } while (0);
#define CHECK_I(x) do { if(!(x)){logErr("CHECK (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return 0;} } while (0);
#define CHECK_P(x) do { if(!(x)){logErr("CHECK (%s) FAIELD.FUNCTION:%s,FILE:%s,LINE:%d",#x,__FUNCTION__,__FILE__,__LINE__);return NULL;} } while(0);

#ifdef _DEBUG
#define DEBUG_ERROR(x,...) do {logErr("FUNCTION:%s,FILE:%s,LINE:%d\n"x,__FUNCTION__,__FILE__,__LINE__,##__VA_ARGS__);break;}while(0);
#define DEBUG_INFO(x,...) do {logInfo("FUNCTION:%s,FILE:%s,LINE:%d\n"x,__FUNCTION__,__FILE__,__LINE__,##__VA_ARGS__);break;}while(0);
#else
#define DEBUG_ERROR(x)
#define DEBUG_INFO(x)
#endif

#endif