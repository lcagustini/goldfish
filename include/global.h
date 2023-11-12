#ifndef GOLDFISH_GLOBAL_H
#define GOLDFISH_GLOBAL_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <render/vkBase.h>
#include <render/vkDraw.h>

struct globalState {
    GLFWwindow *window;

    struct vulkanState vulkanState;
};

extern struct globalState globalState;

void initGlobal(void);
void destroyGlobal(void);

#endif //GOLDFISH_GLOBAL_H