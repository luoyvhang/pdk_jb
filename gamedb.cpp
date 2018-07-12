#include "gamedb.h"
#include "gameServer.h"

DBPlayer::DBPlayer()
{

}

DBPlayer::~DBPlayer()
{

}

bool DBPlayer::CheckUser(OBJECT_ID id, const char* skey)
{
	IRecord* record = this->query("hget hu:%u skey",id);
	IRecordPtr row(record);
	if (row) {
		return 0 == row->GetString().compare(skey);
	}
	return false;
}

bool DBPlayer::GetUserInfo(OBJECT_ID idUser, USERINFO& info)
{
	info.id = 0;
	IRecord* record = this->query("hgetall hu:%u", idUser);
	IRecordPtr row(record);
	if (row) {
		info.id = idUser;
		info.name = row->GetString("nickname");
		info.avatar = row->GetString("avatar");
		info.sex = row->GetInt("sex");
		info.totalTimes = row->GetInt("total_board");
		info.money = row->GetInt("money");
		info.ip = row->GetString("ip");
		info.lat = row->GetString("lat");
		info.lng = row->GetString("lng");
		return true;
	}
	return false;
}

int DBPlayer::GetZid(OBJECT_ID idUser)
{
	IRecord* record = this->query("hget hu:%u zid", idUser);
	IRecordPtr row(record);
	if (row)
	{
		int ret = 0;
		std::stringstream ss;
		ss << row->GetString();
		ss >> ret;
		return ret;
	}
	return 0;
}


int DBPlayer::GetMoney(OBJECT_ID idUser)
{
	IRecord* record = this->query("hget hu:%u money", idUser);
	IRecordPtr row(record);
	if (row) {
        int ret = 0;
        std::stringstream ss;
        ss << row->GetString();
        ss >> ret;
        return ret;
	}
	return 0;
}

bool DBPlayer::IncrMoney(OBJECT_ID idUser, int& value)
{
	IRecord* record = this->query("hincrby hu:%u money %d", idUser, value);
	IRecordPtr row(record);
	if (row) {
		value = (int)row->GetLlong();
		return true;
	}
	return false;
}

bool DBPlayer::inc_GameTimes(OBJECT_ID idUser)
{
	return this->execute("hincrby hu:%u total_board 1", idUser);
}

bool DBPlayer::SetLastRoom(OBJECT_ID idUser, OBJECT_ID idRoom /*= 0*/, OBJECT_ID idTable /*= 0*/, int port /*=0*/)
{
    return this->execute("hmset hu:%u vid %u zid %u port %d", idUser, idRoom, idTable, port);
}








DBEvent::DBEvent()
{

}

DBEvent::~DBEvent()
{

}

bool DBEvent::commit_eventlog(int gameType,int my_uid, int my_tid, int my_alter_value, 
	int my_current_value, int my_type, int my_alter_type,int appid, bool bRsyslog)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int ts = (int)tv.tv_sec;
	int usec = (int)tv.tv_usec;

	EventLog el;
	el.uid = my_uid;
	el.tid = my_tid;
	el.vid = GameVid;
	el.zid = GamePort;
	el.type = my_type;
	el.alter_type = my_alter_type;
	el.alter_value = my_alter_value;
	el.current_value = my_current_value;
	el.ts = ts;
	el.usec = usec;
	el.game_type = gameType;
	el.appid = appid;

	if (bRsyslog)
		return __incr_eventlog2(el);
	else
		return __incr_eventlog(el);
}

bool DBEvent::__incr_eventlog(EventLog &el)
{
	char field[32] = { 0 };
	snprintf(field, 31, "%d%d%d%d%d", el.uid, el.ts, el.tid, el.zid, el.type);

	return this->execute("hmset log:%s game_type %d uid %d tid %d vid %d zid %d type %d alter_type %d alter_value %d current_value %d ts %d app_id %d",
		field, el.game_type, el.uid, el.tid, el.vid, el.zid, el.type, el.alter_type, el.alter_value, el.current_value, el.ts, el.appid);
}

bool DBEvent::__incr_eventlog2(EventLog &el)
{
	char field[32] = { 0 };
	snprintf(field, 31, "%d%d%d%d", el.uid, el.ts, el.tid, el.zid);

	logInfo("#event# log:%s game_type %d uid %d tid %d vid %d zid %d type %d alter_type %d alter_value %d current_value %d ts %d app_id %d",
		field, el.game_type, el.uid, el.tid, el.vid, el.zid, el.type, el.alter_type, el.alter_value, el.current_value, el.ts, el.appid);
	return true;
}
