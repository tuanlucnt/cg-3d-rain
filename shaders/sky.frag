#version 330 core
in vec2 vUV;
uniform vec3 uTop;
uniform vec3 uHorizon;
out vec4 fragColor;
void main() {
    fragColor = vec4(mix(uHorizon, uTop, smoothstep(0.0, 1.0, vUV.y)), 1.0);
}
