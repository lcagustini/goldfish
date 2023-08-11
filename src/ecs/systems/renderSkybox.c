#include <ecs/systems.h>

void renderSkybox(struct systemRunData data) {
    struct skyboxComponent *skyboxes = GET_SYSTEM_COMPONENTS(data, 0);

    componentId cameraTypes[] = { GET_COMPONENT_ID(struct transformComponent), GET_COMPONENT_ID(struct cameraComponent) };
    tableId cameraTable;
    getAllTablesWithComponents(data.world, cameraTypes, 2, &cameraTable, 1);
    struct cameraComponent *camera = getComponentsFromTable(data.world, cameraTable, cameraTypes[1]);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data); i++) {
        struct skyboxComponent *skybox = &skyboxes[i];

        glDepthMask(GL_FALSE);

        glUseProgram(skybox->shaderProgram);

        union mat4 viewMatrix;
        memcpy(&viewMatrix.mat[0][0], &camera->viewMat.mat[0][0], sizeof(union mat4));
        viewMatrix.mat[3][0] = 0;
        viewMatrix.mat[3][1] = 0;
        viewMatrix.mat[3][2] = 0;

        glUniformMatrix4fv(glGetUniformLocation(skybox->shaderProgram, "view"), 1, false, &viewMatrix.mat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skybox->shaderProgram, "projection"), 1, false, &camera->projectionMat.mat[0][0]);

        glBindVertexArray(skybox->VAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
    }
}

