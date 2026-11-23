#version 330 core
in vec4 vColor;
in float vEdge;
in vec3 vWorldPosition;
uniform float uIntensity;
uniform float uSoftEdge;
uniform vec3 uCameraPosition;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform float uFogScale;
out vec4 fragColor;
void main() {
    float softness = 1.0 - smoothstep(0.35, 1.0, abs(vEdge));
    float nearFade = smoothstep(0.5, 1.5, distance(vWorldPosition, uCameraPosition));
    float alpha = vColor.a * uIntensity * mix(1.0, softness * nearFade, uSoftEdge);
    float fog = exp(-uFogDensity * uFogScale * max(distance(vWorldPosition, uCameraPosition) - 6.0, 0.0));
    fragColor = vec4(mix(uFogColor, vColor.rgb, fog), alpha);
}
