add_library(SqDiscord SHARED
        CDiscord.cpp
        CDiscord.h
        EventHandler.h
        Constants.h
        Constants.cpp
        Common.cpp
        Common.hpp
        Module.cpp
        CSession.cpp
        CSession.h
        DEmbed.cpp
        DEmbed.h
        Guild.cpp
        Guild.h
        Role.cpp
        Role.h
        ServerMember.cpp
        ServerMember.h
        Channel.cpp
        Channel.h
        User.cpp
        User.h
        Message.cpp
        Message.h)

# Force 32-bit binaries when necessary
if (FORCE_32BIT_BIN)
    set_target_properties(SqDiscord PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    message("FORCING 32 BIT BINARIES")
endif ()

# Size of a pointer in bytes. To identify CPU architecture.
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message("DEFAULTING TO 64 BIT BINARIES")
endif ()

# Compile definitions
target_compile_definitions(SqDiscord PUBLIC SQMOD_PLUGIN_API=1)

# Determine if build mode
if (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(SqDiscord PRIVATE NDEBUG=1)
else ()
    target_compile_definitions(SqDiscord PRIVATE _DEBUG=1 SQMOD_EXCEPTLOC=1)
endif ()

# Link to base libraries
target_link_libraries(SqDiscord SqAddSDK sleepy-discord)

# Don't prefix the module binary.
set_target_properties(SqDiscord PROPERTIES PREFIX "")

# Custmize module binary name/
if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT FORCE_32BIT_BIN)
        set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_64")
    else ()
        set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_32")
    endif ()
else (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT FORCE_32BIT_BIN)
        set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_64")
    else ()
        set_target_properties(SqDiscord PROPERTIES OUTPUT_NAME "mod_discord_32")
    endif ()
endif (WIN32)

target_include_directories(SqDiscord
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src/
        ${CMAKE_SOURCE_DIR}/readerwriterqueue/
        )
