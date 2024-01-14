cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_FILE OFF CACHE BOOL "" FORCE)
set(SDL_FILESYSTEM OFF CACHE BOOL "" FORCE)
set(SDL_MISC OFF CACHE BOOL "" FORCE)
set(SDL_DIRECTX OFF CACHE BOOL "" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_3DNOW OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG 649556befa156201116a4f25089597463d0efd44
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL)

project(EquinoxReach LANGUAGES C CXX)

# Make sure AssetDef gets recompiled whenever an asset is added or modified
file(GLOB_RECURSE ASSET_FILES
        CONFIGURE_DEPENDS
        "Asset/*.png"
        "Asset/*.obj"
        "Asset/*.glsl"
        "Asset/*.vert"
        "Asset/*.frag"
        "Asset/*.ttf"
)
list(JOIN ASSET_FILES "\;" ASSET_FILES)
set_source_files_properties(Source/AssetDef.cxx PROPERTIES OBJECT_DEPENDS ${ASSET_FILES})

macro(add_equinox_reach_target)
    set(ONE_VALUE_ARGS NAME)
    set(MULTI_VALUE_ARGS DEF SOURCES INCLUDE_DIRS LIBS)
    cmake_parse_arguments(TARGET "" "${ONE_VALUE_ARGS}"
            "${MULTI_VALUE_ARGS}" ${ARGN})

    add_executable(${TARGET_NAME})

    target_compile_definitions(${TARGET_NAME} PRIVATE ${TARGET_DEF})

    target_sources(
            ${TARGET_NAME}
            PRIVATE
            Vendor/glad/gl.c
            Source/Memory.cxx
            Source/AssetDef.cxx
            Source/Utility.cxx
            Source/Main.cxx
            Source/Window.cxx
            Source/Draw.cxx
            Source/Level.cxx
            Source/Blob.cxx
            Source/AssetTools.cxx
            Source/Game.cxx
            ${TARGET_SOURCES}
    )

    target_include_directories(
            ${TARGET_NAME}
            PRIVATE
            Vendor/
            Source/
            ${TARGET_INCLUDE_DIRS}
    )

    target_link_libraries(
            ${TARGET_NAME}
            PRIVATE
            SDL3::SDL3
            SDL3::SDL3-static
    )

    if (WIN32)
        if (CMAKE_BUILD_TYPE MATCHES Rel)
            set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE On)
        endif ()

        if (MINGW)
            set(CMAKE_EXE_LINKER_FLAGS "-static")
        endif ()
    endif ()

    if (CMAKE_BUILD_TYPE MATCHES Debug)
        #        target_compile_options(${NAME} PRIVATE -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wall -Wextra -Wpedantic)
    endif ()
endmacro()

add_equinox_reach_target(
        NAME
        EquinoxReach
        DEF
        EQUINOX_REACH_GAME
)

add_equinox_reach_target(
        NAME
        EquinoxReachDevelopment
        DEF
        EQUINOX_REACH_DEVELOPMENT
        SOURCES
        Source/DevTools.cxx
        Vendor/imgui/imgui.cpp
        Vendor/imgui/imgui_draw.cpp
        Vendor/imgui/imgui_tables.cpp
        Vendor/imgui/imgui_widgets.cpp
        Vendor/imgui/imgui_impl_sdl3.cpp
        Vendor/imgui/imgui_impl_opengl3.cpp
        # INCLUDE_DIRS
        # Vendor/imgui
)
