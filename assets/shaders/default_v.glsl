#version 460

uniform vec3 viewPos;

layout (location = 50) uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightView;
uniform mat4 lightProjection;

layout (location = 0) in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoord;
in vec3 aTangent;

out vec4 positionLightSpace;

out vec3 position;
out vec3 normal;
out vec2 textCoord;

out vec3 tangentPosition;
out vec3 tangentViewPosition;
out mat3 TBN;

void main() {
    position = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;
    textCoord = aTexCoord;

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    vec3 B = cross(N, T);
    TBN = transpose(mat3(T, B, N));

    tangentPosition = TBN * position;
    tangentViewPosition = TBN * viewPos;

    positionLightSpace = lightProjection * lightView * vec4(vec3(model * vec4(aPos, 1.0)), 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
