#ifndef _REDIS_DB_H_268A031E9DBB4AF29D04E8112D61F172
#define _REDIS_DB_H_268A031E9DBB4AF29D04E8112D61F172


//#define REDIS_RESULT_ARRAY

#include "redis_io.h"
#include <sstream>
#include <memory>


class IRecord
{
public:
	virtual	void			Release(void) = 0;

	//HASH属性数.
	virtual int				GetRows(void) = 0;
#ifdef REDIS_RESULT_ARRAY
	//数组数量
	virtual	int				GetCount(void) = 0;
#endif
	//HASH操作
	virtual unsigned int	GetUInt(const char* key) = 0;
	virtual int				GetInt(const char* key) = 0;
	virtual unsigned short	GetUShort(const char* key) = 0;
	virtual short			GetShort(const char* key) = 0;
	virtual long long		GetLlong(const char* key) = 0;
	virtual float			GetFloat(const char* key) = 0;
	virtual double			GetDouble(const char* key) = 0;
	virtual std::string		GetString(const char* key) = 0;
    virtual CRedisClient::MAP_KEY_VALUE_t GetResult() = 0;
#ifdef REDIS_RESULT_ARRAY
	//数组操作
	virtual	unsigned int	GetUInt(int idx) = 0;
	virtual	int				GetInt(int idx) = 0;
	virtual unsigned short	GetUShort(int idx) = 0;
	virtual short			GetShort(int idx) = 0;
	virtual long long		GetLlong(int idx) = 0;
	virtual float			GetFloat(int idx) = 0;
	virtual double			GetDouble(int idx) = 0;
	virtual std::string		GetString(int idx) = 0;
#endif
	//取值操作.Llong自行转换.
	virtual	std::string		GetString(void) = 0;
	virtual long long		GetLlong(void) = 0;
};

struct IRecordPtr
{
	explicit IRecordPtr(IRecord* ptr)
		:_ptr(ptr)
	{

	}
	~IRecordPtr()
	{
		if (_ptr) {
			_ptr->Release();
			_ptr = NULL;
		}
	}

	IRecord*	operator->() 
	{
		return _ptr;
	}

	IRecord*	operator*()
	{
		return _ptr;
	}

	IRecord*	get(void)
	{
		return _ptr;
	}

	IRecord*	release(void)
	{
		IRecord* ptr = _ptr;
		_ptr = NULL;
		return ptr;
	}

	explicit IRecordPtr(IRecordPtr& rhs)
	{
		this->_ptr = rhs.release();
	}

	IRecordPtr& operator=(IRecordPtr& rhs)
	{
		if (this != &rhs) {
			this->_ptr = rhs.release();
		}
		return *this;
	}

	IRecordPtr& operator=(IRecord* ptr)
	{
		release();
		_ptr = ptr;
		return *this;
	}

	operator bool() const
	{
		return _ptr ? true : false;
	}
private:
	explicit IRecordPtr(const IRecordPtr& rhs);
	IRecordPtr& operator=(const IRecordPtr& rhs);
	IRecord*	_ptr;
};

class Database
{
public:
	Database();
	virtual ~Database();

	/*redis数据库信息*/
	bool			InitRedis(const char* host, unsigned short port, const char* pass);

	/*查询类操作*/
	IRecord*		query(const char* fmt, ...);

	/*增/删/改操作*/
	bool			execute(const char* fmt, ...);
private:
	CRedisClient*		_redisClient;
};

#endif
