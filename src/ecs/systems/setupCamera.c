#include <ecs/systems.h>

#include <render/material.h>

#include <stdlib.h>

void setupCamera(struct systemRunData data) {
    struct cameraComponent *camera = GET_SYSTEM_COMPONENT(data);

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    camera->framebuffer = createWindowFramebuffer(FRAMEBUFFER_COLOR_DEPTH);

    camera->shader = malloc(sizeof(struct shader));
    if (!createShader(camera->shader, GOLDFISH_ENGINE_ASSETS "/shaders/quad_v.glsl", GOLDFISH_ENGINE_ASSETS "/shaders/quad_f.glsl")) {
        free(camera->shader);
        camera->shader = NULL;
    }

    GLuint program = camera->shader != NULL ? camera->shader->program : 0;

    glGenVertexArrays(1, &camera->VAO);
    glGenBuffers(1, &camera->VBO);

    glBindVertexArray(camera->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, camera->VBO);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), quadVertices, GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(glGetAttribLocation(program, "aPos"));
    glVertexAttribPointer(glGetAttribLocation(program, "aPos"), 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(glGetAttribLocation(program, "aTexCoords"));
    glVertexAttribPointer(glGetAttribLocation(program, "aTexCoords"), 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
