precision mediump float;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

varying vec3 Pos;
varying vec3 Normal;
varying vec2 TexCoord;

void main()
{
    Pos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;
    TexCoord = aTexCoord;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
