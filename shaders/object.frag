#version 330 core

in vec3 vNormal;
in vec3 vWorldPosition;

uniform vec3 uColor;
uniform vec3 uLightDirection;
uniform float uAmbient;
uniform float uDiffuse;
uniform float uCoverage;
uniform float uFlash;
uniform vec3 uFlashPosition;
uniform vec2 uFlashAttenuation;
uniform float uCloud;
uniform vec2 uCloudHeightRange;
uniform vec3 uCameraPosition;
uniform float uWetness;
uniform float uSurface;
uniform vec3 uFogColor;
uniform float uFogDensity;

out vec4 fragColor;

void main() {
    float lambert = max(dot(normalize(vNormal), normalize(-uLightDirection)), 0.0);
    float d = distance(vWorldPosition, uFlashPosition);
    float attenuation = 1.0 / (1.0 + uFlashAttenuation.x * d + uFlashAttenuation.y * d * d);
    vec3 toLight = uFlashPosition - vWorldPosition;
    float lightLambert = max(dot(normalize(vNormal), toLight / max(d, 0.001)), 0.0);
    vec3 flashLight = vec3(0.70, 0.80, 1.0) * uFlash * attenuation * (0.45 + 0.65 * lightLambert);
    float height = smoothstep(uCloudHeightRange.x, uCloudHeightRange.y, vWorldPosition.y);
    float shade = mix(1.0, mix(0.72, 1.08, height), uCloud);
    float ground = step(0.5, uSurface) * (1.0 - step(1.5, uSurface));
    float water = step(1.5, uSurface);
    vec3 base = uColor * mix(1.0, 0.76, ground * uWetness);
    vec3 viewDirection = (uCameraPosition - vWorldPosition) / max(distance(uCameraPosition, vWorldPosition), 0.001);
    vec3 halfSun = viewDirection + normalize(-uLightDirection);
    vec3 halfFlash = viewDirection + toLight / max(d, 0.001);
    float shininess = mix(24.0, 64.0, uWetness);
    float sunSpec = pow(max(dot(normalize(vNormal), halfSun / max(length(halfSun), 0.001)), 0.0), shininess);
    float flashSpec = pow(max(dot(normalize(vNormal), halfFlash / max(length(halfFlash), 0.001)), 0.0), shininess);
    float specular = min(0.45, (ground * uWetness * 0.14 + water * 0.35) *
                          (sunSpec * 0.5 + flashSpec * uFlash * attenuation * 2.0));
    vec3 color = base * shade * (vec3(uAmbient + uDiffuse * lambert) + flashLight) +
                 vec3(0.7, 0.82, 1.0) * specular;
    float fog = exp(-uFogDensity * mix(1.0, 0.65, uCloud) *
                    max(distance(vWorldPosition, uCameraPosition) - 6.0, 0.0));
    color = mix(uFogColor * mix(1.0, 0.8, uCloud), color, fog);
    fragColor = vec4(color, uCoverage);
}
