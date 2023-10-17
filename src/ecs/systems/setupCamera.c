#include <ecs/systems.h>

#include <render/material.h>

void setupCamera(struct systemRunData data) {
    struct cameraComponent *camera = GET_SYSTEM_COMPONENT(data);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &camera->colorBuffer);

    int width = 1280;
    int height = 720;
    //glfwGetWindowSize(globalState.window, &width, &height);

    float quadVertices[] = {  
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};	

    glGenFramebuffers(1, &camera->FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, camera->FBO);
	glBindTexture(GL_TEXTURE_2D, camera->colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, camera->colorBuffer, 0); 

	glGenRenderbuffers(1, &camera->RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, camera->RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, camera->RBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
