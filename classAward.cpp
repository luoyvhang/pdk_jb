#include "classAward.h"
#include <time.h>
#include <algorithm>

using namespace evwork;

IGameTable::IGameTable()
{

}

IGameTable::~IGameTable()
{

}

int IGameTable::GetNumPacket(void)
{
	return (int)_awards.size();
}

void IGameTable::AddNewPacket(uint32_t money,int type)
{
	int times = (int)_awards.size();
	uint64_t seqid = rand() % 15 + 1;
    uint64_t tid = this->_GetID();
	seqid <<= 16;
	seqid += (tid << 8);
	seqid += times;

	_AWARD_INFO info;
	info.active = true;
	info.money = money;
	info.seqid = seqid;
	info.type = type;
	_awards.insert(std::make_pair(seqid, info));

	evwork::Jpacket msg;
	msg.val["cmd"] = SYSTEM_SERVER_SENDNEW_PACKET;
	msg.val["seq"] = (Json::UInt64)seqid;
	msg.val["type"] = type;
	msg.end();

	_BroadcastMessage(msg.tostring());

	/*
	Json::Value jsLog;
	jsLog["type"] = type;
	jsLog["money"] = money;
	jsLog["seqid"] = (Json::UInt64)seqid;
	jsLog["time"] = (Json::Int64)time(NULL);
	jsLog["table"] = this->_GetID();

	char buf[1024];
	sprintf(buf, "LPUSH redPacket_list %s", jsLog.toStyledString().c_str());
	if (!_RedisWriteLog(buf)) {
		LOG(Error, "redis command error.%s", buf);
	}
	*/
}

void IGameTable::AchievePacket(IGamePlayer* player, uint64_t seqid,int vid)
{
	MAP_AWARDS::iterator iter = _awards.find(seqid);
	evwork::Jpacket msg;
	if (iter == _awards.end()) {
		msg.val["cmd"] = SYSTEM_SERVER_PACKET_ERROR;
		msg.val["msg"] = "无效红包";
		msg.end();
		player->_SendMsg(msg.tostring());
		return;
	}
	_AWARD_INFO& info = iter->second;
	msg.val["cmd"] = SYSTEM_SERVER_PACKET_RESULT;
	msg.val["money"] = info.money;
	msg.val["type"] = info.type;
	if (!info.active) {
		msg.val["result"] = 0;
		msg.val["user"] = info.name;
		msg.end();
		player->_SendMsg(msg.tostring());
		return;
	}
	info.active = false;
	info.name = player->_GetName();
	msg.val["user"] = info.name;
	msg.val["result"] = 1;
	msg.end();
	player->_SendMsg(msg.tostring());

	int uid = (int)player->_GetID();

	char buf[512];
	sprintf(buf, "hincrby hu:%d hongbao %u", uid, info.money);
	if (!player->_RedisCommand(buf)) {
		LOG(Error, "redis command error.%s", buf);
	}

	int64_t currTime =(int64_t)time(NULL);
	sprintf(buf, "hmset hongbao:%d%ld%d uid %d money %u vid %d type %d ts %ld",
		uid, currTime, rand() % 0xFFFF, uid, info.money, vid, info.type, currTime);
	if (!_RedisWriteLog(buf)) {
		LOG(Error, "redis command error.%s", buf);
	}

	this->_OnAchievePacket(player, info.money, info.type);
}



ClassAward::ClassAward()
{

}

ClassAward::~ClassAward()
{

}

void ClassAward::OnRedPacket(evwork::Jpacket& packet, evwork::IConn* pConn)
{
	const Json::Value &val = packet.tojson();
	if (!val["pass"].isString() || !val["money"].isInt()) {
		LOG(Error, "%s,arguments invalid.", __FUNCTION__);
		return;
	}
	const std::string& pass = val["pass"].asString();
	if (!this->_CheckAdmin(pass)) {
		LOG(Error, "%s,password error.", __FUNCTION__);
		return;
	}
	uint32_t money = (uint32_t)val["money"].asInt();
	ProcessRedPacket(money);
}

void ClassAward::OnBigPacket(evwork::Jpacket & packet, evwork::IConn * pConn)
{
	const Json::Value &val = packet.tojson();
	if (!val["pass"].isString() || !val["money"].isInt()) {
		LOG(Error, "%s,arguments invalid.", __FUNCTION__);
		return;
	}
	const std::string& pass = val["pass"].asString();
	if (!this->_CheckAdmin(pass)) {
		LOG(Error, "%s,password error.", __FUNCTION__);
		return;
	}
	uint32_t money = (uint32_t)val["money"].asInt();
	ProcessBigPacket(money);
}

void ClassAward::OnAchievePacket(evwork::Jpacket& packet, evwork::IConn* pConn)
{
	IGamePlayer* pUser = this->_GetPlayerByConn(pConn);
	if (!pUser) {
		LOG(Error, "%s,pUser==NULL!!", __FUNCTION__);
		return;
	}
	IGameTable* pTable = pUser->_GetTable();
	if (!pTable) {
		LOG(Error, "%s,pUser.pTable==NULL!!", __FUNCTION__);
		return;
	}
	const Json::Value& val = packet.tojson();
	//if (!val["seqid"].isInt()) {
	//	LOG(Error, "%s,arguments invalid.", __FUNCTION__);
	//	return;
	//}

	pTable->AchievePacket(pUser, val["seqid"].asUInt64(), this->_GetServerID());
}

void ClassAward::OnCreateTable(IGameTable* pTable)
{
	if (!pTable) {
		LOG(Error, "%s,arguments invalid.", __FUNCTION__);
		return;
	}

	OBJECT_ID idTable = pTable->_GetID();
	_tables.insert(std::make_pair(idTable, pTable));
}

void ClassAward::OnDismissTable(IGameTable* pTable)
{
	if (!pTable) {
		LOG(Error, "%s,arguments invalid.", __FUNCTION__);
		return;
	}

	OBJECT_ID idTable = pTable->_GetID();
	_tables.erase(idTable);
}

void ClassAward::ProcessRedPacket(uint32_t money)
{
	if (money <= 0) {
		LOG(Error, "%s,money <= 0", __FUNCTION__);
		return;
	}
	const int _MAX_TIMES = 3;
	std::vector<IGameTable*> rooms;
	std::vector<IGameTable*> awards;
	for (MAP_GAMETABLE::iterator iter = _tables.begin(); iter != _tables.end(); ++iter) 
	{
		IGameTable* pTable = iter->second;
		//不处于免费时间的房间才能领红包.
		if (pTable)
		{
			if (!pTable->_IsFreeTime()) 
			{
				//加入到列表.当前所有房间都抢过_MAX_TIMES个红包后.防止出现空列表.
				rooms.push_back(pTable);
				if (pTable->GetNumPacket() < _MAX_TIMES)
				{
					//加入到允许抢红包列表.
					awards.push_back(pTable);
				}
			}
		}
		else
		{
			LOG(Error, "%s, pTable is Null", __FUNCTION__);
		}

	}
	if (!awards.size()) awards.swap(rooms);

	int ctRoom = (int)awards.size();
	if (!ctRoom) return;

	//step1.随机打乱.
	std::random_shuffle(awards.begin(), awards.end());
	//step2.再次随机取一个房间.
	int idx = rand() % ctRoom;
	IGameTable* pTable = awards[idx];
	pTable->AddNewPacket(money, 1);
}

void ClassAward::ProcessBigPacket(uint32_t money)
{
	if (money <= 0) {
		LOG(Error, "%s,money <= 0", __FUNCTION__);
		return;
	}
	const int _MAX_TIMES = 3;
	std::vector<IGameTable*> rooms;
	std::vector<IGameTable*> awards;
	for (MAP_GAMETABLE::iterator iter = _tables.begin(); iter != _tables.end(); ++iter) {
		IGameTable* pTable = iter->second;
		//不处于免费时间的房间才能领红包.
		if (pTable)
		{
			if (!pTable->_IsFreeTime()) 
			{
				//加入到列表.当前所有房间都抢过_MAX_TIMES个红包后.防止出现空列表.
				rooms.push_back(pTable);
				if (pTable->GetNumPacket() < _MAX_TIMES) {
					//加入到允许抢红包列表.
					awards.push_back(pTable);
				}
			}
			else
			{
				LOG(Error, "%s,not freetime", __FUNCTION__);
			}
		}
		else
		{
			LOG(Error, "%s, pTable is Null", __FUNCTION__);
		}

	}
	if (!awards.size()) awards.swap(rooms);

	int ctRoom = (int)awards.size();
	if (!ctRoom) return;

	//step1.随机打乱.
	std::random_shuffle(awards.begin(), awards.end());
	//step2.再次随机取一个房间.
	int idx = rand() % ctRoom;
	IGameTable* pTable = awards[idx];
	pTable->AddNewPacket(money, 2);
}

