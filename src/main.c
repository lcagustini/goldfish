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
    CREATE_COMPONENT(&ecsWorld, struct rendererDataComponent);

    ADD_EVENT_SYSTEM(&ecsWorld, SYSTEM_ON_COMPONENT_ADD, setupTransform, GET_COMPONENT_ID(struct transformComponent));

    ADD_FILTER(&ecsWorld, "controllerDataFilter", GET_COMPONENT_ID(struct controllerDataComponent));
    ADD_FILTER(&ecsWorld, "firstPersonFilter", GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct firstPersonComponent));
    ADD_FILTER(&ecsWorld, "cameraFilter", GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct cameraComponent));
    ADD_FILTER(&ecsWorld, "transformFilter", GET_COMPONENT_ID(struct transformComponent));
    ADD_FILTER(&ecsWorld, "rendererDataFilter", GET_COMPONENT_ID(struct rendererDataComponent));
    ADD_FILTER(&ecsWorld, "modelFilter", GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct modelComponent));
    ADD_FILTER(&ecsWorld, "skyboxFilter", GET_COMPONENT_ID(struct skyboxComponent));
    ADD_FILTER(&ecsWorld, "renderCameraFilter", GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct cameraComponent), GET_COMPONENT_ID(struct rendererDataComponent));
    ADD_FILTER(&ecsWorld, "dirLightFilter", GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct dirLightComponent));
    ADD_FILTER(&ecsWorld, "spotLightFilter", GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct spotLightComponent));
    ADD_FILTER(&ecsWorld, "pointLightFilter", GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct pointLightComponent));

    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_PRE_UPDATE, updateControllerData, "controllerDataFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_UPDATE, updateFirstPersonTransform, "firstPersonFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_UPDATE, updateCameraView, "cameraFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_POST_UPDATE, updateTransformMatrix, "transformFilter");

    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_SETUP, rendererGetModels, "modelFilter", "rendererDataFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_SETUP, rendererGetCameras, "renderCameraFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_SETUP, rendererGetSkybox, "skyboxFilter", "rendererDataFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_SETUP, rendererGetDirLights, "dirLightFilter", "rendererDataFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_SETUP, rendererGetSpotLights, "spotLightFilter", "rendererDataFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_SETUP, rendererGetPointLights, "pointLightFilter", "rendererDataFilter");

    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_OPAQUE, rendererOpaqueRender, "rendererDataFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_SKYBOX, renderSkybox, "skyboxFilter");
    ADD_PHASE_SYSTEM(&ecsWorld, SYSTEM_ON_RENDER_TRANSPARENT, rendererTransparentRender, "rendererDataFilter");

    entityId camera = createEntity(&ecsWorld, "Camera");
    ADD_COMPONENT(&ecsWorld, camera, struct transformComponent);
    ADD_COMPONENT(&ecsWorld, camera, struct cameraComponent);
    ADD_COMPONENT(&ecsWorld, camera, struct firstPersonComponent);
    ADD_COMPONENT(&ecsWorld, camera, struct rendererDataComponent);
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

#if 1
    entityId chest1 = loadModel(&ecsWorld, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi", "assets/chest_reflectance.qoi", false);
    transform = GET_COMPONENT(&ecsWorld, chest1, struct transformComponent);
    transform->position = (struct vec3) { 0, -1, -1 };
#endif
#if 1
    entityId chest2 = loadModel(&ecsWorld, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi", "assets/chest_reflectance.qoi", false);
    transform = GET_COMPONENT(&ecsWorld, chest2, struct transformComponent);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
    transform->parent = chest1;
#endif
#if 1
    entityId chest3 = loadModel(&ecsWorld, "assets/chest.obj", "assets/chest.qoi", "assets/chest_normal.qoi", "assets/chest_specular.qoi", "assets/chest_reflectance.qoi", false);
    transform = GET_COMPONENT(&ecsWorld, chest3, struct transformComponent);
    transform->position = (struct vec3) { 0, -1, -1 };
    transform->scale = (struct vec3) { 0.5, 0.5, 0.5 };
    transform->parent = chest2;
#endif

    entityId skybox = createEntity(&ecsWorld, "Skybox");
    ADD_COMPONENT(&ecsWorld, skybox, struct skyboxComponent);
    const char *skyboxPaths[] = { "assets/skybox/clouds1_east.qoi", "assets/skybox/clouds1_west.qoi", "assets/skybox/clouds1_up.qoi", "assets/skybox/clouds1_down.qoi", "assets/skybox/clouds1_north.qoi", "assets/skybox/clouds1_south.qoi" };
    loadSkybox(skyboxPaths, GET_COMPONENT(&ecsWorld, skybox, struct skyboxComponent));

    entityId grass = loadModel(&ecsWorld, "assets/grass.fbx", "assets/grass.qoi", NULL, NULL, NULL, true);
    transform = GET_COMPONENT(&ecsWorld, grass, struct transformComponent);
    transform->position = (struct vec3) { 0, 0, 2 };

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

        runWorldPhase(&ecsWorld, SYSTEM_ON_PRE_UPDATE, deltaTime);

#if 1
        {
            struct transformComponent *transform = GET_COMPONENT(&ecsWorld, chest1, struct transformComponent);
            transform->rotation = quatMult(transform->rotation, (struct quat) { 0, 0.005, 0, 0.9999875 });
        }
#endif

        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_POST_UPDATE, deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        struct rendererDataComponent *renderer = GET_COMPONENT(&ecsWorld, camera, struct rendererDataComponent);
		memset(renderer, 0, sizeof(struct rendererDataComponent));

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SETUP, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_OPAQUE, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_SKYBOX, deltaTime);

#if 0
        {
            print("[Opaque (%u)]\n", renderer->opaqueMeshesLength);
            for (int i = 0; i < renderer->opaqueMeshesLength; i++) {
                struct meshRenderData *meshData = &renderer->opaqueMeshes[i];

                print("(shader: %u) (VAO: %u) (indices: %u)\n", meshData->shader.program, meshData->VAO, meshData->indicesLength);
                for (int j = 0; j < meshData->uniformsLength; j++) {
                    struct uniformRenderData *uniformData = &meshData->uniforms[j];
                    print("uniform (type: %d) (loc: %u) (count: %u) (data: %p)\n", uniformData->type, uniformData->location, uniformData->count, uniformData->data);
                }

                for (int j = 0; j < meshData->texturesLength; j++) {
                    struct textureRenderData *textureData = &meshData->textures[j];
                    print("texture (type: %d) (slot: %u) (buffer: %u)\n", textureData->type, textureData->slot, textureData->buffer);
                }
                print("\n");
            }
			print("\n");

            print("[Transparent (%u)]\n", renderer->transparentMeshesLength);
            for (int i = 0; i < renderer->transparentMeshesLength; i++) {
                struct meshRenderData *meshData = &renderer->transparentMeshes[i];

                print("(shader: %u) (VAO: %u) (indices: %u)\n", meshData->shader.program, meshData->VAO, meshData->indicesLength);
                for (int j = 0; j < meshData->uniformsLength; j++) {
                    struct uniformRenderData *uniformData = &meshData->uniforms[j];
                    print("uniform (type: %d) (loc: %u) (count: %u) (data: %p)\n", uniformData->type, uniformData->location, uniformData->count, uniformData->data);
                }

                for (int j = 0; j < meshData->texturesLength; j++) {
                    struct textureRenderData *textureData = &meshData->textures[j];
                    print("texture (type: %d) (slot: %u) (buffer: %u)\n", textureData->type, textureData->slot, textureData->buffer);
                }
                print("\n");
            }
			print("\n");
        }
#endif

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_TRANSPARENT, deltaTime);
        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER_POST, deltaTime);

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
