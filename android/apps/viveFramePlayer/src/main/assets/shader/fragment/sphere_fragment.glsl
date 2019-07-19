precision mediump float;
uniform vec3 v_Color;
varying vec4 vAmbient;
varying vec4 vDiffuse;
varying vec4 vSpecular;
void main() {
   vec4 finalColor=vec4(v_Color,1);
   gl_FragColor=finalColor*vAmbient + finalColor*vDiffuse;
}
