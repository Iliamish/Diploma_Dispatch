cmake_minimum_required(VERSION 3.10)

# set the project name
project(diploma VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror")

# add the executable
add_executable(algo main.cpp 
                    algorithm.cpp
                    imitation.cpp
                    utils.cpp)


# add libraries
find_package(jsoncpp REQUIRED)
TARGET_LINK_LIBRARIES(algo jsoncpp_lib)

find_package(Threads REQUIRED)
target_link_libraries(algo Threads::Threads)