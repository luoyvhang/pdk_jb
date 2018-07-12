#include "jsonPacket.h"
#include "Common/logfile.h"

JSONPacket::JSONPacket()
	:_cmdType(0)
{

}

JSONPacket::~JSONPacket()
{

}

bool JSONPacket::Init(const char* bufMsg, int length)
{
	_cmdType = 0;
	_packet = std::string(bufMsg, length);
	
	//decrypt
	JSONPacket::Encrypt(const_cast<char*>(_packet.c_str()), length);

	Json::Reader reader;
	if (!reader.parse(_packet, _root)) {
		return false;
	}
	return parse();
}

bool JSONPacket::Init(Json::Value& pack)
{
	_cmdType = 0;
#ifdef _DEBUG
	_packet = pack.toStyledString();
#endif
	_root.swap(pack);
	return parse();
}

std::string JSONPacket::GetPacket(void) const
{
	std::string packet(_root.toStyledString());
#ifdef _DBGMSG
    logDebug("sendDataStyled: %s", packet.c_str());
#endif
	PACK_HEADER header;
	header.length = JSONPacket::Encrypt(packet);
	const char* ptHeader = (const char*)&header;
	packet.insert(packet.begin(), ptHeader, ptHeader + sizeof(PACK_HEADER));
	return packet;
}

std::string JSONPacket::formatPacket(const char* bufMsg, int length)
{
	std::string packet(bufMsg, length);
	PACK_HEADER header;
	header.length = JSONPacket::Encrypt(packet);
	const char* ptHeader = (const char*)&header;
	packet.insert(packet.begin(), ptHeader, ptHeader + sizeof(PACK_HEADER));
	return packet;
}

void JSONPacket::Encrypt(char* bufMsg, int length)
{
	const int KEY = 16;
	for (int i = 0; i < length; ++i) {
		bufMsg[i] ^= KEY;
	}
}

int JSONPacket::Encrypt(std::string& packet)
{
	int length = packet.length();
	JSONPacket::Encrypt(const_cast<char*>(packet.c_str()), length);
	return length;
}

bool JSONPacket::parse(void)
{
	if (!_root["cmd"].isNumeric())
		return false;
	_cmdType = _root["cmd"].asInt();
	return true;
}
