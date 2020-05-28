#include "Guild.h"

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {

// ------------------------------------------------------------------------------------------------
Guild::Guild(SleepyDiscord::Server &server) {
	ID = server.ID;
	Name = server.name;
	OwnerID = server.ownerID;
	Region = server.region;
	UpdateRoles(server.roles);
	UpdateChannels(server.channels);
	UpdateMembers(server.members);
}

// ------------------------------------------------------------------------------------------------
void Guild::UpdateRoles(std::list<SleepyDiscord::Role> &roles) {
	for (auto &role : roles) {
		Roles[role.ID.string()] = role;
	}
}

// ------------------------------------------------------------------------------------------------
void Guild::UpdateMembers(std::list<SleepyDiscord::ServerMember> &members) {
	for (auto &member : members) {
		Members[member.ID.string()] = member;
	}
}

// ------------------------------------------------------------------------------------------------
void Guild::UpdateChannels(std::list<SleepyDiscord::Channel> &channels) {
	for (auto &channel : channels) {
		Channels[channel.ID.string()] = channel;
	}
}

// ------------------------------------------------------------------------------------------------
std::string Guild::GetID() const {
	return ID;
}

// ------------------------------------------------------------------------------------------------
std::string Guild::GetName() const {
	return Name;
}

// ------------------------------------------------------------------------------------------------
std::string Guild::GetOwnerId() const {
	return OwnerID;
}


// ------------------------------------------------------------------------------------------------
std::string Guild::GetRegion() const {
	return Region;
}

// ------------------------------------------------------------------------------------------------
SQInteger Guild::GetRoleCount() const {
	return Roles.size();
}

// ------------------------------------------------------------------------------------------------
Array Guild::GetRolesArray() {
	if (Roles.empty()) {
		return Array(SqVM(), 0);
	}

	Array arr(SqVM(), Roles.size());

	SQInteger idx = 0;

	for (auto &role : Roles) {
		arr.SetValue(idx, role.second);

		++idx;
	}

	return arr;
}

// ------------------------------------------------------------------------------------------------
Table Guild::GetRolesTable() {
	if (Roles.empty()) {
		return Table();
	}

	Table tbl(SqVM(), Roles.size());

	for (auto &role : Roles) {
		tbl.SetValue(role.first.c_str(), role.second);
	}

	return tbl;
}

// ------------------------------------------------------------------------------------------------
SQInteger Guild::GetMemberCount() const {
	return Members.size();
}

// ------------------------------------------------------------------------------------------------
Array Guild::GetMembersArray() {
	if (Members.empty()) {
		return Array(SqVM(), 0);
	}

	Array arr(SqVM(), Members.size());

	SQInteger idx = 0;

	for (auto &member : Members) {
		arr.SetValue(idx, member.second);

		++idx;
	}

	return arr;
}

// ------------------------------------------------------------------------------------------------
Table Guild::GetMembersTable() {
	if (Members.empty()) {
		return Table();
	}

	Table tbl(SqVM(), Members.size());

	for (auto &member : Members) {
		tbl.SetValue(member.first.c_str(), member.second);
	}

	return tbl;
}

// ------------------------------------------------------------------------------------------------
SQInteger Guild::GetChannelCount() const {
	return Channels.size();
}

// ------------------------------------------------------------------------------------------------
Array Guild::GetChannelsArray() {
	if (Channels.empty()) {
		return Array(SqVM(), 0);
	}

	Array arr(SqVM(), Channels.size());

	SQInteger idx = 0;

	for (auto &channel : Channels) {
		arr.SetValue(idx, channel.second);

		++idx;
	}

	return arr;
}

// ------------------------------------------------------------------------------------------------
Table Guild::GetChannelsTable() {
	if (Channels.empty()) {
		return Table();
	}

	Table tbl(SqVM(), Channels.size());

	for (auto &channel : Channels) {
		tbl.SetValue(channel.first.c_str(), channel.second);
	}

	return tbl;
}

// ------------------------------------------------------------------------------------------------
void Guild::Register_Guild(Table &table) {
	table.Bind("Guild",
			   Class<Guild>(table.GetVM(), "Guild")

					   .Ctor()
					   .Ctor<const Guild &>()

					   .Prop("ID", &Guild::GetID)
					   .Prop("Name", &Guild::GetName)
					   .Prop("OwnerID", &Guild::GetOwnerId)
					   .Prop("Region", &Guild::GetRegion)
					   .Prop("RoleCount", &Guild::GetRoleCount)
					   .Prop("RolesArray", &Guild::GetRolesArray)
					   .Prop("RolesTable", &Guild::GetRolesTable)
					   .Prop("MemberCount", &Guild::GetMemberCount)
					   .Prop("MembersArray", &Guild::GetMembersArray)
					   .Prop("MembersTable", &Guild::GetMembersTable)
					   .Prop("ChannelCount", &Guild::GetChannelCount)
					   .Prop("ChannelsArray", &Guild::GetChannelsArray)
					   .Prop("ChannelsTable", &Guild::GetChannelsTable)
	);
}
} // Namespace:: SqDiscord