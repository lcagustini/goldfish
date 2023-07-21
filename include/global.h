#ifndef GLOBAL_H
#define GLOBAL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct globalState {
    //EGLDisplay display;
    //EGLSurface surface;
    //EGLContext context;

    //EGLint surfaceWidth, surfaceHeight;

    GLFWwindow *window;
};

extern struct globalState globalState;

void globalInit();

void globalEnd();

#endif
