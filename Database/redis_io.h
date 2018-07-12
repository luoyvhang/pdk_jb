#pragma once

#include <iostream>
#include <string>
/*
#include <map>
*/
#include <vector>

#if (defined(_MSC_VER) && _MSC_VER >= 1700) || __cplusplus >= 201103L
#include <unordered_map>
using std::unordered_map;
#else
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif
#include <sstream>

#include <hiredis/hiredis.h>

/* 20160707 yaosheng */


class CRedisClient
{
public:
//20160707
//	typedef std::map<std::string, std::string> MAP_KEY_VALUE_t;
	typedef std::vector<std::string> VEC_STRING_t;

	typedef unordered_map<std::string, std::string> MAP_KEY_VALUE_t;

	CRedisClient(const std::string& strHost, unsigned short uPort16, unsigned int uMinSeconds, const std::string& strPass);
	virtual ~CRedisClient();

	int command(const char* szFormat, va_list ap);

	MAP_KEY_VALUE_t& fetchResultMap();
#ifdef REDIS_RESULT_ARRAY
	VEC_STRING_t&	fetchResultArray();
#endif
	long long fetchResultInt64();
	std::string& fetchResultString();

	bool __tryConnect();

private:

	void __tryDisconnect();
	bool __auth();

	int __commPrepareReply(redisReply* pReply);

private:

	std::string m_strHost;
	unsigned short m_uPort16;
	unsigned int m_uMinSeconds;
	std::string m_strPass;

	redisContext* m_pRedisClient;

	MAP_KEY_VALUE_t	m_mapResult;
	long long m_llResult;
	std::string m_strResult;
#ifdef REDIS_RESULT_ARRAY
	VEC_STRING_t m_vecResult;
#endif
};

//20160707
/*
class CRedisIO
{
public:
	CRedisIO();
	virtual ~CRedisIO();

	void addRedis(const std::string& strHost, unsigned short uPort16, unsigned int uMinSeconds, const std::string& strPass);

	int command(int nKey, const char* szFormat, ...);

	long long getchResultInt64();

	const std::string& getResultString();

	int getResultInt();
	
	std::vector<std::string>& getResultVector();

	template <typename R>
	R getHashValue(const std::string& strKey, const R& def)
	{
		R r = def;

		std::string strValue;
		if (!getRawValue(strKey, strValue))
			return r;

		std::stringstream ss;
		ss << strValue;
		ss >> r;
		return r;
	}

private:

	CRedisClient* getRedisClient(int uKey);

	bool getRawValue(const std::string& strKey, std::string& strValue);

private:

	typedef unordered_map<int, CRedisClient*>	MAP_IDX_REDIS_t;
	MAP_IDX_REDIS_t m_mapIdxRedis;

	int m_nLastKey;
};
*/