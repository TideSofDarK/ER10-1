cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)

project(EquinoxReach)

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
        GIT_TAG 5a7ea76ed3ed45a498ddc17ae039323eb213876f
        GIT_PROGRESS TRUE
)
FetchContent_Declare(
        SDL_IMAGE
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
        GIT_TAG 44354bc1a5b472e1755761fb1405d768a69bbb74
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm SDL SDL_IMAGE)

add_executable(${PROJECT_NAME} "")
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${PROJECT_SOURCE_DIR}/Assets/")
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        Vendor/glad/src/gl.c
        Source/Main.cpp
        Source/Window.cpp
        Source/Game.cpp
        Source/Draw.cpp
        Source/Level.cpp
        Source/Player.cpp
)
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        Vendor/glad/include
        Source/
)
if (WIN32)
set(WINDOWS_LIBS Dwmapi.lib)
endif (WIN32)
target_link_libraries(
        ${PROJECT_NAME}
        SDL2::SDL2main
        SDL2::SDL2-static
        SDL2_image::SDL2_image-static
        glm::glm
        ${WINDOWS_LIBS}
)


