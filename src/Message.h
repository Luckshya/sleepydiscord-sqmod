#pragma once

// ------------------------------------------------------------------------------------------------
#include <string>
#include <sleepy_discord/message.h>

// ------------------------------------------------------------------------------------------------
#include "ServerMember.h"
#include "User.h"

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {

// ------------------------------------------------------------------------------------------------
class Message {
public :
	std::string ID;
	std::string ChannelID;
	std::string ServerID;
	User Author;
	ServerMember Member;
	std::string Content;
	std::string Timestamp;
	bool isTTS = false;
	bool MentionEveryone = false;
	std::vector<User> Mentions;
	std::vector<std::string> MentionRoles;

	enum MessageType {
		DEFAULT = 0,
		RECIPIENT_ADD = 1,
		RECIPIENT_REMOVE = 2,
		CALL = 3,
		CHANNEL_NAME_CHANGE = 4,
		CHANNEL_ICON_CHANGE = 5,
		CHANNEL_PINNED_MESSAGE = 6,
		GUILD_MEMBER_JOIN = 7
	} Type = DEFAULT;

	// ------------------------------------------------------------------------------------------------
	Message() = default;

	// ------------------------------------------------------------------------------------------------
	Message(SleepyDiscord::Message &message);

	// ------------------------------------------------------------------------------------------------
	Message(const Message &o) = default;

	// ------------------------------------------------------------------------------------------------
	Message(Message &&o) = default;

	// ------------------------------------------------------------------------------------------------
	Message &operator=(const Message &o) = default;

	// ------------------------------------------------------------------------------------------------
	Message &operator=(Message &&o) = default;

	// ------------------------------------------------------------------------------------------------
	~Message() = default;

	// ------------------------------------------------------------------------------------------------
	void UpdateUserMentions(std::vector<SleepyDiscord::User> &mentions);

	// ------------------------------------------------------------------------------------------------
	void UpdateRoleMentions(std::vector<SleepyDiscord::Snowflake<SleepyDiscord::User>> &mentions);

	// ------------------------------------------------------------------------------------------------
	std::string GetID() const;

	// ------------------------------------------------------------------------------------------------
	SQInteger GetType() const;

	// ------------------------------------------------------------------------------------------------
	std::string GetChannelID() const;

	// ------------------------------------------------------------------------------------------------
	std::string GetServerID() const;

	// ------------------------------------------------------------------------------------------------
	LightObj GetAuthor();

	// ------------------------------------------------------------------------------------------------
	LightObj GetMember();

	// ------------------------------------------------------------------------------------------------
	std::string GetContent() const;

	// ------------------------------------------------------------------------------------------------
	std::string GetTimestamp() const;

	// ------------------------------------------------------------------------------------------------
	bool IsTTS() const;

	// ------------------------------------------------------------------------------------------------
	bool IsMentionEveryone() const;

	// ------------------------------------------------------------------------------------------------
	SQInteger GetUserMentionCount() const;

	// ------------------------------------------------------------------------------------------------
	Array GetUserMentions();

	// ------------------------------------------------------------------------------------------------
	SQInteger GetRoleMentionCount() const;

	// ------------------------------------------------------------------------------------------------
	Array GetRoleMentions();

	// ------------------------------------------------------------------------------------------------
	static void Register_Message(Table &table);
};
}