#include <ecs/systems.h>

#include <render/shadow.h>

#include <stdio.h>

static void setupDirLight(struct transformComponent *transform, struct dirLightComponent *light, struct meshRenderData *renderData, int i, int count) {
	struct vec3 direction = (struct vec3) { 0, 0, -1 };
	direction = vectorRotate(direction, transform->rotation);

	char name[80];
	sprintf(name, "dirLights[%d].direction", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &direction);

	sprintf(name, "dirLights[%d].ambientColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->ambientColor);

	sprintf(name, "dirLights[%d].diffuseColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->diffuseColor);

	sprintf(name, "dirLights[%d].specularColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->specularColor);

	int cascadeCount = light->castShadows ? light->shadowCascadeCount : 0;
	if (cascadeCount < 0) cascadeCount = 0;
	if (cascadeCount > MAX_SHADOW_CASCADES) cascadeCount = MAX_SHADOW_CASCADES;

	sprintf(name, "dirLights[%d].shadowCascadeCount", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, name, &cascadeCount);

	if (cascadeCount > 0) {
		float bias = light->shadowBias > 0 ? light->shadowBias : 0.0005f;
		float normalBias = light->shadowNormalBias > 0 ? light->shadowNormalBias : 0.03f;
		float slopeBias = light->shadowSlopeBias > 0 ? light->shadowSlopeBias : 0.002f;
		float softness = light->shadowSoftness > 0 ? light->shadowSoftness : 1.0f;
		float blend = light->shadowCascadeBlend;

		sprintf(name, "dirLights[%d].shadowBias", i);
		shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &bias);

		sprintf(name, "dirLights[%d].shadowNormalBias", i);
		shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &normalBias);

		sprintf(name, "dirLights[%d].shadowSlopeBias", i);
		shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &slopeBias);

		sprintf(name, "dirLights[%d].shadowSoftness", i);
		shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &softness);

		sprintf(name, "dirLights[%d].shadowCascadeBlend", i);
		shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &blend);

		for (int c = 0; c < cascadeCount; c++) {
			int slot = light->shadowIndices[c];
			if (slot < 0) continue;

			sprintf(name, "dirLights[%d].shadowMatrices[%d]", i, c);
			shaderPushUniform(renderData, RENDERER_UNIFORM_MATRIX_4, name, &light->shadowMatrices[c]);

			sprintf(name, "dirLights[%d].shadowSplits[%d]", i, c);
			shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &light->shadowSplits[c]);

			sprintf(name, "dirLights[%d].shadowIndices[%d]", i, c);
			shaderPushUniform(renderData, RENDERER_UNIFORM_INT, name, &slot);

			renderData->textures[renderData->texturesLength++] = (struct textureRenderData) {
				.type = RENDERER_TEXTURE_2D,
				.slot = GL_TEXTURE0 + SHADOW_2D_UNIT_BASE + slot,
				.buffer = shadowTexture2D(slot),
			};
		}
	}

	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, "dirLightsLength", &count);
}

void rendererGetDirLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct dirLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct dirLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
				setupDirLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}

			for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
				setupDirLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}
        }
    }
}
