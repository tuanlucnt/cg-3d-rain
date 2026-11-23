#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aEdge;
uniform mat4 uViewProjection;
uniform vec3 uCameraPosition;
uniform float uDistanceFade;
out vec4 vColor;
out float vEdge;
out vec3 vWorldPosition;
void main() {
    vColor = aColor;
    vEdge = aEdge;
    vWorldPosition = aPosition;
    float distanceFade = mix(1.0, 0.42, smoothstep(8.0, 34.0, distance(aPosition, uCameraPosition)));
    vColor.a *= mix(1.0, distanceFade, uDistanceFade);
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}
