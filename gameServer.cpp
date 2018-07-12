#include "gameServer.h"

#include <stdio.h>
#include <string.h>
#include <fstream>

#include "include/libevwork/EVWork.h"
#include "include/libevwork/JsonData.h"
#include "include/libevwork/JsonMFC.h"

bool single_instance(const char* pid_file);

GameServer* GameServer::__instance = NULL;

using namespace evwork;

BEGIN_FORM_MAP(GameServer)
	ON_REQUEST_CONN(P_CLIENT_LOGIN_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_LOGOUT_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_READY_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_CHAT_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_CHATMSG_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_BET_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_TABLE_INFO_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_RUN_REQ, &GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_RECHARGE_REQ, &GameServer::ProcessClientMsg)

	ON_REQUEST_CONN(P_CLIENT_FORWARD,&GameServer::ProcessClientMsg)
	ON_REQUEST_CONN(P_CLIENT_CHANGE_TABLE_REQ,&GameServer::ProcessClientMsg)

	ON_REQUEST_CONN(SYSTEM_SERVER, &GameServer::ProcessSystemMsg)
	ON_REQUEST_CONN(SYSTEM_ECHO, &GameServer::ProcessSystemMsg)
	ON_REQUEST_CONN(SYSTEM_ONLINE, &GameServer::ProcessSystemMsg)

	ON_REQUEST_CONN(SYSTEM_AWARD, &GameServer::ProcessSystemMsg)
	ON_REQUEST_CONN(SYSTEM_AWARD2, &GameServer::ProcessSystemMsg)
	ON_REQUEST_CONN(SYSTEM_DAIKAI, &GameServer::ProcessSystemMsg)

	ON_REQUEST_CONN(SYSTEM_CLIENT_ACHIEVE_PACKET, &GameServer::ProcessSystemMsg)

END_FORM_MAP()

GameServer::GameServer()
	:_pRoom(NULL)
{

}

GameServer::~GameServer()
{
	if (_pRoom) {
		delete _pRoom;
		_pRoom = NULL;
	}

	for (SET_TEMP_CLIENT::iterator iter = _setTempClient.begin();
		iter != _setTempClient.end(); ++iter)
	{
		EvTimer* pTimer = iter->second;
		delete pTimer;
	}
	_setTempClient.clear();

	for (MAP_GAMEPLAYER::iterator iter = _setOnline.begin();
		iter != _setOnline.end(); ++iter)
	{
		Player* pUser = iter->second;
		delete pUser;
	}
	_setOnline.clear();

	for (VEC_DBPLAYER::iterator iter = _dbPlayer.begin();
		iter != _dbPlayer.end(); ++iter)
	{
		DBPlayer* pDB = *iter;
		if(pDB) delete pDB;
	}
	_dbPlayer.clear();
}

GameServer* GameServer::GetInstance()
{
	if (!__instance) {
		__instance = new GameServer;
	}
	return __instance;
}

void GameServer::Release()
{
	if (!__instance) {
		delete __instance;
		__instance = NULL;
	}
}

bool GameServer::LoadConf(const char* conf)
{
	std::ifstream file(conf, std::ios::binary);
	if (!file) {
		printf("读配置文件\"%s\"失败.", conf);
		return false;
	}

	Json::Reader reader;
	if (!reader.parse(file, _conf)) {
		printf("解析配置文件\"%s\"失败.", conf);
		return false;
	}

	file.close();
	return true;
}

bool GameServer::InitRedis(void)
{
	const Json::Value& dbConf = _conf["main-db"];
	if (!dbConf.isArray()) {
		printf("玩家数据库配置不正确.\n");
		return false;
	}
	Json::ArrayIndex count = dbConf.size();
	for (Json::ArrayIndex i = 0; i < count; ++i) {
		const Json::Value& dbInfo = dbConf[i];
		DBPlayer* dbPlayer = new DBPlayer;
		if (dbPlayer->InitRedis(dbInfo["host"].asCString(), dbInfo["port"].asInt(), dbInfo["pass"].asCString())) {
			_dbPlayer.push_back(dbPlayer);
		}
		else {
			printf("连接玩家数据库失败.host:%s,port:%d\n", dbInfo["host"].asCString(), dbInfo["port"].asInt());
			return false;
		}
	}
	_sizeDBPlayer = (int)_dbPlayer.size();

	if (!_dbEvent.InitRedis(_conf["eventlog-db"].get("host", "127.0.0.1").asCString(),
		_conf["eventlog-db"]["port"].asInt(), _conf["eventlog-db"]["pass"].asCString())) {
		printf("连接日志数据库失败.\n");
		return false;
	}

	if (!_dbRoom.InitRedis(_conf["room-db"].get("host", "127.0.0.1").asCString(),
		_conf["room-db"]["port"].asInt(), _conf["room-db"]["pass"].asCString())) {
		printf("连接房间数据库失败.\n");
		return false;
	}

	if (!_dbData.InitRedis(_conf["data-db"].get("host", "127.0.0.1").asCString(),
		_conf["data-db"]["port"].asInt(), _conf["data-db"]["pass"].asCString())) {
		printf("连接数据库失败.\n");
		return false;
	}

	printf("连接数据库成功.\n");
	return true;
}

bool GameServer::InitRoom(void)
{
	if (_pRoom) {
		logDebug("重复初始化房间..");
		return false;
	}
	const Json::Value& valRoom = _conf["room"];
	OBJECT_ID	idRoom = valRoom["vid"].asUInt();
	int		    base_money = valRoom["base_money"].asInt();
	int			enter_money = valRoom["enter_money"].asInt();
	int			leave_money = valRoom["leave_money"].asInt();
    int         fee = valRoom["fee"].asInt();
	int			bomb_Money = base_money * 10;//炸弹翻10倍.

	int			room_type = valRoom["room_type"].asInt();		// 房间类型  系统场 0， 好友场 1
	//	游戏特殊配置
	int			game_type = valRoom["game_type"].asInt();		// 游戏类型  0  16张  1 15张
	int			game_rule = valRoom["game_rule"].asInt();		// 游戏规则  第一位表示 是否黑桃三先出  第二位表示是否扎鸟
	int			player = valRoom["player"].asInt();				// 玩家人数  2 3  
	int			fourdaithree = valRoom["fourdaithree"].asInt();	// 是否允许  4带3  0 不允许 1允许
	int			threeAbomb = valRoom["threeAbomb"].asInt();		// 3个A算炸  0 不算 1 算
	int			showcardnum = valRoom["showcardnum"].asInt();	// 显示剩余牌数 0 不显示 1显示

    printf("服务器启动参数vid[%u] base_money[%d] enter_money[%d] leave_money[%d] bomb_money[%d] fee[%d] room_type[%d] game_type[%d] game_rule[%d] player[%d] fourdaithree[%d] threeAbomb[%d].\n", 
           idRoom, base_money, enter_money, leave_money, bomb_Money, fee, room_type, game_type, game_rule, player, fourdaithree, threeAbomb);

	GameRoom*	pRoom = new GameRoom;
	if (!pRoom->Init(idRoom, base_money, enter_money, leave_money, bomb_Money, fee, room_type, game_type, game_rule, player, showcardnum, fourdaithree, threeAbomb)) {
		delete pRoom;
		return false;
	}
	_pRoom = pRoom;
	return true;
}

bool GameServer::InitHorn(void)
{
	const Json::Value& pub_db = _conf["pub-db"];
	if (!pub_db.isArray()) {
		printf("大喇叭数据库配置不正确.\n");
		return false;
	}

	_pubHorn.clear();

	Json::ArrayIndex num = pub_db.size();

	for (Json::ArrayIndex i = 0; i < num; ++i) {
		const Json::Value& dbHorn = pub_db[i];
		if (!dbHorn.isObject()) {
			printf("大喇叭数据库配置不正确.属性配置错误.\n");
			return false;
		}
		const Json::Value& subKey = dbHorn["sub_db"];
		if (!subKey.isArray()) {
			printf("大喇叭数据库配置不正确.子节点错误.\n");
			return false;
		}
		_pubHorn.push_back(HornInfo());
		HornInfo& info = _pubHorn.back();
		const std::string& host = dbHorn.get("host", "127.0.0.1").asString();
		unsigned short port = (unsigned short)dbHorn.get("port", 0).asInt();
		const std::string& pass = dbHorn.get("pass", "123456").asString();
		if (!info._dbHander.InitRedis(host.c_str(), port, pass.c_str())) {
			printf("连接大喇叭数据库[%s:%u]失败.\n", host.c_str(), port);
			return false;
		}
		for (Json::ArrayIndex j = 0; j < subKey.size(); ++j) {
			info._keys.push_back(subKey[j].asInt());
		}
	}

	printf("初始化大喇叭完成..\n");
	return true;
}

bool GameServer::__TransmisMsg(const Json::Value& msg)
{
	if (!_pubHorn.size()) return false;

	const char* _KEY_NAME = "webservice_list";
	const std::string& strMsg = msg.toStyledString();

	std::vector<HornInfo>::iterator iter = _pubHorn.begin();
	for (; iter != _pubHorn.end(); ++iter) {
		std::vector<int>::const_iterator it = iter->_keys.begin();
		for (; it != iter->_keys.end(); ++it) {
			int key = *it;
			if (!iter->_dbHander.execute("LPUSH %s_%u %s", _KEY_NAME, key, strMsg.c_str())) {
				logErr("往大喇叭监控的数据库写数据失败.key:%s_%u", _KEY_NAME, key);
				logErr(strMsg.c_str());
			}
		}
	}
	return true;
}


bool GameServer::InitServer(const char* conf)
{
	if (!LoadConf(conf)) return false;

	if (!single_instance(_conf.get("pid_file", "pdkSvr.pid").asCString())) {
		printf("进程正在运行中.无法重复运行.\n");
		return false;
	}

//	if (!InitRedis()) return false;

//	if (!InitRoom()) return false;

	printf("服务初始化完成..\n");

	return true;
}

bool GameServer::StartServer(void)
{

	CSyslogReport LG;
	CEVLoop LP;
	CConnManager CM;
	CWriter WR;

	CEnv::setLogger(&LG);
	CEnv::setEVLoop(&LP);
	CEnv::setLinkEvent(&CM);
	CEnv::setConnManager(&CM);
	CEnv::setWriter(&WR);

	LP.init();
	CEnv::getEVParam().uConnTimeout = 120;
	CJsonData __DE;
	__DE.setPacketLimit(16 * 1024); // 设置最大请求包长度 
	CEnv::setDataEvent(&__DE);

	CJsonMFC __MFC;
	__DE.setAppContext(&__MFC);


	_port = _conf["game"]["port"].asInt();

	std::string srvHost = _conf["game"]["host"].asString();

	logSetDir(_conf.get("log_path", "log").asCString());

	if (!InitRedis()) return false;

	if (!InitRoom()) return false;

	if (!InitHorn()) return false;

	TIMER_CALLBACK cbTick = CXX11::bind(&GameServer::OnTimer_tick, this);
	_timer_tick.CreateTimer(1, 1, cbTick, true);

	TIMER_CALLBACK cbPrint = CXX11::bind(&GameServer::OnTimer_print, this);
	_timer_print.CreateTimer(10, 10, cbPrint, true);
	
	CM.addLE(this);

	__MFC.addEntry(GameServer::getFormEntries(), this);

	//后台服务相关.
	_adminPass = _conf["game"].get("pass", "123456789").asCString();

	CListenConn __LS(_port, srvHost);

	printf("服务启动完成.监听服务:%s:%u\n",
		srvHost.c_str(), GetPort());

	LP.runLoop();

	return true;
}

void GameServer::KickClient(evwork::IConn* pConn)
{
	unordered_map<uint32_t, Player*>::iterator iter = _setPlayer.find(pConn->getcid());
	if (iter != _setPlayer.end()) {
		Player* pUser = iter->second;
		if (pUser) OnDisconnect(pUser);
		_setPlayer.erase(iter);
	}
	_setConnTick.insert(pConn);
}

bool GameServer::SendMsg(evwork::IConn* pConn, const JSONPacket* packet)
{
	const std::string& msg = packet->GetPacket();
	return pConn->sendBin(msg.c_str(), msg.size());
}


void GameServer::OnTimeOut(evwork::IConn* pConn)
{
	std::string ip;
	uint16_t	port;
	pConn->getPeerInfo(ip, port);
	/*超时未通过验证的*/
	logMsg("客户端[%s:%u]在30秒内无反映.", ip.c_str(), port);
	KickClient(pConn);
}

void GameServer::onConnected(evwork::IConn* pConn)
{
#ifdef _DEBUG
	std::string strIp;
	uint16_t port;
	pConn->getPeerInfo(strIp, port);
	logDebug("onConnected:%s %d", strIp.c_str(), port);
#endif
	/*加入到临时表,指定的时间内,未通过验证进入游戏,踢下线.*/
	TIMER_CALLBACK time_cb = CXX11::bind(&GameServer::OnTimeOut, this, const_cast<evwork::IConn*>(pConn));
	SET_TEMP_CLIENT::iterator it = _setTempClient.find(pConn->getcid());

	EvTimer* pTimer = NULL;

	if (it != _setTempClient.end()) {
		logDebug("客户端连接列表存在重复的会话id.");
		pTimer = it->second;
		pTimer->CreateTimer(TIMEOUT_TEMP_CLIENT, 0, time_cb);
	}
	else {
		pTimer = new EvTimer();
		pTimer->CreateTimer(TIMEOUT_TEMP_CLIENT, 0, time_cb);
		_setTempClient.insert(std::make_pair(pConn->getcid(), pTimer));
	}
}

void GameServer::onClose(evwork::IConn* pConn)
{
	/*删除临时表内对应的客户端超时处理*/
	SET_TEMP_CLIENT::iterator it = _setTempClient.find(pConn->getcid());
	if (it != _setTempClient.end()) {
		delete it->second;
		_setTempClient.erase(it);
	}

	_setConnTick.erase(pConn);

	unordered_map<uint32_t, Player*>::iterator iter = _setPlayer.find(pConn->getcid());
	if (iter != _setPlayer.end()) {
		Player* pUser = iter->second;
		if (pUser) OnDisconnect(pUser);
		_setPlayer.erase(iter);
	}
}


/*处理客户端消息,返回false不再处理,并踢下线.*/
bool GameServer::ProcessMsg(evwork::IConn* pConn, const JSONPacket* packet)
{
	ASSERT_B(packet);
	int cmdType = packet->GetType();
	ASSERT_B(cmdType > 0);

	if (P_CLIENT_LOGIN_REQ == cmdType)
		return msg_Login(pConn, packet);

	Player* pUser = NULL;

	unordered_map<uint32_t, Player*>::iterator iter = _setPlayer.find(pConn->getcid());
	if (iter != _setPlayer.end()) pUser = iter->second;

	if (!pUser) {
		/*
		std::string ip;
		uint16_t port;
		pConn->getPeerInfo(ip, port);
		logErr("非法消息:%s,IP:%s", packet->dumpInfo(), ip.c_str());
		*/
		return false;
	}

    if (P_CLIENT_CHANGE_TABLE_REQ == cmdType)
        return _pRoom->ChangeTable(pUser);

	GameTable*  pTable = pUser->GetTable();
	bool result = pTable ? true : false;
	switch (cmdType){
	case P_CLIENT_READY_REQ:/*准备*/
		if (pTable) result = pTable->handle_ready(pUser);
		break;
	case P_CLIENT_LOGOUT_REQ:/*退出*/
		result = msg_Logout(pUser, packet);
		break;
	case P_CLIENT_CHAT_REQ:/*聊天*/
		if (pTable) result = pTable->handle_chat(pUser, packet);
		break;
	case P_CLIENT_CHATMSG_REQ:/*语音*/
		if (pTable) result = pTable->handle_chatMsg(pUser, packet);
		break;
	case P_CLIENT_TABLE_INFO_REQ:/*获取桌子信息*/
		if (pTable) result = pTable->handle_tableInfo(pUser);
		break;
	case P_CLIENT_BET_REQ:/*出牌*/
		if (pTable) result = pTable->handle_bet(pUser, packet);
		break;
    case P_CLIENT_FORWARD:
		if (pTable) result = pTable->handle_forward(pUser, packet);
        break;
	case P_CLIENT_RUN_REQ:
		if (pTable) result = pTable->handle_run(pUser);
		break;
	case P_CLIENT_RECHARGE_REQ:
		if (pTable) result = pTable->handle_recharge(pUser);
		break;
		/*
	default:
		logWarning("客户端上传了未知类型的消息.type:%d,%s", cmdType, packet->GetInfo().toStyledString().c_str());
		break;
		*/
	}
	if (!result && !pTable) GameTable::EnterResult(pUser, ENTER_FOUND);
	return result;
}

void GameServer::ProcessClientMsg(evwkPack, evwkConn)
{
	JSONPacket msg;
	if (msg.Init(packet.tojson())) {
#ifdef _DBGMSG
		logDebug("recv message:\n%s", msg.dumpInfo());
#endif
//		if (!ProcessMsg(pConn, &msg))
//			KickClient(pConn);
		ProcessMsg(pConn, &msg);
	}else{
		logErr("非法数据包.%s", packet.tostring().c_str());
	}
}


void GameServer::ProcessSystemMsg(evwkPack, evwkConn)
{
	Json::Value pack = packet.tojson();
	JSONPacket msg;
	if (!msg.Init(pack)) {
		logErr("非法数据包.%s", packet.tostring().c_str());
		return;
	}
	int cmdType = msg.GetType();
	ASSERT_V(cmdType > 0);

	if (SYSTEM_ONLINE == cmdType) {
		std::string ip;
		uint16_t	port;
		pConn->getPeerInfo(ip, port);
		logInfo("请求获取在线人数.ip:%s,端口:%d", ip.c_str(), port);
		JSONPacket msg;
		msg.Create(SYSTEM_ONLINE);
		msg.AddInfo("online", (int)_setPlayer.size());
		this->SendMsg(pConn, &msg);
		return;
	}
	else if (SYSTEM_ECHO == cmdType) {
		this->SendMsg(pConn, &msg);
		return;
	}
	else if (SYSTEM_AWARD == cmdType) {
		//普通红包
#ifdef _DEBUG
		logDebug("%s", packet.tojson().toStyledString().c_str());
#endif
		this->OnRedPacket(packet, pConn);
	}
	else if (SYSTEM_AWARD2 == cmdType) {
		//财神红包
#ifdef _DEBUG
		logDebug("%s", packet.tojson().toStyledString().c_str());
#endif
		this->OnBigPacket(packet, pConn);
	}
	else if (SYSTEM_CLIENT_ACHIEVE_PACKET == cmdType) {
		this->OnAchievePacket(packet, pConn);
	}
}


bool GameServer::msg_Login(evwork::IConn* pConn, const JSONPacket* packet)
{
	const Json::Value& info = packet->GetInfo();

	/*check*/
	if (!info["uid"].isNumeric() ||
		!info["skey"].isString() ||
		!info["room"].isNumeric())
	{
		return false;
	}

	OBJECT_ID	idUser = info["uid"].asUInt();
	//OBJECT_ID	idTable = info["room"].asUInt();
	int players = info["players"].asInt();

	if (players > 3 || players < 2)
	{
		players = 3;
	}
	OBJECT_ID idTable = Player::GetZid(idUser);
	if (idTable <= 0)
	{
		idTable = info["room"].asUInt();
	}

	std::string skey = info["skey"].asString();
	bool resume = false;
	Player* pUser = Login(pConn, idUser, skey.c_str(),resume);
	//进程恢复到前台运行时会重复发这个消息.
	if (resume) {
		GameTable* pTable = pUser->GetTable();
		if (pTable) pTable->OnResume(pUser);
		else GameTable::EnterResult(pUser, ENTER_FOUND);
		return true;
	}

	if (pUser) {
		/******************/
		/*删除临时表内对应的客户端超时处理*/
		SET_TEMP_CLIENT::iterator it = _setTempClient.find(pConn->getcid());
		if (it != _setTempClient.end()) {
			delete it->second;
			_setTempClient.erase(it);
		}
		/******************/
		_setPlayer.insert(std::make_pair(pConn->getcid(), pUser));
		//登录成功,进入房间.
		if (pUser->EnterRoom(_pRoom, idTable, 0, players)) return true;
	}
	return false;
}

bool GameServer::msg_Logout(Player* pUser, const JSONPacket* packet)
{
	GameRoom* pRoom = pUser->GetRoom();
	if (pRoom) {
		evwork::IConn* pConn = pUser->getConn();
		//if (!pRoom->Offline(pUser)) {
		if (!pRoom->Logout(pUser)) {
			//保持离线状态.
			UserOffline(pUser);
		}else{
			//可以销毁这个玩家的数据了.
			UserLogout(pUser);
		}
		//evwork::IConn* pConn = pUser->getConn();
		if (pConn) _setPlayer.erase(pConn->getcid());
	}
	else
	{
		logDebug("无法获取玩家[%s]房间信息", pUser->GetName());
	}
	return true;
}

Player* GameServer::Login(evwork::IConn* pConn, OBJECT_ID idUser, const char* skey,bool& resume)
{
	Player* pUser = NULL;

	//验证skey
	if (!DB_MAIN(idUser)->CheckUser(idUser, skey)) {
		JSONPacket msg;
		msg.Create(N_SERVER_LOGIN_SUCC_UC);
		msg.AddInfo("code", 505);
		msg.AddInfo("msg", "skey error");
		SendMsg(pConn, &msg);
		return NULL;
	}

#ifdef _DEBUG
	logDebug("登录请求:{\"uid\":%u,\"skey\":\"%s\"}", idUser, skey);
#endif
	resume = false;
	ITER_PLAYER it = _setOnline.find(idUser);
	if (it != _setOnline.end()) {
		//重复登录,挤掉上一个连接
		Player* player = it->second;
		pUser = player;
		if (player->getConn() != pConn) {
			//QuitGame(player);
			KickOut(player);
			player->setConn(pConn);
			player->login(idUser);
#ifdef _DEBUG
			logDebug("玩家[%s]当前在线,挤掉上一个连接.", pUser->GetName());
#endif
		}
		else {
			resume = true;
#ifdef _DEBUG
			logDebug("玩家[%s]重新连接/恢复连接.", pUser->GetName());
#endif
		}
	}else if((it=_setOffline.find(idUser))!=_setOffline.end()){
		Player* player = it->second;
		player->setConn(pConn);
		player->login(idUser);
		pUser = player;
		_setOffline.erase(it);
		_setOnline.insert(std::make_pair(idUser, pUser));
#ifdef _DEBUG
		logDebug("玩家[%s]重新连接.", pUser->GetName());
#endif
	}else{
		pUser = new Player(pConn);
		pUser->login(idUser);
		_setOnline.insert(std::make_pair(idUser, pUser));
#ifdef _DEBUG
		logDebug("玩家[%s]登录.", pUser->GetName());
#endif
	}
	return pUser;
}

/*被动断开连接*/
void GameServer::OnDisconnect(Player* pUser)
{
#ifdef _DEBUG
	logDebug("玩家[%s]断开连接.", pUser->GetName());
#endif
	pUser->OnDisconnect();
	GameRoom* pRoom = pUser->GetRoom();
	if (pRoom) {
		if (pRoom->Offline(pUser)) {
			UserLogout(pUser);
		}else{
			UserOffline(pUser);
		}
		return;
	}
	UserLogout(pUser);
}

/*主动断开连接,不处理游戏情况用于登录那里处理断线重连*/
void GameServer::KickOut(Player* pUser)
{
	evwork::IConn* pConn = pUser->getConn();
	if (!pConn) {
		ASSERT_V(!"踢玩家下线时,连接为无效连接.");
	}
	_setPlayer.erase(pConn->getcid());
	_setConnTick.insert(pConn);
}

/*玩家正常退出*/
void GameServer::UserLogout(Player* pUser)
{
	pUser->logout();
	OBJECT_ID idUser = pUser->GetID();
	ITER_PLAYER it = _setOnline.find(idUser);
	if (it != _setOnline.end()) {
		_setOnline.erase(it);
		delete pUser;
	}
	else {
		logDebug("退出:列表内没这个玩家.");
	}

	it = _setOffline.find(idUser);
	if (it != _setOffline.end())
		_setOffline.erase(it);

#ifdef _DEBUG
	Player::debugPool();
#endif
}

/*不能退出游戏.加到离线表内*/
void GameServer::UserOffline(Player* pUser)
{
	ASSERT_V(pUser);
	pUser->Offline();
	OBJECT_ID idUser = pUser->GetID();
	ITER_PLAYER it = _setOffline.find(idUser);
	if (it == _setOffline.end()) {
		_setOffline.insert(std::make_pair(idUser, pUser));
	}else{
		logDebug("离线:重复添加到离线表.");
	}
	it = _setOnline.find(idUser);
	if (it != _setOnline.end())
		_setOnline.erase(it);
}

bool GameServer::QuitGame(Player* pUser)
{
	GameRoom* pRoom = pUser->GetRoom();
	//if (pRoom) return pRoom->Offline(pUser);
	if (pRoom) return pRoom->Logout(pUser);
	pUser->SetLastRoom();
	return true;
}

void GameServer::OnTimer_tick()
{
	std::set<evwork::IConn*> tick;
	tick.swap(_setConnTick);
	for (std::set<evwork::IConn*>::iterator iter = tick.begin();
		iter != tick.end(); ++iter) {
		delete(*iter);
	}
}

void GameServer::OnTimer_print()
{
	logInfo("[GameServer::%s] connections:[%d] players:[%d]", __FUNCTION__, _setOnline.size(), _setPlayer.size());

	//for (MAP_UID_PLAYER_t::iterator iter = m_mapUidPlayer.begin(); iter != m_mapUidPlayer.end(); ++iter)
	//{
	//	IPlayer* pPlayer = iter->second;

	//	if (pPlayer->getConn() == NULL)
	//	{
	//		LOG(Info, "[IGame::%s] player uid:[%d] is trust!", __FUNCTION__, pPlayer->getUid());
	//	}
	//}
}


void GameServer::DeleteOffline(Player* pUser)
{
	ASSERT_V(QuitGame(pUser) && "不能删除在游戏中的离线玩家.");
	pUser->logout();
	OBJECT_ID idUser = pUser->GetID();
	ITER_PLAYER it = _setOffline.find(idUser);
	if (it != _setOffline.end()) {
		_setOffline.erase(it);
		delete pUser;
	}else{
		logDebug("未删除离线玩家,不在离线表内.");
	}
#ifdef _DEBUG
	Player::debugPool();
#endif
}

//创建一个离线玩家,进入指定房间和桌子.
Player* GameServer::CreateUser(OBJECT_ID idUser, OBJECT_ID idRoom, OBJECT_ID idTable)
{
	MAP_GAMEPLAYER::const_iterator iter = _setOnline.find(idUser);
	if (iter != _setOnline.end())
		return iter->second;

	//只有重启时才会调用该过程.
	/*
	iter = _setOffline.find(idUser);
	if (iter != _setOffline.end())
	return iter->second;
	*/

	Player* pUser = NULL;
	pUser = new Player(NULL);
	pUser->login(idUser);
	_setOffline.insert(std::make_pair(idUser, pUser));

	//恢复机制更改.玩家不进入房间.
	//pUser->EnterRoom(_pRoom, idTable);
	return pUser;
}

DBPlayer* GameServer::GetDBPlayer(OBJECT_ID idUser)
{
	ASSERT_P(_sizeDBPlayer > 0);
	if (_sizeDBPlayer == 1) return _dbPlayer[0];
	return _dbPlayer[idUser%_sizeDBPlayer];
}

DBPlayer* GameServer::GetDBPlayer(const Player* pUser)
{
	ASSERT_P(pUser);
	OBJECT_ID idUser = pUser->GetID();
	return GetDBPlayer(idUser);
}

bool GameServer::TransmitMsg(char* msg, int clrCode /*= 1*/,
	const Json::Value& parms /*= Json::Value()*/,
	const VEC_OBJECT_ID& targets /*= VEC_OBJECT_ID()*/)
{
	Json::Value jsCode = parms;

	if (targets.size()) {
		jsCode["cmd"] = SYSTEM_MSG_UC;
		for (VEC_OBJECT_ID::const_iterator iter = targets.begin();
			iter != targets.end(); ++iter) {
			jsCode["touid"].append(*iter);
		}
	}
	else jsCode["cmd"] = SYSTEM_MSG_BC;

	jsCode["color"] = clrCode;
	jsCode["str"] = msg;
#ifdef _DEBUG
	logDebug("发送大喇叭:%s", jsCode.toStyledString().c_str());
#endif
	return GAMESERVER->__TransmisMsg(jsCode);
}

int GameServer::_GetServerID(void) const
{
	ASSERT_I(_pRoom);
	return (int)_pRoom->GetID();
}

bool GameServer::_CheckAdmin(const std::string& password) const
{
	return password.compare(_adminPass) ? false : true;
}

IGamePlayer* GameServer::_GetPlayerByConn(evwork::IConn* pConn)
{
	unordered_map<uint32_t, Player*>::iterator iter = _setPlayer.find(pConn->getcid());
	IGamePlayer* player = NULL;
	if (iter != _setPlayer.end()) {
		player = iter->second;
	}
	return player;
}

