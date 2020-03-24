#pragma once

// ------------------------------------------------------------------------------------------------
#include "Base/Utility.hpp"

// ------------------------------------------------------------------------------------------------
namespace SqDiscord {

/* ------------------------------------------------------------------------------------------------
 * SOFTWARE INFORMATION
*/
#define SQDISCORD_NAME "Squirrel Discord Module"
#define SQDISCORD_AUTHOR "Luckshya"
#define SQDISCORD_COPYRIGHT "None"
#define SQDISCORD_HOST_NAME "SqModDiscordHost"
#define SQDISCORD_VERSION 001
#define SQDISCORD_VERSION_STR "0.0.1"
#define SQDISCORD_VERSION_MAJOR 0
#define SQDISCORD_VERSION_MINOR 0
#define SQDISCORD_VERSION_PATCH 1

// ------------------------------------------------------------------------------------------------
enum SessEvents {
	ON_READY = 0,
	ON_MESSAGE
};

// ------------------------------------------------------------------------------------------------
void DRegister_Constants(Sqrat::Table&);
void DRegister_Enums();
SQInteger Regex_Match(HSQUIRRELVM vm);

} // Namespace - SqDiscord
