#include "ServerMember.h"

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {

// ------------------------------------------------------------------------------------------------
ServerMember::ServerMember(SleepyDiscord::ServerMember &member) {
	ID = member.ID;
	User = DUser(member.user);
	Nick = member.nick;
	JoinedAt = member.joinedAt;
	isDeaf = member.deaf;
	isMute = member.mute;

	UpdateRoles(member.roles);
}

// ------------------------------------------------------------------------------------------------
void ServerMember::UpdateRoles(std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> &roles) {
	for (auto &role : roles) {
		Roles.push_back(role.string());
	}
}

// ------------------------------------------------------------------------------------------------
std::string ServerMember::GetID() const {
	return ID;
}

// ------------------------------------------------------------------------------------------------
LightObj ServerMember::GetUser() {
	if (User.ID.empty()) {
		return LightObj{};
	}

	return LightObj(SqTypeIdentity<DUser>{}, SqVM(), User);
}

// ------------------------------------------------------------------------------------------------
std::string ServerMember::GetNick() const {
	return Nick;
}

// ------------------------------------------------------------------------------------------------
SQInteger ServerMember::GetRoleCount() const {
	return Roles.size();
}

// ------------------------------------------------------------------------------------------------
Array ServerMember::GetRoles() {
	if (Roles.empty()) {
		return Array(SqVM(), 0);
	}

	Array arr(SqVM(), Roles.size());

	SQInteger idx = 0;

	for (auto &role : Roles) {
		arr.SetValue(idx, role);

		++idx;
	}

	return arr;
}

// ------------------------------------------------------------------------------------------------
std::string ServerMember::GetJoinedAt() const {
	return JoinedAt;
}

// ------------------------------------------------------------------------------------------------
bool ServerMember::IsDeaf() const {
	return isDeaf;
}

// ------------------------------------------------------------------------------------------------
bool ServerMember::IsMute() const {
	return isMute;
}

// ------------------------------------------------------------------------------------------------
void ServerMember::Register_ServerMember(Table &table) {
	table.Bind("ServerMember",
			   Class<ServerMember>(table.GetVM(), "ServerMember")

					   .Ctor()
					   .Ctor<const ServerMember &>()

					   .Prop("ID", &ServerMember::GetID)
					   .Prop("User", &ServerMember::GetUser)
					   .Prop("Nick", &ServerMember::GetNick)
					   .Prop("RoleCount", &ServerMember::GetRoleCount)
					   .Prop("Roles", &ServerMember::GetRoles)
					   .Prop("JoinedAt", &ServerMember::GetJoinedAt)
					   .Prop("IsDeaf", &ServerMember::IsDeaf)
					   .Prop("IsMute", &ServerMember::IsMute)
	);
}
} //Namespace:: SqDiscord