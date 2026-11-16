find_package(assimp REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(EPOXY REQUIRED epoxy)
pkg_check_modules(LUAJIT REQUIRED luajit)
find_package(glfw3 REQUIRED)

# Defines shared by the build and the native-cdef generator (one environment).
# The CIMGUI trio is needed by the cdef dump directly (it does not include
# global.h); C sources get it transitively from global.h. IMGUI_IMPL_API is
# the only define every target TU needs.
set(GOLDFISH_CIMGUI_DEFINES
    CIMGUI_USE_GLFW
    CIMGUI_USE_OPENGL3
    CIMGUI_DEFINE_ENUMS_AND_STRUCTS
)
set(GOLDFISH_DEFINES
    IMGUI_IMPL_API=extern\ \"C\"
    ${GOLDFISH_CIMGUI_DEFINES}
)

# Native cdef generator: dumps ffi.cdef text for goldfish/tools/nativeTypes.txt
# into build/generated/nativeCdefs.lua at every configure.
get_filename_component(GOLDFISH_CC_NAME "${CMAKE_C_COMPILER}" NAME)
if (NOT "${GOLDFISH_CC_NAME}" MATCHES "clang")
    message(FATAL_ERROR "The native cdef generator requires clang as C compiler (got '${GOLDFISH_CC_NAME}'). compile.lua runs cmake with CC=clang, or set CC=clang yourself.")
endif ()

find_program(LUAJIT_EXECUTABLE NAMES luajit luajit-2.1 luajit-2.0 REQUIRED)

set(GOLDFISH_DUMPTYPES "${GOLDFISH_DIR}/tools/nativeTypes.txt")
set(GOLDFISH_DUMPOUT "${CMAKE_BINARY_DIR}/generated/nativeCdefs.lua")

set(GOLDFISH_DUMPARGS)
foreach (dir ${EPOXY_INCLUDE_DIRS} ${GOLDFISH_DIR}/include ${GOLDFISH_DIR}/vendor/include)
    list(APPEND GOLDFISH_DUMPARGS -I ${dir})
endforeach ()
foreach (def ${GOLDFISH_DEFINES})
    list(APPEND GOLDFISH_DUMPARGS -D ${def})
endforeach ()

execute_process(
    COMMAND ${LUAJIT_EXECUTABLE} ${GOLDFISH_DIR}/tools/dumpCdefs.lua
        ${CMAKE_C_COMPILER}
        ${GOLDFISH_DIR}/include/ecs/components.h
        ${GOLDFISH_DUMPTYPES}
        ${GOLDFISH_DUMPOUT}
        ${GOLDFISH_DUMPARGS}
    RESULT_VARIABLE DUMP_RESULT
)

if (NOT "${DUMP_RESULT}" EQUAL 0)
    message(FATAL_ERROR "Native cdef generation failed (dumpCdefs.lua exit ${DUMP_RESULT}).")
endif ()

# Add any additional include paths here
include_directories(
    ${ASSIMP_INCLUDE_DIRS}
    ${EPOXY_INCLUDE_DIRS}
    ${LUAJIT_INCLUDE_DIRS}

    ${GOLDFISH_DIR}/include
    ${GOLDFISH_DIR}/vendor/include
)

# Add any additional library paths here
# ${CMAKE_CURRENT_BINARY_DIR} lets you use any library currently being built
link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

FILE(GLOB_RECURSE C_ENGINE_SOURCES ${GOLDFISH_DIR}/src/*.c ${GOLDFISH_DIR}/vendor/src/*.c)
FILE(GLOB_RECURSE CPP_ENGINE_SOURCES ${GOLDFISH_DIR}/src/*.cpp ${GOLDFISH_DIR}/vendor/src/*.cpp)

