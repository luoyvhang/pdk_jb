#include "player.h"
#include "gameServer.h"

OBJPOOL_IMPLEMENTATION(Player,PLAYER_POOL_SIZE)


OBJECT_ID Player::_GetID(void) const
{
	return this->GetID();
}

const char* Player::_GetName(void) const
{
	return _userInfo.name.c_str();
}

OBJECT_ID Player::_GetTableID(void) const
{
	if (_pTable) return _pTable->GetID();
	return 0;
}

IGameTable* Player::_GetTable(void) const
{
	return _pTable;
}

void Player::_SendMsg(const std::string& packet)
{
	this->SendMsg(packet);
}

bool Player::_RedisCommand(const char* command)
{
	return DB_PLAYER->execute(command);
}

Player::Player(evwork::IConn* pClient)
	:_conn(pClient)
	,_pRoom(NULL)
	,_pTable(NULL)
{
#ifdef _ROBOT
	_robot = false;
#endif
	_online = _conn ? true : false;
}

Player::~Player()
{
#ifdef _ROBOT
	if (!_robot) {
#endif
		ASSERT_V(!_online && "析构时候连接未断开.");
#ifdef _ROBOT
	}
#endif
}

#ifdef _DEBUG
void Player::debugPool(void)
{
	logDebug("Player pool:%d", OBJPOOL_GETSIZE(Player));
}
#endif

bool Player::login(OBJECT_ID idUser)
{
    /*
	if (_online && _conn) {
		uint16_t port;
		_conn->getPeerInfo(_ip, port);
	}
    */
	_userInfo.id = idUser;
	DB_PLAYER->GetUserInfo(idUser,_userInfo);
	return _userInfo.id != 0;
}

void Player::logout(void)
{
	_online = false;
}

void Player::kickOut(void)
{
	if (_online) {
		if (_conn) {
			GAMESERVER->KickClient(_conn);
			_conn = NULL;
		}
		_online = false;
	}
}

/*关闭连接*/
void Player::OnDisconnect(void)
{
	_online = false;
	_conn = NULL;
	//_ip = "离线";
}

void Player::UpdateInfo(void)
{
#ifdef _ROBOT
	if (_robot) return;
#endif
	DB_PLAYER->GetUserInfo(this->GetID(),_userInfo);
}

/*发送消息给客户端*/
bool Player::SendMsg(const JSONPacket* packet)
{
#ifdef _ROBOT
	if (_robot) return true;
#endif
	ASSERT_B(_online && "客户端离线");
	ASSERT_B(packet);
	const std::string& msg = packet->GetPacket();
	ASSERT_B(_conn->sendBin(msg.c_str(), msg.length()));
	return true;
}

bool Player::SendMsg(const char* bufMsg, int length)
{
#ifdef _ROBOT
	if (_robot) return true;
#endif
	ASSERT_B(_online && "客户端离线");
	ASSERT_B(bufMsg);
	const std::string& msg = JSONPacket::formatPacket(bufMsg, length);
	//debug
	ASSERT_B(msg.length() == length + sizeof(PACK_HEADER));
	ASSERT_B(_conn->sendBin(msg.c_str(), msg.length()));
	return true;
}

/*发送打包好的消息给客户端*/
bool Player::SendMsg(const std::string& packet)
{
#ifdef _ROBOT
	if (_robot) return true;
#endif
	ASSERT_B(_online && "客户端离线");
	ASSERT_B(packet.length() > sizeof(PACK_HEADER));
	ASSERT_B(_conn->sendBin(packet.c_str(), packet.length()));
	return true;
}

bool Player::EnterRoom(GameRoom* pRoom, OBJECT_ID idTable, OBJECT_ID exceptTid, int players)
{
	ASSERT_B(pRoom);
	if (pRoom->EnterRoom(this, idTable, exceptTid, players)) {
		_pRoom = pRoom;
		return true;
	}
	return false;
}

bool Player::JoinTable(GameTable* pTable)
{
	ASSERT_B(pTable);
	if (pTable->Sitdown(this)) {
		_pTable = pTable;
		return true;
	}
	return false;
}

bool Player::QuitGame(void)
{
	GameRoom* pRoom = this->GetRoom();
	if (pRoom) return pRoom->Offline(this);
	return true;
}

/*退出房间*/
void Player::OnLeaveRoom(void)
{
	_pRoom = NULL;
	_pTable = NULL;
}

void Player::OnLeaveTable(void)
{
	_pTable = NULL;
}

OBJECT_ID Player::GetID(void) const
{
	return _userInfo.id;
}

const char* Player::GetName(void) const
{
	return _userInfo.name.c_str();
}

int Player::GetSex(void) const
{
	return _userInfo.sex;
}

int Player::GetTotalBoard(void) const
{
	return _userInfo.totalTimes;
}

const char* Player::GetAvatar(void) const
{
	return _userInfo.avatar.c_str();
}

void Player::SetLastRoom(OBJECT_ID idRoom /*= 0*/, OBJECT_ID idTable /*= 0*/, int port /*=0*/)
{
#ifdef _ROBOT
	if (_robot) return;
#endif
	ASSERT_V(DB_PLAYER->SetLastRoom(this->GetID(), idRoom, idTable, port));
}

bool Player::resetLastRoom(OBJECT_ID idUser)
{
	return DB_MAIN(idUser)->SetLastRoom(idUser, 0, 0, 0);
}

int Player::GetZid(OBJECT_ID idUser)
{
	return DB_MAIN(idUser)->GetZid(idUser);
}


bool Player::inc_playTimes(void)
{
#ifdef _ROBOT
	if (_robot) return true;
#endif
	_userInfo.totalTimes++;
	return DB_PLAYER->inc_GameTimes(this->GetID());
}

int Player::GetMoney(bool db)
{
	if (db)
	{
		_userInfo.money = DB_PLAYER->GetMoney(this->GetID());
	}
    return _userInfo.money;
}

bool Player::IncrMoney(int value, int tid, ALTER_TYPE alter_type)
{
#ifdef _ROBOT
    if (_robot) {
        _userInfo.money += value;
        return true;
    }
#endif
    int cur_value = value;
    if (DB_PLAYER->IncrMoney(this->GetID(), cur_value)) {
        _userInfo.money = cur_value;
        DB_EVENT->commit_eventlog(GameVid, this->GetID(), tid, value, cur_value, alter_type, 1, /*appid*/1, false);
        return true;
    }
    logErr("Player(%u,%s) IncrMoney失败 value:%d", this->GetID(), this->GetName(), value);
    return false;
}

#ifdef _ROBOT
Player* Player::GetRobot(void)
{
	const char* face[] = {
		"http://img.name2012.com/uploads/allimg/14-01/14-030958_536.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031001_229.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031001_469.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031002_661.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031003_335.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031007_948.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031009_812.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031009_129.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031013_79.jpg",
		"http://www.ld12.com/upimg358/20160130/210140271174736.jpg",
		"http://www.ld12.com/upimg358/20160130/205948986174879.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031014_238.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031016_251.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031019_229.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031020_53.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031023_631.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031024_685.jpg",
		"http://img.name2012.com/uploads/allimg/14-01/14-031023_880.jpg",
		"http://img.name2012.com/uploads/allimg/121014/14-015109_214.jpg",
		"http://img.name2012.com/uploads/allimg/121014/14-015110_477.jpg",
		"http://img.name2012.com/uploads/allimg/121014/14-015102_11.jpg",
		"http://img.name2012.com/uploads/allimg/121014/14-015059_122.jpg",
	};
	Player* pUser = new Player(NULL);
	pUser->_robot = true;
	pUser->_userInfo.name = "robot";
	pUser->_userInfo.id = rand() % 9999 + 1;
	pUser->_userInfo.sex = 2;
	pUser->_userInfo.avatar = face[rand() % 22];
    pUser->_userInfo.money = 10000;

	pUser->_userInfo.totalTimes = rand();
	pUser->_online = true;
	//pUser->_ip = "127.0.0.1";
	return pUser;
}

void Player::setName(const char* name)
{
	_userInfo.name = name;
}

#endif
