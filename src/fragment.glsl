precision mediump float;

varying vec3 ourColor;
varying vec2 TexCoord;

//uniform sampler2D textureMap;

void main()
{
    gl_FragColor = vec4(ourColor, 1.0);//texture(textureMap, TexCoord);
}
