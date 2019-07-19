#version 300 es
#extension GL_OVR_multiview : enable
#extension GL_OVR_multiview2 : enable
#extension GL_OVR_multiview_multisampled_render_to_texture : enable

layout(num_views = 2) in;

layout (location = 0) in vec3 position;
uniform mat4 matrix[2];
out vec3 v3fCoord;

void main()
{
    vec4 WVP_Pos = matrix[gl_ViewID_OVR] * vec4(position, 1.0);
    gl_Position = WVP_Pos.xyww;
    v3fCoord = position;
}
