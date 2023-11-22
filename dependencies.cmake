find_package(assimp REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# Add any additional include paths here
include_directories(
    ${ASSIMP_INCLUDE_DIRS}

    goldfish/include
    goldfish/vendor/include
)

# Add any additional library paths here
# ${CMAKE_CURRENT_BINARY_DIR} lets you use any library currently being built
link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

FILE(GLOB_RECURSE C_ENGINE_SOURCES goldfish/src/*.c goldfish/vendor/src/*.c)
FILE(GLOB_RECURSE CPP_ENGINE_SOURCES goldfish/src/*.cpp goldfish/vendor/src/*.cpp)

