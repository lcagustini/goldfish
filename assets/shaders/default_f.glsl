precision mediump float;

#define MAX_LIGHTS 8

#define DIR_LIGHTS_LENGTH 1
#define POINT_LIGHTS_LENGTH 0
#define SPOT_LIGHTS_LENGTH 0

struct dirLight {
    vec3 direction;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct pointLight {
    vec3 position;

    vec3 attenuation;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct spotLight {
    vec3 position;
    vec3 direction;

    vec2 cutOff;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

uniform dirLight dirLights[MAX_LIGHTS];
uniform pointLight pointLights[MAX_LIGHTS];
uniform spotLight spotLights[MAX_LIGHTS];

uniform float shininess;

uniform sampler2D textureMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

varying vec3 position;
varying vec3 normal;
varying vec2 textCoord;

varying vec3 tangentPosition;
varying vec3 tangentViewPosition;
varying mat3 TBN;

vec3 dirLightning() {
    vec3 color = vec3(0.0);

    for (int i = 0; i < DIR_LIGHTS_LENGTH; i++) {
        dirLight dl = dirLights[i];

        vec3 norm = texture2D(normalMap, textCoord).rgb;
        norm = normalize(2.0 * norm - 1.0);

        vec3 lightDir = TBN * normalize(dl.direction);

        vec3 fragColor = texture2D(textureMap, textCoord).rgb;

        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * dl.ambientColor;

        float diffuseStrength = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diffuseStrength * dl.diffuseColor;

        float specularStrength = texture2D(specularMap, textCoord).r;
        vec3 viewDir = normalize(tangentViewPosition - tangentPosition);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * dl.specularColor;

        color += (ambient + diffuse + specular) * fragColor;
    }

    return color;
}

vec3 pointLightning() {
    vec3 color = vec3(0.0);

    for (int i = 0; i < POINT_LIGHTS_LENGTH; i++) {
        pointLight pl = pointLights[i];

        vec3 norm = texture2D(normalMap, textCoord).rgb;
        norm = normalize(2.0 * norm - 1.0);

        vec3 lightDir = (TBN * pl.position) - tangentPosition;
        float lightDistance = length(lightDir);
        lightDir = normalize(lightDir);

        vec3 fragColor = texture2D(textureMap, textCoord).rgb;

        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * pl.ambientColor;

        float diffuseStrength = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diffuseStrength * pl.diffuseColor;

        float specularStrength = texture2D(specularMap, textCoord).r;
        vec3 viewDir = normalize(tangentViewPosition - tangentPosition);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * pl.specularColor;

        float attenuation = 1.0 / (pl.attenuation.x + pl.attenuation.y * lightDistance + pl.attenuation.z * (lightDistance * lightDistance));

        color += attenuation * (ambient + diffuse + specular) * fragColor;
    }

    return color;
}

vec3 spotLightning() {
    vec3 color = vec3(0.0);

    for (int i = 0; i < SPOT_LIGHTS_LENGTH; i++) {
        spotLight sl = spotLights[i];

        vec3 lightDir = normalize((TBN * sl.position) - tangentPosition);
        float theta = dot(lightDir, normalize(TBN * -sl.direction));

        float epsilon = sl.cutOff.x - sl.cutOff.y;
        float intensity = clamp((theta - sl.cutOff.y) / epsilon, 0.0, 1.0);

        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * sl.ambientColor;

        vec3 fragColor = texture2D(textureMap, textCoord).rgb;

        vec3 norm = texture2D(normalMap, textCoord).rgb;
        norm = normalize(2.0 * norm - 1.0);

        float diffuseStrength = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = intensity * diffuseStrength * sl.diffuseColor;

        float specularStrength = texture2D(specularMap, textCoord).r;
        vec3 viewDir = normalize(tangentViewPosition - tangentPosition);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = intensity * specularStrength * spec * sl.specularColor;

        color += (ambient + diffuse + specular) * fragColor;
    }

    return color;
}

void main() {
    vec3 color = vec3(0.0);

    color += dirLightning();
    color += pointLightning();
    color += spotLightning();

    gl_FragColor = vec4(color, 1.0);
}
