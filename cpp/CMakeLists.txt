cmake_minimum_required(VERSION 2.8)
project(cpp)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

#add_executable(cpp
#        include/myutils.h
#        include/simpleNewWords.h
#        src/myutils.cpp
#        src/simpleNewWords.cpp
#        src/main.cpp
#        )

add_executable(cpp
        src/myutils.cpp
        src/fastNewWords.cpp
        src/main_fast.cpp
        )