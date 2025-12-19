cmake_minimum_required(VERSION 4.0.3)
project(SteamLogCollector)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(OpenGL REQUIRED)

option(BUILD_GUI "Build the graphical user interface" ON)

if(BUILD_GUI)
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.5
    )
    FetchContent_MakeAvailable(imgui)

    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

     include_directories(
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    add_executable(steam-log-collector-gui
    src/main_gui.cpp
    src/logger.cpp
    src/steam-utils.cpp
    ${IMGUI_SOURCES}
    )

    target_link_libraries(steam-log-collector-gui
        glfw
        OpenGL::GL
    )

if(WIN32)
        target_link_libraries(steam-log-collector-gui advapi32)
    endif()
endif()

add_executable(steam-log-collector-cli
    src/main.cpp
    src/logger.cpp
    src/steam-utils.cpp
)

if(WIN32)
    target_link_libraries(steam-log-collector-cli advapi32)
endif()