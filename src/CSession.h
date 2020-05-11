#pragma once

// ------------------------------------------------------------------------------------------------
#include "Common.hpp"
#include "CMessage.h"

// ------------------------------------------------------------------------------------------------
#include <thread>

// ------------------------------------------------------------------------------------------------
class CDiscord;

// ------------------------------------------------------------------------------------------------
typedef const char * CCStr;

// ------------------------------------------------------------------------------------------------
using namespace Sqrat;

// ------------------------------------------------------------------------------------------------
namespace SqDiscord
{
// ------------------------------------------------------------------------------------------------
class CSession
{
private:
	CDiscord * client = nullptr;

	void runSleepy(std::string);

	/* --------------------------------------------------------------------------------------------
	 * Script callbacks.
	*/
	Function        m_OnReady;
	Function        m_OnMessage;

public :
	// --------------------------------------------------------------------------------------------
	std::thread * sleepyThread	= nullptr;
	//unsigned short int connID	= 0;
	bool isConnecting			= false;
	bool isConnected			= false;
	
	// Mutex lock to guard while connecting and disconnecting
	std::mutex m_Guard;

	// Mutex lock to guard s_Messages container
	std::mutex m_MsgGuard;

	// Mutex lock to guard s_ReadySessions container
	std::mutex m_ReadyGuard;

	// Container to hold messages
	std::vector<CMessage> s_Messages;

	// Container to hold readyEvent to be called in a Queue
	std::vector<CSession *> s_ReadySession;

	CSession();
	~CSession();

    CSession(const CSession & o) = delete;
    CSession(CSession && o) = delete;

	static void Process();
	static void Terminate();

	void Update();
	void Disconnect();
	void Destroy();
	void Connect(CCStr);
	void Release();
	void BindEvent(int evid, Object & env, Function & func);
	Function& GetEvent(int);

	static SQInteger Message(HSQUIRRELVM);
	static SQInteger MessageEmbed(HSQUIRRELVM);
	static SQInteger GetRoleName(HSQUIRRELVM);
	static SQInteger SetActivity(HSQUIRRELVM);

	void OnReady();
	void OnMessage(CCStr, CCStr, CCStr, CCStr, std::vector<SleepyDiscord::Snowflake<SleepyDiscord::Role>>, CCStr);

	static void DRegister_CSession(Table&);
};
} //Namespace - SqDiscord