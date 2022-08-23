precision mediump float;

uniform sampler2D textureMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

varying vec3 position;
varying vec3 normal;
varying vec2 textCoord;

varying vec3 tangentLightPosition;
varying vec3 tangentViewPosition;
varying vec3 tangentPosition;

void main() {
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    //vec3 norm = normalize(normal);
    vec3 norm = texture2D(normalMap, textCoord).rgb;
    norm = normalize(2.0 * norm - 1.0);

    vec3 lightDir = normalize(tangentLightPosition - tangentPosition);

    vec3 fragColor = texture2D(textureMap, textCoord).rgb;

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    float diffuseStrength = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * lightColor;

    float specularStrength = texture2D(specularMap, textCoord).r;
    vec3 viewDir = normalize(tangentViewPosition - tangentPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = specularStrength * spec * lightColor;

    gl_FragColor = vec4((ambient + diffuse + specular) * fragColor, 1.0);
}
