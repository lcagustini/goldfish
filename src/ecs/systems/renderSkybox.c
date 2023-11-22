#include <ecs/systems.h>

#include <render/framebuffer.h>

#include <string.h>

void renderSkybox(struct systemRunData data) {
    struct skyboxComponent *skyboxes = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct cameraComponent *cameras = GET_SYSTEM_COMPONENTS(data, 1, 0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
        struct cameraComponent *camera = &cameras[k];

        glBindFramebuffer(GL_FRAMEBUFFER, activeFramebuffers[camera->framebuffer].FBO);

        for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
            struct skyboxComponent *skybox = &skyboxes[i];

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
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
