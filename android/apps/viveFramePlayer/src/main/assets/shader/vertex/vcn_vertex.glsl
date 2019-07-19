#version 300 es
uniform mat4 matrix;
uniform mat3 normalm;
uniform vec4 l_dir;
layout(location = 0) in vec3 v3Position;
layout(location = 1) in vec3 v3Color;
layout(location = 2) in vec3 v3Normal;
out vec4 v4Color;
void main()
{
    vec3 norm = normalize(normalm * v3Normal);
    float intensity = max(dot(norm, l_dir.xyz), l_dir.w);
    v4Color = vec4(intensity * v3Color, 1);
    gl_Position = matrix * vec4(v3Position.xyz, 1);
}
