#version 300 es
precision mediump float;
in vec3 v3fCoord;
out vec4 FragColor;
uniform samplerCube atexture;

void main()
{
    FragColor = texture(atexture, v3fCoord);
}
