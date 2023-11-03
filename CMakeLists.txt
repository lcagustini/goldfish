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

FILE(GLOB_RECURSE C_SOURCES src/*.c)
FILE(GLOB_RECURSE CPP_SOURCES src/*.cpp)

## Build and link
# Add all the files needed to compile here
add_executable(${PROJECT_NAME}
    ${CPP_SOURCES}
	${C_SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
	IMGUI_IMPL_API=extern\ \"C\"
	IMGUI_IMPL_OPENGL_LOADER_GL3W
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

