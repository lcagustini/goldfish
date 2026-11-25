#version 460

#include "lights.glsl"

uniform dirLight dirLights[MAX_LIGHTS];
uniform int dirLightsLength;

uniform pointLight pointLights[MAX_LIGHTS];
uniform int pointLightsLength;

uniform spotLight spotLights[MAX_LIGHTS];
uniform int spotLightsLength;

uniform float shininess;
uniform float alphaClipping;

uniform sampler2D textureMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D reflectanceMap;
uniform samplerCube skybox;

uniform sampler2DShadow shadowMaps2D[MAX_SHADOW_2D];
uniform samplerCubeShadow shadowMapsCube[MAX_SHADOW_CUBE];

uniform int receivesShadows;

in vec3 position;
in vec3 normal;
in vec2 textCoord;

in float viewDepth;

in vec3 tangentPosition;
in vec3 tangentViewPosition;
in mat3 TBN;

out vec4 FragColor;

#include "shading.glsl"
#include "shadow.glsl"

vec3 dirLightning() {
    vec3 color = vec3(0.0);

    for (int i = 0; i < dirLightsLength; i++) {
        dirLight dl = dirLights[i];

        vec3 norm = sampledNormal();
        vec3 lightDir = TBN * normalize(dl.direction);

        float shadow = shadowDir(dl, position, normal, normalize(dl.direction), viewDepth);

        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * dl.ambientColor;
        vec3 diffuse = max(dot(norm, lightDir), 0.0) * dl.diffuseColor;

        vec3 lit = (ambient + (1.0 - shadow) * (diffuse + viewEffects(lightDir, norm, dl.specularColor))) * sampledAlbedo();
        color += lit;
    }

    return color;
}

vec3 pointLightning() {
    vec3 color = vec3(0.0);

    for (int i = 0; i < pointLightsLength; i++) {
        pointLight pl = pointLights[i];

        vec3 norm = sampledNormal();
        vec3 lightDir = (TBN * pl.position) - tangentPosition;
        float lightDistance = length(lightDir);
        lightDir = normalize(lightDir);

        float shadow = 0.0;
        if (pl.shadowIndex >= 0) shadow = shadowCube(shadowMapsCube[pl.shadowIndex], pl.position, pl.farPlane, position, normal, pl.shadowBias, pl.shadowNormalBias, pl.shadowSoftness);

        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * pl.ambientColor;
        vec3 diffuse = max(dot(norm, lightDir), 0.0) * pl.diffuseColor;

        float attenuation = 1.0 / (pl.attenuation.x + pl.attenuation.y * lightDistance + pl.attenuation.z * (lightDistance * lightDistance));

        color += attenuation * (ambient + (1.0 - shadow) * (diffuse + viewEffects(lightDir, norm, pl.specularColor))) * sampledAlbedo();
    }

    return color;
}

vec3 spotLightning() {
    vec3 color = vec3(0.0);

    for (int i = 0; i < spotLightsLength; i++) {
        spotLight sl = spotLights[i];

        vec3 lightDir = normalize((TBN * sl.position) - tangentPosition);
        float theta = dot(lightDir, normalize(TBN * -sl.direction));

        float epsilon = sl.cutOff.x - sl.cutOff.y;
        float intensity = clamp((theta - sl.cutOff.y) / epsilon, 0.0, 1.0);

        float shadow = 0.0;
        if (sl.shadowIndex >= 0) shadow = shadow2D(shadowMaps2D[sl.shadowIndex], sl.lightSpaceMatrix, position, normal, normalize(-sl.direction), sl.shadowBias, sl.shadowNormalBias, sl.shadowSlopeBias, sl.shadowSoftness);

        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * sl.ambientColor;

        vec3 norm = sampledNormal();
        vec3 diffuse = intensity * max(dot(norm, lightDir), 0.0) * sl.diffuseColor;

        float specularStrength = texture(specularMap, textCoord).r;
        vec3 viewDir = normalize(tangentViewPosition - tangentPosition);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = intensity * specularStrength * spec * sl.specularColor;

        color += (ambient + (1.0 - shadow) * (diffuse + specular + sampledReflectance(viewDir, norm))) * sampledAlbedo();
    }

    return color;
}

void main() {
    vec3 color = vec3(0.0);

    float alpha = texture(textureMap, textCoord).a;
    if (alpha < alphaClipping) discard;

    color += dirLightning();
    color += pointLightning();
    color += spotLightning();

    FragColor = vec4(color, alpha);
}
