#include <ecs/systems.h>

void renderModel(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0);
    struct modelComponent *models = GET_SYSTEM_COMPONENTS(data, 1);

    componentId cameraTypes[] = { GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct cameraComponent) };
    tableId cameraTable;
    getAllTablesWithComponents(data.world, cameraTypes, 2, &cameraTable, 1);
    struct transformComponent *cameraTransform = getComponentsFromTable(data.world, cameraTable, cameraTypes[0]);
    struct cameraComponent *camera = getComponentsFromTable(data.world, cameraTable, cameraTypes[1]);

    componentId skyboxTypes[] = { GET_COMPONENT_ID(struct skyboxComponent) };
    tableId skyboxTable;
    getAllTablesWithComponents(data.world, skyboxTypes, 1, &skyboxTable, 1);
    struct skyboxComponent *skybox = getComponentsFromTable(data.world, skyboxTable, skyboxTypes[0]);

    componentId dirLightTypes[] = { GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct dirLightComponent) };
    tableId dirLightTables[MAX_LIGHTS];
    unsigned int dirLightTablesLength = getAllTablesWithComponents(data.world, dirLightTypes, 2, dirLightTables, MAX_LIGHTS);

    componentId spotLightTypes[] = { GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct spotLightComponent) };
    tableId spotLightTables[MAX_LIGHTS];
    unsigned int spotLightTablesLength = getAllTablesWithComponents(data.world, spotLightTypes, 2, spotLightTables, MAX_LIGHTS);

    componentId pointLightTypes[] = { GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct pointLightComponent) };
    tableId pointLightTables[MAX_LIGHTS];
    unsigned int pointLightTablesLength = getAllTablesWithComponents(data.world, pointLightTypes, 2, pointLightTables, MAX_LIGHTS);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data); i++) {
        struct transformComponent *transform = &transforms[i];
        struct modelComponent *model = &models[i];

        for (int j = 0; j < model->meshesLength; j++) {
            glUseProgram(model->meshes[j].material.shader.program);

            glUniformMatrix4fv(model->meshes[j].material.shader.modelLoc, 1, false, &transform->modelMatrix.mat[0][0]);
            glUniformMatrix4fv(model->meshes[j].material.shader.viewLoc, 1, false, &camera->viewMat.mat[0][0]);
            glUniformMatrix4fv(model->meshes[j].material.shader.projectionLoc, 1, false, &camera->projectionMat.mat[0][0]);

            glUniform3fv(model->meshes[j].material.shader.cameraPosLoc, 1, &cameraTransform->position.x);

            for (int k = 0; k < dirLightTablesLength; k++) {
                struct transformComponent *lightTransform = getComponentsFromTable(data.world, dirLightTables[k], dirLightTypes[0]);
                struct dirLightComponent *light = getComponentsFromTable(data.world, dirLightTables[k], dirLightTypes[1]);

                struct vec3 direction = (struct vec3) { 0, 0, -1 };
                vectorRotate(direction, lightTransform->rotation);

                struct vec3 ambient = light->ambientColor;
                struct vec3 diffuse = light->diffuseColor;
                struct vec3 specular = light->specularColor;

                glUniform3fv(model->meshes[j].material.shader.dirLightLocs[k].direction, 1, &direction.x);
                glUniform3fv(model->meshes[j].material.shader.dirLightLocs[k].ambientColor, 1, &ambient.x);
                glUniform3fv(model->meshes[j].material.shader.dirLightLocs[k].diffuseColor, 1, &diffuse.x);
                glUniform3fv(model->meshes[j].material.shader.dirLightLocs[k].specularColor, 1, &specular.x);
            }
            glUniform1i(model->meshes[j].material.shader.dirLightsLengthLoc, dirLightTablesLength);

            for (int k = 0; k < spotLightTablesLength; k++) {
                struct transformComponent *lightTransform = getComponentsFromTable(data.world, spotLightTables[k], spotLightTypes[0]);
                struct spotLightComponent *light = getComponentsFromTable(data.world, spotLightTables[k], spotLightTypes[1]);

                struct vec3 position = lightTransform->position;
                struct vec3 direction = (struct vec3) { 0, 0, -1 };
                vectorRotate(direction, lightTransform->rotation);

                struct vec2 cutOff = light->cutOff;

                struct vec3 ambient = light->ambientColor;
                struct vec3 diffuse = light->diffuseColor;
                struct vec3 specular = light->specularColor;

                glUniform3fv(model->meshes[j].material.shader.spotLightLocs[k].position, 1, &position.x);
                glUniform3fv(model->meshes[j].material.shader.spotLightLocs[k].direction, 1, &direction.x);
                glUniform2fv(model->meshes[j].material.shader.spotLightLocs[k].cutOff, 1, &cutOff.x);
                glUniform3fv(model->meshes[j].material.shader.spotLightLocs[k].ambientColor, 1, &ambient.x);
                glUniform3fv(model->meshes[j].material.shader.spotLightLocs[k].diffuseColor, 1, &diffuse.x);
                glUniform3fv(model->meshes[j].material.shader.spotLightLocs[k].specularColor, 1, &specular.x);
            }
            glUniform1i(model->meshes[j].material.shader.spotLightsLengthLoc, spotLightTablesLength);

            for (int k = 0; k < pointLightTablesLength; k++) {
                struct transformComponent *lightTransform = getComponentsFromTable(data.world, pointLightTables[k], pointLightTypes[0]);
                struct pointLightComponent *light = getComponentsFromTable(data.world, pointLightTables[k], pointLightTypes[1]);

                struct vec3 position = lightTransform->position;

                struct vec3 attenuation = light->attenuation;
                struct vec3 ambient = light->ambientColor;
                struct vec3 diffuse = light->diffuseColor;
                struct vec3 specular = light->specularColor;

                glUniform3fv(model->meshes[j].material.shader.pointLightLocs[k].position, 1, &position.x);
                glUniform3fv(model->meshes[j].material.shader.pointLightLocs[k].attenuation, 1, &attenuation.x);
                glUniform3fv(model->meshes[j].material.shader.pointLightLocs[k].ambientColor, 1, &ambient.x);
                glUniform3fv(model->meshes[j].material.shader.pointLightLocs[k].diffuseColor, 1, &diffuse.x);
                glUniform3fv(model->meshes[j].material.shader.pointLightLocs[k].specularColor, 1, &specular.x);
            }
            glUniform1i(model->meshes[j].material.shader.pointLightsLengthLoc, pointLightTablesLength);

            glUniform1f(model->meshes[j].material.shader.shininessLoc, model->meshes[j].material.shininess);
            glUniform1f(model->meshes[j].material.shader.alphaClippingLoc, model->meshes[j].material.alphaClipping);

            glBindVertexArray(model->meshes[j].VAO);

            for (int i = 0; i < model->meshes[j].material.texturesLength; i++) {
                glActiveTexture(GL_TEXTURE0 + model->meshes[j].material.textures[i].type);
                glBindTexture(GL_TEXTURE_2D, model->meshes[j].material.textures[i].textureBuffer);
            }

            glActiveTexture(GL_TEXTURE0 + TEXTURE_CUBEMAP);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture);

            glDrawElements(GL_TRIANGLES, model->meshes[j].indicesLength, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
        }
    }
}

