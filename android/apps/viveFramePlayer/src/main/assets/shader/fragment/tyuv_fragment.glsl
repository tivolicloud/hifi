#version 300 es
precision mediump float;
uniform sampler2D yTexture;
uniform sampler2D uvTexture;
in vec2 v2fCoord;
out vec4 oColor;
void main()
{
    float r, g, b, y, u, v;

    //We had put the Y values of each pixel to the R,G,B components by 
    //GL_LUMINANCE, that's why we're pulling it from the R component,
    //we could also use G or B
    y = texture(yTexture, v2fCoord).r;

    //We had put the U and V values of each pixel to the A and R,G,B 
    //components of the texture respectively using GL_LUMINANCE_ALPHA. 
    //Since U,V bytes are interspread in the texture, this is probably 
    //the fastest way to use them in the shader.
    //In Android YUV_420_888 format, the v comes first, then u.
    v = texture(uvTexture, v2fCoord).a - 0.5;
    u = texture(uvTexture, v2fCoord).r - 0.5;

    //The numbers are just YUV to RGB conversion constants
    r = y + 1.13983*v;
    g = y - 0.39465*u - 0.58060*v;
    b = y + 2.03211*u;

    //We finally set the RGB color of our pixel
    oColor = vec4(r, g, b, 1.0);
}
