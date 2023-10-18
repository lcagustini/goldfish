#include <ecs/systems.h>

#include <render/framebuffer.h>

void finishRender(struct systemRunData data) {
    struct cameraComponent *cameras = GET_SYSTEM_COMPONENTS(data, 0, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        struct cameraComponent *camera = &cameras[i];

		glUseProgram(camera->shader);
		glBindVertexArray(camera->VAO);
		glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, activeFramebuffers[camera->framebuffer].colorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6); 
    }
}
