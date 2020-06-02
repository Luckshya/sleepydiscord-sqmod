#pragma once

// ------------------------------------------------------------------------------------------------
#include "Common.hpp"
#include "Message.h"
#include "Guild.h"

// ------------------------------------------------------------------------------------------------
#include <thread>
#include <mutex>
#include <readerwriterqueue.h>

// ------------------------------------------------------------------------------------------------
typedef const char *CCStr;
typedef const std::string &CString;
typedef const std::vector<std::string> &s_Roles;

// ------------------------------------------------------------------------------------------------
using namespace Sqrat;
using namespace DiscordEvent;
using namespace moodycamel;

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {
// ------------------------------------------------------------------------------------------------
class CSession {
private:
	CDiscord *client = nullptr;

	void runSleepy(std::string);

public :
	// --------------------------------------------------------------------------------------------
	std::thread *sleepyThread = nullptr;
	EventHandler *s_EventHandler = nullptr;

	//unsigned short int connID	= 0;
	bool isConnecting = false;
	bool isConnected = false;
	bool errorEventEnabled = false;
	bool internalCacheEnabled = true;

	/* --------------------------------------------------------------------------------------------
	 * Script callbacks.
	*/
	Function m_OnReady;
	Function m_OnMessage;
	Function m_OnError;
	Function m_OnDisconnect;
	Function m_OnQuit;
	Function m_OnServer;
	Function m_OnServerEdit;
	Function m_OnServerDelete;
	Function m_OnMember;
	Function m_OnMemberEdit;
	Function m_OnMemberRemove;
	Function m_OnChannel;
	Function m_OnChannelEdit;
	Function m_OnChannelDelete;
	Function m_OnRole;
	Function m_OnRoleEdit;
	Function m_OnRoleDelete;
	Function m_OnUserEdit;

	// Mutex lock to guard while connecting and disconnecting
	std::mutex m_Guard;

	std::unordered_map<std::string, SqDiscord::Channel> LatestCopy_OtherChannels;
	std::unordered_map<std::string, SqDiscord::Guild> LatestCopy_Servers;

	ReaderWriterQueue< std::unordered_map<std::string, SqDiscord::Channel> > OtherChannels_Queue;
	ReaderWriterQueue< std::unordered_map<std::string, SqDiscord::Guild> > Servers_Queue;

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

	bool GetInternalCacheEnabled() const;

	void SetInternalCacheEnabled(bool toggle);

	static SQInteger Message(HSQUIRRELVM);

	static SQInteger MessageEmbed(HSQUIRRELVM);

	static SQInteger GetRoleName(HSQUIRRELVM vm);

	static SQInteger EditChannel(HSQUIRRELVM);

	static SQInteger SetActivity(HSQUIRRELVM);

	static void DRegister_CSession(Table &);
};
} //Namespace - SqDiscord