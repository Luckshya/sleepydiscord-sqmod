add_library(mod_discord SHARED
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
	${PROJECT_SOURCE_DIR}/shared/Base/Buffer.cpp
	${PROJECT_SOURCE_DIR}/shared/SqMod.cpp
)

if(BUILD_64)
	target_compile_options(mod_discord PRIVATE -D_SQ64 -DSQMOD_PLUGIN_API -DSCRAT_USE_EXCEPTIONS -DSCRAT_USE_CXX11_OPTIMIZATIONS)
else()
	target_compile_options(mod_discord PRIVATE -DSQMOD_PLUGIN_API -DSCRAT_USE_EXCEPTIONS -DSCRAT_USE_CXX11_OPTIMIZATIONS)
endif()

target_include_directories(mod_discord
	PRIVATE
		${PROJECT_SOURCE_DIR}/src
		${PROJECT_SOURCE_DIR}/shared
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/config/common
		${PROJECT_SOURCE_DIR}/shared/Base
)

target_link_libraries(mod_discord sleepy-discord)