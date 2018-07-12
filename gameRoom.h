#pragma once

#include "headers.h"
#include "gameTable.h"
#include <map>
#include <vector>
#if __cplusplus >= 201103L
#include <unordered_map>
using std::unordered_map;
#else
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif

/*游戏房间*/
class Player;
class GameRoom
{
	friend class GameTable;
public:
	GameRoom();
	virtual ~GameRoom();

	/*初始化房间.*
		@idRoom			房间id.//vid
		@baseMoney		底注,一张牌多少分.
		@bombMoney		炸弹分
		@room_type		房间类型  0  系统匹配房  1 好友房
		@game_type		游戏类型  0  16张  1 15张
		@game_rule		游戏规则
		@player			游戏人数  2 2人  3 3人
		@showcardnum	是否显示剩余牌  0 不显示  1 显示
		@fourdaithree	是否允许  四带三 0 不允许 1 允许
		@threeAbomb		3个A算炸弹	0 不算 1算
	*/
	bool				Init(OBJECT_ID idRoom, int base_money, int enter_money, int leave_money, int bomb_money, int fee, int room_type, int game_type, int game_rule, int player, int showcardnum, int fourdaithree, int threeAbomb );

	bool				BroadcastRoomMsg(const char* bufMsg, int length);
	bool				BroadcastRoomMsg(const JSONPacket& packet);
    bool                Logout(Player* pUser);
	/*玩家离线*/
	bool				Offline(Player* pUser);
	/*玩家进入房间*/
	bool				EnterRoom(Player* pUser,OBJECT_ID idTable, OBJECT_ID exceptTid, int players);
	/*玩家退出房间*/
	bool				LeaveRoom(Player* pUser);

public:
	OBJECT_ID	        GetID(void) const { return _idRoom; }
	int		            GetBaseMoney(void) const { return _baseMoney; }
	int		            GetEnterMoney(void) const { return _enterMoney; }
	int		            GetLeaveMoney(void) const { return _leaveMoney; }
	int		            GetBankerRatio(void) const { return _bankerRatio; }
	int					GetBombMoney(void) const { return _bombMoney; }
    int                 GetFee(void) const { return _fee; }

	int					GetRoomType(void) const { return _room_type; }
	int					GetGameType(void) const { return _game_type; }
	int					GetGameRule(void) const { return _game_rule; }
	int					GetPlayer(void) const { return _player; }
	int					GetFourDaiThree(void) const { return _fourdaithree; }
	int					GetThreeAbomb(void) const { return _threeAbomb; }
	int					GetShowCardNum(void) const { return _showCardNum; }

protected:
	void				OnLeaveTable(Player* pUser, bool clearRoom);
	//写游戏日志到数据库.
	static	bool		WriteGameLog(const char* log);
private:
	void				Release(void);
	void				ReleaseTable(void);
    int                 GenerateTid(void);

	void				OnTimer_dismiss(void);
	EvTimer				_timer_dismiss; 
	std::set<GameTable*>	_arrDismiss;

public:
	void				DismissLater(GameTable *pTable);

public:
	//找不到桌子时,找数据库.
	GameTable*			FindTable(OBJECT_ID idTable, OBJECT_ID idUser, OBJECT_ID exceptTid, int players);
    bool                ChangeTable(Player* pUser);

private:
	OBJECT_ID			_idRoom;
	int				    _baseMoney;     //底分
	int				    _enterMoney;    //进入分
	int				    _leaveMoney;    //离开分
    int                 _bankerRatio;   //最大抢庄倍率
	int					_bombMoney;		//炸弹分
    int                 _fee;           //税收比例（如30% 则_fee==30）
	
	// 游戏房间参数
	int					_room_type;
	int					_game_type;
	int					_game_rule;
	int					_player;
	int					_showCardNum;
	int					_fourdaithree;
	int					_threeAbomb;

	typedef std::map<OBJECT_ID, GameTable*>	MAP_GAME_TABLE;
	MAP_GAME_TABLE		_setTable;			//当前房间的桌子.
	typedef unordered_map<OBJECT_ID, GameTable*>	MAP_USER_TABLE;
	MAP_USER_TABLE		_setUserTable;		//玩家所在的桌子.
	typedef unordered_map<OBJECT_ID, Player*>	MAP_GAME_PLAYER;
	MAP_GAME_PLAYER		_setPlayer;			//在线玩家表.
	MAP_GAME_PLAYER		_setOffline;		//离线玩家表.
};
