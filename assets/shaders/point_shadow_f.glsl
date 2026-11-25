#version 460

uniform vec3 lightPosition;
uniform float farPlane;

in vec3 worldPosition;

void main() {
    gl_FragDepth = length(worldPosition - lightPosition) / farPlane;
}
