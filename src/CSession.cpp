// ------------------------------------------------------------------------------------------------
#include "CSession.h"

// ------------------------------------------------------------------------------------------------
#include <utility>
#include "CDiscord.h"
#include "DEmbed.h"
#include "EventHandler.h"

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {
// variable to hold in case on one connection only
CSession *s_Session = nullptr;

// container to hold connections
std::vector<CSession *> s_Sessions;

// ------------------------------------------------------------------------------------------------
CSession::CSession() {
	try {
		if (!s_Session && s_Sessions.empty()) {
			s_Session = this;
		}
			// Is this the second session instance?
		else if (s_Sessions.empty()) {
			s_Sessions.push_back(s_Session);
			s_Session = nullptr;
			s_Sessions.push_back(this);
		}
			// This is part of multiple session instances
		else {
			s_Sessions.push_back(this);
		}

		s_EventHandler = new EventHandler();
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [CSession]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Process() {
	// Do we only have one Discord session?
	if (s_Session) {
		s_Session->Update();
	}
		// Do we have multiple sessions?
	else if (!s_Sessions.empty()) {
		for (auto &session : s_Sessions) {
			session->Update();
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Update() {
	if (!client) {
		return;
	}

	if (!client->session) {
		return;
	}

	if (!isConnected) {
		return;
	}

	if (internalCacheEnabled) {
		while (Servers_Queue.try_dequeue(LatestCopy_Servers)) {};
		while (OtherChannels_Queue.try_dequeue(LatestCopy_OtherChannels)) {};
	}

	s_EventHandler->PushEvents(this);
}

// ------------------------------------------------------------------------------------------------
void CSession::Release() {
	m_OnReady.Release();
	m_OnMessage.Release();
	m_OnError.Release();
	m_OnDisconnect.Release();
	m_OnQuit.Release();
	m_OnServer.Release();
	m_OnServerEdit.Release();
	m_OnServerDelete.Release();
	m_OnMember.Release();
	m_OnMemberEdit.Release();
	m_OnMemberRemove.Release();
	m_OnChannel.Release();
	m_OnChannelEdit.Release();
	m_OnChannelDelete.Release();
	m_OnRole.Release();
	m_OnRoleEdit.Release();
	m_OnRoleDelete.Release();
	m_OnUserEdit.Release();
	//std::cout << "Script resources released.\n";
}

// ------------------------------------------------------------------------------------------------
Function &CSession::GetEvent(int evid) {
	switch (evid) {
		case ON_READY:
			return m_OnReady;
		case ON_MESSAGE:
			return m_OnMessage;
		case ON_ERROR:
			return m_OnError;
		case ON_DISCONNECT:
			return m_OnDisconnect;
		case ON_QUIT:
			return m_OnQuit;
		case ON_SERVER:
			return m_OnServer;
		case ON_SERVER_EDIT:
			return m_OnServerEdit;
		case ON_SERVER_DELETE:
			return m_OnServerDelete;
		case ON_MEMBER:
			return m_OnMember;
		case ON_MEMBER_EDIT:
			return m_OnMemberEdit;
		case ON_MEMBER_REMOVE:
			return m_OnMemberRemove;
		case ON_CHANNEL:
			return m_OnChannel;
		case ON_CHANNEL_EDIT:
			return m_OnChannelEdit;
		case ON_CHANNEL_DELETE:
			return m_OnChannelDelete;
		case ON_ROLE:
			return m_OnRole;
		case ON_ROLE_EDIT:
			return m_OnRoleEdit;
		case ON_ROLE_DELETE:
			return m_OnRoleDelete;
		case ON_USER_EDIT:
			return m_OnUserEdit;
		default:
			break;
	}

	return NullFunction();
}

// ------------------------------------------------------------------------------------------------
void CSession::Connect(CCStr token) {
	if (isConnecting) {
		STHROWF("Already trying to connect or disconnect");
	}

	if (isConnected) {
		STHROWF("Already connected");
	}

	try {
		// initialize the connection
		this->sleepyThread = new std::thread(&CSession::runSleepy, this, std::string(token));
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [Connect]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::runSleepy(std::string token) {
	try {
		{
			std::lock_guard<std::mutex> lock(m_Guard);

			this->isConnecting = true;
			this->client = new CDiscord(std::move(token));
			this->client->session = this;
		}
		//std::cout << "Unlocked A\n";

		client->run();
	}
	catch (...) {
		OutputError("An Error has occured at [CSession] function => [runSleepy]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::BindEvent(int evid, Object &env, Function &func) {
	Function &event = GetEvent(evid);

	if (func.IsNull()) {
		event.Release();
	} else if (env.IsNull()) {
		event = func;
	} else {
		event = Function(env, func.GetFunc(), SqVM());
	}
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::Message(HSQUIRRELVM vm) {
	SQInteger top = sq_gettop(vm);

	if (top <= 1) {
		return sq_throwerror(vm, "Missing the channel ID value");
	} else if (top <= 2) {
		return sq_throwerror(vm, "Missing the message value");
	}

	CSession *session = nullptr;

	try {
		session = Var<CSession *>(vm, 1).value;
	}
	catch (const Sqrat::Exception &e) {
		return sq_throwerror(vm, e.what());
	}

	if (!session) {
		return sq_throwerror(vm, "Invalid session instance");
	} else if (!session->client) {
		return sq_throwerror(vm, "Invalid Discord client");
	} else if (!session->isConnected) {
		return sq_throwerror(vm, "Session is not connected");
	}

	StackStrF channelID(vm, 2);
	if (SQ_FAILED(channelID.Proc(false))) {
		return channelID.mRes; // Propagate the error!
	}

	StackStrF message(vm, 3);
	if (SQ_FAILED(message.Proc(false))) {
		return message.mRes; // Propagate the error!
	}

	try {
		auto msg = session->client->sendMessage(channelID.mPtr, message.mPtr, SleepyDiscord::Async);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [Message]");
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::MessageEmbed(HSQUIRRELVM vm) {
	SQInteger top = sq_gettop(vm);

	if (top <= 1) {
		return sq_throwerror(vm, "Missing the channel ID value");
	} else if (top <= 2) {
		return sq_throwerror(vm, "Missing the content value");
	} else if (top <= 3) {
		return sq_throwerror(vm, "Missing the Embed value");
	}

	CSession *session = nullptr;

	try {
		session = Sqrat::Var<CSession *>(vm, 1).value;
	}
	catch (const Sqrat::Exception &e) {
		return sq_throwerror(vm, e.what());
	}

	if (!session) {
		return sq_throwerror(vm, "Invalid session instance");
	} else if (!session->client) {
		return sq_throwerror(vm, "Invalid Discord client");
	} else if (!session->isConnected) {
		return sq_throwerror(vm, "Session is not connected");
	}

	StackStrF channelID(vm, 2);
	if (SQ_FAILED(channelID.Proc(false))) {
		return channelID.mRes; // Propagate the error!
	}

	StackStrF content(vm, 3);
	if (SQ_FAILED(content.Proc(false))) {
		return content.mRes; // Propagate the error!
	}

	Embed *embed = nullptr;
	try {
		embed = Sqrat::Var<Embed *>(vm, 4).value;
	}
	catch (const Sqrat::Exception &e) {
		return sq_throwerror(vm, e.what());
	}

	if (!embed) {
		return sq_throwerror(vm, "Invalid Embed instance");
	}

	try {
		auto msg = session->client->sendMessage(channelID.mPtr, "", *(embed->embed), SleepyDiscord::TTS::Default, SleepyDiscord::Async);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [Message]");
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::EditChannel(HSQUIRRELVM vm) {
	SQInteger top = sq_gettop(vm);

	if (top <= 1) {
		return sq_throwerror(vm, "Missing the channel ID value");
	} else if (top <= 2) {
		return sq_throwerror(vm, "Missing the channel name value");
	} else if (top <= 3) {
		return sq_throwerror(vm, "Missing the channel topic value");
	}

	CSession *session = nullptr;

	try {
		session = Sqrat::Var<CSession *>(vm, 1).value;
	}
	catch (const Sqrat::Exception &e) {
		return sq_throwerror(vm, e.what());
	}

	if (!session) {
		return sq_throwerror(vm, "Invalid session instance");
	} else if (!session->client) {
		return sq_throwerror(vm, "Invalid Discord client");
	} else if (!session->isConnected) {
		return sq_throwerror(vm, "Session is not connected");
	}

	StackStrF channelID(vm, 2);
	if (SQ_FAILED(channelID.Proc(false))) {
		return channelID.mRes; // Propagate the error!
	}

	StackStrF name(vm, 3);
	if (SQ_FAILED(name.Proc(false))) {
		return name.mRes; // Propagate the error!
	}

	StackStrF topic(vm, 4);
	if (SQ_FAILED(topic.Proc(false))) {
		return topic.mRes; // Propagate the error!
	}

	try {
		auto response = session->client->editChannel(std::string(channelID.mPtr), std::string(name.mPtr),
													 std::string(topic.mPtr), SleepyDiscord::Async);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [EditChannel]");
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::SetActivity(HSQUIRRELVM vm) {
	SQInteger top = sq_gettop(vm);

	if (top <= 1) {
		return sq_throwerror(vm, "Missing the activity value");
	}

	CSession *session = nullptr;

	try {
		session = Sqrat::Var<CSession *>(vm, 1).value;
	}
	catch (const Sqrat::Exception &e) {
		return sq_throwerror(vm, e.what());
	}

	if (!session) {
		return sq_throwerror(vm, "Invalid session instance");
	} else if (!session->client) {
		return sq_throwerror(vm, "Invalid Discord session");
	} else if (!session->isConnected) {
		return sq_throwerror(vm, "Session is not connected");
	}

	StackStrF activity(vm, 2);
	if (SQ_FAILED(activity.Proc(false))) {
		return activity.mRes; // Propagate the error!
	}

	try {
		session->client->updateStatus(std::string(activity.mPtr));
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [SetActivity]");
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::GetRoleName(HSQUIRRELVM vm) {
	SQInteger top = sq_gettop(vm);

	if (top <= 1) {
		return sq_throwerror(vm, "Missing the server ID value");
	}
	else if (top <= 2) {
		return sq_throwerror(vm, "Missing the role ID value");
	}

	CSession *session = nullptr;

	try {
		session = Sqrat::Var<CSession *>(vm, 1).value;
	}
	catch (const Sqrat::Exception &e) {
		return sq_throwerror(vm, e.what());
	}

	if (!session) {
		return sq_throwerror(vm, "Invalid session instance");
	}
	else if (!session->client) {
		return sq_throwerror(vm, "Invalid Discord client");
	}
	else if (!session->isConnected) {
		return sq_throwerror(vm, "Session is not connected");
	}

	StackStrF serverID(vm, 2);
	if (SQ_FAILED(serverID.Proc(false))) {
		return serverID.mRes; // Propagate the error!
	}

	StackStrF roleID(vm, 3);
	if (SQ_FAILED(roleID.Proc(false))) {
		return roleID.mRes; // Propagate the error!
	}

	try {
		std::string s_serverID = std::string(serverID.mPtr);

		auto rolesIndex = session->LatestCopy_Servers.find(std::string(serverID.mPtr));

		if (rolesIndex == session->LatestCopy_Servers.end()) {
			sq_pushnull(vm);
			return 1;
		}

		auto &roles = (rolesIndex->second).Roles;
		bool found = false;
		CCStr role_name = nullptr;

		for (const auto &role : roles) {
			if (role.second.ID == std::string(roleID.mPtr)) {
				found = true;
				role_name = role.second.Name.c_str();
				sq_pushstring(vm, role_name, -1);
				break;
			}
		}

		if (!found) {
			return sq_throwerror(vm, "Invalid role ID");
		}
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [RoleName]");
	}

	return 1;
}

// ------------------------------------------------------------------------------------------------
bool CSession::GetErrorEventEnabled() const {
	return errorEventEnabled;
}

// ------------------------------------------------------------------------------------------------
void CSession::SetErrorEventEnabled(bool toggle) {
	errorEventEnabled = toggle;
}

// ------------------------------------------------------------------------------------------------
bool CSession::GetInternalCacheEnabled() const {
	return internalCacheEnabled;
}

// ------------------------------------------------------------------------------------------------
void CSession::SetInternalCacheEnabled(bool toggle) {
	if (isConnected || isConnecting) {
		STHROWF("Cannot toggle internal cache while already connected or connecting");
		return;
	}

	internalCacheEnabled = toggle;
}

// ------------------------------------------------------------------------------------------------
void CSession::Disconnect() {
	//std::cout << "Disconnecting\n";
	try {
		// Update events last time
		Update();

		std::lock_guard<std::mutex> lock(m_Guard);

		if (client != nullptr) {
			client->quit();

			if (sleepyThread != nullptr) {
				sleepyThread->join();
				delete sleepyThread;
				sleepyThread = nullptr;
			}
		}

		isConnected = false;
		isConnecting = false;

		delete client;
		client = nullptr;
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [Disconnect]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Destroy() {

	// Disconnect the session
	Disconnect();

	// Release script resources
	Release();

	delete s_EventHandler;
	s_EventHandler = nullptr;
}

// ------------------------------------------------------------------------------------------------
CSession::~CSession() {
	//std::cout << "Destructor\n";
	Destroy();

	// Attempt to find our self in the session pool
	auto itr = std::find(s_Sessions.begin(), s_Sessions.end(), this);
	// Are we in the pool?
	if (itr != s_Sessions.end()) {
		s_Sessions.erase(itr); /* Remove our self from the pool. */
	}
	// Is there a single session and that's us?
	if (s_Session == this) {
		s_Session = nullptr;
	}

	//std::cout << "Destructor Done\n";
}

// ------------------------------------------------------------------------------------------------
void CSession::Terminate() {
	// Do we only have one Discord session?
	if (s_Session) {
		s_Session->Destroy(); /* This should do the job. */
	}
		// Do we have multiple sessions?
	else if (!s_Sessions.empty()) {
		for (auto &session : s_Sessions) {
			session->Destroy();
		}
	}
}

// ------------------------------------------------------------------------------------------------
LightObj CSession::GetGuild(const std::string &serverID) {
	auto serverIndex = LatestCopy_Servers.find(serverID);

	if (serverIndex == LatestCopy_Servers.end()) {
		return LightObj{};
	}

	return LightObj(SqTypeIdentity<Guild>{}, SqVM(), serverIndex->second);
}

// ------------------------------------------------------------------------------------------------
LightObj CSession::GetOtherChannel(const std::string &channelID) {
	auto channelIndex = LatestCopy_OtherChannels.find(channelID);

	if (channelIndex == LatestCopy_OtherChannels.end()) {
		return LightObj{};
	}

	return LightObj(SqTypeIdentity<Channel>{}, SqVM(), channelIndex->second);
}

// ------------------------------------------------------------------------------------------------
void CSession::DRegister_CSession(Table &discordcn) {
	using namespace Sqrat;

	discordcn.Bind("Session",
				   Class < CSession, NoCopy < CSession > > (discordcn.GetVM(), "Session")

					.Ctor()

					.Prop("ErrorEventEnabled", &CSession::GetErrorEventEnabled, &CSession::SetErrorEventEnabled)
					.Prop("InternalCacheEnabled", &CSession::GetInternalCacheEnabled, &CSession::SetInternalCacheEnabled)

					.Func("Connect", &CSession::Connect)
					.Func("Bind", &CSession::BindEvent)
					.Func("Disconnect", &CSession::Disconnect)
					.Func("GetGuild", &CSession::GetGuild)
					.Func("GetOtherChannel", &CSession::GetOtherChannel)

					.SquirrelFunc("Message", &CSession::Message)
					.SquirrelFunc("MessageEmbed", &CSession::MessageEmbed)
					.SquirrelFunc("GetRoleName", &CSession::GetRoleName)
					.SquirrelFunc("EditChannel", &CSession::EditChannel)
					.SquirrelFunc("SetActivity", &CSession::SetActivity)
	);
}
} //Namespace:: SqDiscord
