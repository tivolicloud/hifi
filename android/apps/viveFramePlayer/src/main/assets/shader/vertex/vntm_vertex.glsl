#version 300 es
#extension GL_OVR_multiview : enable
#extension GL_OVR_multiview2 : enable
#extension GL_OVR_multiview_multisampled_render_to_texture : enable

layout(num_views = 2) in;

uniform mat4 matrix[2];
uniform mat3 normalm;
uniform vec4 l_dir;
layout(location = 0) in vec3 v3Position;
layout(location = 1) in vec3 v3Normal;
layout(location = 2) in vec2 v2Coord;
out vec2 v2fCoord;
out float intensity;
void main()
{
    vec3 norm = normalize(normalm * v3Normal);
    intensity = max(dot(norm, l_dir.xyz), l_dir.w);
    v2fCoord = v2Coord;
    gl_Position = matrix[gl_ViewID_OVR] * vec4(v3Position.xyz, 1);
}
