#include <ecs/systems.h>

#include <render/shadow.h>

#include <stdio.h>

static void setupSpotLight(struct transformComponent *transform, struct spotLightComponent *light, struct meshRenderData *renderData, int i, int count) {
	struct vec3 direction = (struct vec3) { 0, 0, -1 };
	direction = vectorRotate(direction, transform->rotation);

	char name[64];
	sprintf(name, "spotLights[%d].position", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &transform->position);

	sprintf(name, "spotLights[%d].direction", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &direction);

	sprintf(name, "spotLights[%d].cutOff", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_2, name, &light->cutOff);

	sprintf(name, "spotLights[%d].ambientColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->ambientColor);

	sprintf(name, "spotLights[%d].diffuseColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->diffuseColor);

	sprintf(name, "spotLights[%d].specularColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->specularColor);

	int shadowIndex = light->castShadows ? light->shadowIndex : -1;
	float bias = light->shadowBias > 0 ? light->shadowBias : 0.0008f;
	float normalBias = light->shadowNormalBias > 0 ? light->shadowNormalBias : 0.02f;
	float slopeBias = light->shadowSlopeBias > 0 ? light->shadowSlopeBias : 0.002f;
	float softness = light->shadowSoftness > 0 ? light->shadowSoftness : 1.0f;

	sprintf(name, "spotLights[%d].shadowIndex", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, name, &shadowIndex);

	sprintf(name, "spotLights[%d].shadowBias", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &bias);

	sprintf(name, "spotLights[%d].shadowNormalBias", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &normalBias);

	sprintf(name, "spotLights[%d].shadowSlopeBias", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &slopeBias);

	sprintf(name, "spotLights[%d].shadowSoftness", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &softness);

	sprintf(name, "spotLights[%d].lightSpaceMatrix", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_MATRIX_4, name, &light->shadowMatrix);

	if (shadowIndex >= 0) {
		renderData->textures[renderData->texturesLength++] = (struct textureRenderData) {
			.type = RENDERER_TEXTURE_2D,
			.slot = GL_TEXTURE0 + SHADOW_2D_UNIT_BASE + shadowIndex,
			.buffer = shadowTexture2D(shadowIndex),
		};
	}

	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, "spotLightsLength", &count);
}

void rendererGetSpotLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct spotLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct spotLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
				setupSpotLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}

			for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
				setupSpotLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}
        }
    }
}
