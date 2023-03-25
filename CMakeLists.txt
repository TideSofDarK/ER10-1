cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)

project(EquinoxReach)

set(EMBEDDED_ASSETS "")
macro(embed_asset ASSET_PATH)
    set(ASSET_PATH_TEMP ${ASSET_PATH})
    cmake_path(RELATIVE_PATH ASSET_PATH_TEMP BASE_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ASSET_PATH_TEMP)

    cmake_path(GET ASSET_PATH_TEMP PARENT_PATH ASSET_DIR)
    cmake_path(GET ASSET_PATH_TEMP STEM ASSET_NAME)
    cmake_path(GET ASSET_PATH_TEMP EXTENSION ASSET_EX)

    cmake_path(ABSOLUTE_PATH ASSET_PATH_TEMP NORMALIZE OUTPUT_VARIABLE ASSET_PATH_TEMP)

    list(APPEND EMBEDDED_ASSETS "${PROJECT_BINARY_DIR}/${ASSET_NAME}${ASSET_EX}.o")

    add_custom_command(
            OUTPUT "${PROJECT_BINARY_DIR}/${ASSET_NAME}${ASSET_EX}.o"
            COMMAND ld -r -b binary -o "${PROJECT_BINARY_DIR}/${ASSET_NAME}${ASSET_EX}.o" "${ASSET_NAME}${ASSET_EX}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/${ASSET_DIR}
            DEPENDS ${ASSET_PATH_TEMP}
            VERBATIM
    )
    add_custom_target(
            "${ASSET_NAME}${ASSET_EX}" ALL
            DEPENDS "${PROJECT_BINARY_DIR}/${ASSET_NAME}${ASSET_EX}.o" ${ASSET_PATH_TEMP}
    )
endmacro()

file(GLOB_RECURSE ASSET_FILES
        "Assets/*.png"
        "Assets/*.obj"
        "Assets/*.glsl"
        "Assets/*.vert"
        "Assets/*.frag")
foreach (ASSET_FILE ${ASSET_FILES})
    embed_asset(${ASSET_FILE})
endforeach ()

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
FetchContent_MakeAvailable(glm SDL)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        Vendor/glad/src/gl.c
        Source/Utility.cxx
        Source/Main.cxx
        Source/Window.cxx
        Source/Game.cxx
        Source/Draw.cxx
        Source/Level.cxx
        Source/Player.cxx
        Source/Tile.cxx
        Source/Resource.cxx
)
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
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
        ${EMBEDDED_ASSETS}
)

if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif ()

if (WIN32)
    # No console
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE On)
endif ()

if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
