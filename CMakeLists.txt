cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project("koinonia")

add_subdirectory("drogon")

# disable ORM
option(BUILD_ORM OFF)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/StreamSock.cpp
  src/Util.cpp
)

if(CMAKE_COMPILER_IS_GNUCXX AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
	target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra")
endif()

target_compile_options("trantor" PRIVATE "-Wno-unused-parameter")

target_link_libraries(${PROJECT_NAME} PRIVATE drogon)

# copy static files
file(COPY static DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
