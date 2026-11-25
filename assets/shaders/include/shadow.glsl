// Include after lights.glsl, the shadow samplers, and uniform int receivesShadows.

const vec3 sampleOffsetDirections[20] = vec3[](
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

// 5x5 PCF with a hardware comparison sampler. Returns 1 when the fragment is shadowed.
float shadow2D(sampler2DShadow map, mat4 lightSpaceMatrix, vec3 fragPos, vec3 norm, vec3 lightDir, float bias, float normalBias, float slopeBias, float softness) {
    if (receivesShadows == 0) return 0.0;

    vec3 normalDir = normalize(norm);
    vec4 lightSpace = lightSpaceMatrix * vec4(fragPos + normalDir * normalBias, 1.0);
    vec3 proj = lightSpace.xyz / lightSpace.w * 0.5 + 0.5;
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z < 0.0 || proj.z > 1.0) return 0.0;

    float ndotl = clamp(dot(normalDir, normalize(lightDir)), 0.0, 1.0);
    float slope = sqrt(1.0 - ndotl * ndotl) / max(ndotl, 0.1);
    float depthBias = max(bias + slopeBias * slope, 0.0);

    vec2 texel = 1.0 / vec2(textureSize(map, 0));
    float lit = 0.0;
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            vec2 offset = vec2(x, y) * texel * max(softness, 0.0);
            lit += texture(map, vec3(proj.xy + offset, proj.z - depthBias));
        }
    }

    return 1.0 - lit / 25.0;
}

// 20-tap PCF over the cube map, using the reference's distance-scaled radius.
float shadowCube(samplerCubeShadow map, vec3 lightPos, float farPlane, vec3 fragPos, vec3 norm, float bias, float normalBias, float softness) {
    if (receivesShadows == 0) return 0.0;

    vec3 fragToLight = fragPos + normalize(norm) * normalBias - lightPos;
    float currentDepth = length(fragToLight) / farPlane;
    if (currentDepth > 1.0) return 0.0;

    float diskRadius = max(softness, 0.0) * (1.0 + currentDepth) / 25.0;

    float lit = 0.0;
    for (int i = 0; i < 20; i++) {
        lit += texture(map, vec4(fragToLight + sampleOffsetDirections[i] * diskRadius, currentDepth - bias));
    }

    return 1.0 - lit / 20.0;
}

// Picks one cascade. Each branch indexes the sampler array with a uniform slot.
float shadowCascade(int cascade, dirLight light, vec3 fragPos, vec3 norm, vec3 lightDir) {
    if (cascade == 0 && light.shadowIndices[0] >= 0) {
        return shadow2D(shadowMaps2D[light.shadowIndices[0]], light.shadowMatrices[0], fragPos, norm, lightDir, light.shadowBias, light.shadowNormalBias, light.shadowSlopeBias, light.shadowSoftness);
    }
    if (cascade == 1 && light.shadowIndices[1] >= 0) {
        return shadow2D(shadowMaps2D[light.shadowIndices[1]], light.shadowMatrices[1], fragPos, norm, lightDir, light.shadowBias, light.shadowNormalBias, light.shadowSlopeBias, light.shadowSoftness);
    }
    if (cascade == 2 && light.shadowIndices[2] >= 0) {
        return shadow2D(shadowMaps2D[light.shadowIndices[2]], light.shadowMatrices[2], fragPos, norm, lightDir, light.shadowBias, light.shadowNormalBias, light.shadowSlopeBias, light.shadowSoftness);
    }
    if (cascade == 3 && light.shadowIndices[3] >= 0) {
        return shadow2D(shadowMaps2D[light.shadowIndices[3]], light.shadowMatrices[3], fragPos, norm, lightDir, light.shadowBias, light.shadowNormalBias, light.shadowSlopeBias, light.shadowSoftness);
    }

    return 0.0;
}

float shadowDir(dirLight light, vec3 fragPos, vec3 norm, vec3 lightDir, float viewDepth) {
    if (light.shadowCascadeCount <= 0 || receivesShadows == 0) return 0.0;

    int last = light.shadowCascadeCount - 1;
    if (viewDepth > light.shadowSplits[last]) return 0.0;

    int cascade = 0;
    if (light.shadowCascadeCount > 1 && viewDepth > light.shadowSplits[0]) cascade = 1;
    if (light.shadowCascadeCount > 2 && viewDepth > light.shadowSplits[1]) cascade = 2;
    if (light.shadowCascadeCount > 3 && viewDepth > light.shadowSplits[2]) cascade = 3;

    float shadow = shadowCascade(cascade, light, fragPos, norm, lightDir);

    if (light.shadowCascadeBlend > 0.0 && cascade < last) {
        float cascadeNear = cascade == 0 ? 0.0 : light.shadowSplits[cascade - 1];
        float blendWidth = light.shadowCascadeBlend * (light.shadowSplits[cascade] - cascadeNear);
        float blendStart = light.shadowSplits[cascade] - blendWidth;

        if (blendWidth > 0.0 && viewDepth > blendStart) {
            float next = shadowCascade(cascade + 1, light, fragPos, norm, lightDir);
            float blend = clamp((viewDepth - blendStart) / blendWidth, 0.0, 1.0);
            shadow = mix(shadow, next, blend);
        }
    }

    return shadow;
}
