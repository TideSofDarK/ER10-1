cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)

project(EquinoxReach LANGUAGES C CXX)

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
FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG eb0d214c17077ffe7cdb9c78ff0713a039aa07e1
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        Vendor/glad/gl.c
        Source/AssetDef.cxx
        Source/Memory.cxx
        Source/Utility.cxx
        Source/Main.cxx
        Source/Window.cxx
        Source/Game.cxx
        Source/Draw.cxx
        Source/Level.cxx
        Source/Player.cxx
        Source/Tile.cxx
        Source/AssetTools.cxx
)

# Make sure AssetDef gets recompiled whenever an asset is added or modified
file(GLOB_RECURSE ASSET_FILES
        CONFIGURE_DEPENDS
        "Asset/*.png"
        "Asset/*.obj"
        "Asset/*.glsl"
        "Asset/*.vert"
        "Asset/*.frag"
        )
list(JOIN ASSET_FILES "\;" ASSET_FILES)
set_source_files_properties(Source/AssetDef.cxx PROPERTIES OBJECT_DEPENDS ${ASSET_FILES})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        Vendor/
        Source/
)

if (WIN32)
    set(PLATFORM_LIBS Dwmapi.lib)
endif ()
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        SDL2::SDL2main
        SDL2::SDL2-static
        ${PLATFORM_LIBS}
)

set(CMAKE_EXE_LINKER_FLAGS "-static")

if (WIN32 AND CMAKE_BUILD_TYPE MATCHES Release)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE On)
endif ()