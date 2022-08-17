#include <render/material.h>

#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include <stdlib.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>
#include <print.h>

GLuint loadShader(const GLchar *shaderSrc, GLenum type, GLint *size) {
    print("Creating Shader...\n");

    GLuint shader = glCreateShader(type);

    if (shader == 0) {
        print("Failed to Create Shader\n");
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, size);

    print("Compiling Shader: %s...\n", shaderSrc);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            print("Error compiling shader:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }

    print("Shader Compiled!\n");
    return shader;
}

GLuint loadShaderFromFile(const char *shaderFile, GLenum type) {
    SceUID file = sceIoOpen(shaderFile, SCE_O_RDONLY, 0777);

    SceIoStat stat;
    sceIoGetstatByFd(file, &stat);

    GLchar *shaderStr = malloc(stat.st_size + 1);
    sceIoRead(file, shaderStr, stat.st_size);
    shaderStr[stat.st_size] = '\0';

    GLuint shader = loadShader(shaderStr, type, NULL);
    sceIoClose(file);
    free(shaderStr);

    return shader;
}

int createShader(struct shader *shader, const char *vertexPath, const char *fragmentPath) {
    GLuint vshader = loadShaderFromFile(vertexPath, GL_VERTEX_SHADER);
    GLuint fshader = loadShaderFromFile(fragmentPath, GL_FRAGMENT_SHADER);

    shader->program = glCreateProgram();
    if (shader->program) {
        glAttachShader(shader->program, vshader);
        glAttachShader(shader->program, fshader);
        glLinkProgram(shader->program);

        GLint status;
        glGetProgramiv(shader->program, GL_LINK_STATUS, &status);
        print("Shader link status: %d\n", status);
        if (status == GL_FALSE) {
            GLchar log[256];
            glGetProgramInfoLog(fshader, 256, NULL, log);

            print("Failed to link shader program: %s\n", log);

            glDeleteProgram(shader->program);
            shader->program = 0;

            return -1;
        }

        shader->modelLoc = glGetUniformLocation(shader->program, "model");
        shader->viewLoc = glGetUniformLocation(shader->program, "view");
        shader->projectionLoc = glGetUniformLocation(shader->program, "projection");

        shader->positionLoc = glGetAttribLocation(shader->program, "aPos");
        shader->normalLoc = glGetAttribLocation(shader->program, "aNormal");
        shader->textCoordLoc = glGetAttribLocation(shader->program, "aTexCoord");

        shader->cameraPosLoc = glGetUniformLocation(shader->program, "viewPos");

        glUseProgram(shader->program);
        glUniform1i(glGetUniformLocation(shader->program, "textureMap"), TEXTURE_DIFFUSE);
        glUniform1i(glGetUniformLocation(shader->program, "normalMap"), TEXTURE_NORMAL);
        glUniform1i(glGetUniformLocation(shader->program, "specularMap"), TEXTURE_SPECULAR);
    }
    else {
        print("Failed to create a shader program\n");

        glDeleteShader(vshader);
        glDeleteShader(fshader);
        return -1;
    }

    /* Shaders are now in the programs */
    glDeleteShader(fshader);
    glDeleteShader(vshader);
    return 0;
}

void loadTexture(struct texture *texture, const char *path, enum textureType textureType) {
    glGenTextures(1, &texture->textureBuffer);

    qoi_desc desc;
    void *rgbPixels = qoi_read(path, &desc, 3);

    glBindTexture(GL_TEXTURE_2D, texture->textureBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, desc.width, desc.height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbPixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    free(rgbPixels);

    texture->type = textureType;
    texture->path = path;
}

void createMaterial(struct material *material, const char *diffusePath, const char *normalPath, const char *specularPath) {
    createShader(&material->shader, "app0:assets/shaders/default_v.glsl", "app0:assets/shaders/default_f.glsl");

    if (diffusePath) {
        loadTexture(&material->textures[material->texturesLength], diffusePath, TEXTURE_DIFFUSE);
        material->texturesLength++;
    }
    if (normalPath) {
        loadTexture(&material->textures[material->texturesLength], normalPath, TEXTURE_NORMAL);
        material->texturesLength++;
    }
    if (specularPath) {
        loadTexture(&material->textures[material->texturesLength], specularPath, TEXTURE_SPECULAR);
        material->texturesLength++;
    }
}
