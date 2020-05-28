#pragma once

// ------------------------------------------------------------------------------------------------
#include <vector>
#include <string>
#include <sleepy_discord/channel.h>

// ------------------------------------------------------------------------------------------------
#include "User.h"
#include "Common.hpp"

// ------------------------------------------------------------------------------------------------
using namespace Sqrat;

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {

// ------------------------------------------------------------------------------------------------
class Channel {
public:
	std::string ID;
	std::string ServerID;
	int Position = 0;
	std::string Name;
	std::string Topic;
	bool isNSFW = false;
	std::unordered_map<std::string, User> Recipients;
	std::string OwnerID;
	std::string ParentID;

	enum ChannelType {
		CHANNEL_TYPE_NONE = -1,
		SERVER_TEXT = 0,
		DM = 1,
		SERVER_VOICE = 2,
		GROUP_DM = 3,
		SERVER_CATEGORY = 4,
		GUILD_NEWS = 5,
		GUILD_STORE = 6
	} Type = CHANNEL_TYPE_NONE;

	// ------------------------------------------------------------------------------------------------
	Channel() = default;

	// ------------------------------------------------------------------------------------------------
	Channel(SleepyDiscord::Channel &channel);

	// ------------------------------------------------------------------------------------------------
	Channel(const Channel &o) = default;

	// ------------------------------------------------------------------------------------------------
	Channel(Channel &&o) = default;

	// ------------------------------------------------------------------------------------------------
	Channel &operator=(const Channel &o) = default;

	// ------------------------------------------------------------------------------------------------
	Channel &operator=(Channel &&o) = default;

	// ------------------------------------------------------------------------------------------------
	~Channel() = default;

	// ------------------------------------------------------------------------------------------------
	void UpdateRecipients(std::vector<SleepyDiscord::User> &recipients);

	// ------------------------------------------------------------------------------------------------
	std::string GetID() const;

	// ------------------------------------------------------------------------------------------------
	SQInteger GetType() const;

	// ------------------------------------------------------------------------------------------------
	std::string GetServerID() const;

	// ------------------------------------------------------------------------------------------------
	int GetPosition() const;

	// ------------------------------------------------------------------------------------------------
	std::string GetName() const;

	// ------------------------------------------------------------------------------------------------
	std::string GetTopic() const;

	// ------------------------------------------------------------------------------------------------
	bool IsNsfw() const;

	SQInteger GetRecipientCount() const;

	// ------------------------------------------------------------------------------------------------
	Array GetRecipientsArray();

	// ------------------------------------------------------------------------------------------------
	Table GetRecipientsTable();

	// ------------------------------------------------------------------------------------------------
	std::string GetOwnerId() const;

	// ------------------------------------------------------------------------------------------------
	std::string GetParentId() const;

	// ------------------------------------------------------------------------------------------------
	static void Register_Channel(Table &table);
};
} // Namespace:: SqDiscord