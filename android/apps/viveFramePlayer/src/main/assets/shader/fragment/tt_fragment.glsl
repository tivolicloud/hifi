#version 300 es
precision mediump float;
uniform sampler2D oTexture;
in vec2 v2fCoord;
out vec4 oColor;
void main()
{
    oColor = texture(oTexture, v2fCoord);
}
