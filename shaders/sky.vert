#version 330 core
layout(location = 0) in vec3 aPosition;
out vec2 vUV;
void main() {
    vUV = (aPosition.xy + 1.0) * 0.5;
    gl_Position = vec4(aPosition.xy, 0.0, 1.0);
}
