#version 300 es
precision mediump float;
uniform sampler2D atexture;
// inputs
in vec4 v_Color;
in vec3 v_Grid;
in vec2 vTextureCoord;

// outputs
out vec4 FragColor;

void main() {
    float depth = gl_FragCoord.z / gl_FragCoord.w;
    /*if ((mod(abs(v_Grid.x), 10.0) < 0.1) || (mod(abs(v_Grid.z), 10.0) < 0.1)) {
        FragColor = max(0.0, (90.0-depth) / 90.0) * vec4(1.0, 1.0, 1.0, 1.0) + min(1.0, depth / 90.0) * v_Color;
    } else {
        FragColor = v_Color;
    }*/
    FragColor = texture(atexture, vTextureCoord)*v_Color;
}