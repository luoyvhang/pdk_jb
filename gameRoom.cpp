#include "gameRoom.h"
#include "player.h"
#include "gameServer.h"
#include <algorithm>
#include <iostream>

const char*		_GAMENAME = "yb_pdk";
const char*     _SYS_TID = "pdk_sys_tid";

GameRoom::GameRoom()
{
	_idRoom = OBJECT_ID_UNDEFINED;
}

GameRoom::~GameRoom()
{
	Release();
}

bool GameRoom::Init(OBJECT_ID idRoom, int base_money, int enter_money, int leave_money, int bomb_money, int fee, int room_type, int game_type, int game_rule, int player, int showcardnum, int fourdaithree, int threeAbomb)
{
	_idRoom = idRoom;
	// 公共参数
	_baseMoney = base_money;
	_enterMoney = enter_money;
	_leaveMoney = leave_money;
	_bombMoney = bomb_money;
    _fee = fee;

	// 游戏参数
	_room_type = room_type;
	_game_type = game_type;
	_game_rule = game_rule;
	_player = player;
	_showCardNum = showcardnum;
	_fourdaithree = fourdaithree;
	_threeAbomb = threeAbomb;

	if (GetRoomType() == 1)
	{
		TIMER_CALLBACK cbDismiss = CXX11::bind(&GameRoom::OnTimer_dismiss, this);
		_timer_dismiss.CreateTimer(10, 10, cbDismiss, true);
	}

	return true;
}

bool GameRoom::BroadcastRoomMsg(const char* bufMsg, int length)
{
	ASSERT_B(bufMsg);
	const std::string& packet = JSONPacket::formatPacket(bufMsg, length);
	for (MAP_GAME_PLAYER::iterator it = _setPlayer.begin(); it != _setPlayer.end(); ++it)
	{
		Player* pUser = it->second;
		if(!pUser || !pUser->IsOnline()) continue;
		pUser->SendMsg(packet);
	}
	return true;
}

bool GameRoom::BroadcastRoomMsg(const JSONPacket& packet)
{
	const std::string& packMsg = packet.GetPacket();
	for (MAP_GAME_PLAYER::iterator it = _setPlayer.begin(); it != _setPlayer.end(); ++it) {
		Player* pUser = it->second;
		if (!pUser || !pUser->IsOnline()) continue;
		pUser->SendMsg(packMsg);
	}
	return true;
}

bool GameRoom::Logout(Player* pUser)
{
    ASSERT_B(pUser);
    GameTable* pTable = pUser->GetTable();
    if (pTable) {
        if (!pTable->Standup(pUser)) {
#ifdef _DEBUG
            logDebug("玩家[%s]在游戏状态中离线.", pUser->GetName());
#endif
            /*加到离线表.*/
            OBJECT_ID idUser = pUser->GetID();
            _setPlayer.erase(idUser);
            _setOffline.insert(std::make_pair(idUser, pUser));

            pTable->Offline(pUser);
            return false;
        }
    }
    else
    {
		logDebug("玩家[%s]不在任何房间中");
    }
    return true;
}

bool GameRoom::Offline(Player* pUser)
{
	ASSERT_B(pUser);
	GameTable* pTable = pUser->GetTable();
	if (pTable) {
		//if (!pTable->Standup(pUser)) {
#ifdef _DEBUG
			logDebug("玩家[%s]在游戏状态中离线.", pUser->GetName());
#endif
			/*加到离线表.*/
			OBJECT_ID idUser = pUser->GetID();
			_setPlayer.erase(idUser);
			_setOffline.insert(std::make_pair(idUser, pUser));

			pTable->Offline(pUser);
			return false;
		//}
	}
	return true;
}

/*
 * idTable 0:表示系统匹配  >0:进指定房间
 * exceptTid 当idTable为 0时 该参数有效
 */
bool GameRoom::EnterRoom(Player* pUser, OBJECT_ID idTable, OBJECT_ID exceptTid, int players)
{
	ASSERT_B(pUser);

	OBJECT_ID idUser = pUser->GetID();
#ifdef _DEBUG
	logDebug("玩家[%s]请求进入房间.idTable:%u", pUser->GetName(),idTable);
#endif
	//备注:离线表,会在游戏结束的时候处理.
	MAP_GAME_PLAYER::iterator it = _setOffline.find(idUser);
	if (it != _setOffline.end()) {
		MAP_USER_TABLE::iterator itGame = _setUserTable.find(idUser);
		if (itGame != _setUserTable.end()) {
#ifdef _DEBUG
            logDebug("玩家[%s]重新进入上次离线时的房间.idTable:%u", pUser->GetName(),itGame->second->GetID());
#endif
            if (!pUser->JoinTable(itGame->second))
                return false;

            if (pUser->IsOnline()) {
                _setOffline.erase(it);
                _setPlayer.insert(std::make_pair(idUser, pUser));
            }
            pUser->SetLastRoom(_idRoom, itGame->second->GetID(), GamePort);
            return true;

		}else{
			logDebug("离线重登录时,未发现关联的游戏房间.");
		}
		_setOffline.erase(it);
	}

	MAP_GAME_PLAYER::iterator iter = _setPlayer.find(idUser);
	if (iter != _setPlayer.end()) {
		logDebug("重复登录的玩家为什么没有被踢下线?");
		iter->second->OnLeaveRoom();
		_setPlayer.erase(idUser);
	}

    //if (pUser->GetMoney() < GetEnterMoney()) {
    //    GameTable::EnterResult(pUser, ENTER_MONEY_LESS);
    //    return false;
    //}
	
	//游戏桌子.
	GameTable* pTable = FindTable(idTable, idUser, exceptTid, players);
	if (!pTable) {
		ASSERT_B(pUser->IsOnline() && "创建一个离线玩家进入到不存在的房间?");
		GameTable::EnterResult(pUser, ENTER_FOUND);
		return false;
	}
	else
	{
		if (!pTable->ExistUser(pUser) && pUser->GetMoney() < pTable->GetEnterMoney() )
		{
			GameTable::EnterResult(pUser, ENTER_MONEY_LESS);
			return false;
		}
	}

	if (!pUser->JoinTable(pTable))
		return false;
	_setUserTable.insert(std::make_pair(idUser, pTable));

	if (pUser->IsOnline()) {
		_setPlayer.insert(std::make_pair(idUser, pUser));
	}
	else {
		_setOffline.insert(std::make_pair(idUser, pUser));
	}
	pUser->SetLastRoom(_idRoom, pTable->GetID(), GamePort);
#ifdef _DEBUG
	logDebug("玩家[%s]进入房间[%u]成功.vid:%d", pUser->GetName(), pTable->GetID(), _idRoom);
#endif
    //GameTable::EnterResult(pUser, ENTER_OK);
	return true;
}

bool GameRoom::LeaveRoom(Player* pUser)
{
	ASSERT_B(pUser);

	OBJECT_ID idUser = pUser->GetID();
	_setPlayer.erase(idUser);
	_setOffline.erase(idUser);
	_setUserTable.erase(idUser);

	pUser->OnLeaveRoom();
	return true;
}

void GameRoom::OnLeaveTable(Player* pUser, bool clearRoom)
{
	ASSERT_V(pUser);
	pUser->OnLeaveTable();
	if (clearRoom) pUser->SetLastRoom();
	LeaveRoom(pUser);
#ifdef _DEBUG
	logDebug("玩家[%s]退出房间正常.", pUser->GetName());
#endif
}

bool GameRoom::WriteGameLog(const char* log)
{
	int times = 0;
	bool result;
	do
	{
		result = DB_ROOM->execute("LPUSH %s_list %s", _GAMENAME, log);
		if (result) break;
	} while (++times < 2);
	if (!result) logMsg(log);
	return result;
}

void GameRoom::Release(void)
{
	ReleaseTable();
	_idRoom = OBJECT_ID_UNDEFINED;
	_setPlayer.clear();
	_setOffline.clear();
	_setUserTable.clear();
}

void GameRoom::ReleaseTable(void)
{
	for (MAP_GAME_TABLE::iterator it = _setTable.begin(); it != _setTable.end(); ++it){
		GameTable* pTable = it->second;
		if (pTable) pTable->Release();
	}
	_setTable.clear();
}

int GameRoom::GenerateTid(void)
{
    int value = 0;
	IRecord* record = DB_DATA->query("hincrby %s_%u tid 1", _SYS_TID, _idRoom);
	IRecordPtr row(record);
	if (row) {
		value = (int)row->GetLlong();
	}
    return value;
}

/*
 * idTable 0:表示需要系统随机匹配  >0:表示指定房间进入
 * exceptTid if(exceptTid>0)idTable一定为0(即系统匹配) 反之idTable>0则execpetTid参数无效
 */
GameTable* GameRoom::FindTable(OBJECT_ID idTable, OBJECT_ID idUser, OBJECT_ID exceptTid/*=0*/, int players)
{
    GameTable* pTable = NULL;
	if(GetRoomType() == 0) //系统匹配
	{
    	if (idTable > 0) 
		{
        MAP_GAME_TABLE::const_iterator iter = _setTable.find(idTable);
        if (iter != _setTable.end()) {
            return iter->second;
        }

        //如果没找到，重新创建
        pTable = GameTable::CreateNew();
        if (!pTable) return NULL;

        if (pTable->Init(this, idTable, 0, _baseMoney, _enterMoney, _leaveMoney, _fee, _game_type, _game_rule, players, _showCardNum, _fourdaithree, _threeAbomb)) 
		{
            //加入列表.
            _setTable.insert(std::make_pair(idTable, pTable));
            return pTable;
        }

	Player::resetLastRoom(idUser);
        logDebug("id:%u进入指定非法房间idTable:%u", idUser, idTable);
        pTable->Release();
        pTable = NULL;
		}
		else 
		{
			//先暂时最蠢的轮询实现
			for (MAP_GAME_TABLE::const_iterator cit = _setTable.begin(); cit != _setTable.end(); ++cit) {
				if (exceptTid == cit->first) continue;
				
				if (cit->second->IsFull()) continue;

				if (cit->second->GetPlayerSize() != players) continue;

				if (cit->second->GetStatus() != TSS_WAITING && cit->second->GetStatus() != TSS_ENDGAME) continue;

				pTable = cit->second;
			}

			//没有空余的房间则新生成一个
			if (!pTable) 
			{
				pTable = GameTable::CreateNew();
				if (pTable)
				{
					//生成唯一ID
					int tid = GenerateTid();
					if (tid > 0) {
						pTable->Init(this, tid, 0, _baseMoney, _enterMoney, _leaveMoney, _fee, _game_type, _game_rule, players, _showCardNum, _fourdaithree, _threeAbomb);
						//加入列表.
						_setTable.insert(std::make_pair(tid, pTable));
					}
					else {
						pTable->Release();
						pTable = NULL;
					}
				}
			}
		}
	}
	else	// 好友房
	{
		MAP_GAME_TABLE::const_iterator iter = _setTable.find(idTable);
		if (iter != _setTable.end())
			return iter->second;
		
#ifdef _DEBUG
		logDebug("从数据库获取房间信息.id:%u", idTable);
#endif
		IRecord* record = DB_ROOM->query("hgetall htid:%u", idTable);
		if (!record || 0 == record->GetRows()) return NULL;
#ifdef _DEBUG
	logDebug("房间属性读取成功.id:%u", idTable);
#endif
		IRecordPtr row(record);
		int gameType = row->GetInt("game_Type");
		int gameRule = row->GetInt("game_rule");
		int player = row->GetInt("player");
		int cardNum = row->GetInt("cardNum");
		int baseMoney = row->GetInt("base_money");
		int enterMoney = row->GetInt("enter_money");
		int leaveMoney = row->GetInt("leave_money");
		int fee = row->GetInt("fee");
		OBJECT_ID idDisp = row->GetInt("room_id");

		int fourdaithree = row->GetInt("fourdaithree");
		int threeAbomb = row->GetInt("threeAbomb");//3个A算炸

		GameTable* pTable = GameTable::CreateNew();
		if(!pTable) return NULL;
		pTable->Init(this, idTable, idDisp, baseMoney, enterMoney, leaveMoney, fee, gameType, gameRule, player, cardNum, fourdaithree, threeAbomb);
		
        //加入列表.
        _setTable.insert(std::make_pair(idTable, pTable));
        return pTable;
	}
	return pTable;
}

/*
 * 换桌子流程 从当前桌子退出->找到新桌子->玩家进新桌子
 *
 */
bool GameRoom::ChangeTable(Player* pUser)
{
	if (GetRoomType() == 1) return false;	// 好有房是不能换桌的
	
    ASSERT_P(pUser);
    OBJECT_ID idUser = pUser->GetID();
    MAP_USER_TABLE::iterator it = _setUserTable.find(idUser);
    if (it == _setUserTable.end()) {
        logDebug("玩家id(%u)请求换桌发现玩家根本不在房间里", idUser);
        return false;
    }

	GameTable* pOrgTable = it->second;

    JSONPacket msg;
    msg.Create(P_SERVER_CHANGE_TABLE_ERROR);
    if (pUser->GetMoney() < pOrgTable->GetEnterMoney()) {
        msg.AddInfo("msg", "金币不够");
		msg.AddInfo("type", MONEY_NOT_ENOUGH);
        pUser->SendMsg(&msg);
        return false;
    }

    //OBJECT_ID org_tid = it->first;

	OBJECT_ID org_tid = pOrgTable->GetID();
    if (!pOrgTable->Standup(pUser, LOGOUT_CHANGE_TABLE)) {
        logDebug("玩家(%u)游戏中请求换桌失败", idUser);
        msg.AddInfo("msg", "游戏中不能换桌");
		msg.AddInfo("type", GAMING);
        pUser->SendMsg(&msg);
        return false;
    }

	int players = pOrgTable->GetPlayerSize();
    //if (!EnterRoom(pUser, 0, org_tid)) 
	if(!pUser->EnterRoom(this, 0, org_tid, players))
	{
        logDebug("玩家(%u)请求换桌新桌EnterRoom失败", idUser);
        msg.AddInfo("msg", " 不能换桌");
		msg.AddInfo("type", ENTER_FAIL);
        pUser->SendMsg(&msg);
        return false;
    }

    logDebug("玩家(%u)请求换桌[%u]->[%u],suceess", idUser, org_tid, pUser->_GetTableID());
    return true;
}

void GameRoom::OnTimer_dismiss(void)
{
	for (std::set<GameTable*>::iterator iter = _arrDismiss.begin();
		iter != _arrDismiss.end(); ++iter) {
		GameTable * pTable = *iter;
		if (pTable) pTable->Release();
		
	}
	_arrDismiss.clear();
}

void GameRoom::DismissLater(GameTable *pTable)
{
	OBJECT_ID idTable = pTable->GetID();

	int times = 0;
	bool result = false;
	do
	{
		result = DB_ROOM->execute("del htid:%u", idTable);
		if (result) break;
	} while (++times < 2);
	if (!result) logDebug("清除房间信息失败, room:%u",  idTable);

	if (pTable) {
		_setTable.erase(pTable->GetID());
		_arrDismiss.insert(pTable);		
	}
}


