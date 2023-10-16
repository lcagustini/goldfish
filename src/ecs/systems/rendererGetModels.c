#include <ecs/systems.h>

void rendererGetModels(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct modelComponent *models = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

	for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
		struct rendererDataComponent *rendererData = &rendererDatas[k];
		memset(rendererData, 0, sizeof(struct rendererDataComponent));
	}

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct modelComponent *model = &models[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < model->meshesLength; j++) {
				struct meshComponent *mesh = &model->meshes[j];
				struct meshRenderData *renderData = mesh->material.transparent ? &rendererData->transparentMeshes[rendererData->transparentMeshesLength++] : &rendererData->opaqueMeshes[rendererData->opaqueMeshesLength++];

				renderData->shader = mesh->material.shader;
				renderData->VAO = model->meshes[j].VAO;
				renderData->indicesLength = model->meshes[j].indicesLength;

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_MATRIX_4,
					.location = renderData->shader.modelLoc,
					.count = 1,
					.data.matrix = transform->modelMatrix
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_FLOAT,
					.location = renderData->shader.shininessLoc,
					.count = 1,
					.data.single = mesh->material.shininess
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_FLOAT,
					.location = renderData->shader.alphaClippingLoc,
					.count = 1,
					.data.single = mesh->material.alphaClipping
				};

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
