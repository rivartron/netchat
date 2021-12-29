cmake_minimum_required(VERSION 3.20)
project(netchat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pthread)

add_executable(netchat_client chat_client.cpp)
add_executable(netchat_server chat_server.cpp)