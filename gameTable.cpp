#include "gameTable.h"
#include "gameServer.h"
#include "Common/FunctionTTL.h"


OBJPOOL_IMPLEMENTATION(GameTable, TABLE_POOL_SIZE)

/*时钟,seconds*/
const char*     _DATA_NAME = "YBPDK_table";
const int       TIMEOUT_ACTION = 20;    //出牌超时时间


OBJECT_ID GameTable::_GetID(void) const
{
	return this->GetID();
}

void GameTable::_BroadcastMessage(const std::string& packet)
{
	for (int i = 0; i < _Game_Users; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		if (!pChair->IsOccupied())
			continue;
		if (pChair->IsUser() && pChair->pUser) {
			pChair->pUser->SendMsg(packet);
		}
	}
}

bool GameTable::_IsFreeTime(void) const
{
    return false;
}

bool GameTable::_RedisWriteLog(const char* command)
{
	return DB_EVENT->execute(command);
}

void GameTable::_OnAchievePacket(IGamePlayer* player, uint32_t money, int type)
{
	//玩家领了红包,写大喇叭.
	Json::Value parms;
	parms["type"] = type;
	parms["msgType"] = MSG_MONEY;

	std::string strName(player->_GetName());
	if (strName.length() > 18) {
		strName.erase(strName.begin() + 18, strName.end());
		strName += "...";
	}

	char buf[256];
	sprintf(buf, "恭喜玩家\"%s\"抢到了$%.2lf$元的现金红包", strName.c_str(), money / 100.0);
	GAMESERVER->TransmitMsg(buf, 1, parms);
}

GameTable::GameTable()
{
	_idTable = OBJECT_ID_UNDEFINED;
}

GameTable::~GameTable()
{

}

bool GameTable::Init(GameRoom* pRoom, OBJECT_ID idTable, OBJECT_ID idDisp, int baseMoney, int enterMoney, int leaveMoney, int fee, int gameType, int gameRule, int player, int showCardNum, int fourdaithree, int threeAbomb)
{
    _pRoom = pRoom;
    _idTable = idTable;
	_idDisp = idDisp;

	_baseMoney = baseMoney;
	//_info.bombMoney = _pRoom->GetBombMoney();
	_enterMoney = enterMoney;
	_leaveMoney = leaveMoney;
	_fee = fee;

	_Game_Players = player;
	if (_Game_Players < 2 || _Game_Players > 3)
	{
		_Game_Players = 3;
	}

	_cardNum = showCardNum;			// 是否显示剩余牌数 0 不显示 1 显示

	//4带3
	if (fourdaithree != 0 && fourdaithree != 1)
	{
		fourdaithree = 0;
	}

	//3个A算炸
	if (threeAbomb != 0 && threeAbomb != 1)
	{
		threeAbomb = 0;
	}

	_fourdaithree = fourdaithree;		// 4带3	
	_threeAbomb = threeAbomb;			// 3个A算炸

	InitChair();

	_firstCard = (gameRule & 1) ? true : false;
	_birdCard = (gameRule & 2) ? true : false;
	_gameType = gameType;
	_gameCardNo = gameType == 0 ? 16 : 15;
	_banker_seatid = -1;

	_gameTimes = 0;

#ifdef _ROBOT
	TIMER_CALLBACK cbRobot = CXX11::bind(&GameTable::OnTimer_robot, this);
	TIMER_CALLBACK cbCall = CXX11::bind(&GameTable::OnTimer_call, this);
	_timer_call.CreateTimer(2, 0, cbCall, false);
	_timer_robot.CreateTimer(5, 0, cbRobot, false);
#endif

    TIMER_CALLBACK cbReady = CXX11::bind(&GameTable::OnTimer_ready, this);
    _timer_ready.CreateTimer(1, 2, cbReady, false);//准备轮询定时器

	TIMER_CALLBACK cbTrustee = CXX11::bind(&GameTable::OnTimer_trustee, this);
	_timer_trustee.CreateTimer(TIMEOUT_ACTION, 0, cbTrustee, false);//托管出牌定时器

	_status = TSS_WAITING;
    _amountUser = 0;

    return LoadInfo();
}

void GameTable::Release()
{
	_timer_ready.Stop();
	_timer_trustee.Stop();
#ifdef _ROBOT
	_timer_call.Stop();
	_timer_robot.Stop();
#endif 
	delete this;
}

GameTable* GameTable::CreateNew(void)
{
	return new GameTable;
}

bool GameTable::Sitdown(Player* pUser)
{
	ASSERT_B(pUser);

	TableChair* pChair = GetChairInfo(pUser);
    if (!pChair && _amountUser >= _Game_Players){
        //		logDebug("桌子都满了,为何还会处理坐下的请求?");
        //房间已满.
        GameTable::EnterResult(pUser, ENTER_FULL);
        return false;
    }
	else if (pChair) {
		if (pChair->offline || pChair->pUser == pUser) {
#ifdef _DEBUG
			logDebug("%s:玩家[%s]离线重新登录.", __FUNCTION__, pUser->GetName());
#endif
			//重连
			if (pChair->pUser && pChair->pUser != pUser) {
				logDebug("离线登录时候,当前记录的玩家指针和新请求坐下的玩家指针不一致.");
			}
			else if (!pChair->pUser) {
				pChair->pUser = pUser;
			}
			pChair->loginType = 1;
            pChair->offline = false;
			EnterTable(pUser, pChair);
		}
		else {
			ASSERT_B(!"为什么没把玩家踢下线啊?都重复登录了啊!");
		}
	}
	else {
        for (int i = 0; i < _Game_Players; ++i) {
			TableChair* pInfo = &_ChairInfo[i];
			if (!pInfo->IsOccupied()) {
				pInfo->loginType = 0;
				pInfo->Sitdown(pUser);
                pInfo->offline = false;
				++_amountUser;
				EnterTable(pUser, pInfo);

                SaveChairInfo(pInfo);
				MarkTableUsers(pUser->GetID(), i, true);
				break;
			}
		}
#ifdef _ROBOT
		if (_amountUser < _Game_Players) {
			_timer_robot.Reset();
		}
		else {
			_timer_robot.Stop();
		}
#endif
	}

    //玩家进入需开启倒计时
    if (_status == TSS_WAITING || _status == TSS_ENDGAME) {
        _timer_ready.Reset();
    }
	return	true;
}

bool GameTable::Standup(Player* pUser, LOGOUT_TYPE type/*=LOGOUT_QUIT*/)
{
	ASSERT_B(pUser);
	TableChair* pInfo = GetChairInfo(pUser);
    if (pInfo) {
        if (IsBetting()) {
            logDebug("玩家(%u,%s)游戏中退出房间(%u)失败", pUser->GetID(), pUser->GetName(), _idTable);
            return false;
        }

        LeaveTable(pUser, pInfo, type);
	}
    else {
        logDebug("Standup异常,玩家根本就不在这张桌子(%u)上.uid:%u.", _idTable, pUser->GetID());
    }
#ifdef _ROBOT
	_timer_robot.Stop();
#endif
	return true;
}

/*只能在非游戏状态下踢人*/
/*
type:
0 退出
5 解散房间
6 被房主踢
*/
void GameTable::KickOut(TableChair* pInfo, LOGOUT_TYPE type/*=LOGOUT_KICKOUT*/)
{
	//离开状态检测.
	ASSERT_V(!IsBetting() && pInfo);
    Player* pUser = pInfo->pUser;
	ASSERT_V(pUser);
#ifndef _ROBOT
    if (pUser->_GetTableID() != _idTable) {
        logDebug("GameTable::KickOut 桌子id:%d user tid:%d", _idTable, pUser->_GetTableID());
        return;
    }
#endif
	LeaveTable(pUser, pInfo, type);
}

bool GameTable::handle_ready(Player* pUser)
{
    ASSERT_B(pUser);
	if (IsBetting()){
		//非准备的时机.
		return true;
	}
    //判断金币是否够够的
    if (pUser->GetMoney() < GetLeaveMoney()) {
	    if (pUser->GetMoney(true) < GetLeaveMoney())
	    {
			JSONPacket msg;
			msg.Create(P_SERVER_READY_ERROR_UC);
			msg.AddInfo("msg", "金币不够");
			pUser->SendMsg(&msg);
			logDebug("金币不够无法准备");
			return true;
	    }
    }

	TableChair* pInfo = GetChairInfo(pUser);
	ASSERT_B(pInfo);
	pInfo->ready = true;

	JSONPacket msg;
	msg.Create(P_SERVER_READY_SUCC_BC);
	msg.AddInfo("ready", 1);
	msg.AddInfo("uid", pUser->GetID());
	msg.AddInfo("seatid", pInfo->idChair);
	msg.AddInfo("money", pUser->GetMoney());
	Broadcast(&msg);

    //判断是否全部准备.
    if (_amountUser == _Game_Players) {
        int ready = 0;
        for (int i = 0; i < _Game_Players; ++i) {
            pInfo = &_ChairInfo[i];
            if (pInfo->IsOccupied() && pInfo->ready)
                ++ready;
        }

        if (ready == _Game_Players) GameStart();
    }

	return true;
}

bool GameTable::handle_chat(Player* pUser, const JSONPacket* packet)
{
	ASSERT_B(pUser && packet);
	const Json::Value& info = packet->GetInfo();
	CHECK_B(info["text"].isString());
	CHECK_B(info["type"].isInt());
	CHECK_B(info["chatid"].isInt());

#ifdef _DEBUG_CMD
	std::string str = info["text"].asCString();
	const char* cstr = str.c_str();
	if (str[0] == '!') {
		_totalTimes = atoi(cstr + 1);
		return true;
	}
#endif

	JSONPacket msg;
	msg.Create(N_SERVER_CHAT_BC);
	msg.AddInfo("uid", pUser->GetID());
	msg.AddInfo("seatid", GetChairID(pUser));
	msg.AddInfo("text", info["text"]);
	msg.AddInfo("chatid", info["chatid"]);
	msg.AddInfo("type", info["type"]);
	Broadcast(&msg);
	return true;
}

bool GameTable::handle_chatMsg(Player* pUser, const JSONPacket* packet)
{
	ASSERT_B(pUser && packet);
	const Json::Value& info = packet->GetInfo();
	JSONPacket msg;
	msg.Create(P_SERVER_CHATMSG_BC);
	msg.AddInfo("uid", pUser->GetID());
	msg.AddInfo("seatid", GetChairID(pUser));
	msg.AddInfo("data", info["data"]);
	Broadcast(&msg);
	return true;
}

bool GameTable::handle_face(Player * pUser, const JSONPacket * packet)
{
	return false;
}

bool GameTable::handle_tableInfo(Player* pUser)
{
	ASSERT_B(pUser);
	TableChair* pChair = GetChairInfo(pUser);
	ASSERT_B(pChair);

	pChair->offline = false;//离线标记为否

	JSONPacket msg;
	msg.Create(N_SERVER_TABLE_INFO_UC);
	msg.AddInfo("game_status", _status);
	msg.AddInfo("dealer", _banker_seatid);
	msg.AddInfo("player_seatid", GetChairID(pUser));
	msg.AddInfo("tid", this->GetID());
	if (_pRoom->GetRoomType() == 1)
	{
		msg.AddInfo("room_id", _idDisp);
	}
	msg.AddInfo("game_type", _gameType);
	msg.AddInfo("game_rule", _firstCard && _Game_Players != 2 ? 1 : 0);
	msg.AddInfo("bird_card", _birdCard ? 1 : 0);
	msg.AddInfo("player", _Game_Players);
	msg.AddInfo("cardNum", _cardNum);			//是否显示剩余牌数

	msg.AddInfo("base_money", GetBaseMoney());
	msg.AddInfo("enter_money", GetEnterMoney());
    msg.AddInfo("leave_money", GetLeaveMoney());
    
	int ready_sec = pChair->GetReadySec();
	if (ready_sec < 0) ready_sec = 0;
	msg.AddInfo("ready_sec", ready_sec);

	Json::Value& val = msg.GetInfo();
	/*游戏中,*/
	if (TSS_BETTING == _status) {
		cardHelper::to_json_array(_last_playCards, &msg, "last_play_card");
		msg.AddInfo("n_seatid", _curr_bet);
		msg.AddInfo("l_seatid", _prev_bet);

		for (int i = 0; i < _Game_Players; ++i) {
			//最后出牌数据
			const TableChair* pSeatInfo = &_ChairInfo[i];
			val["lastplaycards"][i]["seatid"] = i;
			cardHelper::to_json_array(pSeatInfo->last_playCards, val["lastplaycards"][i], "last_play_cards");
		}
		cardHelper::to_json_array(pChair->holeCards.cards, &msg, "hole_cards");

		if (pChair->idChair == _curr_bet)
		{
			// 如果轮到自己出牌 出牌倒计时还有多少秒
			msg.AddInfo("bet_sec", _timer_trustee.GetRemain());
		}

		pChair->loginType = 1;
	}
	else if (TSS_START != _status) {
		pChair->loginType = 0;
	}

	msg.AddInfo("login_type", pChair->loginType);
	//4带3
	if (_fourdaithree == 1)
	{
		msg.AddInfo("fourdaithree", _fourdaithree);
	}

	//3个A算炸
	if (_threeAbomb == 1)
	{
		msg.AddInfo("threeAbomb", _threeAbomb);
	}

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pInfo = &_ChairInfo[i];
		if (pInfo->IsOccupied()) {
			Player*		player = pInfo->pUser;
			Json::Value user;
			user["uid"] = player->GetID();
			user["seatid"] = i;
			user["ready"] = pInfo->ready ? 1 : 0;
			user["name"] = player->GetName();
			user["sex"] = player->GetSex();
			user["money"] = player->GetMoney();
			user["avatar"] = player->GetAvatar();
			user["is_single"] = pInfo->isSingle ? 1 : 0;
			user["is_online"] = player->IsOnline() ? 1 : 0;
			user["ip"] = player->GetIPAddress();
			user["is_pass"] = pInfo->isPass ? 1 : 0;
			//勾选了显示剩余牌数
			if (_cardNum == 1)
			{
				//剩余牌数
				int cardNum = pChair->holeCards.cards.size();
				user["handCardNum"] = cardNum;
			}

			val["players"].append(user);
		}
	}

	pUser->SendMsg(&msg);

	return true;
}

bool GameTable::handle_bet(Player* pUser, const JSONPacket* packet)
{
	ASSERT_B(TSS_BETTING == _status);
	if (TSS_BETTING != _status) {
		logErr("拒绝出牌,当前桌子状态:%d,uid:%u,idTable:%u", _status, pUser->GetID(), _idTable);
		handle_tableInfo(pUser);
		return true;
	}
	ASSERT_B(pUser && packet);
	TableChair* pChair = GetChairInfo(pUser);
	ASSERT_B(pChair);
    if (pChair->idChair != _curr_bet) {
        logDebug("玩家上传了出牌消息,但是当前不是他在出牌.uid:%u, tid%u", pUser->GetID(), _idTable);
        return true;
    }

#ifdef _ROBOT
	_timer_call.Stop();
#endif

	const Json::Value& val = packet->GetInfo();
	CHECK_B(val["action"].isInt());
	int action = val["action"].asInt();

	if (action == ACTION_PLAY) {
		UserPlayCard(pChair, packet);
	}
	else if (action == ACTION_PASS) {
		UserPass(pChair);
	}
	return true;
}

bool GameTable::handle_run(Player* pUser)
{
	ASSERT_B(TSS_BETTING == _status);

	if (_Game_Players != 2) return true;

	if (GetBaseMoney() < 10000)
	{
		return true;
	}
	
	_timer_trustee.Stop();

	GameEndSpecial(GetChairID(pUser));

	return true;
}

bool GameTable::handle_recharge(Player* pUser)
{
	ASSERT_B(pUser);
	TableChair* pChair = GetChairInfo(pUser);
	ASSERT_B(pChair);

	int pre_money = pUser->GetMoney();
	int now_money = pUser->GetMoney(true);

	if (now_money != pre_money)
	{
		JSONPacket msg;
		msg.Create(P_SERVER_SYNC_MONEY_BC);
		msg.AddInfo("uid", pUser->GetID());
		msg.AddInfo("seatid", pChair->idChair);
		msg.AddInfo("money", now_money);
		Broadcast(&msg);
		logInfo("同步玩家(%u,%s)充值 (%d) --> (%d)", pUser->GetID(), pUser->GetName(), pre_money, now_money);
	}
	return true;
}


bool GameTable::handle_forward(Player* pUser, const JSONPacket* packet)
{
	ASSERT_B(pUser && packet);
	TableChair* pChair = GetChairInfo(pUser);
	ASSERT_B(pChair);
	const Json::Value& val = packet->GetInfo();
	int type = val["type"].asInt();
	if (type == 100) // 固定100 扣费互动表情
	{
		//连发
		int count = val["count"].asInt();
		ASSERT_B(count > 0);

		JSONPacket msg;
		msg.Init(const_cast<Json::Value&>(val));
		int cur_money = pUser->GetMoney();
		int need_money = count * 10 + GetLeaveMoney();
		if (cur_money < need_money)
		{
			msg.AddInfo("error", 1);//金币不足
			pUser->SendMsg(&msg);
			return true;
		}

		if (pUser->IncrMoney(-count*10, _idTable, PHOTO))//更新金币

		{
			msg.AddInfo("money", pUser->GetMoney());
			Broadcast(&msg);
		}
	}
	else
	{
		Broadcast(packet);
	}
	return true;
}


void GameTable::Offline(Player* pUser)
{
	ASSERT_V(pUser);
	TableChair* pChair = GetChairInfo(pUser);
	ASSERT_V(pChair);
	pChair->offline = true;
	//pChair->ready = false;
#ifdef _DEBUG
	logDebug("玩家[%s]变为离线状态.", pUser->GetName());
#endif
	JSONPacket msg;
	msg.Create(P_SERVER_PLAYER_CONNINFO);
	msg.AddInfo("is_online", 0);
	msg.AddInfo("seatid", pChair->idChair);
	msg.AddInfo("uid", pUser->GetID());
	Broadcast(&msg, pUser);
}

void GameTable::OnResume(Player* pUser)
{
	ASSERT_V(pUser);
	TableChair* pChair = GetChairInfo(pUser);
	ASSERT_V(pChair);
	pChair->loginType = IsBetting() ? 1 : 0;
	EnterTable(pUser, pChair);
}

void GameTable::Broadcast(const JSONPacket* pMsg, Player* pUser /*= NULL*/)
{
	const std::string& msg = pMsg->GetPacket();
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pInfo = &_ChairInfo[i];
		if (pInfo->IsOccupied() && !pInfo->offline) {
			if(pUser && pUser==pInfo->pUser)
				continue;
			if(!pInfo->pUser->IsOnline())
				continue;
			pInfo->pUser->SendMsg(msg);
		}
	}
}

void GameTable::InitChair(void)
{
	_amountUser = 0;

	for (int i = 0; i < _Game_Users; ++i) {
		_ChairInfo[i].idChair = i;
		_ChairInfo[i].Clear();
	}
}

GameTable::TableChair* GameTable::GetChairInfo(Player* pUser)
{
	ASSERT_P(pUser);
	OBJECT_ID idUser = pUser->GetID();
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pInfo = &_ChairInfo[i];
		if (pInfo->idUser == idUser) {
			return pInfo;
		}
	}
	return NULL;
}

void GameTable::EnterResult(Player* pUser, int code)
{
    ASSERT_V(pUser);
	if (!pUser->IsOnline()) return;

	const char* message = "error";

	switch (code) {
	case ENTER_OK:	
		message = "okay!";
		break;
	case ENTER_FULL:
		message = "房间已满";
		break;
	case ENTER_FOUND:
		message = "房间不存在";
		break;
	case ENTER_MASTER:
		message = "不可坐下预留房主座位";
		break;
    case ENTER_MONEY_LESS:
        message = "金币不够";
        break;
	}

	JSONPacket msg;
	msg.Create(code != ENTER_OK ? P_SERVER_LOGIN_ERR_UC : N_SERVER_LOGIN_SUCC_UC);
	msg.AddInfo("code", code);
	msg.AddInfo("msg", message);

	pUser->SendMsg(&msg);
}

void GameTable::EnterTable(Player* pUser, const TableChair* pInfo)
{
	ASSERT_V(pUser && pInfo);
	EnterResult(pUser, ENTER_OK);

	JSONPacket msg;
	msg.Create(P_SERVER_LOGIN_SUCC_BC);				//4001
	msg.AddInfo("seatid", pInfo->idChair);		//座位号
	msg.AddInfo("ready", pInfo->ready ? 1 : 0);	//是否准备
	msg.AddInfo("uid", pUser->GetID());			//uid
	msg.AddInfo("name", pUser->GetName());		//名称
	msg.AddInfo("sex", pUser->GetSex());		//性别
	msg.AddInfo("money", pUser->GetMoney());	//积分.
	msg.AddInfo("avatar", pUser->GetAvatar());	//头像
	msg.AddInfo("is_online", pUser->IsOnline() ? 1 : 0);//是否在线.
	msg.AddInfo("ip", pUser->GetIPAddress());
    msg.AddInfo("lat", pUser->GetLat());
    msg.AddInfo("lng", pUser->GetLng());

	Broadcast(&msg, pUser);
}

void GameTable::LeaveTable(Player* pUser, TableChair* pInfo, LOGOUT_TYPE type)
{
    JSONPacket msg;
    msg.Create(P_SERVER_LOGOUT_SUCC_BC);
    msg.AddInfo("uid", pUser->GetID());
    msg.AddInfo("seatid", pInfo->idChair);
    msg.AddInfo("type", type);
    Broadcast(&msg);

	logDebug("GameTable::LeaveTable.type:%d,idTable:%u, uid:%d, name:[%s]", type, _idTable, pUser->GetID(), pUser->GetName());

	// 好友房有人离开的时候重置庄家
	if (_pRoom->GetRoomType()== 1)
	{
		if (_banker_seatid != -1)
		{
			_banker_seatid = -1;
			_gameTimes = 0;

			SaveBankerInfo();
		}
	}

#ifdef _ROBOT
	if (!pInfo->IsUser()) {
		delete pUser;
		goto lable_leave;
	}
#endif

	_pRoom->OnLeaveTable(pUser, true);
lable_leave:
	pInfo->Clear();
    DelChairInfo(pInfo);
	--_amountUser;
	MarkTableUsers(pUser->GetID(), pInfo->idChair, false);

	if (_amountUser == 0)
	{
		DeleteInfo();

		if (_pRoom->GetRoomType() == 1)
		{
			_pRoom->DismissLater(this);
		}
	}	
}

bool GameTable::IsBetting(void)
{
	return TSS_START == _status || TSS_BETTING == _status;
}

void GameTable::OnTimer_ready(void)
{
    if (_amountUser == 0) {
        _timer_ready.Stop();
        return;
    }

	if (_pRoom->GetRoomType() == 1)
	{
		_timer_ready.Stop();
		return;
	}

    int n = 0;
    for (int i = 0; i < _Game_Players; ++i) {
        TableChair* pChair = &_ChairInfo[i];
        if (!pChair->IsOccupied())
            continue;

        if (pChair->ready) {
            ++n;
            continue;
        }

		//超时未准备，直接剔出
		if (pChair->GetReadySec() <= 0) {
			KickOut(pChair);
		}	    
    }

	//_timer_ready.Reset();
}

void GameTable::OnTimer_trustee()
{
	if (_pRoom->GetRoomType() == 1) return;

	TableChair* pChair = &_ChairInfo[_curr_bet];
	if (!pChair->IsUser()) return;

	if (TSS_BETTING == _status)
		AutoPlayCard(pChair, false);
}


int GameTable::GetChairID(Player* pUser)
{
	TableChair* pInfo=GetChairInfo(pUser);
	return pInfo ? pInfo->idChair : -1;
}

int GameTable::GetChairID(OBJECT_ID idUser)
{
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		if (pChair->IsOccupied() && pChair->idUser == idUser)
			return i;
	}
	return -1;
}

int GameTable::GetFee()
{
	return _fee;
}

void GameTable::GameStart(void)
{
	FunctionTTL ttl(5, "GameTable::GameStart");

	ASSERT_V(_amountUser == _Game_Players);
	_status = TSS_START;

	++_gameTimes; //游戏次数+1

    _timer_ready.Stop();

	/*初始化游戏数据*/
	_bet_times = 0;
	_pass_times = 0;

	_last_playCards.clear();
	_last_playType = CARD_TYPE_ERROR;
	_last_card_stat.clear();
	//////////////////

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		pChair->ResetFlags();
	}

	//更新房间状态
	UpdateStatus(1);

	JSONPacket msg;
	msg.Create(P_SERVER_GAME_START_BC);
	Json::Value& msgVal = msg.GetInfo();
	//_deck.fill();
	//_deck.shuffle(_idTable);

	int dealer = -1;

	_deck.fill(_gameType);
	_deck.shuffle(_idTable);

	// 做牌
	//_deck.make_card();

	int randCard = 0x33;

	if (_Game_Players < _Game_Users) {
		randCard = _deck.get_random_card();
	}

	_deck.type = 2;

	//=============回放数据===============
	_record.clear();
	_record["cmd"] = 3;
	_record["tid"] = _idTable;
	_record["vid"] = _pRoom->GetID();
	_record["isgold"] = _pRoom->GetRoomType() == 0 ? 1 : 2; // 区分系统场与好友场 这里需要传值进来
	//====================================

	//红桃10鸟.
	int birdCard = 0x2A;
	_birdSeatid = -1;

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChairInfo = &_ChairInfo[i];
		_deck.get_hole_cards(pChairInfo->holeCards);

		// 庄判断
		if (dealer == -1 && pChairInfo->holeCards.findCard(randCard)) {
			dealer = pChairInfo->idChair;
		}

		// 鸟牌座位判断
		if (_birdCard && _birdSeatid == -1) {
			if (pChairInfo->holeCards.findCard(birdCard)) {
				_birdSeatid = pChairInfo->idChair;
			}
		}

		Player* pUser = pChairInfo->pUser;
       
		Json::Value jsUser;
		jsUser["money"] = pUser->GetMoney();
		jsUser["seatid"] = pChairInfo->idChair;
		jsUser["uid"] = pUser->GetID();
		//=============回放数据===============
		_record["players"][i]["seatid"] = i;
		_record["players"][i]["uid"] = pUser->GetID();
		_record["players"][i]["name"] = pUser->GetName();
		_record["players"][i]["avatar"] = pUser->GetAvatar();
		_record["players"][i]["sex"] = pUser->GetSex();
		_record["players"][i]["money"] = pUser->GetMoney();
		_record["players"][i]["gamerule"] = _firstCard ? 1 : 0;
		_record["players"][i]["birdcard"] = _birdCard ? 1 : 0;
		_record["players"][i]["cardNum"] = _cardNum ? 1 : 0;
		_record["players"][i]["game_type"] = 1;
		cardHelper::to_json_array(pChairInfo->holeCards.cards, _record["players"][i], "cards");
		//====================================
		msgVal["players"].append(jsUser);
	}

	//二人玩法,特别的处理.
	//=============回放数据===============
	if (_Game_Players == 2) {
		Json::Value user;
		user["uid"] = 0;
		cardHelper::to_json_array(_deck.cards, user, "cards");
		_record["players"].append(user);
	}
	//====================================


	if (dealer < 0 || dealer >= _Game_Players) {
		dealer = rand() % _Game_Players;
		logMsg("未确定出牌顺序,随机指定为:%d", dealer);
	}

	if (_banker_seatid == -1 || (_banker_seatid < 0 || _banker_seatid >= _Game_Players))
	{
		_banker_seatid = dealer;
	}
	
	msg.AddInfo("random_card", randCard);
	msg.AddInfo("dealer", _banker_seatid);
	msg.AddInfo("base_money", GetBaseMoney());

	msg.AddInfo("game_status", _status);
	Broadcast(&msg);

	//=============回放数据===============
	_record["dealer"] = dealer;
	//====================================

	SaveGameInfo();
    
	PlayCard(_banker_seatid);
}

/*游戏结算
*/
void GameTable::GameEnd(void)
{
	FunctionTTL ttl(5, "GameTable::GameEnd");
#ifdef _DEBUG
	logDebug("idTable(%u)进入结算流程.", _idTable);
#endif
	_status = TSS_ENDGAME;

	Json::Value scoreVal;
	scoreVal["cmd"] = 5;
	scoreVal["tid"] = _idTable;
	scoreVal["vid"] = _pRoom->GetID();
	scoreVal["bankerseatid"] = _banker_seatid;

	int win_seatid = _curr_bet;
	JSONPacket msg;
	msg.Create(N_SERVER_GAME_END_BC);
	msg.AddInfo("win_seatid", win_seatid);
	
	Json::Value& val = msg.GetInfo();

	int cardNum = 0;//牌数

	MONEY useMoney[_Game_Users];
	memset(useMoney, 0, sizeof(useMoney));

	int rate = 1;
	if (_birdCard)
	{
		if (_birdSeatid != -1)
		{
			msg.AddInfo("bird_seatid", _birdSeatid);
		}
		if (_birdSeatid == win_seatid)
			rate = 2;
	}

	// 游戏结束扣除台费
	for (int  i = 0; i < _Game_Players; i++)
	{
		TableChair* pChair = &_ChairInfo[i];
		Player*		pUser = pChair->pUser;
		int fee = GetFee();

		//if (fee > 0)
		{
			pUser->IncrMoney(-fee, _idTable, FEE);
			logDebug("扣台费[%d] [%d] [%d]", -fee, _idTable, FEE);
		}
	}

	//统计牌局情况.
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		scoreVal["scores"][i]["uid"] = pChair->idUser;
		scoreVal["scores"][i]["name"] = pChair->pUser->GetName();
		//=============回放数据===============
		_record["scores"][i]["uid"] = pChair->idUser;
		_record["scores"][i]["name"] = pChair->pUser->GetName();
		//====================================

		int cardno = pChair->holeCards.size();
		scoreVal["scores"][i]["cardNum"] = cardno;
		if (i != win_seatid) {
			MONEY money_t = 0;
			if (cardno >= _gameCardNo) {
				money_t = cardno * GetBaseMoney() * 2;//全关翻倍
				scoreVal["scores"][i]["locked"] = 1;
				scoreVal["scores"][i]["single"] = 0;
				//=============回放数据===============
				_record["scores"][i]["locked"] = 1;
				_record["scores"][i]["single"] = 0;
				//====================================
			}
			else if (cardno > 1) {
				//没报单
				cardNum += cardno;
				money_t = cardno * GetBaseMoney();
				scoreVal["scores"][i]["locked"] = 0;
				scoreVal["scores"][i]["single"] = 0;
				//=============回放数据===============
				_record["scores"][i]["locked"] = 0;
				_record["scores"][i]["single"] = 0;
				//====================================
			}
			else {
				scoreVal["scores"][i]["locked"] = 0;
				scoreVal["scores"][i]["single"] = 1;
				//=============回放数据===============
				_record["scores"][i]["locked"] = 0;
				_record["scores"][i]["single"] = 1;
				//====================================
			}

			if (_birdSeatid == i || rate == 2)
				money_t *= 2;

			// 自定义场输赢置0
			if (GetBaseMoney() >= 10000)
			{
				money_t = 0;
			}

			// 不让金币扣穿
			//if (pChair->pUser->GetMoney() < money_t)
			//{
			//	money_t = pChair->pUser->GetMoney();
			//}

			useMoney[i] = -money_t;
			useMoney[win_seatid] += money_t;
		}
		else {
			scoreVal["scores"][i]["locked"] = 0;
			scoreVal["scores"][i]["single"] = pChair->isSingle ? 1 : 0;
			//=============回放数据===============
			_record["scores"][i]["locked"] = 0;
			_record["scores"][i]["single"] = pChair->isSingle ? 1 : 0;
			//====================================
		}
	}

	bool changed = false;
	MONEY total_adjust = 0;
	// 炸弹分合并 并检查玩家金币是否够扣
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		Player* pUser = pChair->pUser;
		// 增加炸弹分
		useMoney[i] += pChair->bombScore;

		// 如果是负分而且分不够的情况下要调整扣分
		if (useMoney[i] < 0 && pUser->GetMoney() < abs(useMoney[i]))
		{
			changed = true;

			// 如果已经是负数了 就不能继续再扣了
			if (pUser->GetMoney() < 0)
			{
				MONEY adjust = abs(useMoney[i]);
				total_adjust += adjust;
				useMoney[i] = 0;
			}
			else
			{
				MONEY adjust = abs(useMoney[i]) - pChair->pUser->GetMoney();
				total_adjust += adjust;
				useMoney[i] = -pChair->pUser->GetMoney();
			}
		}
	}

	// 输赢分调整 配平 需要从赢分的人里面将调整的分数扣出来
	if (changed && total_adjust > 0)
	{
		for (int i = 0; i < _Game_Players; i++)
		{
			int index = (i + win_seatid + 1) % _Game_Players;
			if (useMoney[index] > 0 )
			{
				if (useMoney[index] >= total_adjust)
				{
					useMoney[index] = useMoney[index] - total_adjust;
					break;
				}
				else
				{
					useMoney[index] = 0;
					total_adjust = total_adjust - useMoney[index];
				}
			}
		}
	}

	for (int i = 0; i < _Game_Players; ++i)
	{
		TableChair* pChair = &_ChairInfo[i];

		Player* pUser = pChair->pUser;

		// 更新用户金币
		pUser->IncrMoney(useMoney[i], _idTable, BALANCE);
		//平台数据更新
		pUser->inc_playTimes();
	}

	//for (int i = 0; i < _Game_Players; ++i) {
	//	TableChair* pChair = &_ChairInfo[i];
	//	if (i != win_seatid) {
	//		processLose(pChair, useMoney[i]);
	//	}
	//}

	//processWin(&_ChairInfo[win_seatid], useMoney[win_seatid]);

	// 好友房赢家做庄 在房间有人进出的情况下要重置这个庄家
	if (_pRoom->GetRoomType() == 1)
	{
		_banker_seatid = win_seatid;
	}
	else
	{
		_banker_seatid = -1;
	}

	// 保存庄家信息
	SaveBankerInfo();

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		Player*		pUser = pChair->pUser;
		
		MONEY money = useMoney[i]; // *(pChair->idChair == win_seatid ? 1 : -1);
		//money += pChair->bombScore;

		scoreVal["scores"][i]["incscore"] = money;
		scoreVal["scores"][i]["bomb"] = pChair->bombTimes;

		val["balance"][i]["money"] = money;
		val["balance"][i]["is_win"] = pChair->idChair == win_seatid ? 1 : 0;
		val["balance"][i]["total_money"] = pUser->GetMoney();
		int cardno = pChair->holeCards.size();
		val["balance"][i]["seatid"] = i;
		val["balance"][i]["cardsno"] = cardno;
		val["balance"][i]["bomb"] = pChair->bombTimes;

		//=============回放数据===============
		_record["scores"][i]["money"] = money;
		_record["scores"][i]["incscore"] = money;
		_record["scores"][i]["is_win"] = pChair->idChair == win_seatid ? 1 : 0;
		_record["scores"][i]["total_money"] = pUser->GetMoney();
		_record["scores"][i]["seatid"] = i;
		_record["scores"][i]["cardsno"] = cardno;
		_record["scores"][i]["bomb"] = pChair->bombTimes;
		cardHelper::to_json_array(pChair->holeCards.cards, _record["scores"][i], "cards");
		//====================================

		cardHelper::to_json_array(pChair->holeCards.cards, val["balance"][i], "cards");
	}
	
	//二人玩法,剩的牌发过去.
	if (_Game_Players == 2) {
		cardHelper::to_json_array(_deck.cards, &msg, "cards");
	}

    _timer_ready.Reset();
    msg.AddInfo("ready_sec", TIMEOUT_READY);
    Broadcast(&msg);

	//写流水日志
	std::string strInfo(scoreVal.toStyledString());
	if (!GameRoom::WriteGameLog(strInfo.c_str())) {
		logMsg("牌局流水数据写入数据库失败.\n%s", strInfo.c_str());
	}
	//写回放记录
	if (_record["cmd"].isInt())
	{
		std::string strRecord(_record.toStyledString());
		if (!GameRoom::WriteGameLog(strRecord.c_str())) {
			logMsg("牌局回放数据写入数据库失败.\n%s", strRecord.c_str());
		}
	}
	
	UpdateStatus(0);

    for (int i = 0; i < _Game_Players; ++i) {
        if(!_ChairInfo[i].IsOccupied())
            continue;
        _ChairInfo[i].ResetFlags();
#ifdef _ROBOT
        Player* pUser = _ChairInfo[i].pUser;
        if (!_ChairInfo[i].IsUser()) {
			logDebug("robot [%d] ready", pUser->GetID());
            handle_ready(pUser);
        }
#endif
    }
}

// 
void GameTable::GameEndSpecial(int idChair)
{
	FunctionTTL ttl(5, "GameTable::GameEnd");
#ifdef _DEBUG
	logDebug("idTable(%u)进入特殊结算流程.", _idTable);
#endif
	_status = TSS_ENDGAME;

	_timer_trustee.Stop();

	Json::Value scoreVal;
	scoreVal["cmd"] = 5;
	scoreVal["tid"] = _idTable;
	scoreVal["vid"] = _pRoom->GetID();
	scoreVal["bankerseatid"] = _banker_seatid;

	int win_seatid = -1;

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];

		if (pChair->IsOccupied() && pChair->idChair != idChair)
		{
			win_seatid = pChair->idChair;
			break;
		}
	}

	JSONPacket msg;
	msg.Create(N_SERVER_GAME_END_BC);
	msg.AddInfo("win_seatid", win_seatid);

	Json::Value& val = msg.GetInfo();

	int cardNum = 0;//牌数

	MONEY useMoney[_Game_Users];
	memset(useMoney, 0, sizeof(useMoney));

	int rate = 1;
	
	//统计牌局情况.
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		scoreVal["scores"][i]["uid"] = pChair->idUser;
		scoreVal["scores"][i]["name"] = pChair->pUser->GetName();
		//=============回放数据===============
		_record["scores"][i]["uid"] = pChair->idUser;
		_record["scores"][i]["name"] = pChair->pUser->GetName();
		//====================================

		int cardno = pChair->holeCards.size();
		scoreVal["scores"][i]["cardNum"] = cardno;
		if (i != win_seatid) {
			MONEY money_t = 0;
			{
				if (idChair == 0  && _ChairInfo[0].pUser->GetMoney(true) >= GetBaseMoney())
				{
					money_t = GetBaseMoney();  // 逃跑扣底分
				}

				scoreVal["scores"][i]["locked"] = 1;
				scoreVal["scores"][i]["single"] = 0;
				//=============回放数据===============
				_record["scores"][i]["locked"] = 1;
				_record["scores"][i]["single"] = 0;
				//====================================
			}
			
			useMoney[i] = money_t;
			useMoney[win_seatid] += money_t;
		}
		else {
			scoreVal["scores"][i]["locked"] = 0;
			scoreVal["scores"][i]["single"] = pChair->isSingle ? 1 : 0;
			//=============回放数据===============
			_record["scores"][i]["locked"] = 0;
			_record["scores"][i]["single"] = pChair->isSingle ? 1 : 0;
			//====================================
		}
	}

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		if (i != win_seatid) {
			processLose(pChair, useMoney[i]);
		}
	}

	processWin(&_ChairInfo[win_seatid], useMoney[win_seatid]);
	
	// 好友房赢家做庄 在房间有人进出的情况下要重置这个庄家
	if (_pRoom->GetRoomType() == 1)
	{
		_banker_seatid = win_seatid;
	}
	else
	{
		_banker_seatid = -1;
	}

	// 保存庄家信息
	SaveBankerInfo();

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		Player*		pUser = pChair->pUser;

		MONEY money = useMoney[i] * (pChair->idChair == win_seatid ? 1 : -1);
		money += pChair->bombScore;

		scoreVal["scores"][i]["incscore"] = money;
		scoreVal["scores"][i]["bomb"] = pChair->bombTimes;

		val["balance"][i]["money"] = money;
		val["balance"][i]["is_win"] = pChair->idChair == win_seatid ? 1 : 0;
		//val["balance"][i]["total_money"] = GetScore(pUser);
		int cardno = pChair->holeCards.size();
		val["balance"][i]["seatid"] = i;
		val["balance"][i]["cardsno"] = cardno;
		val["balance"][i]["bomb"] = pChair->bombTimes;

		//=============回放数据===============
		_record["scores"][i]["money"] = money;
		_record["scores"][i]["incscore"] = money;
		_record["scores"][i]["is_win"] = pChair->idChair == win_seatid ? 1 : 0;
		//_record["scores"][i]["total_money"] = GetScore(pUser);
		_record["scores"][i]["seatid"] = i;
		_record["scores"][i]["cardsno"] = cardno;
		_record["scores"][i]["bomb"] = pChair->bombTimes;
		cardHelper::to_json_array(pChair->holeCards.cards, _record["scores"][i], "cards");
		//====================================

		cardHelper::to_json_array(pChair->holeCards.cards, val["balance"][i], "cards");
	}

	//二人玩法,剩的牌发过去.
	if (_Game_Players == 2) {
		cardHelper::to_json_array(_deck.cards, &msg, "cards");
	}

	_timer_ready.Reset();
	msg.AddInfo("ready_sec", TIMEOUT_READY);
	Broadcast(&msg);

	//写流水日志
	std::string strInfo(scoreVal.toStyledString());
	if (!GameRoom::WriteGameLog(strInfo.c_str())) {
		logMsg("牌局流水数据写入数据库失败.\n%s", strInfo.c_str());
	}
	//写回放记录
	if (_record["cmd"].isInt())
	{
		std::string strRecord(_record.toStyledString());
		if (!GameRoom::WriteGameLog(strRecord.c_str())) {
			logMsg("牌局回放数据写入数据库失败.\n%s", strRecord.c_str());
		}
	}

	UpdateStatus(0);

	for (int i = 0; i < _Game_Players; ++i) {
		if (!_ChairInfo[i].IsOccupied())
			continue;
		_ChairInfo[i].ResetFlags();
#ifdef _ROBOT
		Player* pUser = _ChairInfo[i].pUser;
		if (!_ChairInfo[i].IsUser()) {
			logDebug("robot [%d] ready", pUser->GetID());
			handle_ready(pUser);
		}
#endif
	}
}

/*取上个出牌的座位*/
GameTable::TableChair* GameTable::GetPrevChair(int idChair)
{
	--idChair;
	if (idChair < 0) idChair += _Game_Players;
	return &_ChairInfo[idChair];
}

/*取下个出牌的座位*/
GameTable::TableChair* GameTable::GetNextChair(int idChair)
{
	++idChair;
	if (idChair >= _Game_Players) idChair %= _Game_Players;
	return &_ChairInfo[idChair];
}

/*只剩一张牌了,报单*/
void GameTable::PlaySingle(TableChair* pChair)
{
	pChair->isSingle = true;
	JSONPacket msg;
	msg.Create(P_SERVER_GAME_SINGLECARD);
	msg.AddInfo("uid", pChair->idUser);
	msg.AddInfo("seatid", pChair->idChair);
	Broadcast(&msg);
}

/*炸弹处理.下次出牌的就是出炸弹的.*/
void GameTable::PlayBomb()
{
	ASSERT_V(TSS_BETTING == _status);

	MONEY money = GetBombMoney();

	if (GetBaseMoney() >= 10000)
	{
		money = 0;
	}

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		if (i != _next_bet) {
			pChair->bombScore -= money;
		}
	}

	TableChair* pChair = &_ChairInfo[_next_bet];
	pChair->bombTimes++;
	pChair->bombScore += money * (_Game_Players - 1);

	SaveChairInfo();
}

void GameTable::PlayCard(int dealer)
{
	_status = TSS_BETTING;

	_new_round = true;
	_bet_times = 0;
	_pass_times = 0;

	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChairInfo = &_ChairInfo[i];
		Player* pUser = pChairInfo->pUser;
		if (!pUser->IsOnline())
			continue;
		JSONPacket msg;
		msg.Create(P_SERVER_GAME_START_UC);
		msg.AddInfo("uid", pUser->GetID());
		msg.AddInfo("name", pUser->GetName());
		msg.AddInfo("sex", pUser->GetSex());
		msg.AddInfo("seatid", pChairInfo->idChair);
		msg.AddInfo("dealer", dealer);
#ifdef _DEBUG
		logDebug("playCard::%s", pUser->GetName());
#endif
		Json::Value& msgVal = msg.GetInfo();
		cardHelper::to_json_array(pChairInfo->holeCards.cards, msgVal, "cards");
		pUser->SendMsg(&msg);
	}

	//保存游戏信息.
	SaveGameInfo();

	UpdateCallInfo(dealer);
}

/*更新出牌座位信息*/
void GameTable::UpdateCallInfo(int idChair, bool save)
{
	ASSERT_V(idChair >= 0 && idChair < _Game_Players);
	_curr_bet = idChair;
	_prev_bet = idChair - 1;
	if (_prev_bet < 0) _prev_bet += _Game_Players;
	_next_bet = idChair + 1;
	if (_next_bet >= _Game_Players) _next_bet %= _Game_Players;

	_ChairInfo[_curr_bet].isPass = false;

#ifdef _ROBOT
	if (!_ChairInfo[_curr_bet].IsUser()) {
		_timer_call.Reset();
	}
#endif

	if (save) {
		//保存出牌信息.
		SaveCallInfo();
	}

	if (_ChairInfo[_curr_bet].offline == 1)
	{
		// 断线时判断是否能自动PASS 如果能pass直接返回 不能则还是进行等待
		bool auto_pass = AutoPass();
		if (auto_pass) return;
	}

#ifdef TRUSTEE
	if (_ChairInfo[_curr_bet].IsUser())
	{
		_timer_trustee.Reset();
	}
#endif

}

void GameTable::UserPlayCard(TableChair* pChair, const JSONPacket* packet)
{
	Player* pUser = pChair->pUser;

	//step 1:检查玩家手中是否有这些牌.
	PokerCards playCards;
	cardHelper::to_vector(packet, "cards", playCards);
	MapCards& playerCards = pChair->holeCards.cards;
	for (PokerCards::const_iterator iter = playCards.begin(); iter != playCards.end(); ++iter) {
		const Card& tempCard = *iter;
		if (playerCards.find(tempCard.value) == playerCards.end()) {
			JSONPacket msg;
			msg.Create(P_SERVER_SYNC_CARDS);
			cardHelper::to_json_array(playerCards, &msg, "cards");
			pUser->SendMsg(&msg);
			const Json::Value& val_c = packet->GetInfo();
			const Json::Value& val_s = msg.GetInfo();
			logErr("客户端与服务器牌数据不同步了.uid:%u,手牌:%s,出牌:%s", pUser->GetID(),
				val_s["cards"].toStyledString().c_str(), val_c["cards"].toStyledString().c_str());
			return;
		}
	}
	CardStatistics playcard_stat;
	playcard_stat.statistics(playCards);
	CardAnalysis playcard_ana;
	playcard_ana.analysis(playcard_stat, _gameType, _fourdaithree, _threeAbomb);
	int play_type = playcard_ana.type;
	if (play_type == CARD_TYPE_ERROR) {
		std::string cardsInfo;
		Card::dump_cards(playCards, cardsInfo);
		logErr("客户端打出未知牌型,uid:%u,cards:%s", pUser->GetID(), cardsInfo.c_str());
		JSONPacket msg;
		msg.Create(P_SERVER_BET_ILLEGAL);
		msg.AddInfo("msg", "无法识别的牌型");
		pUser->SendMsg(&msg);
		return;
	}
	//step 2:合法性判断
	//第1局,第1手,黑桃3判断.
	if (_Game_Players == _Game_Users) {
		//三人玩时才有这个判断.
		if (0 == _bet_times && _gameTimes <= 1 && _firstCard) {
			if (playCards.end() == std::find(playCards.begin(), playCards.end(), Card(0x33))) {
				JSONPacket msg;
				msg.Create(P_SERVER_BET_ILLEGAL);
				msg.AddInfo("msg", "首局必须先出黑桃3");
				pUser->SendMsg(&msg);
				logErr("首局必须先出黑桃,uid:%u", pUser->GetID());
				return;
			}
		}
	}

	//非最后一手先出.可以带牌的牌型必须带牌.
	////////////////////////////////////
	bool checkStatus = CheckPlayCard(pChair, play_type, playCards, playcard_stat);
	if (!checkStatus) {
		JSONPacket msg;
		msg.Create(P_SERVER_BET_ILLEGAL);
		msg.AddInfo("msg", "非最后一手必须带牌");
		pUser->SendMsg(&msg);
		return;
	}
	playcard_ana.type = play_type;
	/////////////////////////////////////
	if (!_new_round) {
		if (play_type != _last_playType && play_type != CARD_TYPE_BOMB) {
			logErr("客户端打出非法牌型,上次牌型:%d,打出牌型:%d,uid:%u", _last_playType, play_type, pUser->GetID());
			JSONPacket msg;
			msg.Create(P_SERVER_BET_ILLEGAL);
			msg.AddInfo("msg", "请打出同上家一样的牌型");
			pUser->SendMsg(&msg);
			return;
		}
		if (CARD_TYPE_BOMB != play_type && playCards.size() != _last_playCards.size()) {
			logErr("客户端打出的牌型长度不一致.");
			JSONPacket msg;
			msg.Create(P_SERVER_BET_ILLEGAL);
			msg.AddInfo("msg", "请打出同上家一样的牌型");
			pUser->SendMsg(&msg);
			return;
		}
		if (_last_card_ana.face >= playcard_ana.face) {
			if (play_type != CARD_TYPE_BOMB || (play_type == _last_playType)) {
				std::string cardsInfo1;
				std::string cardsInfo2;
				Card::dump_cards(_last_playCards, cardsInfo1);
				Card::dump_cards(playCards, cardsInfo2);
				logErr("客户端打出的牌不能大过上家出的牌,uid:%u,上家:%s,出牌:%s.",
					pUser->GetID(), cardsInfo1.c_str(), cardsInfo2.c_str());

				JSONPacket msg;
				msg.Create(P_SERVER_BET_SMALL);
				msg.AddInfo("msg", "必须打出大得过上家的牌");
				pUser->SendMsg(&msg);
				return;
			}
		}
	}
	//step 3:单牌判断是否放牌
	if (play_type == CARD_TYPE_ONE) {
		TableChair* pNextChair = GetNextChair(_curr_bet);
		if (pNextChair->isSingle) {
			//下家是报单状态.是否打的最大的牌.
			PokerCards cards;
			pChair->holeCards.copy_cards(&cards);
			Card::sort_by_ascending(cards);
			Card card_t = cards.back();
			if (card_t.face != playCards[0].face) {
				JSONPacket msg;
				msg.Create(P_SERVER_BET_ILLEGAL);
				msg.AddInfo("msg", "下家报单了,必须出最大的牌!");
				pUser->SendMsg(&msg);
				return;
			}
		}
	}
#ifdef TRUSTEE
	_timer_trustee.Stop();
#endif
	CallPlayCard(pChair, playCards, play_type, playcard_stat, playcard_ana);
}

void GameTable::UserPass(TableChair* pChair, bool checked /*= false*/)
{
	if (!checked) {
		if (_new_round) {
			logErr("玩家本轮第1个出牌,客户端上传了pass操作.uid:%u", pChair->idUser);
			JSONPacket msg;
			msg.Create(P_SERVER_BET_MUSTCALL);
			msg.AddInfo("msg", "错误,本轮你先出牌");
			pChair->pUser->SendMsg(&msg);
			return;
		}
		//判断是否过牌.
		CardFind card_find;
		PokerCards	cards_v;
		pChair->holeCards.copy_cards(&cards_v);
		card_find.tip(_last_playCards, cards_v, _gameType, _fourdaithree, _threeAbomb);
		if (card_find.results.size() > 0) {

			AutoPlayCard(pChair, true);
			JSONPacket msg;
			msg.Create(P_SERVER_SYNC_CARDS);
			cardHelper::to_json_array(pChair->holeCards.cards, &msg, "cards");
			pChair->pUser->SendMsg(&msg);
			return;
		}
	}

#ifdef TRUSTEE
	_timer_trustee.Stop();
#endif

	_pass_times++;
	_bet_times++;
	pChair->isPass = true;

	//=============回放数据===============
	Json::Value& record = _record["records"][_bet_times - 1];
	record["seatid"] = pChair->idChair;
	record["action"] = 0;
	record["cards"].append(0);
	record["type"] = 0;
	record["single"] = pChair->isSingle ? 1 : 0;
	//====================================

	JSONPacket msg;
	msg.Create(P_SERVER_BET_SUCC_BC);
	msg.AddInfo("action", ACTION_PASS);
	msg.AddInfo("uid", pChair->idUser);
	msg.AddInfo("seatid", pChair->idChair);
	msg.AddInfo("n_seatid", _next_bet);
	msg.AddInfo("end_round", (_pass_times >= _Game_Players - 1) ? 1 : 0);
	msg.AddInfo("is_betting", 1);
	Broadcast(&msg);

	if (_pass_times >= _Game_Players - 1) {
		_new_round = true;
		_pass_times = 0;

		if (CARD_TYPE_BOMB == _last_playType) {
			PlayBomb();
		}

		//最后出的牌清空
		for (int i = 0; i < _Game_Players; ++i) {
			TableChair* pChair = &_ChairInfo[i];
			pChair->last_playCards.clear();
			pChair->isPass = false;
		}

		_last_playCards.clear();
		_last_playType = CARD_TYPE_ERROR;
		_last_card_stat.clear();

		msg.Create(P_SERVER_NEXT_BET_BC);
		msg.AddInfo("seatid", _next_bet);
		Broadcast(&msg);

	}
	else {
		pChair->last_playCards.clear();
	}
	UpdateCallInfo(_next_bet);
}

bool GameTable::CheckPlayCard(TableChair* pChair, int& cardType, const PokerCards& cards, CardStatistics& card_stat)
{

	int cardNum = pChair->holeCards.size();

	//不允许出三顺
	if (CARD_TYPE_THREELINE == cardType) {
		//三顺当成飞机打.
		bool isAll = (int)cards.size() == cardNum;
		cardNum = card_stat.card3.size();

		if (cardNum % 5 == 0) {
			//333 444 555 66 67 77 //合法的飞机带翅膀.
			cardType = CARD_TYPE_PLANEWITHWING;
			return true;
		}
		else if (isAll) {
			//最后一手,无所谓了
			cardType = CARD_TYPE_PLANWHITHLACK;
			return true;
		}
		return false;
	}


	//选择了可4带3
	if (1 == _fourdaithree)
	{
		if ((cardType == CARD_TYPE_FOURWITHTWO || cardType == CARD_TYPE_FOURWITHONE)
			&& cards.size() == cardNum)
		{
			return true;
		}
	}


	//打出的牌数和手中剩余的牌数相等,是最后一手牌.
	if ((int)cards.size() == cardNum) return true;

	//不是最后一手,带牌的牌型判断.
	OBJECT_ID idUser = pChair->pUser->GetID();

	std::string cardsInfo;
	switch (cardType) {
	case CARD_TYPE_THREE:			//三张
	case CARD_TYPE_THREEWITHONE:	//三带一
	case CARD_TYPE_FOURWITHONE:		//四带一
	case CARD_TYPE_FOURWITHTWO:		//四带二
	case CARD_TYPE_PLANEWITHONE:	//飞机带羿,带一
	case CARD_TYPE_PLANWHITHLACK:	//飞机带翼,少牌
		Card::dump_cards(cards, cardsInfo);
		logErr("客户端非最后一手,打出的牌不符合规则.uid:%u,牌型:%d,牌:%s", idUser, cardType, cardsInfo.c_str());
		return false;
	}
	return	true;
}

/*出牌,调用该过程,上层必须检测当前座位的玩家手中是否有这些牌.
pChair		座位信息
cards		出牌数据
cardType	牌型,避免多次分析直接由上层传过来,
isTimeout	是否超时出牌.
*/
void GameTable::CallPlayCard(TableChair* pChair, PokerCards& cards, int cardType, CardStatistics& card_stat, CardAnalysis card_ana)
{
	_bet_times++;
	_pass_times = 0;//出牌后pass次数清0,
	pChair->isPass = false;

	pChair->holeCards.remove(cards);
	pChair->last_playCards = cards;
	_last_playCards = cards;
	_last_playType = cardType;
	_last_card_stat.swap(card_stat);
	_last_card_ana = card_ana;

	_new_round = false;

	int cardNum = pChair->holeCards.size();
	if (!cardNum) {
		/*结束时,处理炸弹.*/
		if (CARD_TYPE_BOMB == cardType) {
			//处理炸弹过程，是根据俩次pass，下一次出牌的就是出炸弹的那个处理的。
			//把下次出牌的更改.
			_next_bet = pChair->idChair;
			PlayBomb();
		}
		_status = TSS_ENDGAME;
	}
	else if (cardNum == 1) {
		PlaySingle(pChair);
	}

	JSONPacket msg;
	msg.Create(P_SERVER_BET_SUCC_BC);
	msg.AddInfo("uid", pChair->idUser);
	msg.AddInfo("seatid", pChair->idChair);
	msg.AddInfo("money", pChair->pUser->GetMoney());
	msg.AddInfo("action", ACTION_PLAY);
	msg.AddInfo("n_seatid", _next_bet);
	msg.AddInfo("cardtype", cardType);
	msg.AddInfo("is_betting", TSS_BETTING == _status ? 1 : 0);
	//如果勾选了显示剩余牌数功能，则发送cardnum给客户端
	if (_cardNum == 1)
	{
		msg.AddInfo("handCardNum", cardNum);
	}
	cardHelper::to_json_array(cards, &msg, "cards");
	Broadcast(&msg);
	logDebug("[GameTable::%s] msg:%s", __FUNCTION__, msg.dumpInfo());

	//=============回放数据===============
	Json::Value& record = _record["records"][_bet_times - 1];
	record["seatid"] = pChair->idChair;
	record["action"] = 1;
	cardHelper::to_json_array(cards, record, "cards");
	record["type"] = _last_playType;
	record["single"] = (cardNum == 1) ? 1 : 0;
	//====================================

	if (TSS_BETTING == _status) {
		//让下一个出牌.
		UpdateCallInfo(_next_bet);
	}
	else if (TSS_ENDGAME == _status) {
		GameEnd();
	}
}

void GameTable::processLose(const TableChair* pChair, MONEY loseMoney)
{
	proUpdateScore(pChair, loseMoney, 1);
}

void GameTable::processWin(const TableChair* pChair, MONEY winMoney)
{
	proUpdateScore(pChair, winMoney, 0);
}

void GameTable::proUpdateScore(const TableChair* pChair, MONEY score, int type)
{
	ASSERT_V(pChair);
	if (0 != type) score = -1 * score; //type非0值输了游戏.

	score += pChair->bombScore; //炸弹不是现结了.

	Player* pUser = pChair->pUser;

	//战绩更新.
	int awardExp = 0;

	if (0 != type) {
		awardExp = _info.loseExp;
	}
	else {
		awardExp = _info.winExp;
	}

	int money = score;
	// 更新用户金币
	pUser->IncrMoney(money, _idTable, BALANCE);
	//平台数据更新
	pUser->inc_playTimes();
	//pUser->AddExp(awardExp);

}

void GameTable::SaveBankerInfo(void)
{
	FunctionTTL ttl(5, "GameTable::SaveBankerInfo");
	int times = 0;
	bool result;

	//保存庄家信息.只在游戏结束时保存一次.
	do
	{
		result = DB_DATA->execute("hmset %s_%d_%u status %d banker %d gameTimes %d", _DATA_NAME, _pRoom->GetID(), _idTable, _status, _banker_seatid, _gameTimes);
		if (result) break;
	} while (++times < 2);
	ASSERT_V(result && "保存庄家信息");
}

void GameTable::SaveGameInfo(void)
{
	FunctionTTL ttl(5, "GameTable::SaveGameInfo");
	int times = 0;
	bool result;

	//俩人玩法,特别处理,写入多出的那一手牌
	if (_Game_Players == 2) {
		Json::Value jsCards;
		cardHelper::to_json_array(_deck.cards, jsCards, "cards");
		std::string strCards(jsCards.toStyledString());
		times = 0;
		do
		{
			result = DB_DATA->execute("hmset %s_%d_%u cards %s",
			 	_DATA_NAME, _pRoom->GetID(), _idTable, strCards.c_str());
			if (result) break;
		} while (++times < 2);
		ASSERT_V(result && "保存牌信息");
	}

	//保存游戏信息.只在发牌时保存一次.
	times = 0;
	do
	{
		result = DB_DATA->execute("hmset %s_%d_%u status %d banker %d gameTimes %d", _DATA_NAME, _pRoom->GetID(), _idTable, _status, _banker_seatid, _gameTimes);
		if (result) break;
	} while (++times < 2);
	ASSERT_V(result && "保存牌局信息");

	for (int i = 0; i < _Game_Users; ++i) {
		const TableChair* pChair = &_ChairInfo[i];
		if(!pChair->IsOccupied()) continue;
		SaveChairInfo(&_ChairInfo[i]);
	}

	//座位信息保存
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		const std::string& strInfo = pChair->Serializable();
		const char* chairInfo = strInfo.c_str();
		times = 0;
		do
		{
			result = DB_DATA->execute("hmset %s_%d_%u chair%d %s", _DATA_NAME, _pRoom->GetID(), _idTable, i, chairInfo);
			if (result) break;
		} while (++times < 2);
		ASSERT_V(result && "保存座位");
	}
}

void GameTable::SaveChairInfo(const TableChair* pChair)
{
	FunctionTTL ttl(5, "GameTable::SaveChairInfo");
	if (!pChair->IsOccupied()) return;

	const std::string& strInfo = pChair->Serializable();
	const char* chairInfo = strInfo.c_str();
	int times = 0;
	bool result = false;
	do
	{
		result = DB_DATA->execute("hset %s_%d_%u chair%d %s", _DATA_NAME, _pRoom->GetID(), _idTable, pChair->idChair, chairInfo);
		if (result) break;
	} while (++times < 2);
	ASSERT_V(result && "保存座位信息");
#ifdef _DEBUG
	logDebug("保存座位idTable(%d)%d信息.\n%s", _idTable, pChair->idChair, chairInfo);
#endif
}

void GameTable::SaveCallInfo(void)
{
	//牌局回放数据.
	int times = 0;
	bool result = false;

	//保存出牌数据.更新出牌信息时保存一次.
	TableChair* pChair = &_ChairInfo[_prev_bet];
	const std::string& strCall = pChair->Serializable();
	const char* callInfo = strCall.c_str();
	Json::Value jsonCards;
	cardHelper::to_json_array(_last_playCards, jsonCards, "lastCards");
	std::string lastCards(jsonCards.toStyledString());

	times = 0;
	do
	{
		//保存当前牌局数据.
		//保存上个出牌的座位数据.
		//保存最后出牌信息.
		result = DB_DATA->execute("hmset %s_%d_%u curr_bet %d betTimes %d passTimes %d newRound %d chair%d %s lastCards %s",
			_DATA_NAME, _pRoom->GetID(), _idTable, _curr_bet, _bet_times, _pass_times, _new_round ? 1 : 0, pChair->idChair, callInfo, lastCards.c_str());
		if (result) break;
	} while (++times < 2);
#ifdef _DEBUG
	logDebug("保存出牌信息.上个出牌的座位[%d]信息:\n%s\n最后出牌数据:\n%s", _prev_bet, callInfo, lastCards.c_str());
#endif
	ASSERT_V(result && "保存出牌数据");
}

void GameTable::SaveChairInfo(void)
{
	int times = 0;
	bool result = false;
	Json::Value jsonCards;
	cardHelper::to_json_array(_last_playCards, jsonCards, "lastCards");
	std::string lastCards(jsonCards.toStyledString());
	do
	{
		//保存当前牌局数据.
		//保存上个出牌的座位数据.
		//保存最后出牌信息.
		result = DB_DATA->execute("hmset %s_%d_%u curr_bet %d betTimes %d passTimes %d newRound %d lastCards %s",
			_DATA_NAME, _pRoom->GetID(), _idTable, _curr_bet, _bet_times, _pass_times, _new_round ? 1 : 0, lastCards.c_str());
		if (result) break;
	} while (++times < 2);
	ASSERT_V(result && "保存所有座位信息");
	//座位信息保存
	for (int i = 0; i < _Game_Players; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		const std::string& strInfo = pChair->Serializable();
		const char* chairInfo = strInfo.c_str();
		times = 0;
		result = false;
		do
		{
			result = DB_DATA->execute("hmset %s_%d_%u chair%d %s", _DATA_NAME, _pRoom->GetID(), _idTable, i, chairInfo);
			if (result) break;
		} while (++times < 2);
		ASSERT_V(result && "保存座位");
	}
}

void GameTable::DelChairInfo(const TableChair* pChair)
{
	int times = 0;
	bool result = false;
	do
	{
		result = DB_DATA->execute("hdel %s_%d_%u chair%d", _DATA_NAME, _pRoom->GetID(), _idTable, pChair->idChair);
		if (result) break;
	} while (++times < 2);
	ASSERT_V(result && "删除座位信息");
#ifdef _DEBUG
	logDebug("删除座位%d[%d]信息.\n", _idTable, pChair->idChair);
#endif
}

void GameTable::DeleteInfo(void)
{
	int times = 0;
	bool result = false;
	do
	{
		result = DB_DATA->execute("del %s_%d_%u", _DATA_NAME, _pRoom->GetID(), _idTable);
		if (result) break;
	} while (++times < 2);
	ASSERT_V(result && "删除房间数据");
}

bool GameTable::LoadInfo()
{
	IRecord* record = DB_DATA->query("hgetall %s_%d_%u", _DATA_NAME, _pRoom->GetID(), _idTable);
	
	ASSERT_B(record && "加载桌子数据");
	IRecordPtr row(record);
	if (!row || row->GetRows() <= 0)
		return false;

	_status = row->GetInt("status");
	_banker_seatid = row->GetInt("banker");
	_gameTimes = row->GetInt("gameTimes");

#ifdef _DEBUG
	logDebug("加载房间(%u)数据 status:%d banker:%d.", _idTable, _status, _banker_seatid);
#endif

	for (int i = 0; i < _Game_Users; ++i) {
		TableChair* pChair = &_ChairInfo[i];
		char buf[16];
		snprintf(buf, 16, "chair%d", pChair->idChair);
		const std::string& strInfo = row->GetString(buf);
#ifdef _DEBUG
		logDebug("读取[%s]数据.%s", buf ,strInfo.c_str());
#endif
		if (!strInfo.empty()) {
			if (pChair->Unserializable(strInfo)) {
                pChair->pUser = GAMESERVER->CreateUser(pChair->idUser, GameVid, _idTable);
				++_amountUser;
			}
			else {
				logErr("反序列化座位信息失败.桌子id:%u\n%s", _idTable, strInfo.c_str());
			}
		}
	}

	if (TSS_BETTING == _status) {
		//俩人玩法,特别处理,多出的那一手牌
		if (_Game_Players == 2) {
			_deck.cards.clear();
			const std::string& strCards = row->GetString("cards");
			if (!strCards.empty()) {
				Json::Reader reader;
				Json::Value  jsCards;
				if (reader.parse(strCards, jsCards)) {
					cardHelper::to_vector(jsCards, "cards", _deck.cards);
				}
				else {
					logErr("跑得快二人玩法,多出的一手牌,解析失败.idTable:%u,%u\n%s", _idTable, GameVid, strCards.c_str());
				}
			}
			else {
				logErr("跑得快二人玩法,多出的那一手牌,恢复错误.idTable:%u,%u", _idTable, GameVid);
			}
		}

		_last_playCards.clear();

		_birdSeatid = row->GetInt("birdSeatid");
		//没有勾选红桃10扎鸟的情况下 _birdSeatid一定要设置为-1
		if (!_birdCard)
		{
			_birdSeatid = -1;
		}
		_curr_bet = row->GetInt("curr_bet");
		_bet_times = row->GetInt("betTimes");
		_pass_times = row->GetInt("passTimes");
		_new_round = row->GetInt("newRound") ? true : false;
		const std::string& lastCards = row->GetString("lastCards");
		Json::Value jsonCards;
		Json::Reader reader;
		if (!reader.parse(lastCards, jsonCards)) {
			logErr("解析最后出牌信息失败.\n%s", lastCards.c_str());
			//解析失败.根据pass次数算牌.
			if (_pass_times == 0 || _pass_times >= (_Game_Players - 1)) {
				_pass_times = 0;
				_new_round = true;
				for (int i = 0; i < _Game_Players; ++i) {
					_ChairInfo[i].last_playCards.clear();
				}
			}
			else if (_pass_times == (_Game_Players - 2)) {
				int temp = _curr_bet -= 2;
				if (temp < 0) temp += _Game_Players;
				_last_playCards = _ChairInfo[temp].last_playCards;
			}
			else {
				_new_round = true;
				_pass_times = 0;
			}
		}
		else {
			cardHelper::to_vector(jsonCards, "lastCards", _last_playCards);
		}
		if (_last_playCards.size() > 0) {
			_last_card_stat.statistics(_last_playCards);
			_last_card_ana.analysis(_last_card_stat, _gameType, _fourdaithree, _threeAbomb);
			_last_playType = _last_card_ana.type;
		}
		UpdateCallInfo(_curr_bet, false);
	}

	if (_status == TSS_WAITING || _status == TSS_ENDGAME) 
	{
        _timer_ready.Reset();
    }

	if (TSS_START == _status)
	{
		_status = TSS_WAITING;
	}
	return true;
}

void GameTable::AutoPlayCard(TableChair* pChair, bool isTimeout)
{
	ASSERT_V(pChair && pChair->IsOccupied());

	if (_new_round) {
		//第1个出牌.
		if (0 == _bet_times && _firstCard && _Game_Players == 3)
		{
			//托管时，首局必须黑桃3
			//第1个出牌.
			PokerCards cards;
			CardStatistics playcard_stat;
			CardAnalysis playcard_ana;
			int play_type = CARD_TYPE_ONE;

			Card card_t = Card(0x33);
			cards.clear();
			cards.push_back(card_t);
			playcard_stat.statistics(cards);
			playcard_ana.analysis(playcard_stat, _gameType, _fourdaithree, _threeAbomb);
			CallPlayCard(pChair, cards, play_type, playcard_stat, playcard_ana);
		}
		else
		{
			PokerCards cards;
			pChair->holeCards.robot(cards);
			CardStatistics playcard_stat;
			playcard_stat.statistics(cards);
			CardAnalysis playcard_ana;
			playcard_ana.analysis(playcard_stat, _gameType, _fourdaithree, _threeAbomb);
			int play_type = playcard_ana.type;
			/*自动出单牌时要判断下家是不是报单*/
			if (play_type == CARD_TYPE_ONE) {
				TableChair* pNextChair = GetNextChair(pChair->idChair);
				if (pNextChair->isSingle) {
					//下家是报单状态.打最大的牌.
					cards.clear();
					pChair->holeCards.copy_cards(&cards);
					Card::sort_by_ascending(cards);
					Card card_t = cards.back();
					cards.clear();
					cards.push_back(card_t);
					playcard_stat.statistics(cards);
					playcard_ana.analysis(playcard_stat, _gameType, _fourdaithree, _threeAbomb);
				}
			}
			CallPlayCard(pChair, cards, play_type, playcard_stat, playcard_ana);
		}
	}
	else {
		PokerCards cards;
		pChair->holeCards.copy_cards(&cards);
		if (_last_playType == CARD_TYPE_ONE) {
			TableChair* pNextChair = GetNextChair(pChair->idChair);
			if (pNextChair->isSingle) {
				//下家是报单状态.打最大的牌.
				Card::sort_by_ascending(cards);
				Card card_t = cards.back();
				if (card_t.face > _last_playCards[0].face) {
					cards.clear();
					cards.push_back(card_t);
					CardStatistics playcard_stat;
					playcard_stat.statistics(cards);
					CardAnalysis playcard_ana;
					playcard_ana.analysis(playcard_stat, _gameType, _fourdaithree, _threeAbomb);
					CallPlayCard(pChair, cards, _last_playType, playcard_stat, playcard_ana);
					return;
				}
			}
		}
		CardFind card_find;
		CardStatistics my_card_stat;
		my_card_stat.statistics(cards);
		card_find.find(_last_card_ana, _last_card_stat, my_card_stat);
		if (card_find.results.size() > 0) {
			PokerCards playCards = *card_find.results.begin();
			CardStatistics playcard_stat;
			playcard_stat.statistics(playCards);
			CardAnalysis playcard_ana;
			playcard_ana.analysis(playcard_stat, _gameType, _fourdaithree, _threeAbomb);
			int play_type = playcard_ana.type;
			CallPlayCard(pChair, playCards, play_type, playcard_stat, playcard_ana);
		}
		else {
			UserPass(pChair, true);
		}
	}
}

bool GameTable::AutoPass()
{
	if (_new_round ) return false;
	
	TableChair* pChair = &_ChairInfo[_curr_bet];

	PokerCards cards;
	pChair->holeCards.copy_cards(&cards);
	
	CardFind card_find;
	card_find.tip(_last_playCards, cards, _gameType, _fourdaithree, _threeAbomb);
	if (card_find.results.size() > 0) {
		return false;
	}
	
	// 自动pass
	UserPass(pChair, true);

	return true;
}

void GameTable::UpdateStatus(int status)
{
	if (_pRoom->GetRoomType() != 1) return;

	DB_ROOM->execute("hset htid:%d gamestate %d", _idTable, status);
}

void GameTable::MarkTableUsers(int uid, int seatid, bool insert)
{
	if (_pRoom->GetRoomType() != 1) return;

	std::ostringstream oss;
	oss << "uid" << seatid + 1;
	if (insert)
		DB_ROOM->execute("hset htid:%d %s %d", _idTable, oss.str().data(), uid);
	else
		DB_ROOM->execute("hdel htid:%d %s", _idTable, oss.str().data());
}

#ifdef _ROBOT

void GameTable::OnTimer_robot(void)
{
	char name[64];
	for (int i = _amountUser; i < _Game_Users; ++i) {
		Player* pUser = Player::GetRobot();
		sprintf(name, "robot%d", i);
		pUser->setName(name);
		if (!Sitdown(pUser)) {
			break;
		}
		handle_ready(pUser);
	}
}

void GameTable::OnTimer_call(void)
{
	TableChair* pChair = &_ChairInfo[_curr_bet];
	if (pChair->IsUser()) return;

	AutoPlayCard(pChair, false);

	{
		JSONPacket msg;
		msg.Create(P_SERVER_TIMEOUT_NOTIFY_BC);
		Broadcast(&msg);
	}
}

#endif
