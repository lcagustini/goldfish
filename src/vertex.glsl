precision mediump float;

uniform mat4 u_mvpMat;

attribute vec4 a_position;
attribute vec4 a_color;

varying vec4 v_color;

void main()
{
    gl_Position = u_mvpMat * a_position;
    v_color = vec4(1, 1, 1, 1);
}
