cmake_minimum_required(VERSION 4.00)
project(SteamLogCollector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GTKMM gtkmm-4.0 QUIET)
endif()

if(NOT GTKMM_FOUND)
    message(STATUS "GTK4/gtkmm-4.0 not found via PkgConfig, fetching dependencies...")
    
    FetchContent_Declare(
        glib
        GIT_REPOSITORY https://gitlab.gnome.org/GNOME/glib.git
        GIT_TAG 2.78.0
        GIT_SHALLOW TRUE
    )
    
    FetchContent_Declare(
        gtk4
        GIT_REPOSITORY https://gitlab.gnome.org/GNOME/gtk.git
        GIT_TAG 4.12.0
        GIT_SHALLOW TRUE
    )
    
    FetchContent_Declare(
        gtkmm4
        GIT_REPOSITORY https://gitlab.gnome.org/GNOME/gtkmm.git
        GIT_TAG 4.12.0
        GIT_SHALLOW TRUE
    )
    
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    set(GLIB_BUILD_TESTS OFF CACHE BOOL "Build glib tests" FORCE)
    set(GTK_BUILD_EXAMPLES OFF CACHE BOOL "Build GTK examples" FORCE)
    set(GTK_BUILD_TESTS OFF CACHE BOOL "Build GTK tests" FORCE)
    set(GTKMM_BUILD_EXAMPLES OFF CACHE BOOL "Build gtkmm examples" FORCE)
    set(GTKMM_BUILD_TESTS OFF CACHE BOOL "Build gtkmm tests" FORCE)
    
    FetchContent_MakeAvailable(glib gtk4 gtkmm4)
    
    set(GTKMM_INCLUDE_DIRS 
        ${gtkmm4_SOURCE_DIR}
        ${gtk4_SOURCE_DIR}
        ${glib_SOURCE_DIR}
        ${gtkmm4_BINARY_DIR}
        ${gtk4_BINARY_DIR}
        ${glib_BINARY_DIR}
    )
    set(GTKMM_LIBRARIES gtkmm-4.0)
    set(GTKMM_LIBRARY_DIRS)
    set(GTKMM_CFLAGS_OTHER)
    set(GTKMM_FOUND TRUE)
    
    message(STATUS "GTK4/gtkmm-4.0 will be built from source")
endif()

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