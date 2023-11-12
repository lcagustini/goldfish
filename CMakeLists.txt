cmake_minimum_required(VERSION 3.5)

project(goldfish)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

#add_definitions("-march=native -ftree-vectorize -ffast-math -fsigned-char -fno-optimize-sibling-calls")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11 -Og -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -Og -g")

FILE(GLOB_RECURSE C_SOURCES src/*.c)
FILE(GLOB_RECURSE CPP_SOURCES src/*.cpp)

add_executable(${PROJECT_NAME}
        ${CPP_SOURCES}
        ${C_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
        vulkan
        glfw

        m
)

target_include_directories(${PROJECT_NAME} PRIVATE
        #${ASSIMP_INCLUDE_DIRS}
        include
)

# ${CMAKE_CURRENT_BINARY_DIR} lets you use any library currently being built
target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)
