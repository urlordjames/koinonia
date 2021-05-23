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

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/SocketInfo.cpp
	src/StreamSock.cpp
	src/Util.cpp
	src/Messages.cpp
)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
	endif()
endif()

# disable unused drogon stuff
option(BUILD_CTL OFF)
option(BUILD_EXAMPLES OFF)
option(BUILD_ORM OFF)

FetchContent_MakeAvailable(drogon_git)

target_link_libraries(${PROJECT_NAME} PRIVATE drogon)

# copy static files
file(COPY static DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
