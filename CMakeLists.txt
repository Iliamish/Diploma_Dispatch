cmake_minimum_required(VERSION 3.10)

# set the project name
project(diploma VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug)

# add the executable
add_executable(algo main.cpp algorithm.cpp imitation.cpp)