// Include after the sampler uniforms and tangent varyings.

// Gamma-decoded diffuse map.
vec3 sampledAlbedo() {
    return pow(texture(textureMap, textCoord).rgb, vec3(2.2));
}

// Tangent-space normal from the normal map.
vec3 sampledNormal() {
    return normalize(2.0 * texture(normalMap, textCoord).rgb - 1.0);
}

// Skybox reflectance, without a light intensity factor.
vec3 sampledReflectance(vec3 viewDir, vec3 norm) {
    float reflectanceStrength = texture(reflectanceMap, textCoord).r;
    vec3 reflectionDir = reflect(viewDir, norm);
    return reflectanceStrength * texture(skybox, reflectionDir).rgb;
}

// Specular highlight plus skybox reflectance for one light.
vec3 viewEffects(vec3 lightDir, vec3 norm, vec3 specularColor) {
    float specularStrength = texture(specularMap, textCoord).r;
    vec3 viewDir = normalize(tangentViewPosition - tangentPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * specularColor;

    return specular + sampledReflectance(viewDir, norm);
}
