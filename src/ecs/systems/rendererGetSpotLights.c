#include <ecs/systems.h>

void rendererGetSpotLights(struct systemRunData data) {
    struct transformComponent *transforms = GET_SYSTEM_COMPONENTS(data, 0, 0);
    struct spotLightComponent *lights = GET_SYSTEM_COMPONENTS(data, 0, 1);

    struct rendererDataComponent *rendererDatas = GET_SYSTEM_COMPONENTS(data, 1, 0);

    for (int i = 0; i < GET_SYSTEM_COMPONENTS_LENGTH(data, 0); i++) {
		struct transformComponent *transform = &transforms[i];
		struct spotLightComponent *light = &lights[i];

        for (int k = 0; k < GET_SYSTEM_COMPONENTS_LENGTH(data, 1); k++) {
			struct rendererDataComponent *rendererData = &rendererDatas[k];

			for (int j = 0; j < rendererData->meshesLength; j++) {
				struct meshRenderData *renderData = &rendererData->meshes[j];

				struct vec3 direction = (struct vec3) { 0, 0, -1 };
				vectorRotate(direction, transform->rotation);

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.spotLightLocs[i].position,
					.count = 1,
					.data.vector3 = transform->position
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.spotLightLocs[i].direction,
					.count = 1,
					.data.vector3 = direction
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_2,
					.location = renderData->shader.spotLightLocs[i].cutOff,
					.count = 1,
					.data.vector2 = light->cutOff
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.spotLightLocs[i].ambientColor,
					.count = 1,
					.data.vector3 = light->ambientColor
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.spotLightLocs[i].diffuseColor,
					.count = 1,
					.data.vector3 = light->diffuseColor
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_VECTOR_3,
					.location = renderData->shader.spotLightLocs[i].specularColor,
					.count = 1,
					.data.vector3 = light->specularColor
				};

				renderData->uniforms[renderData->uniformsLength++] = (struct uniformRenderData) {
					.type = RENDERER_UNIFORM_INT,
					.location = renderData->shader.spotLightsLengthLoc,
					.count = 1,
					.data.integer = GET_SYSTEM_COMPONENTS_LENGTH(data, 0)
				};
			}
        }
    }
}
