cmake_minimum_required(VERSION 3.10)
project(SteamLogCollector)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(steam-log-collector
    src/main.cpp
    src/logger.cpp
    src/steam-utils.cpp
)

if(WIN32)
    target_link_libraries(steam-log-collector advapi32)
endif()