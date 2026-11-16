cmake_minimum_required(VERSION 3.5)

# The engine never builds alone: it builds a game. compile.lua passes the
# game's definition; configuring the engine without one is a misuse.
set(GOLDFISH_GAME_NAME "" CACHE STRING "Game name from its game.lua (set by compile.lua)")
set(GOLDFISH_GAME_DIR "" CACHE PATH "Game project root (set by compile.lua)")
if ("${GOLDFISH_GAME_NAME}" STREQUAL "" OR "${GOLDFISH_GAME_DIR}" STREQUAL "")
    message(FATAL_ERROR "The engine builds only a game. Configure it through a game's compile.lua (luajit <game>/../goldfish/compile.lua <game>.lua --build); direct engine configuration is not supported.")
endif ()

project(${GOLDFISH_GAME_NAME})

# This file is the engine root; every engine path hangs off GOLDFISH_DIR.
set(GOLDFISH_DIR ${CMAKE_SOURCE_DIR})

#add_definitions("-march=native -ftree-vectorize -ffast-math -fsigned-char -fno-optimize-sibling-calls")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c11 -Og -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11 -Og -g")

include(dependencies.cmake)

## Build and link
# A game is scripts and assets; the executable's sources are the engine's.
add_executable(${PROJECT_NAME}
    ${CPP_ENGINE_SOURCES}
    ${C_ENGINE_SOURCES}
)

# Vendored third-party code: imgui headers memset non-trivially-copyable types.
# clang warns (-Wnontrivial-memcall); we don't fix vendored code, we silence it here only.
set_source_files_properties(${C_ENGINE_SOURCES} ${CPP_ENGINE_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-Wno-nontrivial-memcall;-Wno-uninitialized-const-pointer;-Wno-unknown-warning-option"
)

include(linking.cmake)
