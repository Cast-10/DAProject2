cmake_minimum_required(VERSION 3.10)
project(DAProject2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(myProg
        Src/myProg.cpp
)
target_include_directories(myProg PRIVATE
        Src/DataStructures
)