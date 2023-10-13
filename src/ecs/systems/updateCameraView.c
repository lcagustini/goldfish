#include <ecs/systems.h>

void updateCameraView(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct cameraComponent *cameras = GET_SYSTEM_COMPONENTS(data, 0, 1);

    int width, height;
    glfwGetWindowSize(globalState.window, &width, &height);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        struct transformComponent *transform = &transforms[i];
        struct cameraComponent *camera = &cameras[i];

        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, -1 }, transform->rotation);
        struct vec3 worldUp = { 0, 1, 0 };

        lookAt(&camera->viewMat, transform->position, dir, worldUp);

        createProjectionMatrix(&camera->projectionMat, camera->fov, (float)width/(float)height, camera->near, camera->far);
    }
}
