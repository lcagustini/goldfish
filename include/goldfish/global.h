#ifndef GOLDFISH_GLOBAL_H
#define GOLDFISH_GLOBAL_H

#include <stdbool.h>

#include <epoxy/gl.h>
#include <GLFW/glfw3.h>

#ifndef CIMGUI_USE_GLFW
#define CIMGUI_USE_GLFW
#endif
#ifndef CIMGUI_USE_OPENGL3
#define CIMGUI_USE_OPENGL3
#endif
#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include <imgui/cimgui.h>
#include <imgui/cimgui_impl.h>

struct globalState {
    GLFWwindow *window;
    bool headless;
    bool quitRequested;
};

extern struct globalState globalState;

void globalInit();

void globalEnd();

#endif
