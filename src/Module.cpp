// ------------------------------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#endif

// ------------------------------------------------------------------------------------------------
#include "Common.hpp"
#include "CSession.h"
#include "DEmbed.h"
#include "Channel.h"
#include "Guild.h"
#include "Role.h"
#include "ServerMember.h"
#include "User.h"

// ------------------------------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {

/* ------------------------------------------------------------------------------------------------
 * Register the module API under the obtained virtual machine.
*/
static bool RegisterAPI(HSQUIRRELVM vm) {
	// Make sure there's a valid virtual machine before proceeding
	if (!vm) {
		OutputError("%s: Cannot register API without a valid virtual machine", SQDISCORD_NAME);
		// Registration failed
		return false;
	}

	Table discordcn(vm);
	Table embeds(vm);

	CSession::DRegister_CSession(discordcn);
	DRegister_Constants(discordcn);
	Channel::Register_Channel(discordcn);
	Guild::Register_Guild(discordcn);
	Role::Register_Role(discordcn);
	ServerMember::Register_ServerMember(discordcn);
	User::Register_User(discordcn);
	Message::Register_Message(discordcn);

	Register_Embeds(embeds);
	discordcn.Bind("Embed", embeds);
	RootTable(vm).Bind("SqDiscord", discordcn);

	RootTable(vm).SquirrelFunc("Regex_Match", Regex_Match);

	// Registration was successful
	return true;
}

/* ------------------------------------------------------------------------------------------------
 * Load the module on the virtual machine provided by the host module.
*/
static bool OnSquirrelLoad() {
	// Make sure that we have a valid module API
	if (!SqMod_GetSquirrelVM()) {
		OutputError("%s: Cannot obtain the Squirrel virtual machine without the module API", SQDISCORD_NAME);
		// Unable to proceed!
		return false;
	}
	// Obtain the Squirrel virtual machine from the host plug-in
	DefaultVM::Set(SqMod_GetSquirrelVM());
	// Make sure that a valid virtual machine exists
	if (!DefaultVM::Get()) {
		OutputError("%s: Squirrel virtual machine obtained from the host plug-in is invalid", SQDISCORD_NAME);
		// Unable to proceed!
		return false;
	}
	// Prevent common null objects from using dead virtual machine
	NullFunction() = Function();

	// Register the module API
	if (RegisterAPI(DefaultVM::Get())) {
		OutputMessage("Registered: %s", SQDISCORD_NAME);
	} else {
		return false;
	}
	// At this point, the module was successfully loaded
	return true;
}

/* ------------------------------------------------------------------------------------------------
 * The virtual machine is about to be terminated and script resources should be released.
*/
static void OnSquirrelTerminate() {
	OutputMessage("Terminating: %s", SQDISCORD_NAME);
	// Release null objects just in case
	NullFunction().Release();
	// Release script resources...
}

/* ------------------------------------------------------------------------------------------------
 * The virtual machined is about to be closed. Last chance to release anything manually.
*/
static void OnSquirrelClosing() {
	CSession::Terminate();
}

/* ------------------------------------------------------------------------------------------------
 * The virtual machined was closed and all memory associated with it was released.
*/
static void OnSquirrelReleased() {
	// Release the current virtual machine, if any
	DefaultVM::Set(nullptr);
}

/* ------------------------------------------------------------------------------------------------
 * React to command sent by other plug-ins.
*/
static uint8_t OnPluginCommand(uint32_t command_identifier, CCStr message) {
	switch (command_identifier) {
		case SQMOD_INITIALIZE_CMD: {
			if (CheckModuleAPIVer(message, SQDISCORD_NAME)) {
				try {
					ImportModuleAPI(_Func, SQDISCORD_NAME);
				}
				catch (const Sqrat::Exception &e) {
					OutputError("%s", e.what());
					// Failed to initialize
					return 0;
				}
			}
		}
			break;
		case SQMOD_LOAD_CMD: {
			return OnSquirrelLoad();
		}
			break;
		case SQMOD_TERMINATE_CMD: {
			OnSquirrelTerminate();
		}
			break;
		case SQMOD_CLOSING_CMD: {
			OnSquirrelClosing();
		}
			break;
		case SQMOD_RELEASED_CMD: {
			OnSquirrelReleased();
		}
			break;
		default:
			break;
	}
	return 1;
}

static void OnServerFrame(float) {
	CSession::Process();
}

/* ------------------------------------------------------------------------------------------------
 * The server was initialized and this plug-in was loaded successfully.
*/
static uint8_t OnServerInitialise() {
	return 1; // Initialization was successful
}

/* ------------------------------------------------------------------------------------------------
 * The server is about to shutdown gracefully.
*/
static void OnServerShutdown(void) {
	// The server may still send callbacks
	_Clbk->OnServerInitialise = nullptr;
	_Clbk->OnServerShutdown = nullptr;
	_Clbk->OnPluginCommand = nullptr;
	_Clbk->OnServerFrame = nullptr;
}

} // Namespace:: SqDiscord

// ------------------------------------------------------------------------------------------------
SQMOD_API_EXPORT unsigned int VcmpPluginInit(PluginFuncs *functions, PluginCallbacks *callbacks, PluginInfo *info) {
	using namespace SqDiscord;
	// Output plug-in header
	puts("");
	OutputMessage("--------------------------------------------------------------------");
	OutputMessage("Plug-in: %s", SQDISCORD_NAME);
	OutputMessage("Author: %s", SQDISCORD_AUTHOR);
	OutputMessage("Legal: %s", SQDISCORD_COPYRIGHT);
	OutputMessage("--------------------------------------------------------------------");
	puts("");
	// Make sure that the module was loaded after the host plug-in
	if (!CheckModuleOrder(functions, info->pluginId, SQDISCORD_NAME)) {
		return SQMOD_FAILURE;
	}
	// Store server proxies
	_Func = functions;
	_Clbk = callbacks;
	_Info = info;
	// Assign plug-in version
	_Info->pluginVersion = SQDISCORD_VERSION;
	_Info->apiMajorVersion = PLUGIN_API_MAJOR;
	_Info->apiMinorVersion = PLUGIN_API_MINOR;
	// Assign the plug-in name
	std::snprintf(_Info->name, sizeof(_Info->name), "%s", SQDISCORD_HOST_NAME);
	// Bind to the server callbacks
	_Clbk->OnServerInitialise = OnServerInitialise;
	_Clbk->OnServerShutdown = OnServerShutdown;
	_Clbk->OnPluginCommand = OnPluginCommand;
	_Clbk->OnServerFrame = OnServerFrame;
	// Notify that the plug-in was successfully loaded
	OutputMessage("Successfully loaded %s", SQDISCORD_NAME);
	// Dummy spacing
	puts("");
	// Done!
	return SQMOD_SUCCESS;
}
