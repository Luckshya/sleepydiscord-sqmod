// ------------------------------------------------------------------------------------------------
#include "CSession.h"
#include "CDiscord.h"
#include "Common.hpp"
#include "DEmbed.h"

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

        for (auto & session : s_ReadySession)
        {
            if (session != nullptr && session->client != nullptr)
            {
                OnReady();
            }
        }

		s_ReadySession.clear();
	}

	if (!s_Messages.empty())
	{
        std::lock_guard<std::mutex> lockB(m_MsgGuard);

        for (auto & message : s_Messages)
        {
            OnMessage((message.channelID).c_str(), (message.author).c_str(), (message.authorNick).c_str(), (message.authorID).c_str(), message.roles, (message.message).c_str());
        }

        s_Messages.clear();
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Release()
{
	m_OnReady.Release();
	m_OnMessage.Release();
	//std::cout << "Script resources released.\n";
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
            std::lock_guard <std::mutex> lock(m_Guard);

            this->isConnecting = true;
            this->client = new CDiscord(token);
            this->client->session = this;
        }
        //std::cout << "Unlocked A\n";

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
		event.Release();
	}
	
	else if (env.IsNull()) {
		event = func;
	}
	
	else {
		event = Function(env, func.GetFunc(), SqVM());
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
	if (SQ_FAILED(message.Proc(false)))
	{
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
	const int top = sq_gettop(vm);

    if (top <= 1)
    {
        return sq_throwerror(vm, "Missing the channel ID value");
    }

    else if (top <= 2)
    {
        return sq_throwerror(vm, "Missing the content value");
    }

    else if (top <= 3)
    {
        return sq_throwerror(vm, "Missing the Embed value");
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

    StackStrF content(vm, 3);
    if (SQ_FAILED(channelID.Proc(false)))
    {
        return content.mRes; // Propagate the error!
    }

	Embed * embed = nullptr;
	try {
		embed = Sqrat::Var< Embed * >(vm, 4).value;
	}
	catch (const Sqrat::Exception& e) {
		return sq_throwerror(vm, e.what());
	}

	if (!embed) {
		return sq_throwerror(vm, "Invalid Embed instance");
	}

	try {
		auto msg = session->client->sendMessage(channelID.mPtr, "", *(embed->embed), false, SleepyDiscord::Async);
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
		CCStr role_name = nullptr;

        for (const auto & role : roles) {
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
	//std::cout << "Disconnecting\n";
	try
	{
        std::lock_guard<std::mutex> lock(m_Guard);

		if (client != nullptr)
		{
			client->quit();

			if (sleepyThread != nullptr)
			{
				sleepyThread->join();
				delete sleepyThread;
				sleepyThread = nullptr;
			}
		}

		isConnected		= false;
		isConnecting	= false;

		delete client;
		client = nullptr;
	}
	catch (...)
	{
		SqMod_LogErr("An Error has occured at [CSession] function => [Disconnect]");
	}
}

// ------------------------------------------------------------------------------------------------
void CSession::Destroy() {

    // Release script resources
    Release();

    //std::cout << "Destroying\n";
    {
        std::lock_guard <std::mutex> lock(m_Guard);

        if (!client) {
            return;
        }
    }
	// Disconnect the session
	Disconnect();
}

// ------------------------------------------------------------------------------------------------
CSession::~CSession()
{
	//std::cout << "Destructor\n";
	Destroy();

	// Attempt to find our self in the session pool
	auto itr = std::find(s_Sessions.begin(), s_Sessions.end(), this);
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

	//std::cout << "Destructor Done\n";
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
        for (auto & session : s_Sessions)
        {
            session->Destroy();
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
