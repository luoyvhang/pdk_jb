#include "FunctionTTL.h"
#include "logfile.h"

FunctionTTL::FunctionTTL(const unsigned long ttl, const char* func)
:m_ttl(ttl)
,m_fun_name(func)
{
    clock_gettime(CLOCK_REALTIME, &m_tv_1);
}

FunctionTTL::~FunctionTTL()
{
    clock_gettime(CLOCK_REALTIME, &m_tv_2);
    unsigned long long end = m_tv_2.tv_sec*1000L + m_tv_2.tv_nsec/1000000L;
    unsigned long long begin = m_tv_1.tv_sec*1000L + m_tv_1.tv_nsec/1000000L;
    unsigned long interval = end-begin;
    if(interval > m_ttl)
    {
        if (interval >= 100)
            logErr("FBI %s执行时间间隔 idx:100 实际%u毫秒", m_fun_name, interval);
        else
            logErr("FBI %s执行时间间隔 idx:%d 实际%u毫秒", m_fun_name, interval/10, interval);
    }
}


