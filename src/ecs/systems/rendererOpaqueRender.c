#include <ecs/systems.h>

#include <render/framebuffer.h>

void rendererOpaqueRender(struct systemRunData data) {
    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 0, 0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct rendererDataComponent *rendererData = &rendererDatas[i];

        glBindFramebuffer(GL_FRAMEBUFFER, activeFramebuffers[rendererData->target].FBO);

        for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
            struct meshRenderData mesh = rendererData->opaqueMeshes[j];

            glUseProgram(mesh.shader.program);

            for (int k = 0; k < mesh.uniformsLength; k++) {
                shaderApplyUniform(&mesh.uniforms[k]);
            }

            glBindVertexArray(mesh.VAO);

            for (int k = 0; k < mesh.texturesLength; k++) {
                struct textureRenderData texture = mesh.textures[k];

                glActiveTexture(texture.slot);
                switch (texture.type) {
                    case RENDERER_TEXTURE_2D:
						glBindTexture(GL_TEXTURE_2D, texture.buffer);
						break;
                    case RENDERER_TEXTURE_CUBE_MAP:
						glBindTexture(GL_TEXTURE_CUBE_MAP, texture.buffer);
						break;
                }
            }

            glDrawElements(GL_TRIANGLES, mesh.indicesLength, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}