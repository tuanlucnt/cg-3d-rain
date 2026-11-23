#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;
uniform float uTreeHeight;
uniform float uModelTime;
uniform vec2 uTreeWind;

out vec3 vNormal;
out vec3 vWorldPosition;
out vec3 vLocalPosition;

void main() {
    vec3 position = aPosition;
    vec3 normal = aNormal;
    if (uTreeHeight > 0.0) {
        float h = clamp(position.y / uTreeHeight, 0.0, 1.0);
        float phase = uModel[3].x * 0.37 + uModel[3].z * 0.29;
        vec2 amplitude = uTreeWind * (0.05 * sin(uModelTime * 0.9 + phase));
        position.xz += amplitude * h * h;
        // Inverse-transpose of the height-dependent bend's Jacobian. Root h=0
        // stays fixed; bark, branches and foliage use the identical deformation.
        normal.y -= dot(normal.xz, amplitude * (2.0 * h / uTreeHeight));
    }
    vNormal = uNormalMatrix * normal;
    vLocalPosition = aPosition;
    vec4 worldPosition = uModel * vec4(position, 1.0);
    vWorldPosition = worldPosition.xyz;
    gl_Position = uProjection * uView * worldPosition;
}
