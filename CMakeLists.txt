cmake_minimum_required (VERSION 3.6)
project(sleepydiscord-sqmod)

# Several plugin options
option(FORCE_32BIT_BIN "Create a 32-bit executable binary if the compiler defaults to 64-bit." OFF)
option(FORCE_DEBUG_BIN "Create debug binary." OFF)
option(PLUGIN_DEVEL "Plugin development mode." ON)
set(BUILD_SHARED_LIBS OFF)

if(FORCE_DEBUG_BIN)
	set(CMAKE_BUILD_TYPE Debug)
	message("Configuring Debug builds.")
else()
	set(CMAKE_BUILD_TYPE Release)
	message("Configuring Release builds.")
endif()

# Enable position independent code
if (UNIX)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(USE_SYSTEM_CURL OFF)
	set(CMAKE_USE_OPENSSL ON)
endif()

include(ExternalProject)
include(buildtools/cmake/DownloadProject.cmake)

download_project(
		PROJ           sleepy-discord
		GIT_REPOSITORY https://github.com/yourWaifu/sleepy-discord.git
		GIT_TAG        develop
		SOURCE_DIR     ${PROJECT_SOURCE_DIR}/sleepy-discord
		UPDATE_DISCONNECTED 1
)

# Include SDK library
add_subdirectory(sdk)

add_subdirectory(sleepy-discord)

add_subdirectory(src)