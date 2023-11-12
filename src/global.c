#include <global.h>

#include <render/vkBase.h>

struct globalState globalState;

void initGlobal(void) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    globalState.window = glfwCreateWindow(800, 600, "goldfish", NULL, NULL);

    initVulkan();
}

void destroyGlobal(void) {
    destroyVulkan();

    glfwDestroyWindow(globalState.window);

    glfwTerminate();
}
