#include <ecs/systems.h>

#include <render/shadow.h>

#include <stdio.h>

static void setupPointLight(struct transformComponent *transform, struct pointLightComponent *light, struct meshRenderData *renderData, int i, int count) {
	char name[64];
	sprintf(name, "pointLights[%d].position", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &transform->position);

	sprintf(name, "pointLights[%d].attenuation", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->attenuation);

	sprintf(name, "pointLights[%d].ambientColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->ambientColor);

	sprintf(name, "pointLights[%d].diffuseColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->diffuseColor);

	sprintf(name, "pointLights[%d].specularColor", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_VECTOR_3, name, &light->specularColor);

	int shadowIndex = light->castShadows ? light->shadowIndex : -1;
	float range = light->range > 0 ? light->range : 25.0f;
	float bias = light->shadowBias > 0 ? light->shadowBias : 0.02f;
	float normalBias = light->shadowNormalBias > 0 ? light->shadowNormalBias : 0.02f;
	float softness = light->shadowSoftness > 0 ? light->shadowSoftness : 1.0f;

	sprintf(name, "pointLights[%d].farPlane", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &range);

	sprintf(name, "pointLights[%d].shadowIndex", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, name, &shadowIndex);

	sprintf(name, "pointLights[%d].shadowBias", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &bias);

	sprintf(name, "pointLights[%d].shadowNormalBias", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &normalBias);

	sprintf(name, "pointLights[%d].shadowSoftness", i);
	shaderPushUniform(renderData, RENDERER_UNIFORM_FLOAT, name, &softness);

	if (shadowIndex >= 0) {
		renderData->textures[renderData->texturesLength++] = (struct textureRenderData) {
			.type = RENDERER_TEXTURE_CUBE_MAP,
			.slot = GL_TEXTURE0 + SHADOW_CUBE_UNIT_BASE + shadowIndex,
			.buffer = shadowTextureCube(shadowIndex),
		};
	}

	shaderPushUniform(renderData, RENDERER_UNIFORM_INT, "pointLightsLength", &count);
}

void rendererGetPointLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct pointLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct pointLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->opaqueMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->opaqueMeshes[j];
				setupPointLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}

			for (int j = 0; j < rendererData->transparentMeshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->transparentMeshes[j];
				setupPointLight(transform, light, renderData, i, GET_SYSTEM_COMPONENTS_LENGTH(data, 0));
			}
        }
    }
}
