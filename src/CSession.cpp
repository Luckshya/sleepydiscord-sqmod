// ------------------------------------------------------------------------------------------------
#include "CSession.h"

// ------------------------------------------------------------------------------------------------
#include "CDiscord.h"
#include "Common.hpp"
#include "DEmbed.h"

using namespace SqMod;

// ------------------------------------------------------------------------------------------------
namespace SqDiscord
{
// variable to hold in case on one connection only
CSession * s_Session = nullptr;

// container to hold connections
std::vector< CSession* > s_Sessions;

// ------------------------------------------------------------------------------------------------
CSession::CSession()
{
	try
	{
		token = NULL;
		client = nullptr;

		if (!s_Session && s_Sessions.empty())
		{
			s_Session = this;
		}
		// Is this the second session instance?
		else if (s_Sessions.empty())
		{
			s_Sessions.push_back(s_Session);
			s_Session = nullptr;
			s_Sessions.push_back(this);
		}
		// This is part of multiple session instances
		else
		{
			s_Sessions.push_back(this);
		}
	}
	catch (...)
	{
		SqMod_LogErr("An Error has occured at [CSession] function => [CSession]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Process()
{
	// Do we only have one Discord session?
	if (s_Session)
	{
		s_Session->Update();
	}
	// Do we have multiple sessions?
	else if (!s_Sessions.empty())
	{
		for (auto itr = s_Sessions.begin(); itr != s_Sessions.end(); ++itr)
		{
			(*itr)->Update();
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Update()
{
	if (!client)
	{
		return;
	}

	if (!client->session)
	{
		return;
	}

	if (!isConnected)
	{
		return;
	}

	if (!s_ReadySession.empty())
	{
		std::lock_guard<std::mutex> lockA(m_ReadyGuard);

		for (auto itr = s_ReadySession.begin(); itr != s_ReadySession.end(); ++itr)
		{
			if ((*itr) != nullptr && (*itr)->client != nullptr)
			{
				OnReady();
			}
		}

		s_ReadySession.clear();
	}

	if (!s_Messages.empty())
	{
		std::lock_guard<std::mutex> lockB(m_MsgGuard);

		for (auto itr = s_Messages.begin(); itr != s_Messages.end(); ++itr)
		{
			OnMessage((itr->channelID).c_str(), (itr->author).c_str(), (itr->authorNick).c_str(), (itr->authorID).c_str(), itr->roles, (itr->message).c_str());
		}

		s_Messages.clear();
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Release()
{
	m_OnReady.ReleaseGently();
	m_OnMessage.ReleaseGently();
}

// ------------------------------------------------------------------------------------------------
Function& CSession::GetEvent(int evid) {
	switch (evid) {
		case ON_READY:			return m_OnReady;
		case ON_MESSAGE:		return m_OnMessage;

		default: break;
	}

	return NullFunction();
}

// ------------------------------------------------------------------------------------------------
void CSession::Connect(CCStr token)
{
	if (isConnecting) {
		STHROWF("Already trying to connect or disconnect");
	}

	if (isConnected) {
		STHROWF("Already connected");
	}

	try {
		// initialize the connection
		this->client = new CDiscord(token);
		this->isConnecting = true;
		this->token = token;
		this->sleepyThread = new std::thread(&CSession::runSleepy, this);
		this->client->session = this;
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [Connect]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::runSleepy() {
	try {
		client->run();
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [runSleepy]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::BindEvent(int evid, Object & env, Function & func)
{
	Function & event = GetEvent(evid);
	
	if (func.IsNull()) {
		event.ReleaseGently();
	}
	
	else if (env.IsNull()) {
		event = func;
	}
	
	else {
		event = Function(DefaultVM::Get(), env, func.GetFunc());
	}
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::Message(HSQUIRRELVM vm) {
	const int top = sq_gettop(vm);
	
	if (top <= 1)
	{
		return sq_throwerror(vm, "Missing the channel ID value");
	}

	else if (top <= 2)
	{
		return sq_throwerror(vm, "Missing the message value");
	}

	CSession * session = nullptr;

	try {
		session = Var< CSession * >(vm, 1).value;
	}
	catch (const Sqrat::Exception& e) {
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

	StackStrF channelID(vm, 2);
	if (SQ_FAILED(channelID.Proc(false)))
	{
		return channelID.mRes; // Propagate the error!
	}

	StackStrF message(vm, 3);
	if (SQ_FAILED(message.Proc(true)))
	{
		return message.mRes; // Propagate the error!
	}

	else if (std::string(message.mPtr).empty()) {
		return sq_throwerror(vm, "Trying to send an empty message");
	}

	try {
		auto msg = session->client->sendMessage(channelID.mPtr, message.mPtr);
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [Message]");
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::MessageEmbed(HSQUIRRELVM vm) {
	const int top = sq_gettop(vm);

	if (top <= 1)
	{
		return sq_throwerror(vm, "Missing the channel ID value");
	}

	else if (top <= 2)
	{
		return sq_throwerror(vm, "Missing the embed value");
	}

	CSession * session = nullptr;

	try {
		session = Sqrat::Var< CSession * >(vm, 1).value;
	}
	catch (const Sqrat::Exception& e) {
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

	StackStrF channelID(vm, 2);
	if (SQ_FAILED(channelID.Proc(false)))
	{
		return channelID.mRes; // Propagate the error!
	}

	Embed * embed = nullptr;
	try {
		embed = Sqrat::Var< Embed * >(vm, 3).value;
	}
	catch (const Sqrat::Exception& e) {
		return sq_throwerror(vm, e.what());
	}

	if (!embed) {
		return sq_throwerror(vm, "Invalid Embed instance");
	}

	try {
		auto msg = session->client->sendMessage(channelID.mPtr, "", *(embed->embed));
	}
	catch (...) {
		SqMod_LogErr("An Error has occured at [CSession] function => [Message]");
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
SQInteger CSession::GetRoleName(HSQUIRRELVM vm) {
	const int top = sq_gettop(vm);

	if (top <= 1)
	{
		return sq_throwerror(vm, "Missing the server ID value");
	}

	else if (top <= 2)
	{
		return sq_throwerror(vm, "Missing the role ID value");
	}

	CSession * session = nullptr;

	try {
		session = Sqrat::Var< CSession * >(vm, 1).value;
	}
	catch (const Sqrat::Exception& e) {
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
	if (SQ_FAILED(serverID.Proc(false)))
	{
		return serverID.mRes; // Propagate the error!
	}

	StackStrF roleID(vm, 3);
	if (SQ_FAILED(roleID.Proc(false)))
	{
		return roleID.mRes; // Propagate the error!
	}

	try {
		std::list<SleepyDiscord::Role> roles = session->client->s_servers.at(std::string(serverID.mPtr)).roles;

		bool found = false;
		CCStr role_name = NULL;

		for (SleepyDiscord::Role role : roles) {
			if (role.ID.string() == std::string(roleID.mPtr)) {
				found = true;
				role_name = role.name.c_str();
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
SQInteger CSession::SetActivity(HSQUIRRELVM vm) {
	const int top = sq_gettop(vm);

	if (top <= 1)
	{
		return sq_throwerror(vm, "Missing the activity value");
	}

	CSession * session = nullptr;

	try {
		session = Sqrat::Var< CSession * >(vm, 1).value;
	}
	catch (const Sqrat::Exception& e) {
		return sq_throwerror(vm, e.what());
	}

	if (!session) {
		return sq_throwerror(vm, "Invalid session instance");
	}

	else if (!session->client) {
		return sq_throwerror(vm, "Invalid Discord session");
	}

	else if (!session->isConnected) {
		return sq_throwerror(vm, "Session is not connected");
	}

	StackStrF activity(vm, 2);
	if (SQ_FAILED(activity.Proc(false)))
	{
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
void CSession::OnReady() {
	Function & listener = m_OnReady;

	if (listener.IsNull()) {
		listener.ReleaseGently();
		return;
	}

	try {
		listener.Execute();
	}
	catch (Sqrat::Exception& e) {
		SqMod_LogErr("Discord event [%s] => Squirrel error [%s]", "READY", e.what());
	}
	catch (const std::exception & e)
	{
		SqMod_LogErr("Discord event [%s] => Program error [%s]", "READY", e.what());
	}
	catch (...)
	{
		SqMod_LogErr("Discord event [%s] => Unknown error", "READY");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::OnMessage(CCStr channelID, CCStr author, CCStr authorNick, CCStr authorID, std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>> roles, CCStr message) {
	Function & listener = m_OnMessage;

	Array rolesArray(DefaultVM::Get(), roles.size());

	for (unsigned int i = 0; i < roles.size(); ++i) {
		rolesArray.SetValue(i, roles.at(i).string().c_str());
	}
	
	if (listener.IsNull()) {
		return;
	}

	try {
		listener.Execute(channelID, author, authorNick, authorID, rolesArray, message);
	}
	catch (Sqrat::Exception& e) {
		SqMod_LogErr("Discord event [%s] => Squirrel error [%s]", "MESSAGE", e.what());
	}
	catch (const std::exception & e)
	{
		SqMod_LogErr("Discord event [%s] => Program error [%s]", "MESSAGE", e.what());
	}
	catch (...)
	{
		SqMod_LogErr("Discord event [%s] => Unknown error", "MESSAGE");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Disconnect()
{
	try
	{
		if (isConnected)
		{
			std::lock_guard<std::mutex> lock(m_Guard);
			client->quit();
			isConnected = false;
			isConnecting = false;
		}
	}
	catch (...)
	{
		SqMod_LogErr("An Error has occured at [CSession] function => [Disconnect]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Destroy()
{
	if (!client)
	{
		return;
	}

	// Disconnect the session
	Disconnect();

	client->session = nullptr;
	delete client;
	client = nullptr;

	// Release script resources
	Release();
}

// ------------------------------------------------------------------------------------------------
CSession::~CSession()
{
	Destroy();

	// Attempt to find our self in the session pool
	Sessions::iterator itr = std::find(s_Sessions.begin(), s_Sessions.end(), this);
	// Are we in the pool?
	if (itr != s_Sessions.end())
	{
		s_Sessions.erase(itr); /* Remove our self from the pool. */
	}
	// Is there a single session and that's us?
	if (s_Session == this)
	{
		s_Session = nullptr;
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Terminate()
{
	// Do we only have one Discord session?
	if (s_Session)
	{
		s_Session->Destroy(); /* This should do the job. */
	}
	// Do we have multiple sessions?
	else if (!s_Sessions.empty())
	{
		for (Sessions::iterator itr = s_Sessions.begin(); itr != s_Sessions.end(); ++itr)
		{
			(*itr)->Destroy();
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::DRegister_CSession(Table& discordcn)
{
	using namespace Sqrat;

	discordcn.Bind("Session",
		Class< CSession, NoCopy< CSession > >(discordcn.GetVM(), "Session")

		.Ctor()

		.Func("Connect", &CSession::Connect)
		.Func("Bind", &CSession::BindEvent)
		.Func("Disconnect", &CSession::Disconnect)

		.SquirrelFunc("Message", &CSession::Message)
		.SquirrelFunc("MessageEmbed", &CSession::MessageEmbed)
		.SquirrelFunc("GetRoleName", &CSession::GetRoleName)
		.SquirrelFunc("SetActivity", &CSession::SetActivity)
	);
}
}
