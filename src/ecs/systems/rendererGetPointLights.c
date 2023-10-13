#include <ecs/systems.h>

void rendererGetPointLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct pointLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct pointLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->meshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->meshes[j];

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.spotLightLocs[i].position,
					.count = 1,
					.data.vector3 = transform->position
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.pointLightLocs[i].attenuation,
					.count = 1,
					.data.vector3 = light->attenuation
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.pointLightLocs[i].ambientColor,
					.count = 1,
					.data.vector3 = light->ambientColor
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.pointLightLocs[i].diffuseColor,
					.count = 1,
					.data.vector3 = light->diffuseColor
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.pointLightLocs[i].specularColor,
					.count = 1,
					.data.vector3 = light->specularColor
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_INT,
					.location = renderData->shader.pointLightsLengthLoc,
					.count = 1,
					.data.integer = GET_SYSTEM_COMPONENTS_LENGTH(data, 0)
				};
			}
        }
    }
}
