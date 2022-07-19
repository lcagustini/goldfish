precision mediump float;

varying vec3 Pos;
varying vec3 Normal;
varying vec2 TexCoord;

uniform sampler2D textureMap;

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 lightPos = vec3(0.5, 1.0, 0.5);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - Pos);

    vec3 fragColor = texture2D(textureMap, TexCoord).rgb;

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    float diffuseStrength = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * lightColor;

    gl_FragColor = vec4((ambient + diffuse) * fragColor, 1.0);
}
