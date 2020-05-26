cmake_minimum_required(VERSION 3.16)
project(BoardMaster)

set(CMAKE_CXX_STANDARD 14)

add_executable(BoardMaster main.cpp client.cpp server.cpp BoardMasterGame.h BoardMasterGame.cpp)