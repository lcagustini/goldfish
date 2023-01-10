#include <ecs/systems.h>

void setupCamera(struct systemRunData data) {
    struct cameraComponent *camera = GET_SYSTEM_COMPONENT(data);

    int width, height;
    glfwGetWindowSize(globalState.window, &width, &height);
    createProjectionMatrix(&camera->projectionMat, 75, (float)width/(float)height);
}

