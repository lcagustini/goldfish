#include <ecs/systems.h>

static void setupSkybox(struct skyboxComponent *skybox, struct meshRenderData *renderData) {
	renderData->textures[renderData->texturesLength++] = (struct textureRenderData) {
		.type = RENDERER_TEXTURE_CUBE_MAP,
		.slot = GL_TEXTURE0 + TEXTURE_CUBEMAP,
		.buffer = skybox->texture
	};
}

void rendererGetSkybox(struct systemRunData data) {
    struct skyboxComponent *skybox = GET_SYSTEM_COMPONENTS(data, 0, 0);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); i++) {
        struct rendererDataComponent *rendererData = &rendererDatas[i];

        for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
			struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
            setupSkybox(skybox, renderData);
        }

        for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
			struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
            setupSkybox(skybox, renderData);
        }
    }
}
