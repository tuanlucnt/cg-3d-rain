# 05 — GLSL và pipeline CPU/GPU

[Tổng quan](01_PROJECT_OVERVIEW.md) · [Hàm shader trong walkthrough](02_SOURCE_CODE_WALKTHROUGH.md)

Sáu file GLSL 330 Core tạo ba linked programs:object, effects, sky. Có 9 hàm GLSL:6 main và 3 helper trong object.frag. Shader::compile/link kiểm status/info log; Shader::uniform yêu cầu active location≥0. Paths từ CMake RAIN_SHADER_DIR, không load shader từ current working directory.

## Mục lục

- [Dữ liệu đi qua pipeline](#s01)
- [Các khái niệm cần giải thích khi báo cáo](#s02)
- [Phân công CPU và GPU](#s03)
- [shaders/object.vert — Vertex](#s04)
- [shaders/object.frag — Fragment](#s05)
- [shaders/effects.vert — Vertex](#s06)
- [shaders/effects.frag — Fragment](#s07)
- [shaders/sky.vert — Vertex](#s08)
- [shaders/sky.frag — Fragment](#s09)
- [Phân nhánh object.frag theo MaterialKind](#s10)
- [Alpha và depth không do shader tự bật](#s11)
- [Liên kết công thức](#s12)

<a id="s01"></a>

## Dữ liệu đi qua pipeline

```mermaid
flowchart LR
    CPU[CPU tạo mesh hoặc cập nhật particles] --> Buffer[VBO qua glBufferData hoặc glBufferSubData]
    Buffer --> VAO[VAO mô tả location và stride]
    VAO --> VS[Vertex shader]
    VS --> Clip[Clipping và chia w]
    Clip --> Raster[Rasterization / nội suy varyings]
    Raster --> FS[Fragment shader]
    FS --> Test[Depth test / blending theo GL state]
    Test --> FB[Default back framebuffer]
    FB --> Swap[glutSwapBuffers]
```

Không có geometry/tessellation/compute stage, texture sampling, off screen FBO hoặc EBO. GPU driver có thể sắp xếp early-depth tối ưu nội bộ, sơ đồ là trình tự logic để học.

<a id="s02"></a>

## Các khái niệm cần giải thích khi báo cáo

| Ký hiệu/API | Ý nghĩa trong project |
|---|---|
| layout(location=0/1/2) | Nối attribute GLSL tới glVertexAttribPointer trong Mesh/DynamicBatch; VAO không chứa shader program |
| uniform | Giá trị cho draw call/program, CPU upload bằng glUniform sau use; không tự thay theo từng vertex |
| in/out | Vertex in từ VBO; vertex out→fragment in được rasterizer nội suy; fragment out là màu framebuffer |
| gl_Position | Clip coordinates vec4 của mỗi vertex; PVM trong object, VP trong effects, clip trực tiếp trong sky |
| normalize/dot | Normalization, cosine và specular; zero-vector cần guard, một số chỗ có max(.001) |
| mix/clamp/smoothstep | Nội suy tuyến tính/chặn khoảng/nội suy cubic có mép mềm |
| exp/pow | Fog và pulse decay; specular, Fresnel, rim theo lũy thừa |
| fract/floor/mod/fwidth | Procedural tile/brick/stone; row stagger; fragment derivative để mềm grid |
| reflect | Direction của fake sky reflection water; không đọc ảnh phản chiếu |
| cross | Không xuất hiện trong GLSL hiện tại; CPU dùng cho normal, basis, billboard |
| gl_FragCoord/discard | Không xuất hiện trong 6 shader hiện tại; README có mô tả lịch sử đã bỏ stochastic discard |
| glUniformMatrix* GL_FALSE | Upload ma trận GLM theo column-major, không transpose; shader nhân vector cột |

<a id="s03"></a>

## Phân công CPU và GPU

CPU:geometry generation, normal face/winding, model/normal/view/projection matrices, particle physics/impact/pools, cloud advection/lightning subdivision, pulse intensity, time/lighting colors, sort cloud fade và build billboard/staging. GPU:tree bend+normal Jacobian, vertex transform, rain tail/near/distance fade, rasterization, procedural material/Lambert/Blinn/Fresnel/fog, depth/blending. Không mô phỏng 1000 particle trên GPU.


<a id="s04"></a>

## shaders/object.vert — Vertex

**Source:** [shaders/object.vert](../shaders/object.vert#L1). **C++ điều khiển:** Renderer.cpp:135–146, Mesh.cpp:21–26.

**Inputs/layout:** Mesh attr0 vec3 position, attr1 vec3 normal; stride sizeof(Vertex), offset offsetof.

**Trình tự và kết quả:** Sway nếu treeHeight>0; giữ aPosition gốc làm vLocalPosition cho pattern không trôi theo bend. n.y được sửa bằng inverse-transpose Jacobian rồi uNormalMatrix, chưa normalize tại vertex để fragment normalize sau interpolation. world=uModel·bentPosition, gl_Position=uProjection·uView·world.

### Attributes và varyings

| Qualifier | Kiểu | Tên | Dòng |
|---|---|---|---:|
| layout(location = 0) in | vec3 | `aPosition` |3|
| layout(location = 1) in | vec3 | `aNormal` |4|
|  out | vec3 | `vNormal` |14|
|  out | vec3 | `vWorldPosition` |15|
|  out | vec3 | `vLocalPosition` |16|

### Toàn bộ uniforms

| Kiểu/tên | Dòng | Ý nghĩa | C++ truyền dữ liệu |
|---|---:|---|---|
| `mat4 uModel` |6| mat4 local→world, có translation phase cây | SceneObject.model; draw glUniformMatrix4fv |
| `mat4 uView` |7| mat4 world→eye | Camera::view; draw glUniformMatrix4fv |
| `mat4 uProjection` |8| mat4 perspective eye→clip | Camera::projection(width/height); draw glUniformMatrix4fv |
| `mat3 uNormalMatrix` |9| mat3 inverse transpose của model | SceneObject.normal; draw glUniformMatrix3fv |
| `float uTreeHeight` |10| height world-unit; 0 tắt bend | SceneObject.treeHeight; mỗi object glUniform1f, cloud 0 |
| `float uModelTime` |11| giây sway; pause đóng băng | Scene::animationTime; draw glUniform1f |
| `vec2 uTreeWind` |12| wind XZ cho sway | WeatherSystem.windVelocity; draw glUniform2f |


<a id="s05"></a>

## shaders/object.frag — Fragment

**Source:** [shaders/object.frag](../shaders/object.frag#L1). **C++ điều khiển:** Renderer.cpp:111–187 và constructor:65–80.

**Inputs/layout:** vNormal/worldPosition/localPosition được nội suy từ object.vert.

**Trình tự và kết quả:** Ba helper moistureField/soilColor/waterInterior được main gọi. Main lần lượt phân loại surface; moisture/normal perturb; Lambert và point flash; base/shade; half-vector specular; procedural detail/material response; emission và warm lamp; Fresnel/glint hoặc cloud rim; fog; fragColor(color, uCoverage). Không ghi gl_FragDepth; depth do pipeline rasterization.

### Attributes và varyings

| Qualifier | Kiểu | Tên | Dòng |
|---|---|---|---:|
|  in | vec3 | `vNormal` |3|
|  in | vec3 | `vWorldPosition` |4|
|  in | vec3 | `vLocalPosition` |5|
|  out | vec4 | `fragColor` |32|

### Toàn bộ uniforms

| Kiểu/tên | Dòng | Ý nghĩa | C++ truyền dữ liệu |
|---|---:|---|---|
| `vec3 uColor` |7| RGB base của object/cloud | SceneObject.color hoặc cloud(.40, .44, .49); glUniform3fv/3f |
| `vec3 uLightDirection` |8| hướng tia sáng đi tới surface; shader dùng dấu âm | Lighting::lightDirection; draw glUniform3fv |
| `vec3 uAmbientColor` |9| RGB nền ngày/đêm/storm | Lighting::ambientColor; draw glUniform3fv |
| `vec3 uDirectionalColor` |10| RGB directional nền | Lighting::directionalColor; draw glUniform3fv |
| `float uWindowStrength` |11| emission cửa sổ và đèn ấm; 0 ngày, 1 đêm | Lighting::windowStrength; draw glUniform1f |
| `vec3 uSkyReflection` |12| màu đỉnh trời hiện tại gồm flash | Lighting::skyTop; draw glUniform3fv |
| `float uCoverage` |13| alpha cả object; cloud fade theo seam | draw 1 cho Scene; CloudSystem::coverage cho cloud; glUniform1f |
| `float uFlash` |14| cường độ xung lightning[0, 1] | Lighting::flash; draw glUniform1f |
| `vec3 uFlashPosition` |15| vị trí đầu bolt đại diện nguồn điểm | Lighting::flashPosition; draw glUniform3fv |
| `vec2 uFlashAttenuation` |16| hệ số linear.025/quadratic.018 | Constructor Renderer:66; glUniform2f một lần |
| `float uCloud` |17| 0 vật thường, 1 cloud; đổi shade/fog/rim | Renderer draw glUniform1f |
| `vec2 uCloudHeightRange` |18| min/max Y world của skin hiện tại | Cached local min/max +clusterY; draw glUniform2f |
| `vec3 uCameraPosition` |19| camera world cho distance/view vector | Camera::position; glUniform3fv ở object/effects mỗi frame |
| `float uWetness` |20| độ ướt toàn cảnh[0, 1] | Lighting::wetness; draw glUniform1f |
| `float uSurface` |21| 0 object, 1 ground, 2 water | SceneObject.ground/puddle; draw glUniform1f |
| `vec3 uFogColor` |22| horizon color hiện tại gồm flash | Lighting::fogColor; glUniform3fv cho object/effects |
| `float uFogDensity` |23| density.018 mặc định, 0 khi fog off | weather.fogEnabled?density:0; glUniform1f |
| `int uMaterialKind` |24| enum int 0..10 | SceneObject.material.kind; draw glUniform1i |
| `vec4 uMaterialResponse` |25| ambient, diffuse, specStrength, shininess | SceneObject.material.response; draw glUniform4fv |
| `float uMaterialWetGloss` |26| thêm specular khi bề mặt ướt | SceneObject.material.wetGloss; draw glUniform1f |
| `vec3 uGroundColor` |27| RGB(.23, .30, .26) nền đất thống nhất | Renderer:68 constructor glUniform3fv |
| `vec4 uPuddles[3]` |28| 3 vec4(centerX, centerZ, scale X, scaleZ) | Renderer:69–74 constructor glUniform4f từng index |
| `vec2 uPuddleRotation[3]` |29| 3 vec2(cos Yaw, sin Yaw), dùng inverse transform | Renderer:69–74 constructor glUniform2f từng index |
| `vec3 uContacts[7]` |30| 7 vec3(centerX, centerZ, radius) cây/đá | Renderer:75–80 contactPoint lambda glUniform3f một lần |


<a id="s06"></a>

## shaders/effects.vert — Vertex

**Source:** [shaders/effects.vert](../shaders/effects.vert#L1). **C++ điều khiển:** Renderer.cpp:190–224 và DynamicBatch.cpp:19–29.

**Inputs/layout:** DynamicBatch attr0 position vec3, attr1 color vec4, attr2 edge float; stride sizeof(EffectVertex).

**Trình tự và kết quả:** Nếu rain pass, hai đỉnh cùng head nhưng tail edge=length; color.xyz chứa velocity. Dựng tail GPU theo hướng vận tốc và camera distance, đổi color sang RGB mưa, near fade và distance fade. Pass khác truyền RGBA/edge; worldPosition sau dịch tail để fragment tính fog; gl_Position=VP·position.

### Attributes và varyings

| Qualifier | Kiểu | Tên | Dòng |
|---|---|---|---:|
| layout(location = 0) in | vec3 | `aPosition` |2|
| layout(location = 1) in | vec4 | `aColor` |3|
| layout(location = 2) in | float | `aEdge` |4|
|  out | vec4 | `vColor` |9|
|  out | float | `vEdge` |10|
|  out | vec3 | `vWorldPosition` |11|

### Toàn bộ uniforms

| Kiểu/tên | Dòng | Ý nghĩa | C++ truyền dữ liệu |
|---|---:|---|---|
| `mat4 uViewProjection` |5| P·V đưa effect world→clip | Renderer draw glUniformMatrix4fv trước mỗi effect pass |
| `vec3 uCameraPosition` |6| camera world cho distance/view vector | Camera::position; glUniform3fv ở object/effects mỗi frame |
| `float uDistanceFade` |7| 1 rain distance alpha, 0 các effect còn lại | Renderer draw glUniform1f |
| `float uRainPass` |8| 1 giải mã velocity/tail length; 0 RGBA thông thường | Renderer draw đặt 0 đầu frame, 1 rain, sau rain trả 0 |


<a id="s07"></a>

## shaders/effects.frag — Fragment

**Source:** [shaders/effects.frag](../shaders/effects.frag#L1). **C++ điều khiển:** Renderer.cpp:225–342; common uniforms:102–110.

**Inputs/layout:** Nhận vColor/vEdge/vWorldPosition từ effects.vert.

**Trình tự và kết quả:** Softness 1−smoothstep(.35, 1, abs(edge)), nearFade smoothstep(.5, 1.5, d); alpha=color.a×intensity×mix(1, softness×nearFade, softEdge). RGB tint weather và exponential fog; alpha không nhân fog. Bolt edge±1 interpolate qua quad tạo viền mềm; line dùng softEdge 0.

### Attributes và varyings

| Qualifier | Kiểu | Tên | Dòng |
|---|---|---|---:|
|  in | vec4 | `vColor` |2|
|  in | float | `vEdge` |3|
|  in | vec3 | `vWorldPosition` |4|
|  out | vec4 | `fragColor` |12|

### Toàn bộ uniforms

| Kiểu/tên | Dòng | Ý nghĩa | C++ truyền dữ liệu |
|---|---:|---|---|
| `float uIntensity` |5| nhân alpha; 1 rain/splash/ripple, flashIntensity bolt | Renderer draw glUniform1f theo pass |
| `float uSoftEdge` |6| 1 bolt softness/nearFade, 0 line particles | Renderer draw glUniform1f theo pass |
| `vec3 uCameraPosition` |7| camera world cho distance/view vector | Camera::position; glUniform3fv ở object/effects mỗi frame |
| `vec3 uFogColor` |8| horizon color hiện tại gồm flash | Lighting::fogColor; glUniform3fv cho object/effects |
| `float uFogDensity` |9| density.018 mặc định, 0 khi fog off | weather.fogEnabled?density:0; glUniform1f |
| `float uFogScale` |10| rain.35, bolt.2, splash/ripple 1 | Renderer draw glUniform1f theo pass |
| `vec3 uWeatherLight` |11| RGB tint particle theo giờ/flash; bolt trắng | Lighting::particleColor đầu frame; bolt glUniform3f(1, 1, 1) |


<a id="s08"></a>

## shaders/sky.vert — Vertex

**Source:** [shaders/sky.vert](../shaders/sky.vert#L1). **C++ điều khiển:** Renderer.cpp:91–101, Geometry.cpp:23.

**Inputs/layout:** Mesh sky attr0 vec3 clip position; attr1 normal có trong VAO nhưng shader không đọc.

**Trình tự và kết quả:** Fullscreen triangle positions(−1, −1), (3, −1), (−1, 3). vUV=(xy+1)/2; gl_Position=(xy, 0, 1). Rasterizer clip tam giác vượt viewport, interpolate UV để vUV visible ở[0, 1].

### Attributes và varyings

| Qualifier | Kiểu | Tên | Dòng |
|---|---|---|---:|
| layout(location = 0) in | vec3 | `aPosition` |2|
|  out | vec2 | `vUV` |3|

### Toàn bộ uniforms

| Kiểu/tên | Dòng | Ý nghĩa | C++ truyền dữ liệu |
|---|---:|---|---|
| Không có |—|Nhận vertex clip trực tiếp|—|


<a id="s09"></a>

## shaders/sky.frag — Fragment

**Source:** [shaders/sky.frag](../shaders/sky.frag#L1). **C++ điều khiển:** Renderer.cpp:91–101.

**Inputs/layout:** vUV interpolated từ sky.vert.

**Trình tự và kết quả:** Mix horizon→top theo smoothstep(0, 1, vUV.y); alpha 1. Không fog thêm vì gradient chính là background/fog reference. Không input thời gian trực tiếp: CPU Lighting đổi uniform theo giờ.

### Attributes và varyings

| Qualifier | Kiểu | Tên | Dòng |
|---|---|---|---:|
|  in | vec2 | `vUV` |2|
|  out | vec4 | `fragColor` |5|

### Toàn bộ uniforms

| Kiểu/tên | Dòng | Ý nghĩa | C++ truyền dữ liệu |
|---|---:|---|---|
| `vec3 uTop` |3| màu đỉnh gradient | Lighting::skyTop; glUniform3fv sky |
| `vec3 uHorizon` |4| màu chân trời gradient | Lighting::fogColor; glUniform3fv sky |


<a id="s10"></a>

## Phân nhánh object.frag theo MaterialKind

| ID | Kiểu | Detail/emission thực tế |
|---:|---|---|
|0|Plain|Không procedural detail riêng; shininess mặc định theo wetness nếu không surface override|
|1|Plaster|.98+.02 sin(local.x×5+local.y×9+local.z×4)|
|2|RoofTile|Grid theo local.z và abs(local.x); tile/grout mép fwidth|
|3|Wood|Vân sin theo local x 45+local z 31+.6 sin(local y 6)|
|4|Glass|Emission(.58, .255, .065)×windowStrength; opaque|
|5|Stone|Grid lớn dựa mặt x hoặc z, grout|
|6|Foliage|Detail theo local height, không alpha-leaf texture|
|7|Brick|Grid dày5/9 và stagger row|
|8|Metal|Không detail riêng; response mạnh specular|
|9|Rock|Foot darkening và sin theo normal, không metal gloss|
|10|Grass|Height gradient.64→1.12|

Các RGB material/state khởi tạo tại [Scene.cpp:19](../src/Scene.cpp#L19). Enum số phải khớp C++ MaterialKind ở [Material.hpp:4](../include/Material.hpp#L4); HousePart là enum khác để chia 9 mesh.

<a id="s11"></a>

## Alpha và depth không do shader tự bật

object.frag luôn trả alpha=uCoverage; Scene water/Glass không alpha transparency vì coverage 1 và blend off. Mây opaque ghi depth; fade blend standard và không ghi depth. Rain/splash/ripple/bolt blend standard, depth test vẫn bật; bolt tắt cull faces tạm. Fragment shader tạo RGBA; GL state của [Renderer::draw](../src/Renderer.cpp#L84) quyết định composite.

Fog dùng RGB mix riêng, không phải thay alpha. Lightning core/halo là hai quad mỗi segment, chung 1 draw, không screen-space bloom. Water Fresnel dùng sky color uniforms, không texture/capture cảnh. Thời gian trong shader đến từ CPU animationTime hoặc màu Lighting; không có shader clock đọc hệ điều hành.

<a id="s12"></a>

## Liên kết công thức

[Normal Matrix và sway](04_MATHEMATICAL_FORMULAS.md#m08), [rain streak](04_MATHEMATICAL_FORMULAS.md#m18), [lighting](04_MATHEMATICAL_FORMULAS.md#m25), [Blinn–Phong](04_MATHEMATICAL_FORMULAS.md#m26), [Fresnel](04_MATHEMATICAL_FORMULAS.md#m27), [fog](04_MATHEMATICAL_FORMULAS.md#m28).
