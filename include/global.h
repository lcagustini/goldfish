#ifndef GLOBAL_H
#define GLOBAL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <imgui/cimgui.h>
#include <imgui/cimgui_impl.h>

struct globalState {
    GLFWwindow *window;
};

extern struct globalState globalState;

void globalInit();

void globalEnd();

#endif
