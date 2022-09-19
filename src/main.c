#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include <PVR_PSP2/EGL/eglplatform.h>
#include <PVR_PSP2/EGL/egl.h>
#include <PVR_PSP2/gpu_es4/psp2_pvr_hint.h>
#include <PVR_PSP2/GLES2/gl2.h>
#include <PVR_PSP2/GLES2/gl2ext.h>

#include <psp2/ctrl.h>

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

int _newlib_heap_size_user = 200 * 1024 * 1024;
unsigned int sceLibcHeapSize = 32 * 1024 * 1024;

void updateCameraView(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENT(data, 0);
    struct cameraComponent *cameras = GET_SYSTEM_COMPONENT(data, 1);

    for (int i = 0; i < GET_SYSTEM_COMPONENT_LENGTH(data); i++) {
        struct transformComponent *transform = &transforms[i];
        struct cameraComponent *camera = &cameras[i];

        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, -1 }, transform->rotation);
        lookAt(&camera->viewMat, transform->position, vectorAdd(transform->position, dir), (struct vec3) { 0, 1, 0 });
    }
}

void updateControllerData(struct systemRunData data) {
    struct controllerDataComponent *controllers = GET_SYSTEM_COMPONENT(data, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENT_LENGTH(data); i++) {
        struct controllerDataComponent *controllerData = &controllers[i];

        sceCtrlPeekBufferPositive(0, &controllerData->data, 1);
    }
}

void updateTransformMatrix(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENT(data, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENT_LENGTH(data); i++) {
        struct transformComponent *transform = &transforms[i];

        loadIdentity(&transform->modelMatrix);
        scalingMatrix(&transform->modelMatrix, transform->scale);
        rotationMatrix(&transform->modelMatrix, transform->rotation);
        translationMatrix(&transform->modelMatrix, transform->position);
    }
}

void setupTransform(struct systemRunData data) {
    struct transformComponent *transform =  getComponent(data.world, data.entity, data.system->components[0]);
    transform->position = (struct vec3) {0};
    transform->rotation = (struct quat) {0, 0, 0, 1};
    transform->scale = (struct vec3) {1, 1, 1};
}

void setupCamera(struct systemRunData data) {
    struct cameraComponent *camera = getComponent(data.world, data.entity, data.system->components[0]);

    createProjectionMatrix(&camera->projectionMat, 75, (float)globalState.surfaceWidth/(float)globalState.surfaceHeight);
}

void updateFirstPersonTransform(struct systemRunData data) {
    componentId controllerType[] = { GET_COMPONENT_ID(data.world, struct controllerData) };
    tableId controllerTable;
    getAllTablesWithComponents(data.world, controllerType, 1, &controllerTable, 1);

    struct controllerDataComponent *controllerData = getComponentFromTable(data.world, controllerTable, controllerType[0]);

    struct transformComponent *transforms = GET_SYSTEM_COMPONENT(data, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENT_LENGTH(data); i++) {
        struct transformComponent *transform = &transforms[i];

        SceCtrlData ctrl = controllerData->data;

        struct vec2 leftAnalog = { (ctrl.lx - 128) / 128.0f , (ctrl.ly - 128) / 128.0f };
        struct vec2 rightAnalog = { (ctrl.rx - 128) / 128.0f , (ctrl.ry - 128) / 128.0f };

        if (vectorLenSquared2D(leftAnalog) < 0.1f) leftAnalog = (struct vec2) {0};
        else leftAnalog = vectorScale2D(1 / 60.0f, leftAnalog);
        if (vectorLenSquared2D(rightAnalog) < 0.1f) rightAnalog = (struct vec2) {0};
        else rightAnalog = vectorScale2D(1 / 60.0f, rightAnalog);

        struct quat xQuat = quatFromAxisAngle((struct vec3) { 0, -1, 0 }, rightAnalog.x);
        struct quat yQuat = quatFromAxisAngle((struct vec3) { -1, 0, 0 }, rightAnalog.y);
        struct quat rot = quatMult(xQuat, yQuat);
        transform->rotation = quatMult(rot, transform->rotation);

        struct vec3 dir = vectorRotate((struct vec3) { 0, 0, 1 }, transform->rotation);
        transform->position = vectorAdd(transform->position, vectorScale(leftAnalog.y, dir));

        struct vec3 right = vectorRotate((struct vec3) { 1, 0, 0 }, transform->rotation);
        transform->position = vectorAdd(transform->position, vectorScale(leftAnalog.x, right));
    }
}

void renderModel(struct systemRunData data) {
/*
    enum componentType types[] = { COMPONENT_TRANSFORM, COMPONENT_MODEL };
    unsigned int *entities = getEntitiesWithComponents(world, types, 2);

    enum componentType cameraType[] = { COMPONENT_TRANSFORM, COMPONENT_CAMERA };
    unsigned int *cameraEntity = getEntitiesWithComponents(world, cameraType, 2);

    enum componentType dirLightType[] = { COMPONENT_TRANSFORM, COMPONENT_DIR_LIGHT };
    unsigned int *dirLightEntities = getEntitiesWithComponents(world, dirLightType, 2);

    enum componentType spotLightType[] = { COMPONENT_TRANSFORM, COMPONENT_SPOT_LIGHT };
    unsigned int *spotLightEntities = getEntitiesWithComponents(world, spotLightType, 2);

    enum componentType pointLightType[] = { COMPONENT_TRANSFORM, COMPONENT_POINT_LIGHT };
    unsigned int *pointLightEntities = getEntitiesWithComponents(world, pointLightType, 2);

    for (int i = 1; i < entities[0]; i++) {
        struct transformComponent *transform = getComponent(world, entities[i], COMPONENT_TRANSFORM);

        struct modelComponent *model = getComponent(world, entities[i], COMPONENT_MODEL);

        for (int j = 0; j < model->model.meshesLength; j++) {
            glUniformMatrix4fv(model->model.meshes[j].material.shader.modelLoc, 1, false, &transform->modelMatrix.mat[0][0]);
            glUniformMatrix4fv(model->model.meshes[j].material.shader.viewLoc, 1, false, &world->componentAllocator.cameraComponents[world->entities[cameraEntity[1]].components[COMPONENT_CAMERA]].viewMat.mat[0][0]);
            glUniformMatrix4fv(model->model.meshes[j].material.shader.projectionLoc, 1, false, &world->componentAllocator.cameraComponents[world->entities[cameraEntity[1]].components[COMPONENT_CAMERA]].projectionMat.mat[0][0]);

            glUniform3fv(model->model.meshes[j].material.shader.cameraPosLoc, 1, &world->componentAllocator.transformComponents[world->entities[cameraEntity[1]].components[COMPONENT_TRANSFORM]].position.x);

            for (int k = 1; k <= 1; k++) {
                struct vec3 direction = {0};
                struct vec3 ambient = {0};
                struct vec3 diffuse = {0};
                struct vec3 specular = {0};

                if (k < dirLightEntities[0]) {
                    struct transformComponent *lightTransform = getComponent(world, dirLightEntities[k], COMPONENT_TRANSFORM);
                    struct dirLightComponent *light = getComponent(world, dirLightEntities[k], COMPONENT_DIR_LIGHT);

                    direction = (struct vec3) { 0, 0, -1 };
                    vectorRotate(direction, lightTransform->rotation);

                    ambient = light->ambientColor;
                    diffuse = light->diffuseColor;
                    specular = light->specularColor;
                }

                char directionString[] = "dirLights[0].direction";
                directionString[10] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, directionString), 1, &direction.x);

                char ambientString[] = "dirLights[0].ambientColor";
                ambientString[10] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, ambientString), 1, &ambient.x);

                char diffuseString[] = "dirLights[0].diffuseColor";
                diffuseString[10] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, diffuseString), 1, &diffuse.x);

                char specularString[] = "dirLights[0].specularColor";
                specularString[10] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, specularString), 1, &specular.x);
            }

            for (int k = 1; k <= 0; k++) {
                struct vec3 position = {0};
                struct vec3 attenuation = {0};
                struct vec3 ambient = {0};
                struct vec3 diffuse = {0};
                struct vec3 specular = {0};

                if (k < pointLightEntities[0]) {
                    struct transformComponent *lightTransform = getComponent(world, pointLightEntities[k], COMPONENT_TRANSFORM);
                    struct pointLightComponent *light = getComponent(world, pointLightEntities[k], COMPONENT_POINT_LIGHT);

                    position = lightTransform->position;

                    attenuation = light->attenuation;
                    ambient = light->ambientColor;
                    diffuse = light->diffuseColor;
                    specular = light->specularColor;
                }

                char positionString[] = "pointLights[0].position";
                positionString[12] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, positionString), 1, &position.x);

                char attenuationString[] = "pointLights[0].attenuation";
                attenuationString[12] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, attenuationString), 1, &attenuation.x);

                char ambientString[] = "pointLights[0].ambientColor";
                ambientString[12] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, ambientString), 1, &ambient.x);

                char diffuseString[] = "pointLights[0].diffuseColor";
                diffuseString[12] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, diffuseString), 1, &diffuse.x);

                char specularString[] = "pointLights[0].specularColor";
                specularString[12] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, specularString), 1, &specular.x);
            }

            for (int k = 1; k <= 0; k++) {
                struct vec3 position = {0};
                struct vec3 direction = {0};
                struct vec2 cutOff = {0};
                struct vec3 ambient = {0};
                struct vec3 diffuse = {0};
                struct vec3 specular = {0};

                if (k < pointLightEntities[0]) {
                    struct transformComponent *lightTransform = getComponent(world, spotLightEntities[k], COMPONENT_TRANSFORM);
                    struct spotLightComponent *light = getComponent(world, spotLightEntities[k], COMPONENT_SPOT_LIGHT);

                    position = lightTransform->position;

                    direction = (struct vec3) { 0, 0, -1 };
                    vectorRotate(direction, lightTransform->rotation);

                    cutOff = light->cutOff;
                    ambient = light->ambientColor;
                    diffuse = light->diffuseColor;
                    specular = light->specularColor;
                }

                char positionString[] = "spotLights[0].position";
                positionString[11] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, positionString), 1, &position.x);

                char directionString[] = "spotLights[0].direction";
                directionString[11] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, directionString), 1, &direction.x);

                char cutOffString[] = "spotLights[0].cutOff";
                cutOffString[11] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, cutOffString), 1, &cutOff.x);

                char ambientString[] = "spotLights[0].ambientColor";
                ambientString[11] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, ambientString), 1, &ambient.x);

                char diffuseString[] = "spotLights[0].diffuseColor";
                diffuseString[11] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, diffuseString), 1, &diffuse.x);

                char specularString[] = "spotLights[0].specularColor";
                specularString[11] = '0' + (k - 1);
                glUniform3fv(glGetUniformLocation(model->model.meshes[j].material.shader.program, specularString), 1, &specular.x);
            }
        }
        drawModel(&model->model);
    }

    free(entities);
    free(cameraEntity);
    free(dirLightEntities);
    free(spotLightEntities);
    free(pointLightEntities);
*/
}

int main() {
    resetPrint();

    globalInit();

    print("All init OK.\n");

    struct world ecsWorld = {0};

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

    entityId input = createEntity(&ecsWorld);
    addComponent(&ecsWorld, input, controllerDataId);

    entityId chest = createEntity(&ecsWorld);
    addComponent(&ecsWorld, chest, transformId);
    addComponent(&ecsWorld, chest, modelId);
    struct modelComponent *model = getComponent(&ecsWorld, chest, modelId);
    model->model = loadModel("app0:assets/chest.obj", "app0:assets/chest.qoi", "app0:assets/chest_normal.qoi", "app0:assets/chest_specular.qoi");

    entityId light = createEntity(&ecsWorld);
    addComponent(&ecsWorld, light, transformId);
    addComponent(&ecsWorld, light, dirLightId);
    struct dirLightComponent *dirLight = getComponent(&ecsWorld, light, dirLightId);
    dirLight->ambientColor = (struct vec3) { 1, 1, 1 };
    dirLight->diffuseColor = (struct vec3) { 1, 1, 1 };
    dirLight->specularColor = (struct vec3) { 1, 1, 1 };

    while (1) {
        struct transformComponent *transform = getComponent(&ecsWorld, chest, transformId);
        transform->rotation = quatMult(transform->rotation, (struct quat) { 0, 0.005, 0, 0.9999875 });

        runWorldPhase(&ecsWorld, SYSTEM_ON_UPDATE);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        runWorldPhase(&ecsWorld, SYSTEM_ON_RENDER);

        eglSwapBuffers(globalState.display, globalState.surface);
    }

    globalEnd();

    return 0;
}
