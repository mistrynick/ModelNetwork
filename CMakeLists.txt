cmake_minimum_required(VERSION 3.10)

project(SimpleCMakeProject)

set(CMAKE_CXX_STANDARD 11)

add_executable(client client.cpp)
add_executable(server server.cpp)
