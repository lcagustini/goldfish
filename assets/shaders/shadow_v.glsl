#version 460

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

in vec3 aPos;

out vec3 worldPosition;

void main() {
    vec4 world = model * vec4(aPos, 1.0);
    worldPosition = world.xyz;
    gl_Position = lightSpaceMatrix * world;
}
