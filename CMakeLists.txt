cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project("koinonia")

# attempt to find drogon
find_package(Drogon)

# clone specified release of drogon from GitHub if drogon not installed
if(NOT Drogon_FOUND)
	include(FetchContent)
	FetchContent_Declare(drogon_git
		GIT_REPOSITORY "https://github.com/drogonframework/drogon.git"
		GIT_TAG "v1.7.5"
	)
endif()

set(PROJECT_SOURCES
	src/main.cpp
	src/SocketInfo.cpp
	src/StreamSock.cpp
	src/Util.cpp
	src/Messages.cpp
	src/Room.cpp
)

# currently in limbo
option(USE_LUA_PLUGINS "compile with Lua to allow for running user created plugins" OFF)

if(USE_LUA_PLUGINS)
	list(APPEND PROJECT_SOURCES
		src/Plugin.cpp
		src/PluginManager.cpp
	)
endif()

add_executable(${PROJECT_NAME}
	${PROJECT_SOURCES}
)

set(STATIC_ROOT "static" CACHE PATH "directory to serve static files from")
target_compile_definitions(${PROJECT_NAME} PRIVATE STATIC_ROOT="${STATIC_ROOT}")

if(USE_LUA_PLUGINS)
	find_package(Lua REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})
	target_compile_definitions(${PROJECT_NAME} PRIVATE "USE_LUA_PLUGINS")
endif()

if(NOT Drogon_FOUND)
	# disable unused drogon stuff if building from source
	option(BUILD_CTL OFF)
	option(BUILD_EXAMPLES OFF)
	option(BUILD_ORM OFF)
	FetchContent_MakeAvailable(drogon_git)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
	if(NOT Drogon_FOUND)
		target_compile_options(trantor PRIVATE "-Wno-unused-parameter")
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
	endif()
endif()

if(Drogon_FOUND)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${DROGON_LIBRARIES})
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
endif()

SET(STATIC_PATH "nginx/static" CACHE PATH "directory to copy static files from")

# copy static files
if(EXISTS "${CMAKE_SOURCE_DIR}/${STATIC_PATH}")
	file(COPY ${STATIC_PATH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
	message(WARNING "static directory does not exist, will not attempt to copy")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY ${STATIC_PATH} DESTINATION extras)
