#include <render/material.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

    print("Compiling Shader...\n");
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
    FILE *file = fopen(shaderFile, "r");

    fseek(file, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(file);

    GLchar *shaderStr = malloc(len + 1);
    memset(shaderStr, 0, len + 1);
    fseek(file, 0, SEEK_SET);
    fread(shaderStr, len, 1, file);

    GLuint shader = loadShader(shaderStr, type, NULL);
    fclose(file);
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
        shader->tangentLoc = glGetAttribLocation(shader->program, "aTangent");

        shader->cameraPosLoc = glGetUniformLocation(shader->program, "viewPos");

        for (int i = 0; i < MAX_LIGHTS; i++) {
            char loc[100];
            sprintf(loc, "dirLights[%d].direction", i);
            shader->dirLightLocs[i].direction = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "dirLights[%d].ambientColor", i);
            shader->dirLightLocs[i].ambientColor = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "dirLights[%d].diffuseColor", i);
            shader->dirLightLocs[i].diffuseColor = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "dirLights[%d].specularColor", i);
            shader->dirLightLocs[i].specularColor = glGetUniformLocation(shader->program, loc);
        }
        shader->dirLightsLengthLoc = glGetUniformLocation(shader->program, "dirLightsLength");

        for (int i = 0; i < MAX_LIGHTS; i++) {
            char loc[100];
            sprintf(loc, "pointLights[%d].position", i);
            shader->pointLightLocs[i].position = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "pointLights[%d].attenuation", i);
            shader->pointLightLocs[i].attenuation = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "pointLights[%d].ambientColor", i);
            shader->pointLightLocs[i].ambientColor = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "pointLights[%d].diffuseColor", i);
            shader->pointLightLocs[i].diffuseColor = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "pointLights[%d].specularColor", i);
            shader->pointLightLocs[i].specularColor = glGetUniformLocation(shader->program, loc);
        }
        shader->pointLightsLengthLoc = glGetUniformLocation(shader->program, "pointLightsLength");

        for (int i = 0; i < MAX_LIGHTS; i++) {
            char loc[100];
            sprintf(loc, "spotLights[%d].position", i);
            shader->spotLightLocs[i].position = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "spotLights[%d].direction", i);
            shader->spotLightLocs[i].direction = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "spotLights[%d].cutOff", i);
            shader->spotLightLocs[i].cutOff = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "spotLights[%d].ambientColor", i);
            shader->spotLightLocs[i].ambientColor = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "spotLights[%d].diffuseColor", i);
            shader->spotLightLocs[i].diffuseColor = glGetUniformLocation(shader->program, loc);
            sprintf(loc, "spotLights[%d].specularColor", i);
            shader->spotLightLocs[i].specularColor = glGetUniformLocation(shader->program, loc);
        }
        shader->spotLightsLengthLoc = glGetUniformLocation(shader->program, "spotLightsLength");

        shader->shininessLoc = glGetUniformLocation(shader->program, "shininess");

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
    createShader(&material->shader, "assets/shaders/default_v.glsl", "assets/shaders/default_f.glsl");

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

    material->shininess = 64.0f;
}
