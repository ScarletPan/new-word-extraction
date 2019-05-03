cmake_minimum_required(VERSION 3.14)
project(cpp)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(cpp
        include/infoEnt.h
        src/infoEnt.cpp
        src/main.cpp)
