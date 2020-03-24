#pragma once

// ------------------------------------------------------------------------------------------------
#include "CSession.h"
#include "sleepy_discord/websocketpp_websocket.h"

// ------------------------------------------------------------------------------------------------
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>

// ------------------------------------------------------------------------------------------------
namespace SqDiscord
{
class CSession;
} // Namespace - SqDiscord

// ------------------------------------------------------------------------------------------------
class CDiscord : public SleepyDiscord::DiscordClient {

public:
	using SleepyDiscord::DiscordClient::DiscordClient;

	//std::vector<std::vector<SleepyDiscord::ServerMember>> s_members;
	std::unordered_map<std::string, SleepyDiscord::Server> s_servers;
	SqDiscord::CSession * session;
	bool isConnected = false;
	bool isConnecting = false;

	void onMessage(SleepyDiscord::Message message) override;
	void onReady(SleepyDiscord::Ready readyData) override;
	void onServer(SleepyDiscord::Server server) override;
	void onEditMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user, std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> roles, std::string nick) override;
};