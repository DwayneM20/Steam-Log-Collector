cmake_minimum_required(VERSION 3.10)
project(SteamLogCollector)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(steam-log0llector
    src/main.cpp
    src/logger.cpp
)