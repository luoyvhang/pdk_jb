#ifndef _FUNCTION_TTL_H
#define _FUNCTION_TTL_H

#include <time.h>

class FunctionTTL
{
private:
    struct timespec m_tv_1;
    struct timespec m_tv_2;
    const unsigned long m_ttl;  //需记录的最短时长
    const char *m_fun_name;
public:
    FunctionTTL(const unsigned long ttl, const char *func);

    ~FunctionTTL();
};

#endif

