cmake_minimum_required(VERSION 2.8)
project(cpp)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_COMPILER /usr/bin/gcc)

set(CMAKE_CXX_COMPILER /usr/bin/g++)

include_directories(include)

add_definitions(-std=c++11)

#add_executable(cpp
#        src/myutils.cpp
#        src/simpleNewWords.cpp
#        src/main_simple.cpp
#        )

add_executable(cpp
       src/myutils.cpp
       src/fastNewWords.cpp
       src/main_fast.cpp
       )

#add_executable(cpp
#       src/myutils.cpp
#       src/trieNewWords.cpp
#       src/main_mr.cpp
#       )