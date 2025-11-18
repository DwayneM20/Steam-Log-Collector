cmake_minimum_required(VERSION 3.10)
project(SteamLogCollector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-4.0 REQUIRED)

if(WIN32)
    set(PLATFORM_LIBS)
elseif(APPLE)
    set(PLATFORM_LIBS)
else()
    set(PLATFORM_LIBS pthread)
endif()

add_library(steam_core
    src/core/logger.cpp
    src/core/steam-utils.cpp
    src/core/steam-core.cpp
)

target_include_directories(steam_core PUBLIC src/core)
target_link_libraries(steam_core ${PLATFORM_LIBS})

if(MSVC)
    target_compile_options(steam_core PRIVATE /W4)
else()
    target_compile_options(steam_core PRIVATE -Wall -Wextra -pedantic)
endif()

add_executable(steam-log-collector-cli
    src/cli/main.cpp
)

target_link_libraries(steam-log-collector-cli steam_core ${PLATFORM_LIBS})

add_executable(steam-log-collector-gui
    src/gui/main.cpp
    src/gui/mainwindow.cpp
    src/gui/gamelistview.cpp
    src/gui/logfileview.cpp
)

target_include_directories(steam-log-collector-gui PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(steam-log-collector-gui PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(steam-log-collector-gui PRIVATE ${GTKMM_CFLAGS_OTHER})
target_link_libraries(steam-log-collector-gui steam_core ${GTKMM_LIBRARIES} ${PLATFORM_LIBS})

if(WIN32)
    target_link_libraries(steam_core advapi32)
    if(MSVC)
        set_property(TARGET steam-log-collector-gui PROPERTY WIN32_EXECUTABLE TRUE)
        set_property(TARGET steam-log-collector-cli PROPERTY WIN32_EXECUTABLE FALSE)
    endif()
endif()

install(TARGETS steam-log-collector-cli steam-log-collector-gui
        RUNTIME DESTINATION bin)