#pragma once

#if __cplusplus >= 201103L || defined(_MSC_VER)
#include <unordered_map>
using std::unordered_map;
#else
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif
#include <stdint.h>
#include <string>
#include <vector>
#include "include/libevwork/Logger.h"

#include "include/libevwork/FormDef.h"
#include "include/libevwork/EVWork.h"



//服务器发给客户端来红包了协议ID
//{"cmd":788888,"seqid":123456}
#define SYSTEM_SERVER_SENDNEW_PACKET	788888
//服务器发给客户端抢红包结果.
//{"cmd":788889,"result":0,"user":"xxxxx","money":88.88}
//{"cmd":788889,"result":1,"user":"self","money":88.88}
#define SYSTEM_SERVER_PACKET_RESULT		788889
//服务器发送给客户端抢红包错误.
//{"cmd":788887,"msg":"红包不存在"}
#define SYSTEM_SERVER_PACKET_ERROR		788887
//客户端发给服务器抢红包协议ID.
//{"cmd":888888,"seqid":123456}
#define SYSTEM_CLIENT_ACHIEVE_PACKET	888888


typedef uint32_t	OBJECT_ID;

class ClassAward;
class IGameTable;

//玩家类继承该类.并实现对应的纯虚函数.
class IGamePlayer
{
public:
	//获取玩家uid.
	virtual	OBJECT_ID	_GetID(void) const = 0;
	//获取玩家名称.
	virtual	const char*	_GetName(void) const = 0;
	//获取玩家桌子id.
	virtual	OBJECT_ID	_GetTableID(void) const = 0;
	//获取玩家所在桌子
	virtual IGameTable*	_GetTable(void) const = 0;
	//给玩家发送消息.
	virtual	void		_SendMsg(const std::string& packet) = 0;
	//执行redis命令.
	virtual	bool		_RedisCommand(const char* command) = 0;
};

//游戏桌子(房间)类继承该类.并实现对应的纯虚函数.
class IGameTable
{
	friend class ClassAward;
protected:
	IGameTable();
	virtual ~IGameTable();
public:
	//获取桌子id.
	virtual	OBJECT_ID	_GetID(void) const = 0;
	//实现广播给桌子中玩家消息的函数.
	virtual	void	_BroadcastMessage(const std::string& packet) = 0;
	//实现当前桌子是否处于解散不扣房卡状态.
	virtual	bool	_IsFreeTime(void) const = 0;
	//写红包日志.成功返回true,
	virtual	bool	_RedisWriteLog(const char* command) = 0;
protected:
	//玩家领取红包事件.在这里写大喇叭事件.
	virtual	void	_OnAchievePacket(IGamePlayer* player, uint32_t money, int type) = 0;

	int			GetNumPacket(void);
	void		AddNewPacket(uint32_t money,int type);
	void		AchievePacket(IGamePlayer* player,uint64_t seqid,int vid);

	struct	_AWARD_INFO 
	{
		uint64_t	seqid;
		uint32_t	money;
		int			type;	//1普通红包2财神红包
		bool		active;
		std::string	name;

		_AWARD_INFO() {
			active = false;
			money = 0;
			seqid = 0;
			type = 0;
			name.clear();
		}
	};
	typedef unordered_map<uint64_t, _AWARD_INFO> MAP_AWARDS;
	MAP_AWARDS		_awards;
};


class ClassAward
{
protected:
	ClassAward();
	virtual ~ClassAward();
public:
	//获取服务器VID.
	virtual	int		_GetServerID(void) const = 0;
	//检测服务器后台密码是否正确.
	virtual	bool	_CheckAdmin(const std::string& password) const = 0;
	//根据连接获取玩家.
	virtual	IGamePlayer*	_GetPlayerByConn(evwork::IConn* pConn) = 0;

	//以下俩个函数,作为evwork协议连接接口.分别监听普通红包和财神红包.
	//普通红包
	void		OnRedPacket(evwork::Jpacket& packet,evwork::IConn* pConn);
	//财神红包
	void		OnBigPacket(evwork::Jpacket& packet, evwork::IConn* pConn);
	//客户端上传抢红包操作.
	void		OnAchievePacket(evwork::Jpacket& packet, evwork::IConn* pConn);

	//创建房间时调用该函数反馈.
	void		OnCreateTable(IGameTable* pTable);
	//房间结束或者解散时调用该函数反馈.
	void		OnDismissTable(IGameTable* pTable);
private:
	void		ProcessRedPacket(uint32_t money);
	void		ProcessBigPacket(uint32_t money);
private:
	typedef unordered_map<OBJECT_ID, IGameTable*>	MAP_GAMETABLE;
	MAP_GAMETABLE	_tables;
};
