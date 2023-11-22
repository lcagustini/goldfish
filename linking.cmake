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

