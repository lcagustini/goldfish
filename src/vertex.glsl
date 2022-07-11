precision mediump float;

uniform mat4 mvpMat;

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

varying vec3 ourColor;
varying vec2 TexCoord;

void main()
{
    gl_Position = mvpMat * vec4(aPos, 1.0);
    ourColor = abs(aNormal);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
