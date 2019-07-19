#version 300 es
precision mediump float;
uniform sampler2D atexture;
in vec2 v2fCoord;
out vec4 oColor;
void main()
{
   oColor = texture(atexture, v2fCoord);
}
