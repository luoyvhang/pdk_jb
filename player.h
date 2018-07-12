#pragma once

#include "headers.h"
#include "include/libevwork/FormDef.h"
#define PLAYER_POOL_SIZE	2048

//money变化类型
enum ALTER_TYPE {
    FEE             = 1,    //台费
    BALANCE         = 2,    //结算
	PHOTO			= 3,	// 扣费互动表情
};

class GameRoom;
class GameTable;
class Player:public IGamePlayer
{
public:
	//获取玩家uid.
	virtual	OBJECT_ID	_GetID(void) const;
	//获取玩家名称.
	virtual	const char*	_GetName(void) const;
	//获取玩家桌子id.
	virtual	OBJECT_ID	_GetTableID(void) const;
	//获取玩家所在桌子
	virtual IGameTable*	_GetTable(void) const;
	//给玩家发送消息.
	virtual	void		_SendMsg(const std::string& packet);
	//执行redis命令.
	virtual	bool		_RedisCommand(const char* command);

	friend class GameServer;
	OBJPOOL_DECLARATION(Player,PLAYER_POOL_SIZE)
protected:
	explicit Player(evwork::IConn* pConn);
	virtual ~Player();

	Player&	swapConn(Player& p) {
		if (this != &p) {
			evwork::IConn* oldConn = _conn;
			bool		oldOnline = _online;
			_conn = p._conn;
			_online = p._online;
			p._conn = oldConn;
			p._online = oldOnline;
		}
		return *this;
	}
#ifdef _DEBUG
	//debug pool.
	static	void	debugPool(void);
#endif

	void			setConn(evwork::IConn* pConn) { _conn = pConn; _online = true; }
	evwork::IConn*	getConn(void) const { return _conn; }

	bool			login(OBJECT_ID idUser);
	void			logout(void);
	
	void			OnDisconnect(void);
	void			UpdateInfo(void);

	void			Offline(void) { _online = false; }
public:
	void			kickOut(void);
	bool			SendMsg(const JSONPacket* packet);
	bool			SendMsg(const char* bufMsg, int length);
	bool			SendMsg(const std::string& packet);
	bool			IsOnline(void) const { return _online; }
	/*玩家进入房间前处理.
		@pRoom			目标房间.
		@idTable		桌子id.
		进入失败由房间自己负责向客户端发送失败原因.并返回false
	*/
	bool			EnterRoom(GameRoom* pRoom, OBJECT_ID idTable, OBJECT_ID exceptTid, int players);
	/*玩家进入桌子前处理.
		@pTable			目标桌子.
		进入失败由桌子自己负责向客户端发送失败原因.并返回false
	*/
	bool			JoinTable(GameTable* pTable);

	bool			QuitGame(void);

	void			OnLeaveRoom(void);
	void			OnLeaveTable(void);

	GameRoom*		GetRoom(void)const { return _pRoom; }
	GameTable*		GetTable(void)const { return _pTable; }

	OBJECT_ID		GetID(void) const;
	const char*		GetName(void) const;
	int				GetSex(void) const;
	int				GetTotalBoard(void) const;
	const char*		GetAvatar(void) const;

	void			SetLastRoom(OBJECT_ID idRoom = 0, OBJECT_ID idTable = 0, int port = 0);

	bool			inc_playTimes(void);
	int				GetMoney(bool db = false);
	bool			IncrMoney(int value, int tid, ALTER_TYPE alter_type);
    static	int		GetZid(OBJECT_ID idUser);

    const std::string&  GetIPAddress(void) const { return _userInfo.ip; }
    const std::string&  GetLat(void) const { return _userInfo.lat; }
    const std::string&  GetLng(void) const { return _userInfo.lng; }

	static	bool	resetLastRoom(OBJECT_ID idUser);
private:
	evwork::IConn*	_conn;
	USERINFO		_userInfo;
	GameRoom*		_pRoom;
	GameTable*		_pTable;
	bool			_online;
	//std::string		_ip;

#ifdef _ROBOT
public:
	bool			_robot;
	static Player*	GetRobot(void);
	void			setName(const char* name);
#endif
	friend class GameTable;
};
