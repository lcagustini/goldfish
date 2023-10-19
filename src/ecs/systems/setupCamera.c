#include <ecs/systems.h>

#include <render/material.h>

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

    GLuint vshader = loadShaderFromFile("assets/shaders/quad_v.glsl", GL_VERTEX_SHADER);
    GLuint fshader = loadShaderFromFile("assets/shaders/quad_f.glsl", GL_FRAGMENT_SHADER);

    camera->shader = glCreateProgram();
    if (camera->shader) {
        glAttachShader(camera->shader, vshader);
        glAttachShader(camera->shader, fshader);
        glLinkProgram(camera->shader);
    }

    glGenVertexArrays(1, &camera->VAO);
    glGenBuffers(1, &camera->VBO);

    glBindVertexArray(camera->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, camera->VBO);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), quadVertices, GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(glGetAttribLocation(camera->shader, "aPos"));
    glVertexAttribPointer(glGetAttribLocation(camera->shader, "aPos"), 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(glGetAttribLocation(camera->shader, "aTexCoords"));
    glVertexAttribPointer(glGetAttribLocation(camera->shader, "aTexCoords"), 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
