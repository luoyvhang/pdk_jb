#include "database.h"
#include "../Common/objPool.h"
#include "../Common/logfile.h"

#define RECORDSET_POOL_SIZE 32

class DataRecord :public IRecord
{
	friend class Database;
	OBJPOOL_DECLARATION(DataRecord, RECORDSET_POOL_SIZE)
protected:
	typedef CRedisClient::MAP_KEY_VALUE_t::const_iterator ITER_ROW;
	DataRecord(const CRedisClient::MAP_KEY_VALUE_t&);
#ifdef REDIS_RESULT_ARRAY
	DataRecord(CRedisClient::MAP_KEY_VALUE_t& rs, CRedisClient::VEC_STRING_t& arr, long long llValue, std::string& strValue)
	{
		_arr.swap(arr);
		_count = _arr.size();
#else
	DataRecord(CRedisClient::MAP_KEY_VALUE_t& rs, long long llValue, std::string& strValue)
	{
#endif
		_rs.swap(rs);
		_rows = _rs.size();
		_llvalue = llValue;
		_strValue.swap(strValue);
	}
	virtual ~DataRecord()
	{

	}
public:
	virtual	void			Release(void)
	{
		delete this;
	}
	virtual int				GetRows(void) 
	{
		return _rows;
	}
#ifdef REDIS_RESULT_ARRAY
	virtual	int				GetCount(void)
	{
		return _count;
	}
#endif
	virtual unsigned int	GetUInt(const char* key)
	{
		return GetValue<unsigned>(key, 0);
	}
	virtual int				GetInt(const char* key)
	{
		return (int)GetUInt(key);
	}
	virtual unsigned short	GetUShort(const char* key)
	{
		return GetValue<unsigned short>(key, 0);
	}
	virtual short			GetShort(const char* key)
	{
		return (short)GetUShort(key);
	}
	virtual long long		GetLlong(const char* key)
	{
		return GetValue<long long>(key, 0);
	}
	virtual float			GetFloat(const char* key)
	{
		return GetValue<float>(key, 0.0f);
	}
	virtual double			GetDouble(const char* key)
	{
		return GetValue<double>(key, 0.0);
	}
	virtual std::string		GetString(const char* key)
	{
		ITER_ROW row = _rs.find(key);
		if (row != _rs.end()) {
			return row->second;
		}
		return "";
	}

    virtual CRedisClient::MAP_KEY_VALUE_t GetResult()
    {
        return _rs;
    }

#ifdef REDIS_RESULT_ARRAY
	virtual unsigned int	GetUInt(int idx)
	{
		return GetArr<unsigned>(idx, 0);
	}

	virtual int				GetInt(int idx)
	{
		return (int)GetUInt(idx);
	}

	virtual unsigned short	GetUShort(int idx)
	{
		return GetArr<unsigned short>(idx, 0);
	}

	virtual short			GetShort(int idx)
	{
		return (short)GetUShort(idx);
	}

	virtual long long		GetLLong(int idx)
	{
		return GetArr<long long>(idx, 0);
	}

	virtual float			GetFloat(int idx)
	{
		return GetArr<float>(idx, 0.0f);
	}

	virtual double			GetDouble(int idx)
	{
		return GetArr<double>(idx, 0.0);
	}

	virtual std::string		GetString(int idx)
	{
		if (idx >= 0 && idx < _count) {
			return _arr[idx];
		}
		return "";
	}
#endif
	virtual std::string		GetString(void)
	{
		return _strValue;
	}
	virtual long long		GetLlong(void)
	{
		return _llvalue;
	}
private:
	template<typename T>
	T	GetValue(const char* key,const T& def)
	{
		T val = def;
		ITER_ROW row = _rs.find(key);
		if (row != _rs.end()) {
			std::stringstream ss;
			ss << row->second;
			ss >> val;
		}
		return val;
	}
#ifdef REDIS_RESULT_ARRAY
	template<typename T>
	T	GetArr(int idx, const T& def)
	{
		T val = def;
		if (idx >= 0 && idx < _count) {
			std::stringstream ss;
			ss << _arr[idx];
			ss >> val;
		}
		return val;
	}
#endif
private:
	CRedisClient::MAP_KEY_VALUE_t	_rs;
#ifdef REDIS_RESULT_ARRAY
	CRedisClient::VEC_STRING_t		_arr;
#endif
	int							_count;
	int							_rows;
	long long					_llvalue;
	std::string					_strValue;
};

OBJPOOL_IMPLEMENTATION(DataRecord, RECORDSET_POOL_SIZE)


Database::Database()
	:_redisClient(NULL)
{

}

Database::~Database()
{
	if (_redisClient) {
		delete _redisClient;
		_redisClient = NULL;
	}
}

bool Database::InitRedis(const char* host, unsigned short port, const char* pass)
{
	if (_redisClient) delete _redisClient;
	_redisClient = new CRedisClient(host, port, 3000, pass);
	return _redisClient->__tryConnect();
}

IRecord* Database::query(const char* fmt, ...)
{
	ASSERT_P(_redisClient);
	va_list argptr;
	va_start(argptr, fmt);
	int result = _redisClient->command(fmt, argptr);
	va_end(argptr);
	if (0 == result) {

		IRecord* rs = new DataRecord(_redisClient->fetchResultMap(),
#ifdef REDIS_RESULT_ARRAY
			_redisClient->fetchResultArray(),
#endif
			_redisClient->fetchResultInt64(),
			_redisClient->fetchResultString());

		return rs;
	}
	return NULL;
}

bool Database::execute(const char* fmt, ...)
{
	ASSERT_B(_redisClient);
	va_list argptr;
	va_start(argptr, fmt);
	int result = _redisClient->command(fmt, argptr);
	va_end(argptr);
	return 0 == result;
}
