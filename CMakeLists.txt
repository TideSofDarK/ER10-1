cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)

project(EquinoxReach LANGUAGES C CXX)

set(BUILD_SHARED_LIBS OFF)
set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
set(SDL_STATIC_ENABLED_BY_DEFAULT ON)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)
FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG eb0d214c17077ffe7cdb9c78ff0713a039aa07e1
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm SDL)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        Vendor/glad/src/gl.c
        Source/Memory.cxx
        Source/Utility.cxx
        Source/Main.cxx
        Source/Window.cxx
        Source/Game.cxx
        Source/Draw.cxx
        Source/Level.cxx
        Source/Player.cxx
        Source/Tile.cxx
        Source/Asset.cxx
)
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        Vendor/incbin/
        Vendor/stb
        Vendor/glad/include
        Source/
)
if (WIN32)
    set(PLATFORM_LIBS Dwmapi.lib)
endif (WIN32)
target_link_libraries(
        ${PROJECT_NAME}
        SDL2::SDL2main
        SDL2::SDL2-static
        glm::glm
        ${PLATFORM_LIBS}
)

if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif ()

if (WIN32 AND CMAKE_BUILD_TYPE MATCHES Release)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE On)
endif ()

if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()