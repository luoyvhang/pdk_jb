#pragma once

#include "Database/database.h"

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

typedef unsigned int OBJECT_ID;
typedef /*unsigned*/ int MONEY;
typedef double		MONEY_REAL;

/*再次包装,以后可能更改数据库
	全逻辑数据库操作全写在这里.
*/

/*玩家信息*/
struct USERINFO
{
	OBJECT_ID			id;			//id
	std::string			name;		//昵称
	std::string			avatar;		//照片.
	uint8_t				sex;		//1男,2女,
	int					totalTimes;	//累计玩牌次数
	int					money;		//金币
    std::string         ip;
    std::string         lat;
    std::string         lng;
};

//游戏数据库
//主要处理玩家数据.
class DBPlayer:public Database
{
public:
	DBPlayer();
	virtual ~DBPlayer();

	/*匹配用户skey*/
	bool			CheckUser(OBJECT_ID id, const char* skey);

//	USERINFO		GetUserInfo(OBJECT_ID idUser);
	bool			GetUserInfo(OBJECT_ID idUser, USERINFO& info);
	int				GetZid(OBJECT_ID idUser);

	/*获取金币数量*/
	int				GetMoney(OBJECT_ID idUser);
	bool			IncrMoney(OBJECT_ID idUser, int& value);

	/*游戏次数+1*/
	bool			inc_GameTimes(OBJECT_ID idUser);

	/*置最后进行游戏的房间*/
	bool			SetLastRoom(OBJECT_ID idUser, OBJECT_ID idRoom = 0, OBJECT_ID idTable = 0, int port = 0);
};

//日志数据库.
class DBEvent :public Database
{
public:
	DBEvent();
	virtual ~DBEvent();

	struct EventLog
	{
		int uid;           // uid为0，表示台费
		int tid;
		int vid;
		int zid;
		int type;          // 流水类型 200玩牌 201大喇叭 202淘汰场金币 203兑奖券 204RMB 205彩票 206互动表情
		int alter_type;    // 更改类型   1 rmb  2 money  3 coin
		int alter_value;   // 更改的值
		int current_value; // 当前的值
		int ts;            // 当前时间戳 秒
		int usec;          // 当前时间戳 微秒
		int game_type;		//游戏类型.
		int	appid;
	};

	bool		commit_eventlog(int gameType, int my_uid, int my_tid, int my_alter_value, 
		int my_current_value, int my_type, int my_alter_type, int appid, bool bRsyslog);
private:
	bool		__incr_eventlog(EventLog &el);
	bool		__incr_eventlog2(EventLog &el);
};
