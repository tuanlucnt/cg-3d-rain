#version 330 core

in vec3 vNormal;
in vec3 vWorldPosition;
in vec3 vLocalPosition;

uniform vec3 uColor;
uniform vec3 uLightDirection;
uniform vec3 uAmbientColor;
uniform vec3 uDirectionalColor;
uniform float uWindowStrength;
uniform vec3 uSkyReflection;
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
uniform int uMaterialKind;
uniform vec4 uMaterialResponse;
uniform float uMaterialWetGloss;
uniform vec3 uGroundColor;
uniform vec4 uPuddles[3]; // center XZ, scale XZ; uploaded once
uniform vec2 uPuddleRotation[3];
uniform vec3 uContacts[7];

out vec4 fragColor;

float moistureField(vec2 p) {
    // Broad continuous patches, anchored in world space (no random pixel noise).
    return clamp(0.5 + 0.28 * sin(p.x * 0.34 + sin(p.y * 0.21) * 0.8) +
                       0.18 * cos(p.y * 0.47 - p.x * 0.12), 0.0, 1.0);
}
vec3 soilColor(float field, float wet) {
    vec3 soil = mix(vec3(0.255,0.265,0.215), uGroundColor, smoothstep(0.1,0.9,field));
    vec2 p=vWorldPosition.xz;
    float pathX=-1.0+0.13*sin(p.y*0.7);
    float path=(1.0-smoothstep(0.48,0.75,abs(p.x-pathX)))*smoothstep(1.3,1.7,p.y)*(1.0-smoothstep(9.2,9.8,p.y));
    soil=mix(soil,vec3(0.30,0.275,0.235),path*0.8);
    vec2 house=abs(p-vec2(-1.0,-1.0))-vec2(2.12,1.92);
    float contact=0.11*(1.0-smoothstep(0.0,0.48,max(house.x,house.y)));
    for (int i=0;i<7;++i) contact=max(contact,0.10*(1.0-smoothstep(0.0,uContacts[i].z,distance(p,uContacts[i].xy))));
    return soil * mix(1.0,0.72,mix(wet,max(wet,uWetness*0.9),path))*(1.0-contact);
}
float waterInterior(vec2 world) {
    vec2 local = vec2(0.0);
    float best = 1e6;
    for (int i=0;i<3;++i) {
        vec2 d=world-uPuddles[i].xy, cs=uPuddleRotation[i];
        vec2 p=vec2(cs.x*d.x-cs.y*d.y,cs.y*d.x+cs.x*d.y)/uPuddles[i].zw;
        float r2=dot(p,p);
        if (r2<best) { best=r2; local=p; }
    }
    float angle=best>1e-8 ? atan(local.y,local.x) : 0.0;
    float radius=1.0+0.10*sin(3.0*angle)+0.055*cos(5.0*angle)+0.035*sin(7.0*angle+0.6);
    float r=sqrt(best)/radius;
    return 1.0-smoothstep(0.84,0.985,r); // Opaque shore blends into the same wet soil.
}

void main() {
    float ground = step(0.5, uSurface) * (1.0 - step(1.5, uSurface));
    float water = step(1.5, uSurface);
    float field=0.5, localWet=uWetness, interior=1.0;
    vec3 N=normalize(vNormal);
    if (ground+water>0.5) {
        field=moistureField(vWorldPosition.xz);
        localWet=uWetness*mix(0.42,1.0,smoothstep(0.15,0.88,field));
        if (water>0.5) interior=waterInterior(vWorldPosition.xz);
        // Shading only: Y=0 geometry and impact/placement heights stay unchanged.
        vec2 slope=vec2(0.018*cos(vWorldPosition.x*0.9+vWorldPosition.z*0.3),
                        0.014*sin(vWorldPosition.z*0.8-vWorldPosition.x*0.2));
        N=normalize(N+vec3(slope.x,0.0,slope.y)*mix(1.0,0.25,water*interior));
    }
    float lambert = max(dot(N, normalize(-uLightDirection)), 0.0);
    if (uCloud>0.5) lambert=0.12+0.88*lambert;
    float d = distance(vWorldPosition, uFlashPosition);
    float attenuation = 1.0 / (1.0 + uFlashAttenuation.x * d + uFlashAttenuation.y * d * d);
    vec3 toLight = uFlashPosition - vWorldPosition;
    float lightLambert = max(dot(N, toLight / max(d, 0.001)), 0.0);
    vec3 flashLight = vec3(0.70, 0.80, 1.0) * uFlash * attenuation * (0.45 + 0.65 * lightLambert);
    float height = smoothstep(uCloudHeightRange.x, uCloudHeightRange.y, vWorldPosition.y);
    float shade = mix(1.0, mix(0.68, 1.10, height), uCloud);
    vec3 base = uColor;
    if (ground>0.5) base=soilColor(field,localWet);
    if (water>0.5) base=mix(soilColor(field,localWet),uColor,interior);
    vec3 viewDirection = (uCameraPosition - vWorldPosition) / max(distance(uCameraPosition, vWorldPosition), 0.001);
    vec3 halfSun = viewDirection + normalize(-uLightDirection);
    vec3 halfFlash = viewDirection + toLight / max(d, 0.001);
    float shininess = uMaterialKind == 0 ? mix(24.0, 64.0, uWetness) : uMaterialResponse.w;
    if (ground+water>0.5) shininess=mix(mix(18.0,42.0,localWet),72.0,water*interior);
    float sunSpec = pow(max(dot(N, halfSun / max(length(halfSun), 0.001)), 0.0), shininess);
    sunSpec*=clamp(max(max(uDirectionalColor.r,uDirectionalColor.g),uDirectionalColor.b),0.0,1.0);
    float flashSpec = pow(max(dot(N, halfFlash / max(length(halfFlash), 0.001)), 0.0), shininess);
    float specular = min(0.45, (ground * localWet * 0.11 + water * mix(localWet*0.11,0.42,interior)) *
                          (sunSpec * 0.5 + flashSpec * uFlash * attenuation * 2.0));
    float detail = 1.0;
    if (uMaterialKind == 1) { // Broad, low-contrast plaster variation; no pixel discard.
        detail = 0.98 + 0.02 * sin(vLocalPosition.x * 5.0 + vLocalPosition.y * 9.0 + vLocalPosition.z * 4.0);
    } else if (uMaterialKind == 2 || uMaterialKind == 7 || uMaterialKind == 5) {
        vec2 grid;
        if (uMaterialKind == 2) grid = vec2(vLocalPosition.z * 3.2, abs(vLocalPosition.x) * 3.4);
        else grid = vec2((abs(vNormal.x) > 0.7 ? vLocalPosition.z : vLocalPosition.x) *
                         (uMaterialKind == 7 ? 5.0 : 1.7), vLocalPosition.y * (uMaterialKind == 7 ? 9.0 : 4.0));
        float row = floor(grid.y);
        grid.x += mod(row, 2.0) * 0.5;
        vec2 cell = fract(grid), aa = max(fwidth(grid), vec2(0.015));
        vec2 edge = smoothstep(vec2(0.025), vec2(0.025) + aa, min(cell, 1.0 - cell));
        float grout = 1.0 - min(edge.x, edge.y);
        detail = mix(0.74, 1.0, 1.0 - grout) * (0.97 + 0.03 * sin(floor(grid.x) * 3.1 + row * 2.7));
    } else if (uMaterialKind == 3) {
        detail = 0.96 + 0.04 * sin(vLocalPosition.x * 45.0 + vLocalPosition.z * 31.0 +
                                  0.6 * sin(vLocalPosition.y * 6.0));
    } else if (uMaterialKind == 6) {
        detail = mix(0.84, 1.07, smoothstep(0.8, 4.2, vLocalPosition.y));
    } else if (uMaterialKind == 9) {
        // Large facets and a dark foot, not grain noise or metal-like highlights.
        detail = mix(0.73,1.0,smoothstep(-0.55,0.4,vLocalPosition.y)) *
                 (0.97+0.035*sin(dot(normalize(vNormal),vec3(3.1,1.7,4.3))));
    } else if (uMaterialKind == 10) {
        detail=mix(0.64,1.12,smoothstep(0.005,0.22,vLocalPosition.y));
    }
    specular = min(0.45, specular + (uMaterialResponse.z + uMaterialWetGloss * uWetness) *
                            (sunSpec * 0.5 + flashSpec * uFlash * attenuation * 1.8));
    vec3 color = base * detail * shade *
                 (uAmbientColor * uMaterialResponse.x + uDirectionalColor * lambert * uMaterialResponse.y +
                  flashLight * uMaterialResponse.y) +
                 vec3(0.7, 0.82, 1.0) * specular;
    if (uMaterialKind==4) color+=vec3(0.58,0.255,0.065)*uWindowStrength;
    if (uCloud<0.5) {
        vec3 lamp=vec3(-1.0,1.65,1.1)-vWorldPosition;
        float warm=0.20*uWindowStrength/(1.0+0.85*dot(lamp,lamp));
        color+=base*vec3(1.0,0.58,0.25)*warm*(0.35+max(dot(N,normalize(lamp)),0.0));
    }
    if (water>0.5) {
        float fresnel=0.02+0.98*pow(1.0-max(dot(N,viewDirection),0.0),5.0);
        float skyAngle=clamp(reflect(-viewDirection,N).y,0.0,1.0);
        vec3 reflection=mix(uFogColor,uSkyReflection,smoothstep(0.0,0.7,skyAngle));
        color=mix(color,reflection,interior*clamp(fresnel*0.65,0.0,0.65));
        float broadGlint=pow(max(dot(N,halfFlash/max(length(halfFlash),0.001)),0.0),18.0);
        color+=vec3(0.12,0.17,0.22)*uFlash*attenuation*interior*(0.3+0.7*broadGlint);
    }
    if (uCloud>0.5) {
        float rim=pow(1.0-max(dot(N,viewDirection),0.0),3.0);
        color+=vec3(0.055,0.07,0.10)*rim*uFlash*attenuation;
    }
    float fog = exp(-uFogDensity * mix(1.0, 0.65, uCloud) *
                    max(distance(vWorldPosition, uCameraPosition) - 6.0, 0.0));
    color = mix(uFogColor * mix(1.0, 0.8, uCloud), color, fog);
    fragColor = vec4(color, uCoverage);
}
