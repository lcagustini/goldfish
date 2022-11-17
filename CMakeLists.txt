cmake_minimum_required(VERSION 3.2)

project(psvita-opengl)

find_package(assimp REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

#add_definitions("-marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -ffast-math -fsigned-char -fno-optimize-sibling-calls")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Og -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Og -std=c++11 -g")

# Add any additional include paths here
include_directories(
    ${ASSIMP_INCLUDE_DIRS}
    include
)

# Add any additional library paths here
# ${CMAKE_CURRENT_BINARY_DIR} lets you use any library currently being built
link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

## Build and link
# Add all the files needed to compile here
add_executable(${PROJECT_NAME}
    src/math/matrix.c
    src/math/vector.c

    src/render/model.c
    src/render/material.c

    src/ecs/world.c

    src/print.c
    src/global.c

    src/main.c
)

target_link_libraries(${PROJECT_NAME}
    ${GLEW_LIBRARIES}

    ${ASSIMP_LIBRARIES}

    glfw
    opengl32

    m
)
