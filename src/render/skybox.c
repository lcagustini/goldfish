#include <stdlib.h>

#include <GL/glew.h>

#include <render/skybox.h>

#include <qoi.h>

#include <print.h>
#include <render/material.h>

void loadSkybox(const char *facePaths[6], struct skyboxComponent *skybox) {
    glGenTextures(1, &skybox->texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture);

    for (int i = 0; i < 6; i++) {
        qoi_desc desc;
        void *rgbPixels = qoi_read(facePaths[i], &desc, 3);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, desc.width, desc.height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbPixels);

        free(rgbPixels);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    float skyboxVertices[] = {
        -1, -1,  1, //0
        1, -1,  1, //1
        -1,  1,  1, //2
        1,  1,  1, //3
        -1, -1, -1, //4
        1, -1, -1, //5
        -1,  1, -1, //6
        1,  1, -1  //7
    };

    unsigned int skyboxIndices[] = {
        //Top
        2, 6, 7,
        7, 3, 2,

        //Bottom
        5, 4, 0,
        0, 1, 5,

        //Left
        6, 2, 0,
        0, 4, 6,

        //Right
        1, 3, 7,
        7, 5, 1,

        //Front
        0, 2, 3,
        3, 1, 0,

        //Back
        7, 6, 4,
        4, 5, 7
    };

    glGenVertexArrays(1, &skybox->VAO);
    glGenBuffers(1, &skybox->VBO);
    glGenBuffers(1, &skybox->EBO);

    glBindVertexArray(skybox->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, skybox->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

    GLuint vshader = loadShaderFromFile("assets/shaders/skybox_v.glsl", GL_VERTEX_SHADER);
    GLuint fshader = loadShaderFromFile("assets/shaders/skybox_f.glsl", GL_FRAGMENT_SHADER);

    skybox->shaderProgram = glCreateProgram();
    if (skybox->shaderProgram) {
        glAttachShader(skybox->shaderProgram, vshader);
        glAttachShader(skybox->shaderProgram, fshader);
        glLinkProgram(skybox->shaderProgram);

        GLint status;
        glGetProgramiv(skybox->shaderProgram, GL_LINK_STATUS, &status);
        print("Shader link status: %d\n", status);
        if (status == GL_FALSE) {
            GLchar log[256];
            glGetProgramInfoLog(fshader, 256, NULL, log);

            print("Failed to link shader program: %s\n", log);

            glDeleteProgram(skybox->shaderProgram);
            skybox->shaderProgram = 0;
        }

        glDeleteShader(fshader);
        glDeleteShader(vshader);

        glUseProgram(skybox->shaderProgram);
        glUniform1i(glGetUniformLocation(skybox->shaderProgram, "skybox"), 0);

        GLuint positionLoc = glGetAttribLocation(skybox->shaderProgram, "aPos");

        // vertex positions
        glEnableVertexAttribArray(positionLoc);
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void destroySkybox(struct skyboxComponent *skybox) {
}
