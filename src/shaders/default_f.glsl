precision mediump float;

uniform vec3 viewPos;

uniform sampler2D textureMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

varying vec3 Pos;
varying vec3 Normal;
varying vec2 TexCoord;

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 lightPos = vec3(80.0, 80.0, 80.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - Pos);

    vec3 fragColor = texture2D(textureMap, TexCoord).rgb;

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    float diffuseStrength = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * lightColor;

    float specularStrength = texture2D(specularMap, TexCoord).r;
    vec3 viewDir = normalize(viewPos - Pos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    gl_FragColor = vec4((ambient + diffuse + specular) * fragColor, 1.0);
}
