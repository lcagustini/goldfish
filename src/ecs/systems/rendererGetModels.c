#include <ecs/systems.h>

#include <render/shader.h>
#include <render/shadow.h>

#include <string.h>

void rendererGetModels(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct modelComponent *models = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    // Shadow map slots are claimed for one frame by the shadow passes.
    shadowBeginFrame();

    // This system runs once per model table. Only the first run starts a new list.
    struct filter *modelFilter = getFilter(data.world, data.system->filters[0]);
    if (data.filterResults[0].table == modelFilter->results[0]) {
        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
            struct rendererDataComponent *rendererData = &rendererDatas[k];
            memset(rendererData, 0, sizeof(struct rendererDataComponent));
        }
    }

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
        struct transformComponent *transform = &transforms[i];
        struct modelComponent *model = &models[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
            struct rendererDataComponent *rendererData = &rendererDatas[k];

            for (int j = 0; j < model->meshesLength; j++) {
                struct meshComponent *mesh = &model->meshes[j];

                // createShader failed at load; this mesh has no program.
                if (mesh->material.shader == NULL) continue;

                struct meshRenderData *renderData = mesh->material.transparent ? &rendererData->transparentMeshes[rendererData->transparentMeshesLength++] : &rendererData->opaqueMeshes[rendererData->opaqueMeshesLength++];

                renderData->shader = *mesh->material.shader;
                renderData->VAO = model->meshes[j].VAO;
                renderData->indicesLength = model->meshes[j].indicesLength;
                renderData->modelMatrix = transform->modelMatrix;
                renderData->castShadows = model->castShadows;
                renderData->receiveShadows = model->receiveShadows;

                int receivesShadows = model->receiveShadows ? 1 : 0;
                shaderPushUniform(renderData, RENDERER_UNIFORM_INT, "receivesShadows", &receivesShadows);

                shaderPushUniform(renderData, RENDERER_UNIFORM_MATRIX_4, "model", &transform->modelMatrix);
                shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, "shininess", &mesh->material.shininess);
                shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, "alphaClipping", &mesh->material.alphaClipping);

                for (int i = 0; i < model->meshes[j].material.texturesLength; i++) {
                    renderData->textures[renderData->texturesLength++] = (struct textureRenderData) {
                        .type = RENDERER_TEXTURE_2D,
                            .slot = GL_TEXTURE0 + mesh->material.textures[i].type,
                            .buffer = mesh->material.textures[i].textureBuffer
                    };
                }
            }
        }
    }
}
