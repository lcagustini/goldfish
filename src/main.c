#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/model.h>

#include <ecs/world.h>

#include <print.h>
#include <global.h>

#include <ecs/systems.h>

int main() {
    resetPrint();

    globalInit();

    print("All init OK.\n");

    struct world ecsWorld = createWorld();

    componentId transformId = CREATE_COMPONENT(&ecsWorld, struct transformComponent);
    componentId cameraId = CREATE_COMPONENT(&ecsWorld, struct cameraComponent);
    componentId firstPersonId = CREATE_COMPONENT(&ecsWorld, struct firstPersonComponent);
    componentId controllerDataId = CREATE_COMPONENT(&ecsWorld, struct controllerDataComponent);
    componentId modelId = CREATE_COMPONENT(&ecsWorld, struct modelComponent);
    componentId dirLightId = CREATE_COMPONENT(&ecsWorld, struct dirLightComponent);
    componentId spotLightId = CREATE_COMPONENT(&ecsWorld, struct spotLightComponent);
    componentId pointLightId = CREATE_COMPONENT(&ecsWorld, struct pointLightComponent);

    ADD_SYSTEM(&ecsWorld, 0, SYSTEM_ON_CREATE, setupTransform, transformId);
    ADD_SYSTEM(&ecsWorld, 0, SYSTEM_ON_CREATE, setupCamera, cameraId);

    ADD_SYSTEM(&ecsWorld, 0, SYSTEM_ON_UPDATE, updateControllerData, controllerDataId);
    ADD_SYSTEM(&ecsWorld, 1, SYSTEM_ON_UPDATE, updateFirstPersonTransform, transformId, firstPersonId);
    ADD_SYSTEM(&ecsWorld, 2, SYSTEM_ON_UPDATE, updateCameraView, transformId, cameraId);
    ADD_SYSTEM(&ecsWorld, 10, SYSTEM_ON_UPDATE, updateTransformMatrix, transformId);

    ADD_SYSTEM(&ecsWorld, 0, SYSTEM_ON_RENDER, renderModel, transformId, modelId);

    entityId camera = createEntity(&ecsWorld);
    addComponent(&ecsWorld, camera, transformId);
    addComponent(&ecsWorld, camera, cameraId);
    addComponent(&ecsWorld, camera, firstPersonId);

    addSingletonComponent(&ecsWorld, controllerDataId);

    struct transformComponent *transform;
    struct modelComponent *model;
#if 1
    entityId chest1 = createEntity(&ecsWorld);
    addComponent(&ecsWorld, chest1, transformId);
    addComponent(&ecsWorld, chest1, modelId);
    model = getComponent(&ecsWorld, chest1, modelId);
    loadModel(model, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi");
    transform = getComponent(&ecsWorld, chest1, transformId);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
#endif

#if 1
    entityId chest2 = createEntity(&ecsWorld);
    addComponent(&ecsWorld, chest2, transformId);
    addComponent(&ecsWorld, chest2, modelId);
    model = getComponent(&ecsWorld, chest2, modelId);
    loadModel(model, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi");
    transform = getComponent(&ecsWorld, chest2, transformId);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
    transform->parent = chest1;
#endif

#if 1
    entityId chest3 = createEntity(&ecsWorld);
    addComponent(&ecsWorld, chest3, transformId);
    addComponent(&ecsWorld, chest3, modelId);
    model = getComponent(&ecsWorld, chest3, modelId);
    loadModel(model, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi");
    transform = getComponent(&ecsWorld, chest3, transformId);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
    transform->parent = chest2;
#endif

    entityId light = createEntity(&ecsWorld);
    addComponent(&ecsWorld, light, transformId);
    addComponent(&ecsWorld, light, pointLightId);
    struct pointLightComponent *dirLight = getComponent(&ecsWorld, light, pointLightId);
    dirLight->attenuation = (struct vec3) { 1, 0.7, 1.8 };
    dirLight->ambientColor = (struct vec3) { 1, 1, 1 };
    dirLight->diffuseColor = (struct vec3) { 1, 1, 1 };
    dirLight->specularColor = (struct vec3) { 1, 1, 1 };

    printWorld(&ecsWorld);

    double currentTime = glfwGetTime();
    double lastTime = currentTime;
    while (!glfwWindowShouldClose(globalState.window)) {
        currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        {
            struct transformComponent *transform = getComponent(&ecsWorld, chest1, transformId);
            transform->rotation = quatMult(transform->rotation, (struct quat) { 0, 0.005, 0, 0.9999875 });
        }

        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE, deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER, deltaTime);

        glfwSwapBuffers(globalState.window);
    }

    model = getComponent(&ecsWorld, chest1, modelId);
    destroyModel(model);
    model = getComponent(&ecsWorld, chest2, modelId);
    destroyModel(model);
    model = getComponent(&ecsWorld, chest3, modelId);
    destroyModel(model);

    destroyWorld(&ecsWorld);

    globalEnd();

    return 0;
}
