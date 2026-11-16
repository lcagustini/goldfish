target_compile_definitions(${PROJECT_NAME} PRIVATE
    IMGUI_IMPL_API=extern\ \"C\"
)

target_link_libraries(${PROJECT_NAME}
    ${EPOXY_LIBRARIES}

    ${ASSIMP_LIBRARIES}

    ${LUAJIT_LIBRARIES}

    glfw

    m
)

# Scripts call goldfish/src/math/*.c through the FFI by symbol name; exporting
# the executable makes those symbols resolvable without any marshaling.
set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

if (WIN32)
target_link_libraries(${PROJECT_NAME}
    opengl32
)
else ()
target_link_libraries(${PROJECT_NAME}
    GL
)
endif (WIN32)

