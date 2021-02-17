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

target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
