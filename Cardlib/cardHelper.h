#pragma once


#include "card.h"
#include <vector>
#include <map>
#include "../jsonPacket.h"

namespace cardHelper
{
	void	to_json_array(const std::vector<Card>& cards, JSONPacket* packet, const char* key);

	void	to_json_array(const std::map<int, Card>& cards, JSONPacket* packet, const char* key);

	void	to_json_array(const std::vector<Card>& cards, Json::Value& jsonVal, const char* key);

	void	to_json_array(const std::map<int, Card>& cards, Json::Value& jsonVal, const char* key);

	void	to_vector(const JSONPacket* packet, const char* key, std::vector<Card>& out);

	void	to_vector(const Json::Value& jsonVal, const char* key, std::vector<Card>& out);

	void	to_map(const Json::Value& jsonVal, const char* key, std::map<int, Card>& out);
};