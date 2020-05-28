#pragma once

// ------------------------------------------------------------------------------------------------
#include "Common.hpp"
#include "Message.h"
#include "CError.h"
#include "Guild.h"

// ------------------------------------------------------------------------------------------------
#include <thread>
#include <mutex>

// ------------------------------------------------------------------------------------------------
typedef const char *CCStr;
typedef const std::string &CString;
typedef const std::vector<std::string> &s_Roles;

// ------------------------------------------------------------------------------------------------
using namespace Sqrat;

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {
// ------------------------------------------------------------------------------------------------
class CSession {
private:
	CDiscord *client = nullptr;

	void runSleepy(std::string);

	/* --------------------------------------------------------------------------------------------
	 * Script callbacks.
	*/
	Function m_OnReady;
	Function m_OnMessage;
	Function m_OnError;
	Function m_OnDisconnect;
	Function m_OnQuit;

public :
	// --------------------------------------------------------------------------------------------
	std::thread *sleepyThread = nullptr;
	//unsigned short int connID	= 0;
	bool isConnecting = false;
	bool isConnected = false;
	bool errorEventEnabled = false;
	bool internalCacheEnabled = true;

	// Mutex lock to guard while connecting and disconnecting
	std::mutex m_Guard;

	// Mutex lock to guard s_Messages container
	std::mutex m_MsgGuard;

	// Mutex lock to guard s_ReadySessions container
	std::mutex m_ReadyGuard;

	// Mutex lock to guard s_Errors container
	std::mutex m_ErrorGuard;

	// Mutex lock to guard s_Disconnects container
	std::mutex m_DisconnectsGuard;

	// Mutex lock to guard s_Quits container
	std::mutex m_QuitsGuard;

	// Mutex lock to guard s_Servers container
	std::mutex m_ServersGuard;

	// Mutex lock to guard s_OtherChannels container
	std::mutex m_OtherChannelsGuard;

	// Container to hold messages
	std::vector<Message *> s_Messages;

	// Container to hold readyEvent to be called in a Queue
	std::vector<CSession *> s_ReadySession;

	// Container to hold error messages
	std::vector<CError> s_Errors;

	// Container to hold Disconnect Event to be called in a Queue
	std::vector<CSession *> s_Disconnects;

	// Container to hold Quit Event to be called in a Queue
	std::vector<CSession *> s_Quits;

	CSession();

	~CSession();

	CSession(const CSession &o) = delete;

	CSession(CSession &&o) = delete;

	static void Process();

	static void Terminate();

	void Update();

	void Disconnect();

	void Destroy();

	void Connect(CCStr);

	void Release();

	void BindEvent(int evid, Object &env, Function &func);

	Function &GetEvent(int);

	LightObj GetGuild(const std::string &serverID);

	LightObj GetOtherChannel(const std::string &channelID);

	bool GetErrorEventEnabled() const;

	void SetErrorEventEnabled(bool toggle);

	static SQInteger Message(HSQUIRRELVM);

	static SQInteger MessageEmbed(HSQUIRRELVM);

	static SQInteger GetRoleName(HSQUIRRELVM);

	static SQInteger EditChannel(HSQUIRRELVM);

	static SQInteger SetActivity(HSQUIRRELVM);

	void OnReady();

	void OnMessage(SqDiscord::Message *message);

	void OnError(int, CString);

	void OnDisconnect();

	void OnQuit();

	static void DRegister_CSession(Table &);
};
} //Namespace - SqDiscord