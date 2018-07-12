#pragma once

#include "headers.h"
#include "player.h"
#include "Cardlib/card.h"
#include "Cardlib/hole_cards.h"
#include "Cardlib/deck.h"
#include "Cardlib/cardHelper.h"
#include "Cardlib/card_type.h"
#include "Cardlib/card_analysis.h"
#include "Cardlib/card_statistics.h"
#include "Cardlib/card_find.h"

#include <time.h>

class GameRoom;

const int	_Game_Score = 1000;		//初始积分
const int   _Game_Users = 3;		//最大人数
const int   TIMEOUT_READY = 10;     //准备倒计时

#define TABLE_POOL_SIZE	512
#define TRUSTEE 1

enum TABLE_STATUS
{
	TSS_WAITING = 0,		//等待中
	TSS_START,				//游戏开始,发牌中.
	TSS_BETTING,			//游戏中.
	TSS_ENDGAME,			//游戏结束/结算中.
	TSS_DISMISS,			//解散状态.
};

enum TABLE_ENTER_CODE
{
	ENTER_OK		    =	0,
	ENTER_FULL		    =	302,		//房间已满
	ENTER_FOUND		    =	500,		//未找到房间
	ENTER_SKEY		    =	505,		//sessionKey错误
	ENTER_MASTER	    =	666,		//预留房主座位
    ENTER_MONEY_LESS    =   506,        //房卡不够
};

enum LOGOUT_TYPE
{
	LOGOUT_QUIT		    =	0,
	LOGOUT_DISMISS	    =	5,
	LOGOUT_KICKOUT	    =	6,
    LOGOUT_CHANGE_TABLE =   7,  //换桌
};

enum CHANGE_TABLE_CODE
{
	CHANGE_OK			= 0,
	MONEY_NOT_ENOUGH,					// 金币不足
	GAMING,								// 游戏中
	ENTER_FAIL,							// 进入新桌失败
};

enum GAME_TYPE
{
	GTYPE_16			= 0,
	GTYPE_15			   ,
};

struct TableInfo {
	int			baseMoney;		//底注
	int			bombMoney;		//炸弹奖励金币.
	int			loseExp;		//输了加经验
	int			winExp;			//赢了加经验
};

class GameTable :public IGameTable
{
public:
	//获取桌子id.
	virtual	OBJECT_ID	_GetID(void) const;
	//实现广播给桌子中玩家消息的函数.
	virtual	void	_BroadcastMessage(const std::string& packet);
	//实现当前桌子是否处于解散不扣房卡状态.
	virtual	bool	_IsFreeTime(void) const;
	//写红包日志.
	virtual	bool	_RedisWriteLog(const char* command);

	virtual	void	_OnAchievePacket(IGamePlayer* player, uint32_t money, int type);

	OBJPOOL_DECLARATION(GameTable, TABLE_POOL_SIZE)

	typedef std::vector<Card>		PokerCards;
	typedef std::map<int, Card>		MapCards;

	struct TableChair
	{
		int					idChair;
		OBJECT_ID			idUser;				//玩家id,
		Player*				pUser;				//玩家
		bool				ready;				//是否准备
        int                 ready_tp;           //准备倒计时开始时间
		PokerCards			last_playCards;		//最后出的牌
		bool				offline;			//是否离线
		bool				isSingle;			//是否报单
		bool				isPass;				//是否过牌
		HoleCards			holeCards;
		int					loginType;
		int					bombTimes;			//打出炸弹次数
		int					bombScore;			//炸弹得分

		bool		IsUser(void) const
		{
			return idUser >= 10000;	//小于10000的都是机器人.
		}
		bool		IsOccupied(void) const
		{
			return idUser != OBJECT_ID_UNDEFINED && pUser;
		}
		void		Clear(void)
		{
			idUser = OBJECT_ID_UNDEFINED;
			pUser = NULL;
			offline = true;
			ResetFlags();
		}
		void		Sitdown(Player* player)
		{
			pUser = player;
			idUser = player->GetID();
			offline = !player->IsOnline();
			ResetFlags();
		}
		void		ResetFlags(void)
		{
			ready = false;
            ready_tp = time(NULL);
			isSingle = false;
			isPass = false;
			last_playCards.clear();
			holeCards.clear();
			bombTimes = 0;
			bombScore = 0;
		}
        int         GetReadySec(void) 
        {
            return TIMEOUT_READY-(time(NULL)-ready_tp);
        }

		std::string Serializable(void) const
		{
			Json::Value val;
			val["idUser"] = idUser;
			val["ready"] = ready ? 1 : 0;
			cardHelper::to_json_array(last_playCards, val, "last_playCards");
			val["single"] = isSingle ? 1 : 0;
			val["pass"] = isPass ? 1 : 0;
			cardHelper::to_json_array(holeCards.cards, val, "holeCards");
			val["bombTimes"] = bombTimes;
			val["bombScore"] = bombScore;
			return val.toStyledString();
		}

		bool		Unserializable(const std::string& str)
		{
			Json::Reader reader;
			Json::Value val;
			if (!reader.parse(str, val))
				return false;
			idUser = val["idUser"].asUInt();
			ready = val["ready"].asInt() ? true : false;
			offline = true;//需要序列化处理的,一定离线,服务端重启嘛.
			cardHelper::to_vector(val, "last_playCards", last_playCards);
			isSingle = val["single"].asInt() ? true : false;
			isPass = val["pass"].asInt() ? true : false;
			cardHelper::to_map(val, "holeCards", holeCards.cards);
			bombTimes = val["bombTimes"].asInt();
			bombScore = val["bombScore"].asInt();
			return true;
		}
	};

protected:
	GameTable();
	virtual ~GameTable();

public:
	void		Release();
	static GameTable*	CreateNew(void);
	bool		Init(GameRoom* pRoom, OBJECT_ID idTable, OBJECT_ID idDisp, int baseMoney, int enterMoney, int leaveMoney, int fee, int gameType, int gameRule, int player, int showCardNum, int fourdaithree, int threeAbomb);
	int			GetStatus(void)const { return _status; }

    bool        IsFull() const { return _amountUser >= _Game_Players; };
	bool		Sitdown(Player* pUser);
	bool		Standup(Player* pUser, LOGOUT_TYPE type = LOGOUT_QUIT);
	void		KickOut(TableChair* pInfo, LOGOUT_TYPE type = LOGOUT_KICKOUT);

public:
	bool		handle_ready(Player* pUser);
	bool		handle_chat(Player* pUser, const JSONPacket* packet);
	bool		handle_chatMsg(Player* pUser, const JSONPacket* packet);
	bool		handle_face(Player* pUser, const JSONPacket* packet);
	bool		handle_tableInfo(Player* pUser);
	bool		handle_bet(Player* pUser, const JSONPacket* packet);
	bool		handle_run(Player* pUser);
	bool		handle_recharge(Player* pUser);
	bool		handle_forward(Player* pUser, const JSONPacket* packet);

	void		Offline(Player* pUser);
	void		OnResume(Player* pUser);
	bool		ExistUser(Player* pUser) { return GetChairInfo(pUser) != NULL; }

	void		Broadcast(const JSONPacket* pMsg, Player* pUser = NULL);
	static void	EnterResult(Player* pUser, int code);

private:
	void		InitChair(void);
	TableChair*	GetChairInfo(Player* pUser);
	
	void		EnterTable(Player* pUser, const TableChair* pInfo);
	void		LeaveTable(Player* pUser, TableChair* pInfo, LOGOUT_TYPE type);
	bool		IsBetting(void);
    
public:
	OBJECT_ID	GetID(void) const { return _idTable; }
	int			GetUserAmount(void) const { return _amountUser; }
	int			GetChairID(Player* pUser);
	int			GetChairID(OBJECT_ID idUser);
	int			GetPlayerSize(void) const { return _Game_Players;}
	int			GetBaseMoney(void) const { return _baseMoney; }
	int			GetBombMoney(void) const { return _baseMoney * 10; }
	int		    GetEnterMoney(void) const { return _enterMoney; }
	int		    GetLeaveMoney(void) const { return _leaveMoney; }
	int 		GetFee();

private:
	void		GameStart(void);
	void		GameEnd(void);
	void		GameEndSpecial(int idChair);
	/*发牌,*/
	void		PlayCard(int dealer);
	/*更新出牌顺序.*/
	void		UpdateCallInfo(int idChair, bool save = true);
	/*获取指定座位的上家座位信息*/
	/*玩家出牌*/
	void		UserPlayCard(TableChair* pChair, const JSONPacket* packet);
	/*玩家过牌*/
	void		UserPass(TableChair* pChair, bool checked = false);
	/*玩家出牌合法性检测*/
	bool		CheckPlayCard(TableChair* pChair, int& cardType, const PokerCards& cards, CardStatistics& card_stat);
	/*自动出牌*/
	void		CallPlayCard(TableChair* pChair, PokerCards& cards, int cardType,
		CardStatistics& card_stat, CardAnalysis card_ana);
	/*获取指定座位的上家座位信息*/
	TableChair*	GetPrevChair(int idChair);
	/*获取指定座位的下家座位信息*/
	TableChair*	GetNextChair(int idChair);
	/*报单*/
	void		PlaySingle(TableChair* pChair);
	/*玩家打出炸弹*/
	void		PlayBomb();
	/*失败处理*/
	void		processLose(const TableChair* pChair, MONEY loseMoney);
	/*胜利处理*/
	void		processWin(const TableChair* pChair, MONEY winMoney);

	/*type 0 胜利*/
	void		proUpdateScore(const TableChair* pChair, MONEY score, int type);

	//保存庄家信息
	void		SaveBankerInfo(void);
	//保存游戏信息
	void		SaveGameInfo(void);
	//保存座位信息.
	void		SaveChairInfo(void);
	void		SaveChairInfo(const TableChair* pChair);
	void		DelChairInfo(const TableChair* pChair);
	//保存出牌信息.
	void		SaveCallInfo(void);
	//保存积分信息.
	void		SaveScoreInfo(void);
	void		DeleteInfo(void);
	bool		LoadInfo();

	// 一键进入
	void        UpdateStatus(int status);
	void        MarkTableUsers(int uid, int seatid, bool insert);

private:
	OBJECT_ID		_idTable;	//桌子id
	OBJECT_ID		_idDisp;	//显示房号
	GameRoom*		_pRoom;		//所属房间(vid)

	TableInfo		_info;
	int				_status;

	int				_next_bet;			//下个出牌的座位
	int				_prev_bet;			//上个出牌的座位
	int				_curr_bet;			//当前出牌的座位
	int				_bet_times;			//出牌次数.
	int				_pass_times;		//pass次数.
	PokerCards		_last_playCards;	//最后出的牌.
	int				_last_playType;		//最后出的牌型.
	CardStatistics	_last_card_stat;
	CardAnalysis	_last_card_ana;
	int				_banker_seatid;		//庄家座位.
	bool			_new_round;			//新的一轮

	Deck			_deck;				//牌库

	int				_gameTimes;		//牌局次数
    int             _gameType;

	int				_gameCardNo;		//牌数
	int				_totalTimes;		//总次数
	bool			_firstCard;			//先出黑桃3
	bool			_birdCard;			//抓鸟
	int				_birdSeatid;
	int             _fourdaithree;		//4带3
	int             _threeAbomb;		//3个A算炸

	int				_Game_Players;
    TableChair      _ChairInfo[_Game_Users];

	int				_fee;				// 台费
	int 			_baseMoney;			// 底分
	int				_enterMoney;		//进入分
	int				_leaveMoney;		//离开分
	int				_amountUser;
	int             _cardNum;			//显示剩余牌数

	//牌局回放相关
	Json::Value		_record;

	void			AutoPlayCard(TableChair* pChair, bool isTimeout);
	bool			AutoPass();

private:
    void            OnTimer_ready(void);
	void			OnTimer_trustee(void);
    EvTimer         _timer_ready;
	EvTimer			_timer_trustee;	//托管玩家出牌

#ifdef _ROBOT
public:
	void			OnTimer_robot(void);
	void			OnTimer_call(void);
	EvTimer			_timer_robot;	//调用机器人
	EvTimer			_timer_call;	//机器人出牌
#endif
};
