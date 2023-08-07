cmake_minimum_required(VERSION 3.5)

project(psvita-opengl)

find_package(assimp REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

#add_definitions("-march=native -ftree-vectorize -ffast-math -fsigned-char -fno-optimize-sibling-calls")

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

    src/ecs/systems/updateCameraView.c
    src/ecs/systems/updateControllerData.c
    src/ecs/systems/updateTransformMatrix.c
    src/ecs/systems/setupCamera.c
    src/ecs/systems/setupTransform.c
    src/ecs/systems/renderModel.c
    src/ecs/systems/updateFirstPersonTransform.c

    src/data/hashtable.c

    src/print.c
    src/global.c

    src/main.c
)

target_link_libraries(${PROJECT_NAME}
    ${GLEW_LIBRARIES}

    ${ASSIMP_LIBRARIES}

    glfw

    m
)

if (WIN32)
target_link_libraries(${PROJECT_NAME}
    opengl32
)
else ()
target_link_libraries(${PROJECT_NAME}
    GL
)
endif (WIN32)

