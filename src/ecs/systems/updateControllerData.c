#include <ecs/systems.h>

void updateControllerData(struct systemRunData data) {
    struct controllerDataComponent *controllers = GET_SYSTEM_COMPONENTS(data, 0, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        struct controllerDataComponent *controllerData = &controllers[i];

        memset(controllerData, 0, sizeof (struct controllerDataComponent));

        if (glfwGetKey(globalState.window, GLFW_KEY_W) == GLFW_PRESS) {
            controllerData->ly = 0;
        }
        else if (glfwGetKey(globalState.window, GLFW_KEY_S) == GLFW_PRESS) {
            controllerData->ly = 255;
        }
        else {
            controllerData->ly = 128;
        }

        if (glfwGetKey(globalState.window, GLFW_KEY_A) == GLFW_PRESS) {
            controllerData->lx = 0;
        }
        else if (glfwGetKey(globalState.window, GLFW_KEY_D) == GLFW_PRESS) {
            controllerData->lx = 255;
        }
        else {
            controllerData->lx = 128;
        }

        if (glfwGetKey(globalState.window, GLFW_KEY_UP) == GLFW_PRESS) {
            controllerData->ry = 0;
        }
        else if (glfwGetKey(globalState.window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            controllerData->ry = 255;
        }
        else {
            controllerData->ry = 128;
        }

        if (glfwGetKey(globalState.window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            controllerData->rx = 0;
        }
        else if (glfwGetKey(globalState.window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            controllerData->rx = 255;
        }
        else {
            controllerData->rx = 128;
        }

        if (glfwGetKey(globalState.window, GLFW_KEY_Q) == GLFW_PRESS) {
            controllerData->lb = true;
        }
        else {
            controllerData->lb = false;
        }

        if (glfwGetKey(globalState.window, GLFW_KEY_E) == GLFW_PRESS) {
            controllerData->rb = true;
        }
        else {
            controllerData->rb = false;
        }
    }
}

