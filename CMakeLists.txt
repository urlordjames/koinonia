cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project("koinonia")

# clone specified release of drogon from GitHub
include(FetchContent)
FetchContent_Declare(drogon_git
	GIT_REPOSITORY "https://github.com/an-tao/drogon.git"
	GIT_TAG "v1.6.0"
)

set(PROJECT_SOURCES
	src/main.cpp
	src/SocketInfo.cpp
	src/StreamSock.cpp
	src/Util.cpp
	src/Messages.cpp
)

option(USE_LUA_PLUGINS "compile with Lua to allow for running user created plugins" ON)

if(USE_LUA_PLUGINS)
	list(APPEND PROJECT_SOURCES
		src/Plugin.cpp
		src/PluginManager.cpp
	)
endif()

add_executable(${PROJECT_NAME}
	${PROJECT_SOURCES}
)

if(USE_LUA_PLUGINS)
	find_package(Lua REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})
	target_compile_definitions(${PROJECT_NAME} PRIVATE "USE_LUA_PLUGINS")
endif()

# disable unused drogon stuff
option(BUILD_CTL OFF)
option(BUILD_EXAMPLES OFF)
option(BUILD_ORM OFF)

FetchContent_MakeAvailable(drogon_git)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
	target_compile_options(trantor PRIVATE "-Wno-unused-parameter")
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
	endif()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE drogon)

# copy static files
file(COPY static DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
