#pragma once

#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "RedisClient.h"

namespace tinyredis
{
	struct SRedisParam
	{
		std::string m_strIp;
		uint16_t m_uPort16;
		std::string m_strPass;
		uint32_t m_uMiniSeconds;

		SRedisParam(const std::string& strIp, uint16_t uPort16, const std::string& strPass, uint32_t uMiniSeconds)
			: m_strIp(strIp), m_uPort16(uPort16), m_strPass(strPass), m_uMiniSeconds(uMiniSeconds)
		{
		}
	};

	typedef std::vector<SRedisParam> VEC_REDIS_PARAM_t;

	class CRedisMap
	{
	public:
		typedef std::map<uint32_t, CRedisClient*> MAP_IDX_REDIS_t;

		CRedisMap(const VEC_REDIS_PARAM_t& vecParam);
		virtual ~CRedisMap();

		CRedisClient* getRedis(uint32_t uKey);
		CRedisClient* getRedis(const std::string& strKey);

	private:
		uint32_t __toIndex(uint32_t uKey);
		uint32_t __toKey(const std::string& strKey);

	private:
		MAP_IDX_REDIS_t m_mapIdxRedis;
	};

	class CRedisFactory
	{
	public:
		CRedisFactory();
		virtual ~CRedisFactory();

		void addRedis(const std::string& strIp, uint16_t uPort16, const std::string& strPass, uint32_t uMiniSeconds);

		CRedisClient* getRedis(uint32_t uKey);
		CRedisClient* getRedis(const std::string& strKey);

	private:

		CRedisMap* __getRedisMap();

		void __cleanRedisMap();

	private:
		VEC_REDIS_PARAM_t m_vecRedisParam;
		CRedisMap* m_pRedisMap;
	};

}
