#include "cardHelper.h"

#include "../Common/logfile.h"

void cardHelper::to_json_array(const std::vector<Card>& cards, JSONPacket* packet, const char * key)
{
	ASSERT_V(packet && key);
	if (cards.size() == 0) {
		packet->Append(key, 0);
		return;
	}
	for (std::vector<Card>::const_iterator iter = cards.begin(); iter != cards.end(); ++iter) {
		packet->Append(key, iter->value);
	}
}

void cardHelper::to_json_array(const std::map<int, Card>& cards, Json::Value& jsonVal, const char* key)
{
	ASSERT_V(key);
	if (cards.size() == 0) {
		jsonVal[key].append(0);
		return;
	}
	for (std::map<int, Card>::const_iterator iter = cards.begin(); iter != cards.end(); ++iter) {
		jsonVal[key].append(iter->second.value);
	}
}

void cardHelper::to_json_array(const std::vector<Card>& cards, Json::Value& jsonVal, const char* key)
{
	ASSERT_V(key);
	if (cards.size() == 0) {
		jsonVal[key].append(0);
		return;
	}
	for (std::vector<Card>::const_iterator iter = cards.begin(); iter != cards.end(); ++iter) {
		jsonVal[key].append(iter->value);
	}
}

void cardHelper::to_json_array(const std::map<int, Card>& cards, JSONPacket* packet, const char* key)
{
	ASSERT_V(packet && key);
	if (cards.size() == 0) {
		packet->Append(key, 0);
		return;
	}
	for (std::map<int, Card>::const_iterator iter = cards.begin(); iter != cards.end(); ++iter) {
		packet->Append(key, iter->second.value);
	}
}

void cardHelper::to_vector(const JSONPacket* packet, const char * key, std::vector<Card>& out)
{
	ASSERT_V(packet && key);
	const Json::Value& val = packet->GetInfo();
	to_vector(val, key, out);
}

void cardHelper::to_map(const Json::Value& jsonVal, const char* key, std::map<int, Card>& out)
{
	ASSERT_V(key);
	out.clear();
	const Json::Value& arr = jsonVal[key];
	if (!arr.isArray()) return;
	Json::ArrayIndex count = arr.size();
	if (count == 1 && arr[0].asInt() == 0)
		return;
	for (Json::ArrayIndex i = 0; i < count; ++i) {
		Card temp(arr[i].asInt());
		out.insert(std::make_pair(temp.value, temp.value));
	}
}

void cardHelper::to_vector(const Json::Value& jsonVal, const char* key, std::vector<Card>& out)
{
	ASSERT_V(key);
	out.clear();
	const Json::Value& arr = jsonVal[key];
	if (!arr.isArray()) return;
	Json::ArrayIndex count = arr.size();
	if (count == 1 && arr[0].asInt() == 0)
		return;
	for (Json::ArrayIndex i = 0; i < count; ++i) {
		out.push_back(Card(arr[i].asInt()));
	}
}
