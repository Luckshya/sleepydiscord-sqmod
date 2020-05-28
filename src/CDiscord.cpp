// ------------------------------------------------------------------------------------------------
#include "Common.hpp"
#include "CDiscord.h"

// ------------------------------------------------------------------------------------------------
#include <list>
#include <mutex>

// ------------------------------------------------------------------------------------------------
using namespace SqDiscord;

// ------------------------------------------------------------------------------------------------
void CDiscord::onMessage(SleepyDiscord::Message message) {
	//std::lock_guard<std::mutex> lock(session->m_Guard);
	try {
		Guild server;

		std::lock_guard<std::mutex> guard(this->session->m_MsgGuard);
		this->session->s_Messages.push_back(new Message(message));
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onMessage]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onReady(SleepyDiscord::Ready readyData) {
	//std::lock_guard<std::mutex> lock(session->m_Guard);
	try {
		this->session->isConnected = true;
		this->session->isConnecting = false;

		std::lock_guard<std::mutex> guard(this->session->m_ReadyGuard);
		this->session->s_ReadySession.push_back(this->session);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onReady]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onError(SleepyDiscord::ErrorCode errorCode, std::string errorMessage) {
	if (!session->errorEventEnabled) {
		if (errorCode != 0)
			std::cout << "Error " << errorCode << ": " + errorMessage + '\n';
		else
			std::cout << "Error: " + errorMessage + '\n';
	} else {
		//std::lock_guard<std::mutex> lock(session->m_Guard);
		try {
			std::lock_guard<std::mutex> guard(this->session->m_ErrorGuard);
			this->session->s_Errors.emplace_back(errorCode, errorMessage);
		}
		catch (...) {
			SqMod_LogErr("An Error has occured at [CDiscord] function => [onError]");
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onDisconnect() {
	//std::lock_guard<std::mutex> lock(session->m_Guard);
	try {
		std::lock_guard<std::mutex> guard(this->session->m_DisconnectsGuard);
		this->session->s_Disconnects.emplace_back(session);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onDisconnect]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onQuit() {
	//std::lock_guard<std::mutex> lock(session->m_Guard);
	try {
		std::lock_guard<std::mutex> guard(this->session->m_QuitsGuard);
		this->session->s_Quits.emplace_back(session);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onQuit]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onServer(SleepyDiscord::Server server) {
	//std::lock_guard<std::mutex> lock(session->m_Guard);
	try {
		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		s_Servers[server.ID.string()] = Guild(server);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onServer]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onEditServer(SleepyDiscord::Server server) {
	try {
		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		s_Servers[server.ID.string()] = Guild(server);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onEditServer]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onDeleteServer(SleepyDiscord::UnavailableServer server) {
	try {
		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		s_Servers.erase(server.ID.string());
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onDeleteServer]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> _serverID, SleepyDiscord::ServerMember member) {
	try {
		auto &serverID = _serverID.string();

		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		auto index = s_Servers.find(serverID);

		if (index != s_Servers.end()) {
			index->second.Members[member.ID.string()] = member;
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onMember]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onEditMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user,
							std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> roles, std::string nick) {
	//std::lock_guard<std::mutex> lock(session->m_Guard);
	try {
		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		auto serverIndex = s_Servers.find(serverID.string());
		if (serverIndex == s_Servers.end()) return;

		auto &members = serverIndex->second.Members;
		auto index = members.find(user.ID.string());

		if (index != members.end()) {
			auto &member = index->second;

			member.User = user;
			member.Nick = nick;
			member.UpdateRoles(roles);
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onEditMember]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onRemoveMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> _serverID, SleepyDiscord::User user) {
	try {
		auto &serverID = _serverID.string();

		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		auto serverIndex = s_Servers.find(serverID);

		if (serverIndex != s_Servers.end()) {
			auto &members = serverIndex->second.Members;
			members.erase(user.ID.string());
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onRemoveMember]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onChannel(SleepyDiscord::Channel channel) {
	try {
		auto &serverID = channel.serverID.string();

		{
			std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
			auto index = s_Servers.find(serverID);

			if (index != s_Servers.end()) {
				index->second.Channels[channel.ID.string()] = channel;
				return;
			}
		}

		std::lock_guard<std::mutex> lck(this->session->m_OtherChannelsGuard);
		s_OtherChannels[channel.ID.string()] = channel;
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onChannel]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onEditChannel(SleepyDiscord::Channel channel) {
	try {
		auto &serverID = channel.serverID.string();

		{
			std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
			auto index = s_Servers.find(serverID);

			if (index != s_Servers.end()) {
				auto &server = index->second;
				server.Channels[channel.ID.string()] = channel;
				return;
			}
		}

		std::lock_guard<std::mutex> lck(this->session->m_OtherChannelsGuard);
		s_OtherChannels[channel.ID.string()] = channel;
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onEditChannel]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onDeleteChannel(SleepyDiscord::Channel channel) {
	try {
		auto &serverID = channel.serverID.string();

		{
			std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
			auto serverIndex = s_Servers.find(serverID);

			if (serverIndex != s_Servers.end()) {
				auto &channels = serverIndex->second.Channels;
				channels.erase(channel.ID.string());
				return;
			}
		}

		std::lock_guard<std::mutex> lck(this->session->m_OtherChannelsGuard);
		s_OtherChannels.erase(channel.ID.string());
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onDeleteChannel]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onRole(SleepyDiscord::Snowflake<SleepyDiscord::Server> _serverID, SleepyDiscord::Role role) {
	try {
		auto &serverID = _serverID.string();

		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		auto index = s_Servers.find(serverID);

		if (index != s_Servers.end()) {
			index->second.Roles[role.ID.string()] = role;
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onRole]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onEditRole(SleepyDiscord::Snowflake<SleepyDiscord::Server> _serverID, SleepyDiscord::Role role) {
	try {
		auto &serverID = _serverID.string();

		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		auto index = s_Servers.find(serverID);

		if (index != s_Servers.end()) {
			auto &server = index->second;
			server.Roles[role.ID.string()] = role;
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onEditRole]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onDeleteRole(SleepyDiscord::Snowflake<SleepyDiscord::Server> _serverID,
							SleepyDiscord::Snowflake<SleepyDiscord::Role> roleID) {
	try {
		auto &serverID = _serverID.string();

		std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
		auto serverIndex = s_Servers.find(serverID);

		if (serverIndex != s_Servers.end()) {
			auto &roles = serverIndex->second.Roles;
			roles.erase(roleID.string());
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onDeleteRole]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onEditUser(SleepyDiscord::User user) {
	try {
		auto &userID = user.ID.string();

		{
			std::lock_guard<std::mutex> lck(this->session->m_ServersGuard);
			for (auto &server : s_Servers) {
				auto memberIndex = server.second.Members.find(userID);

				if (memberIndex != server.second.Members.end()) {
					memberIndex->second.User = user;
				}
			}
		}

		std::lock_guard<std::mutex> lck(this->session->m_OtherChannelsGuard);
		for (auto &channel : s_OtherChannels) {
			auto userIndex = channel.second.Recipients.find(userID);

			if (userIndex != channel.second.Recipients.end()) {
				userIndex->second = user;
			}
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onEditUser]");
	}
}