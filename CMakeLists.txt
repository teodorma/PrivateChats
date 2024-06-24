cmake_minimum_required(VERSION 3.28)
project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ljsoncpp")

add_executable(Server main.cpp
        Requests/Requests.h
        DB/DB.cpp
        Requests/Requests.cpp
        Server/Server.cpp)
