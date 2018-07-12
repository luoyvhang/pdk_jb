#pragma once

#include "headers.h"

#include "gameRoom.h"

#if __cplusplus >= 201103L
#include <unordered_map>
using std::unordered_map;
#else
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif
#include <set>
#include <map>
#include <string>
#include <vector>
#include "include/libevwork/FormDef.h"
#include <time.h>

typedef unordered_map<uint32_t, EvTimer*> SET_TEMP_CLIENT;

/*多少秒后,未通过验证,踢下线*/
const int		TIMEOUT_TEMP_CLIENT = 30;

#define evwkConn	evwork::IConn* pConn
#define evwkPack	evwork::Jpacket& packet


class GameServer :public evwork::PHClass, public evwork::ILinkEvent, public ClassAward
{
public:
	DECLARE_FORM_MAP;
	//获取服务器VID
	virtual	int			_GetServerID(void) const;
	//测试后台密码是否正确
	virtual	bool		_CheckAdmin(const std::string& password) const;
	//根据连接获取玩家
	virtual	IGamePlayer*	_GetPlayerByConn(evwork::IConn* pConn);
protected:
	GameServer();
	virtual ~GameServer();
	static	GameServer*		__instance;
private:
	bool					LoadConf(const char* conf);
	bool					InitRedis(void);
	bool					InitRoom(void);

	struct HornInfo {
		Database				_dbHander;
		std::vector<int>		_keys;
	};

	std::vector<HornInfo>		_pubHorn;	//大喇叭
	bool					InitHorn(void);
	bool					__TransmisMsg(const Json::Value& msg);
public:
	static	GameServer*		GetInstance(void);
	static	void			Release(void);
	/*初始化服务.
		@conf			配置文件路径
	*/
	bool					InitServer(const char* conf);
	/*开始提供服务.*/
	bool					StartServer(void);
	/*获得玩家数据库操作类*/
	DBPlayer*				GetDBPlayer(OBJECT_ID idUser);
	DBPlayer*				GetDBPlayer(const Player* pUser);
	/*获得日志数据库操作类*/
	DBEvent*				GetDBEvent(void) { return &_dbEvent; }
	Database*				GetDBRoom(void) { return &_dbRoom; }
	Database*				GetDBData(void) { return &_dbData; }
	/*踢客户端下线*/
	void					KickClient(evwork::IConn* pConn);
	/*发送消息给客户端*/
	bool					SendMsg(evwork::IConn* pConn, const JSONPacket* packet);
	uint16_t				GetPort(void) const { return _port; }
	int						GetVid(void) const {
		if (_pRoom) return (int)_pRoom->GetID();
		return -1;
	}
    GameRoom*               GetRoom() { return _pRoom; };

private:
	void					OnTimeOut(evwork::IConn* pConn);
private:
	virtual void			onConnected(evwork::IConn* pConn);
	virtual void			onClose(evwork::IConn* pConn);
	
	bool					ProcessMsg(evwork::IConn* pConn, const JSONPacket* packet);
	void					ProcessClientMsg(evwkPack, evwkConn);
	void					ProcessSystemMsg(evwkPack, evwkConn);
private:
	bool					msg_Login(evwork::IConn* pConn, const JSONPacket* packet);
	bool					msg_Logout(Player* pUser, const JSONPacket* packet);
private:
	Player*					Login(evwork::IConn* pConn, OBJECT_ID idUser, const char* skey,bool& resume);
	void					OnDisconnect(Player* pUser);
	bool					QuitGame(Player* pUser);
public:
	void					UserLogout(Player* pUser);
	void					UserOffline(Player* pUser);
	void					KickOut(Player* pUser);
	/*删除离线玩家.*/
	void					DeleteOffline(Player* pUser);

	Player*					CreateUser(OBJECT_ID idUser, OBJECT_ID idRoom, OBJECT_ID idTable);
private:
	void					OnTimer_tick();
	void					OnTimer_print();

public:
	typedef std::vector<OBJECT_ID> VEC_OBJECT_ID;

	static bool				TransmitMsg(char* msg, int clrCode = 1,
		const Json::Value& parms = Json::Value(),
		const VEC_OBJECT_ID& targets = VEC_OBJECT_ID());
private:
	/*连接的客户端,未通过验证的帐号*/
	SET_TEMP_CLIENT			_setTempClient;

	std::string				_adminPass;	//后台管理密码.

	GameRoom*				_pRoom;//一个进程一个房间

	unordered_map<uint32_t, Player*> _setPlayer;
	typedef unordered_map<OBJECT_ID, Player*> MAP_GAMEPLAYER;
	typedef MAP_GAMEPLAYER::iterator		ITER_PLAYER;
	MAP_GAMEPLAYER			_setOnline;
	MAP_GAMEPLAYER			_setOffline;

	int						_sizeDBPlayer;

	typedef std::vector<DBPlayer*>  VEC_DBPLAYER;
	VEC_DBPLAYER			_dbPlayer;
	DBEvent					_dbEvent;
	Database				_dbRoom;
	Database				_dbData;

	Json::Value				_conf;

	uint16_t				_port;

	std::set<evwork::IConn*>	_setConnTick;
	EvTimer					_timer_tick;
	EvTimer					_timer_print;
};

#define			GAMESERVER		GameServer::GetInstance()
#define			GamePort		GameServer::GetInstance()->GetPort()
#define			GameVid			GameServer::GetInstance()->GetVid()
#define			DB_PLAYER		GameServer::GetInstance()->GetDBPlayer(this)
#define			DB_MAIN(idUser) GameServer::GetInstance()->GetDBPlayer(idUser)
#define			DB_EVENT		GameServer::GetInstance()->GetDBEvent()
#define			DB_ROOM			GameServer::GetInstance()->GetDBRoom()
#define			DB_DATA			GameServer::GetInstance()->GetDBData()

