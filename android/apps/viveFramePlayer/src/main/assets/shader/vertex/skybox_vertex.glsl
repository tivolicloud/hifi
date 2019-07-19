#version 300 es
layout (location = 0) in vec3 position;
uniform mat4 matrix;
out vec3 v3fCoord;

void main()
{
    vec4 WVP_Pos = matrix * vec4(position, 1.0);
    gl_Position = WVP_Pos.xyww;
    v3fCoord = position;
}
