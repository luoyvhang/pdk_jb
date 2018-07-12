#include "redis_io.h"
#include <stdarg.h>

#include "../Common/logfile.h"

struct authFreeReply
{
	redisReply *m_pReply;

	authFreeReply(redisReply *pReply) : m_pReply(pReply) {}
	~authFreeReply() { if (m_pReply) freeReplyObject(m_pReply); }
};

CRedisClient::CRedisClient(const std::string& strHost, unsigned short uPort16, unsigned int uMinSeconds, const std::string& strPass)
: m_pRedisClient(NULL)
, m_mapResult(MAP_KEY_VALUE_t())
, m_llResult(0)
{
	m_strHost = strHost;
	m_uPort16 = uPort16;
	m_uMinSeconds = uMinSeconds;
	m_strPass = strPass;
}
CRedisClient::~CRedisClient()
{
	__tryDisconnect();
}

bool CRedisClient::__tryConnect()
{
	if (m_pRedisClient)
		return true;

	// 连接

	struct timeval tvConnect;
	tvConnect.tv_sec = (m_uMinSeconds/1000);
	tvConnect.tv_usec = (m_uMinSeconds%1000)*1000;

	m_pRedisClient = redisConnectWithTimeout(m_strHost.c_str(), m_uPort16, tvConnect);

	if (m_pRedisClient == NULL || m_pRedisClient->err != REDIS_OK)
	{
		std::string strErrTmp = "";
		if (m_pRedisClient)
			strErrTmp = m_pRedisClient->errstr;

		__tryDisconnect();

		logErr("[CRedisClient::__tryConnect] connect redis[%s:%hu] failed, desc:%s \n", m_strHost.c_str(), m_uPort16, strErrTmp.c_str());
		return false;
	}

	// 认证

	if (!__auth())
	{
		std::string strErrTmp = "";
		if (m_pRedisClient)
			strErrTmp = m_pRedisClient->errstr;

		__tryDisconnect();

		logErr("[CRedisClient::__tryConnect] auth ****** redis[%s:%hu] failed, desc:%s \n", m_strHost.c_str(), m_uPort16, strErrTmp.c_str());
		return false;
	}

	logInfo("[CRedisClient::__tryConnect] connect redis[%s:%hu] ok \n", m_strHost.c_str(), m_uPort16);
	return true;
}

void CRedisClient::__tryDisconnect()
{
	if (m_pRedisClient)
	{
		redisFree(m_pRedisClient);
		m_pRedisClient = NULL;
	}
}

bool CRedisClient::__auth()
{
	if (m_pRedisClient == NULL)
		return false;

	redisReply *pReply = (redisReply*)redisCommand(m_pRedisClient, "auth %s", m_strPass.c_str());

	authFreeReply authRepley(pReply);

	if (pReply == NULL || m_pRedisClient->err != REDIS_OK)
		return false;

	if (pReply->type != REDIS_REPLY_STATUS)
		return false;

	std::string strStatus(pReply->str, pReply->len);
	if (strStatus != "OK" && strStatus != "ok")
		return false;

	return true;
}

int CRedisClient::__commPrepareReply(redisReply* pReply)
{
	if (pReply == NULL || m_pRedisClient->err != REDIS_OK)
	{
		__tryDisconnect();
		return -1; // 网络错误
	}

	if (pReply->type == REDIS_REPLY_ERROR)
		return 1; // 指令错误

	return 0;
}


int CRedisClient::command(const char* szFormat, va_list ap)
{
	int nRetryCount = 2;
	int nRet = -1;

	m_mapResult.clear();
	m_llResult = 0;
	m_strResult = "";
#ifdef REDIS_RESULT_ARRAY
	m_vecResult.clear();
#endif

	do
	{
		/* bug. >> while(0)
		if (--nRetryCount <= 0)
			break;
		*/

		if (!__tryConnect())
			continue;

		redisReply* pReply = (redisReply*)redisvCommand(m_pRedisClient, szFormat, ap);

		authFreeReply authRepley(pReply);

		// 网络错误，尝试重连
		int nCode = __commPrepareReply(pReply);
		if (nCode < 0)
		{
			logErr("[CRedisClient::command] redis[%s:%hu] net error, retry times[%d] \n", m_strHost.c_str(), m_uPort16, nRetryCount);
			continue;
		}

		// 指令错误
		if (nCode > 0)
		{
			logErr("[CRedisClient::command] redis[%s:%hu] command error \n", m_strHost.c_str(), m_uPort16);
			logErr(szFormat);
			break;
		}

		// 取数组操作
		if (pReply->type == REDIS_REPLY_ARRAY)
		{
			// 处理hash
			for (size_t i = 0; i + 1 < pReply->elements; i += 2)
			{
				redisReply* pKeyReply = pReply->element[i];
				redisReply* pValueReply = pReply->element[i + 1];

				std::string strKey, strValue;
				strKey.assign(pKeyReply->str, pKeyReply->len);
				strValue.assign(pValueReply->str, pValueReply->len);

				m_mapResult.insert(std::make_pair(strKey, strValue));
			}
#ifdef REDIS_RESULT_ARRAY
			//处理数组
			for (size_t i = 0; i < pReply->elements; ++i)
			{
				redisReply* pSubReply = pReply->element[i];

				std::string strString(pSubReply->str, pSubReply->len);

				m_vecResult.push_back(strString);
			}
#endif
		}

		// 取值操作
		if (pReply->type == REDIS_REPLY_INTEGER)
		{
			m_llResult = pReply->integer;
		}

		// 取串操作
		if (pReply->type == REDIS_REPLY_STRING)
		{
			m_strResult.assign(pReply->str, pReply->len);
		}

		nRet = 0;
		break;
	} while (--nRetryCount <= 0);

	return nRet;
}

CRedisClient::MAP_KEY_VALUE_t& CRedisClient::fetchResultMap()
{
	return m_mapResult;
}

#ifdef REDIS_RESULT_ARRAY
CRedisClient::VEC_STRING_t& CRedisClient::fetchResultArray()
{
	return m_vecResult;
}
#endif

long long CRedisClient::fetchResultInt64()
{
	return m_llResult;
}

std::string& CRedisClient::fetchResultString()
{
	return m_strResult;
}


//20160707
/*
CRedisIO::CRedisIO()
: m_nLastKey(0)
{
}
CRedisIO::~CRedisIO()
{
	for (MAP_IDX_REDIS_t::iterator iter = m_mapIdxRedis.begin(); iter != m_mapIdxRedis.end(); ++iter)
	{
		delete iter->second;
	}
	m_mapIdxRedis.clear();
}

void CRedisIO::addRedis(const std::string& strHost, unsigned short uPort16, unsigned int uMinSeconds, const std::string& strPass)
{
	CRedisClient* pNew = new CRedisClient(strHost, uPort16, uMinSeconds, strPass);
	
	m_mapIdxRedis.insert(std::make_pair(m_mapIdxRedis.size(), pNew));
}

int CRedisIO::command(int nKey, const char* szFormat, ...)
{
	m_nLastKey = nKey;

	if (nKey != -1)
	{
		CRedisClient* pRedisClient = getRedisClient(nKey);
		if (pRedisClient == NULL)
			return -1;

		va_list ap;
		va_start(ap, szFormat);
		int nRet = pRedisClient->command(szFormat, ap);
		va_end(ap);

		return nRet;
	}
	else
	{
		va_list ap;
		va_start(ap, szFormat);
		for (MAP_IDX_REDIS_t::iterator iter = m_mapIdxRedis.begin(); iter != m_mapIdxRedis.end(); ++iter)
		{
			iter->second->command(szFormat, ap);
		}
		va_end(ap);
	}

	return 0;
}

CRedisClient* CRedisIO::getRedisClient(int uKey)
{
	int uIndex = uKey % m_mapIdxRedis.size();

	MAP_IDX_REDIS_t::iterator iter = m_mapIdxRedis.find(uIndex);
	if (iter == m_mapIdxRedis.end())
		return NULL;

	return iter->second;
}

bool CRedisIO::getRawValue(const std::string& strKey, std::string& strValue)
{
	CRedisClient* pRedisClient = getRedisClient(m_nLastKey);
	if (pRedisClient == NULL)
		return false;

	CRedisClient::MAP_KEY_VALUE_t& mapResult = pRedisClient->fetchResultMap();

	CRedisClient::MAP_KEY_VALUE_t::iterator iter = mapResult.find(strKey);
	if (iter == mapResult.end())
		return false;

	strValue = iter->second;
	return true;
}

long long CRedisIO::getchResultInt64()
{
	CRedisClient* pRedisClient = getRedisClient(m_nLastKey);
	if (pRedisClient == NULL)
		return 0;

	return pRedisClient->fetchResultInt64();
}

const std::string& CRedisIO::getResultString()
{
	static std::string strNull = "";

	CRedisClient* pRedisClient = getRedisClient(m_nLastKey);
	if (pRedisClient == NULL)
		return strNull;

	return pRedisClient->fetchResultString();
}

int CRedisIO::getResultInt()
{
	return (int)getchResultInt64();
}

std::vector<std::string>& CRedisIO::getResultVector()
{
	static std::vector<std::string> vecNull;

	CRedisClient* pRedisClient = getRedisClient(m_nLastKey);
	if (pRedisClient == NULL)
		return vecNull;

	return pRedisClient->fetchResultVector();
}
*/