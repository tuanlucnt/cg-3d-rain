#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aEdge;
uniform mat4 uViewProjection;
uniform vec3 uCameraPosition;
uniform float uDistanceFade;
uniform float uRainPass;
out vec4 vColor;
out float vEdge;
out vec3 vWorldPosition;
void main() {
    vColor = aColor;
    vEdge = aEdge;
    vec3 position=aPosition;
    float d=distance(aPosition,uCameraPosition);
    if (uRainPass>0.5) {
        float streakScale=mix(1.12,0.62,smoothstep(4.0,30.0,d));
        position-=aColor.xyz/max(length(aColor.xyz),0.001)*aEdge*streakScale;
        vColor=vec4(0.57,0.72,0.86,aColor.a*0.78*smoothstep(0.25,1.4,d));
        vEdge=0.0;
    }
    vWorldPosition = position;
    float distanceFade = mix(1.0, 0.28, smoothstep(4.0, 32.0, d));
    vColor.a *= mix(1.0, distanceFade, uDistanceFade);
    gl_Position = uViewProjection * vec4(position, 1.0);
}
