#include <ecs/systems.h>

void rendererGetSkybox(struct systemRunData data) {
    struct skyboxComponent *skybox = GET_SYSTEM_COMPONENTS(data, 0, 0);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); i++) {
        struct rendererDataComponent *rendererData = &rendererDatas[i];

        for (int j = 0; j < rendererData->meshesLength; j++) {
			struct meshRenderData *renderData = &rendererData->meshes[j];

			renderData->textures[renderData->texturesLength++] = (struct textureRenderData) {
				.type = RENDERER_TEXTURE_CUBE_MAP,
				.slot = GL_TEXTURE0 + TEXTURE_CUBEMAP,
				.buffer = skybox->texture
			};
        }
    }
}
