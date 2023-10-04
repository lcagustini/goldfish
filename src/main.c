#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math/vector.h>
#include <math/matrix.h>

#include <render/model.h>
#include <render/skybox.h>

#include <ecs/world.h>

#include <print.h>
#include <global.h>

#define QOI_IMPLEMENTATION
#include <qoi.h>

#include <ecs/systems.h>

int main() {
    resetPrint();

    globalInit();

    print("All init OK.\n");

    struct world ecsWorld = createWorld();

    CREATE_COMPONENT(&ecsWorld, struct transformComponent);
    CREATE_COMPONENT(&ecsWorld, struct cameraComponent);
    CREATE_COMPONENT(&ecsWorld, struct firstPersonComponent);
    CREATE_COMPONENT(&ecsWorld, struct controllerDataComponent);
    CREATE_COMPONENT(&ecsWorld, struct modelComponent);
    CREATE_COMPONENT(&ecsWorld, struct dirLightComponent);
    CREATE_COMPONENT(&ecsWorld, struct spotLightComponent);
    CREATE_COMPONENT(&ecsWorld, struct pointLightComponent);
    CREATE_COMPONENT(&ecsWorld, struct skyboxComponent);

    // TODO: Apply GET_COMPONENT_ID to variadic arguments
    ADD_SYSTEM(&ecsWorld, 0, SYSTEM_ON_CREATE, setupTransform, GET_COMPONENT_ID(struct transformComponent));

    ADD_SYSTEM(&ecsWorld, 0, SYSTEM_ON_UPDATE, updateControllerData, GET_COMPONENT_ID(struct controllerDataComponent));
    ADD_SYSTEM(&ecsWorld, 1, SYSTEM_ON_UPDATE, updateFirstPersonTransform, GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct firstPersonComponent));
    ADD_SYSTEM(&ecsWorld, 2, SYSTEM_ON_UPDATE, updateCameraView, GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct cameraComponent));
    ADD_SYSTEM(&ecsWorld, 10, SYSTEM_ON_UPDATE, updateTransformMatrix, GET_COMPONENT_ID(struct transformComponent));

    ADD_SYSTEM(&ecsWorld, 0, SYSTEM_ON_RENDER, renderModel, GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct modelComponent));
    ADD_SYSTEM(&ecsWorld, 1, SYSTEM_ON_RENDER, renderSkybox, GET_COMPONENT_ID(struct skyboxComponent));

    entityId camera = createEntity(&ecsWorld, "Camera");
    ADD_COMPONENT(&ecsWorld, camera, struct transformComponent);
    ADD_COMPONENT(&ecsWorld, camera, struct cameraComponent);
    ADD_COMPONENT(&ecsWorld, camera, struct firstPersonComponent);
    struct cameraComponent *cameraComponent = GET_COMPONENT(&ecsWorld, camera, struct cameraComponent);
    cameraComponent->fov = 60.0f;
    cameraComponent->near = 0.1f;
    cameraComponent->far = 100.0f;
    struct firstPersonComponent *firstPerson = GET_COMPONENT(&ecsWorld, camera, struct firstPersonComponent);
    firstPerson->rotation = (struct vec2) { 0, 0 };
    firstPerson->rotationSpeed = 1.3f;
    firstPerson->moveSpeed = 2.0f;

    ADD_SINGLETON_COMPONENT(&ecsWorld, struct controllerDataComponent);

    struct transformComponent *transform;
#if 0
    entityId chest1 = createEntity(&ecsWorld, "Chest 1");
    addComponent(&ecsWorld, chest1, transformId);
    loadModel(&ecsWorld, chest1, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi");
    transform = getComponent(&ecsWorld, chest1, transformId);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
#endif

#if 0
    entityId chest2 = createEntity(&ecsWorld, "Chest 2");
    addComponent(&ecsWorld, chest2, transformId);
    loadModel(&ecsWorld, chest2, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi");
    transform = getComponent(&ecsWorld, chest2, transformId);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
    transform->parent = chest1;
#endif

#if 0
    entityId chest3 = createEntity(&ecsWorld, "Chest 3");
    addComponent(&ecsWorld, chest3, transformId);
    loadModel(&ecsWorld, chest3, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi");
    transform = getComponent(&ecsWorld, chest3, transformId);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
    transform->parent = chest2;
#endif

    entityId skybox = createEntity(&ecsWorld, "Skybox");
    ADD_COMPONENT(&ecsWorld, skybox, struct skyboxComponent);
    const char *skyboxPaths[] = { "assets/skybox/clouds1_east.qoi", "assets/skybox/clouds1_west.qoi", "assets/skybox/clouds1_up.qoi", "assets/skybox/clouds1_down.qoi", "assets/skybox/clouds1_north.qoi", "assets/skybox/clouds1_south.qoi" };
    loadSkybox(skyboxPaths, GET_COMPONENT(&ecsWorld, skybox, struct skyboxComponent));

    entityId grass = loadModel(&ecsWorld, "assets/grass.fbx", "assets/grass.qoi", NULL, NULL, NULL);
    transform = GET_COMPONENT(&ecsWorld, grass, struct transformComponent);
    transform->position = (struct vec3) { 0, 0, 2 };

#if 1
    entityId cubes = loadModel(&ecsWorld, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi", "assets/chest_reflectance.qoi");
    transform = GET_COMPONENT(&ecsWorld, cubes, struct transformComponent);
    transform->position = (struct vec3) { 0, -1, -1 };
#endif

    entityId light = createEntity(&ecsWorld, "Light");
    ADD_COMPONENT(&ecsWorld, light, struct transformComponent);
    ADD_COMPONENT(&ecsWorld, light, struct dirLightComponent);
    struct dirLightComponent *dirLight = GET_COMPONENT(&ecsWorld, light, struct dirLightComponent);
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

#if 1
        {
            struct transformComponent *transform = GET_COMPONENT(&ecsWorld, cubes, struct transformComponent);
            transform->rotation = quatMult(transform->rotation, (struct quat) { 0, 0.005, 0, 0.9999875 });
        }
#endif

        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE, deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER, deltaTime);

        glfwSwapBuffers(globalState.window);
    }

#if 0
    model = getComponent(&ecsWorld, chest1, modelId);
    destroyModel(model);
    model = getComponent(&ecsWorld, chest2, modelId);
    destroyModel(model);
    model = getComponent(&ecsWorld, chest3, modelId);
    destroyModel(model);
#endif

    destroyWorld(&ecsWorld);

    globalEnd();

    return 0;
}
