#include <ecs/systems.h>

#include <render/framebuffer.h>

#include <print.h>

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
                struct uniformRenderData uniform = mesh.uniforms[k];

                switch (uniform.type) {
                    case RENDERER_UNIFORM_MATRIX_4:
                        glUniformMatrix4fv(uniform.location, uniform.count, false, &uniform.data.matrix.a1);
                        break;
                    case RENDERER_UNIFORM_VECTOR_3:
                        glUniform3fv(uniform.location, uniform.count, &uniform.data.vector3.x);
                        break;
                    case RENDERER_UNIFORM_VECTOR_2:
                        glUniform2fv(uniform.location, uniform.count, &uniform.data.vector2.x);
                        break;
                    case RENDERER_UNIFORM_INT:
                        glUniform1i(uniform.location, uniform.data.integer);
                        break;
                    case RENDERER_UNIFORM_FLOAT:
                        glUniform1f(uniform.location, uniform.data.single);
                        break;
                }
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
