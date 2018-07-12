#pragma once

#include <json/json.h>


struct PACK_HEADER
{
	int	length;
};

class JSONPacket
{
public:
	JSONPacket();
	~JSONPacket();

	bool	Init(Json::Value& pack);
	bool	Init(const char* bufMsg, int length);
	int		GetType(void) const { return _cmdType; }

	std::string		GetPacket(void) const;

	Json::Value& operator[](const char* key) {
		return _root[key];
	}

	const Json::Value& operator[](const char* key) const 
	{
		return _root[key];
	}

	const Json::Value& GetInfo(void) const
	{
		return _root;
	}

	Json::Value& GetInfo(void)
	{
		return _root;
	}

	static	std::string formatPacket(const char* bufMsg, int length);

	template<typename T>
	void	AddInfo(const char* key,const T& value)
	{
		_root[key] = value;
		__debug();
	}
	template<typename T>
	void	AddInfo(const char* key, Json::ArrayIndex index,const char* eleKey, const T& value)
	{
		_root[key][index][eleKey] = value;
		__debug();
	}

	template<typename T>
	void	Append(const char* key,const T& value)
	{
		_root[key].append(value);
		__debug();
	}
	template<typename T>
	void Append(const char* key, Json::ArrayIndex index, const char* eleKey, const T& value)
	{
		_root[key][index][eleKey].append(value);
		__debug();
	}

	bool	Create(int cmdType)
	{
		_root.clear();
		_root["cmd"] = cmdType;
		__debug();
		return true;
	}

	static	void Encrypt(char* bufMsg, int length);
	static	int  Encrypt(std::string& packet);

	//debug
	const char*	dumpInfo(void) const
	{
		return _packet.c_str();
	}

	void		__debug(void) 
	{
#ifdef _DEBUG
		_packet = _root.toStyledString();
#endif
	}
private:
	bool	parse(void);
private:
	int				_cmdType;
	std::string		_packet;
	Json::Value		_root;
};