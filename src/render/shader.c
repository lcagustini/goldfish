#include <render/shader.h>

#include <PVR_PSP2/GLES2/gl2.h>

#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include <stdlib.h>

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

int initShaders(struct shader *shader) {
    GLuint fshader = loadShaderFromFile("app0:assets/shaders/default_f.glsl", GL_FRAGMENT_SHADER);
    GLuint vshader = loadShaderFromFile("app0:assets/shaders/default_v.glsl", GL_VERTEX_SHADER);

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
        glUniform1i(glGetUniformLocation(shader->program, "textureMap"), 0);
        glUniform1i(glGetUniformLocation(shader->program, "normalMap"), 1);
        glUniform1i(glGetUniformLocation(shader->program, "specularMap"), 2);
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
