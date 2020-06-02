#pragma once

// ------------------------------------------------------------------------------------------------
#include <SqMod.h>
#include <sqrat.h>
#include <vcmp.h>

// ------------------------------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>

// ------------------------------------------------------------------------------------------------
class CDiscord;

namespace DiscordEvent {
template< class T > struct EventType;
class EventHandler;
}

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {

// ------------------------------------------------------------------------------------------------
using namespace Sqrat;

// ------------------------------------------------------------------------------------------------
typedef const char *CCStr;
typedef const SQChar *CSStr;

// ------------------------------------------------------------------------------------------------
class CSession;

/* ------------------------------------------------------------------------------------------------
 * SOFTWARE INFORMATION
*/
#define SQDISCORD_NAME "Squirrel Discord Module"
#define SQDISCORD_AUTHOR "Luckshya"
#define SQDISCORD_COPYRIGHT "Copyright (c) 2020 Luckshya"
#define SQDISCORD_HOST_NAME "SqModDiscordHost"
#define SQDISCORD_VERSION 001
#define SQDISCORD_VERSION_STR "0.0.1"
#define SQDISCORD_VERSION_MAJOR 0
#define SQDISCORD_VERSION_MINOR 0
#define SQDISCORD_VERSION_PATCH 1


/* ------------------------------------------------------------------------------------------------
 * LOGGING LOCATION
*/

#define SQMOD_TRUESTRINGIZE(x) #x
#define SQMOD_STRINGIZEWRAP(x) SQMOD_TRUESTRINGIZE(x)

#if defined(_DEBUG) || defined(SQMOD_EXCEPTLOC)
#define SQMOD_MSGLOC(m) (m " =>[" __FILE__ ":" SQMOD_STRINGIZEWRAP(__LINE__) "] ")
#else
#define SQMOD_MSGLOC(m) (m)
#endif // _DEBUG

/* ------------------------------------------------------------------------------------------------
 * EXCEPTION THROWING
*/

#if defined(_DEBUG) || defined(SQMOD_EXCEPTLOC)
#define STHROW(e, m, ...) throw e(m " =>[" __FILE__ ":" SQMOD_STRINGIZEWRAP(__LINE__) "] ", ##__VA_ARGS__)
#define STHROWF(m, ...) SqThrowF(m " =>[" __FILE__ ":" SQMOD_STRINGIZEWRAP(__LINE__) "] ", ##__VA_ARGS__)
#define STHROWLASTF(m, ...) SqThrowLastF(m " =>[" __FILE__ ":" SQMOD_STRINGIZEWRAP(__LINE__) "] ", ##__VA_ARGS__)
#else
#define STHROW(e, m, ...) throw e(m, ##__VA_ARGS__)
#define STHROWF(m, ...) SqThrowF(m, ##__VA_ARGS__)
#define STHROWLASTF(m, ...) SqThrowLastF(m, ##__VA_ARGS__)
#endif // _DEBUG

/* ------------------------------------------------------------------------------------------------
 * GENERAL RESPONSES
*/
#define SQMOD_SUCCESS     1
#define SQMOD_FAILURE     0
#define SQMOD_UNKNOWN     (-1)

/* ------------------------------------------------------------------------------------------------
 * Proxies to communicate with the server.
*/
extern PluginFuncs *_Func;
extern PluginCallbacks *_Clbk;
extern PluginInfo *_Info;

/* ------------------------------------------------------------------------------------------------
 * Generate a formatted string and throw it as a sqrat exception.
*/
void SqThrowF(const char *str, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a message only if the _DEBUG was defined.
*/
void OutputDebug(const SQChar *msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted user message to the console.
*/
void OutputMessage(const SQChar *msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted error message to the console.
*/
void OutputError(const SQChar *msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Validate the module API to make sure we don't run into issues.
*/
bool CheckModuleAPIVer(const SQChar *ver, const SQChar *mod);

/* ------------------------------------------------------------------------------------------------
 * Make sure that the module was loaded after the host plug-in.
*/
bool CheckModuleOrder(PluginFuncs *vcapi, uint32_t mod_id, const SQChar *mod);

/* ------------------------------------------------------------------------------------------------
 * Used by the modules to import the API from the host plug-in.
*/
void ImportModuleAPI(PluginFuncs *vcapi, const SQChar *mod);

/* ------------------------------------------------------------------------------------------------
 * Retrieve a reference to a null script function.
*/
Function &NullFunction();
} // Namespace:: SqDiscord

