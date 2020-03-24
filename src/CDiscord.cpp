// ------------------------------------------------------------------------------------------------
#include "CDiscord.h"

// ------------------------------------------------------------------------------------------------
#include <list>
#include <mutex>

// ------------------------------------------------------------------------------------------------
#include "Common.hpp"

// ------------------------------------------------------------------------------------------------
// guard to lock while disconnecting
std::mutex m_Guard;

using namespace SqDiscord;

// ------------------------------------------------------------------------------------------------
void CDiscord::onServer(SleepyDiscord::Server server) {
	std::lock_guard<std::mutex> lock(m_Guard);
	try {
		s_servers[server.ID.string()] = server;
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onServer]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onMessage(SleepyDiscord::Message message) {
	std::lock_guard<std::mutex> lock(m_Guard);
	try {
		std::string author_nick;
		SleepyDiscord::Server server;
		std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> roles;

		bool isDM = false;

		SleepyDiscord::Channel chan = this->getChannel(message.channelID);

		SleepyDiscord::ObjectResponse<SleepyDiscord::Channel> getChannelResponse = this->getChannel(message.channelID);
		if (getChannelResponse.error())
		{
			std::cout << getChannelResponse.text;
			return;
		}

		if (chan.type == SleepyDiscord::Channel::ChannelType::DM || chan.type == SleepyDiscord::Channel::ChannelType::GROUP_DM) {
			isDM = true;
		}

		if (!isDM) {
			server = s_servers.at(message.serverID.string());
			for (SleepyDiscord::ServerMember member : server.members) {
				if (member.user.ID.number() == message.author.ID.number()) {
					roles = member.roles;
					break;
				}
			}

			author_nick = server.findMember(message.author.ID)->nick;
		}

		std::lock_guard<std::mutex> guard(this->session->m_MsgGuard);
		this->session->s_Messages.emplace_back(message.channelID.string(), message.author.username, author_nick, message.author.ID.string(), roles, message.content);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onMessage]");
	}
}

// ------------------------------------------------------------------------------------------------
void CDiscord::onReady(SleepyDiscord::Ready readyData) {
	std::lock_guard<std::mutex> lock(m_Guard);
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
void CDiscord::onEditMember(SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID, SleepyDiscord::User user, std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> nroles, std::string nnick) {
	std::lock_guard<std::mutex> lock(m_Guard);
	try {
		std::list<SleepyDiscord::ServerMember> & members = s_servers.at(serverID).members;

		for (std::list<SleepyDiscord::ServerMember>::iterator it = members.begin(); it != members.end(); ++it) {
			if ((*it).ID.number() == user.ID.number()) {
				(*it).roles = nroles;
				(*it).nick = nnick;
				break;
			}
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CDiscord] function => [onEditMember]");
	}
}
