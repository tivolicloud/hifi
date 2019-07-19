#version 300 es
uniform mat4 matrix;
layout(location = 0) in vec3 v3Position;
out vec4 v4Color;
void main()
{
    gl_Position = matrix * vec4(v3Position.xyz, 1);
    v4Color = vec4(0.7f, 0.7f, 0.7f, 1);
}
