#version 300 es
uniform mat4 matrix;
layout(location = 0) in vec3 v3Position;
layout(location = 1) in vec2 v2Coord;
out vec2 v2fCoord;
void main() {
    gl_Position = matrix * vec4(v3Position.xyz, 1);
    v2fCoord = v2Coord;
}
