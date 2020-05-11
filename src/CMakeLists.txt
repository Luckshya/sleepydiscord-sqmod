add_library(SqDiscord SHARED
		CDiscord.cpp
		CDiscord.h
		CMessage.cpp
		CMessage.h
		Common.cpp
		Common.hpp
		Module.cpp
		CSession.cpp
		CSession.h
		DEmbed.cpp
		DEmbed.h
)

# Force 32-bit binaries when necessary
if(FORCE_32BIT_BIN)
	set_target_properties(SqDiscord PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	message("FORCING 32 BIT BINARIES")
endif()

# Size of a pointer in bytes. To identify CPU architecture.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	message("DEFAULTING TO 64 BIT BINARIES")
endif()

# Compile definitions
target_compile_definitions(SqDiscord PUBLIC SQMOD_PLUGIN_API=1)

# Determine if build mode
if(CMAKE_BUILD_TYPE MATCHES Release)
	target_compile_definitions(SqDiscord PRIVATE NDEBUG=1)
	message("aassa")
else()
	target_compile_definitions(SqDiscord PRIVATE _DEBUG=1 SQMOD_EXCEPTLOC=1)
	message("aasa")
endif()

# Link to base libraries
target_link_libraries(SqDiscord VCMP SquirrelAPI Sqrat SqSDK sleepy-discord)

# Don't prefix the module binary.
set_target_properties(SqDiscord PROPERTIES PREFIX "")

# Custmize module binary name/
if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT FORCE_32BIT_BIN)
		set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_64")
	else()
		set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_32")
	endif()
else(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT FORCE_32BIT_BIN)
		set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_64")
	else()
		set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_32")
	endif()
endif(WIN32)

target_include_directories(SqDiscord
	PRIVATE
		${PROJECT_SOURCE_DIR}/src
)
