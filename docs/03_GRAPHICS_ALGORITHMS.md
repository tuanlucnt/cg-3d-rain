# 03 — Thuật toán đồ họa thực sự được triển khai

[Tổng quan](01_PROJECT_OVERVIEW.md) · [Công thức/ví dụ](04_MATHEMATICAL_FORMULAS.md)

Đây là 33 nhóm thuật toán/kỹ thuật được phân loại cho review; không có nghĩa project chứa 33 thuật toán độc lập trong thư viện. Các primitive chỉ dùng test được đánh dấu rõ. Trình bày dựa trên thân hàm hiện tại, khác một số mô tả lịch sử ở README.

## Mục lục

- [A01 — Triangle mesh, normal và winding](#a01)

- [A02 — Ground/box/roof primitive](#a02)

- [A03 — Cone/cylinder/tube](#a03)

- [A04 — Sphere và ellipsoid parameters](#a04)

- [A05 — Nhà procedural theo material batches](#a05)

- [A06 — Cây procedural](#a06)

- [A07 — Đá seeded low-poly](#a07)

- [A08 — Placement cỏ và mesh hai mặt](#a08)

- [A09 — Polygon vũng nước dùng chung](#a09)

- [A10 — Skin mây từ tổng Gaussian](#a10)

- [A11 — Transforms và free camera](#a11)

- [A12 — Sway cây bằng vertex shader](#a12)

- [A13 — Depth/culling/alpha/state](#a13)

- [A14 — Sort mây đang fade](#a14)

- [A15 — Lambert, ambient RGB, flash và đèn ấm](#a15)

- [A16 — Vật liệu procedural và anti-alias grid](#a16)

- [A17 — Blinn–Phong wet highlights](#a17)

- [A18 — Schlick và fake sky reflection](#a18)

- [A19 — Ground moisture/path/contact](#a19)

- [A20 — Sky gradient và exponential fog](#a20)

- [A21 — Fixed timestep accumulator](#a21)

- [A22 — Mưa semi-implicit Euler và recycling](#a22)

- [A23 — Impact plane và water containment](#a23)

- [A24 — Splash pool và lifetime](#a24)

- [A25 — Ripple pool/containment/expansion](#a25)

- [A26 — Cloud advection/wrap/fade](#a26)

- [A27 — Midpoint displacement ràng buộc](#a27)

- [A28 — Sét chọn nguồn/branch/taper](#a28)

- [A29 — Pulse/billboard/core-halo/cache](#a29)

- [A30 — Day/night clock và ánh sáng](#a30)

- [A31 — Wetness tăng/khô tuyến tính](#a31)

- [A32 — Reuse/pool/batch/state management](#a32)

- [A33 — Đo hiệu năng và nonblocking query](#a33)


<a id="a01"></a>

## A01 — Triangle mesh, normal và winding

**Ý tưởng/Input→Output:** Đỉnh a/b/c và optional outward → triangle-list Vertex; quad→2 triangle.

**Quy trình thực tế:** Tính cross cạnh, normalize; Model/Environment helper đổi b/c khi dot normal với outward âm, Geometry helper yêu cầu caller CCW. Flat normals lặp vertex theo mặt.

**Công thức/chi phí:** N=normalize((b-a)×(c-a)); O(1)/triangle.

**Implementation:** [triangle (src/Geometry.cpp:8)](../src/Geometry.cpp#L8), [quad (src/Geometry.cpp:15)](../src/Geometry.cpp#L15), [triangle (src/EnvironmentGeometry.cpp:11)](../src/EnvironmentGeometry.cpp#L11), [tri (src/ModelGeometry.cpp:10)](../src/ModelGeometry.cpp#L10), [quad (src/ModelGeometry.cpp:15)](../src/ModelGeometry.cpp#L15).

**Kết quả và giới hạn:** Không có wireframe renderer hoặc index/EBO. Helper không chặn tam giác suy biến; CPU tests kiểm các mesh mặc định.


<a id="a02"></a>

## A02 — Ground/box/roof primitive

**Ý tưởng/Input→Output:** Kích thước quy ước cố định → ground 2, cube12, roof 8 triangles.

**Quy trình thực tế:** Ghép quad/triangle với winding theo từng mặt. Ground thuộc XZ y 0; cube unit tâm 0; roof là prism kín.

**Công thức/chi phí:** O(1), T/R/S được áp sau hoặc bake CPU.

**Implementation:** [ground (src/Geometry.cpp:36)](../src/Geometry.cpp#L36), [box (src/Geometry.cpp:43)](../src/Geometry.cpp#L43), [roof (src/Geometry.cpp:56)](../src/Geometry.cpp#L56).

**Kết quả và giới hạn:** Ground đang render; box là unit cached cho nhà. geometry::roof hiện chỉ dùng test, nhà dùng shell riêng.


<a id="a03"></a>

## A03 — Cone/cylinder/tube

**Ý tưởng/Input→Output:** Cone segments 3..64 hoặc a/b, ra/rb, sides → closed low-poly mesh.

**Quy trình thực tế:** Cone tạo sides và base; tube tạo basis vuông góc axis, hai vòng bán kính, mặt bên và hai caps.

**Công thức/chi phí:** Cone2n, tube 4n triangles; O(n).

**Implementation:** [cone (src/Geometry.cpp:69)](../src/Geometry.cpp#L69), [tube (src/ModelGeometry.cpp:31)](../src/ModelGeometry.cpp#L31).

**Kết quả và giới hạn:** Tube dùng trunk/branch/handle, không có cylinder API độc lập. Không smooth normal tròn; tube helper tin input nội bộ.


<a id="a04"></a>

## A04 — Sphere và ellipsoid parameters

**Ý tưởng/Input→Output:** slices/stacks → sphere triangle list; CloudLobe offset/scale→tham số ellipsoid.

**Quy trình thực tế:** Sphere lấy longitude/latitude, bỏ một triangle tại mỗi cực. Cloud không vẽ sphere này, dùng lobe làm density field.

**Công thức/chi phí:** p=(sinφcosθ, cosφ, sinφsinθ); 2s(t−1)triangles.

**Implementation:** [sphere (src/Geometry.cpp:98)](../src/Geometry.cpp#L98), [sphere::point@103 (src/Geometry.cpp:103)](../src/Geometry.cpp#L103), [CloudSystem::reset (src/CloudSystem.cpp:12)](../src/CloudSystem.cpp#L12).

**Kết quả và giới hạn:** Sphere mặc định 100 triangles là primitive/test, không tính vào cloud render hiện tại.


<a id="a05"></a>

## A05 — Nhà procedural theo material batches

**Ý tưởng/Input→Output:** Kích thước/layout hard-code → House 9 vector/734 triangles.

**Quy trình thực tế:** Bake unit boxes, beam/tube, roof shell có dày, gable khớp mặt dưới; cửa, khung, kính, bậc, ống khói/cavity. Nhóm theo HousePart để mỗi part 1 draw.

**Công thức/chi phí:** M=parent·T·S; gable edge nội suy tuyến tính.

**Implementation:** [box (src/ModelGeometry.cpp:18)](../src/ModelGeometry.cpp#L18), [beam (src/ModelGeometry.cpp:24)](../src/ModelGeometry.cpp#L24), [roofShell (src/ModelGeometry.cpp:46)](../src/ModelGeometry.cpp#L46), [gable (src/ModelGeometry.cpp:62)](../src/ModelGeometry.cpp#L62), [house (src/ModelGeometry.cpp:79)](../src/ModelGeometry.cpp#L79), [house::window@109 (src/ModelGeometry.cpp:109)](../src/ModelGeometry.cpp#L109).

**Kết quả và giới hạn:** Không CSG/khoét cửa hoặc interior; nhiều khối giao nhau. Glass opaque, không refraction.


<a id="a06"></a>

## A06 — Cây procedural

**Ý tưởng/Input→Output:** TreeParameters height/radius/canopy/branchCount/seed → bark+foliage.

**Quy trình thực tế:** Trunk taper 8 sector; nhánh 6 sector góc vàng+jitter. Tán 12 ring×10 sector có width/height profile và nhiễu sin theo seed; nối skin kín.

**Công thức/chi phí:** branch angle=phase+i·2.3999632+jitter; bark 32+24b, foliage 240 triangles.

**Implementation:** [treeLayout (src/ModelGeometry.cpp:130)](../src/ModelGeometry.cpp#L130), [tree (src/ModelGeometry.cpp:140)](../src/ModelGeometry.cpp#L140), [tree::random@149 (src/ModelGeometry.cpp:149)](../src/ModelGeometry.cpp#L149).

**Kết quả và giới hạn:** 4 cây hiện có 1904 triangles; không L-system hoặc mô phỏng sinh trưởng. Budget params hữu hạn, STL RNG không hứa identical mọi platform.


<a id="a07"></a>

## A07 — Đá seeded low-poly

**Ý tưởng/Input→Output:** seed và RockInstance transform →48 triangles/rock.

**Quy trình thực tế:** 3 ring×8 sector, jitter góc/bán kính/y, lean; nối rings rồi cap, sửa winding theo centroid.

**Công thức/chi phí:** r=base Radius·width[k]·jitter; O(3×8).

**Implementation:** [rocks (src/EnvironmentGeometry.cpp:79)](../src/EnvironmentGeometry.cpp#L79), [rock (src/EnvironmentGeometry.cpp:88)](../src/EnvironmentGeometry.cpp#L88), [rock::random@91 (src/EnvironmentGeometry.cpp:91)](../src/EnvironmentGeometry.cpp#L91), [rock::add@108 (src/EnvironmentGeometry.cpp:108)](../src/EnvironmentGeometry.cpp#L108).

**Kết quả và giới hạn:** 3 đá hiện tại dùng builder này; octahedron geometry::rock 8 triangles chỉ là primitive/test.


<a id="a08"></a>

## A08 — Placement cỏ và mesh hai mặt

**Ý tưởng/Input→Output:** Seed 824, các footprint →64 tuft/384 triangles.

**Quy trình thực tế:** Rejection sampling≤4096 candidate; tránh nhà/path/tree/rock/water bằng grassAllowed. Mỗi tuft 3 blade triangle với mặt đảo explicit.

**Công thức/chi phí:** angle=tuft Angle+blade·2π/3; O(attempts×collision Polygon Cost).

**Implementation:** [grassAllowed (src/EnvironmentGeometry.cpp:19)](../src/EnvironmentGeometry.cpp#L19), [grassTufts (src/EnvironmentGeometry.cpp:33)](../src/EnvironmentGeometry.cpp#L33), [grassTufts::tufts@34 (src/EnvironmentGeometry.cpp:34)](../src/EnvironmentGeometry.cpp#L34), [grass (src/EnvironmentGeometry.cpp:50)](../src/EnvironmentGeometry.cpp#L50).

**Kết quả và giới hạn:** Dựng/upload một lần; cỏ không sway, placement test dùng probe khoảng trống xấp xỉ.


<a id="a09"></a>

## A09 — Polygon vũng nước dùng chung

**Ý tưởng/Input→Output:** Outline 32 điểm và 3 transform →3 world polygon/96 triangles.

**Quy trình thực tế:** r(θ) sin/cos, scale XZ, quay Y, cộng center; fan tâm và cạnh. Cache static dùng chung render/containment.

**Công thức/chi phí:** r=1+.10 sin 3θ+.055 cos 5θ+.035 sin(7θ+.6); O(3×32).

**Implementation:** [puddles (src/Surface.cpp:7)](../src/Surface.cpp#L7), [outlineRadius (src/Surface.cpp:15)](../src/Surface.cpp#L15), [outline (src/Surface.cpp:19)](../src/Surface.cpp#L19), [polygon (src/Surface.cpp:32)](../src/Surface.cpp#L32), [polygon::polygons@35 (src/Surface.cpp:35)](../src/Surface.cpp#L35), [puddles (src/EnvironmentGeometry.cpp:65)](../src/EnvironmentGeometry.cpp#L65).

**Kết quả và giới hạn:** Water height 0.012, opaque; không simulation height field hoặc mesh chuyển theo wetness.


<a id="a10"></a>

## A10 — Skin mây từ tổng Gaussian

**Ý tưởng/Input→Output:** 5 lobe/cluster →728 triangles/cluster.

**Quy trình thực tế:** Field tổng Gaussian; 28×15 hướng có cực; 24 bisection trên radius 0..6 để đạt iso 0.18; normal negative gradient +15%face normal; nối vòng không mặt bên trong.

**Công thức/chi phí:** D(p)=Σexp(−2||((p-o)/s)||²); O(slices·stacks·iterations·lobes).

**Implementation:** [cloud (src/EnvironmentGeometry.cpp:121)](../src/EnvironmentGeometry.cpp#L121), [cloud::field@129 (src/EnvironmentGeometry.cpp:129)](../src/EnvironmentGeometry.cpp#L129), [cloud::add@161 (src/EnvironmentGeometry.cpp:161)](../src/EnvironmentGeometry.cpp#L161).

**Kết quả và giới hạn:** Chỉ làm lúc initialization. Giả định field quanh gốc có một outer crossing trên tia; không kiểm tra mọi input tùy ý, không Marching Cubes/volumetric raymarch.


<a id="a11"></a>

## A11 — Transforms và free camera

**Ý tưởng/Input→Output:** Local mesh, input, camera →clip position.

**Quy trình thực tế:** Scene M=T·Ry·S; CPU normal inverse-transpose; camera yaw/pitch→forward, lookAt, perspective. Normalize composite move để không tăng tốc đường chéo.

**Công thức/chi phí:** clip=P·V·M·p; O(1)/vertex trên GPU vàO(1)/camera update.

**Implementation:** [Scene::add (src/Scene.cpp:61)](../src/Scene.cpp#L61), [Camera::updateDirection (src/Camera.cpp:27)](../src/Camera.cpp#L27), [Camera::rotate (src/Camera.cpp:35)](../src/Camera.cpp#L35), [Camera::move (src/Camera.cpp:41)](../src/Camera.cpp#L41), [Camera::view (src/Camera.cpp:55)](../src/Camera.cpp#L55), [Camera::projection (src/Camera.cpp:59)](../src/Camera.cpp#L59), [main (shaders/object.vert:18)](../shaders/object.vert#L18).

**Kết quả và giới hạn:** Near 0.1/far 80, FOV 55°; camera chỉ clamp volume, có thể xuyên object. Không roll/quaternion.


<a id="a12"></a>

## A12 — Sway cây bằng vertex shader

**Ý tưởng/Input→Output:** treeHeight, time, wind, model translation →bent vertex/normal.

**Quy trình thực tế:** h=y/H; amplitude theo sin và phase vị trí; dịch xz theo h² giữ gốc; sửa normal bằng inverse-transpose Jacobian trước model normal matrix.

**Công thức/chi phí:** xz′=xz+A h²; ny′=ny−dot(nxz, 2Ah/H).

**Implementation:** [main (shaders/object.vert:18)](../shaders/object.vert#L18), [Scene::update (src/Scene.cpp:75)](../src/Scene.cpp#L75).

**Kết quả và giới hạn:** Bark/foliage cùng deform, grass không; không mô phỏng động lực học gió hoặc collision canopy.


<a id="a13"></a>

## A13 — Depth/culling/alpha/state

**Ý tưởng/Input→Output:** Triangles/lines sau raster →fragment được chấp nhận.

**Quy trình thực tế:** Depth LESS và back face CCW/BACK cho cảnh. Sky tắt depth; opaque ghi depth, effect/fading tắt write và SRC_ALPHA blend rồi phục hồi.

**Công thức/chi phí:** Cout=αCs+(1−α)Cd; Z-test là kiểm độ sâu per-fragment.

**Implementation:** [Renderer::Renderer (src/Renderer.cpp:12)](../src/Renderer.cpp#L12), [Renderer::draw (src/Renderer.cpp:84)](../src/Renderer.cpp#L84), [Application::inspectFramebuffer (src/Application.cpp:352)](../src/Application.cpp#L352).

**Kết quả và giới hạn:** Không polygon offset hoặc OIT. Water/ripple nâng Y chống z-fighting; lines không bị face culling như triangles.


<a id="a14"></a>

## A14 — Sort mây đang fade

**Ý tưởng/Input→Output:** 4 cluster/coverage, camera →opaque draws rồi sorted fades.

**Quy trình thực tế:** coverage 0 bỏ, 1 vẽ opaque, <1 lưu depth=dot(center-eye, forward), sort giảm, blend không depth-write. Single skin tránh lobe internal intersection.

**Công thức/chi phí:** Sort O(klogk), k≤4.

**Implementation:** [CloudSystem::coverage (src/CloudSystem.cpp:44)](../src/CloudSystem.cpp#L44), [Renderer::draw (src/Renderer.cpp:84)](../src/Renderer.cpp#L84), [Renderer::draw::drawCloud@158 (src/Renderer.cpp:158)](../src/Renderer.cpp#L158), [Renderer::draw::lambda@180 (src/Renderer.cpp:180)](../src/Renderer.cpp#L180).

**Kết quả và giới hạn:** Chỉ sort cluster đang fade; không sort triangle, không sort rain/splash/ripple/bolt chung. Mesh giao nhau vẫn transparency gần đúng.


<a id="a15"></a>

## A15 — Lambert, ambient RGB, flash và đèn ấm

**Ý tưởng/Input→Output:** World normal, lights/material →base lighting.

**Quy trình thực tế:** Ambient theo màu; directional diffuse max dot; flash point ở đầu bolt với fill+Lambert và distance attenuation; non-cloud thêm đèn cửa fixed position.

**Công thức/chi phí:** Ld=max(N·normalize(−dir), 0); Att=1/(1+.025d+.018 d²).

**Implementation:** [LightingSystem::setEnvironment (src/LightingSystem.cpp:14)](../src/LightingSystem.cpp#L14), [LightingSystem::update (src/LightingSystem.cpp:46)](../src/LightingSystem.cpp#L46), [LightingSystem::attenuationAt (src/LightingSystem.cpp:58)](../src/LightingSystem.cpp#L58), [main (shaders/object.frag:65)](../shaders/object.frag#L65).

**Kết quả và giới hạn:** Không shadow map; flash đại diện một point, đèn ấm attenuation 1/(1+.85 d²), không light object riêng trên CPU.


<a id="a16"></a>

## A16 — Vật liệu procedural và anti-alias grid

**Ý tưởng/Input→Output:** MaterialKind, local position/normal →detail multiplier.

**Quy trình thực tế:** Plaster sin rộng; tile/stone/brick stagger grid, row parity, fract, grout; fwidth làm mềm mép. Wood sin, foliage/rock/grass height/tint; Glass emission đêm.

**Công thức/chi phí:** aa=max(fwidth(grid), .015); detail là hệ số màu, không đổi geometry.

**Implementation:** [main (shaders/object.frag:65)](../shaders/object.frag#L65), [Scene::Scene (src/Scene.cpp:10)](../src/Scene.cpp#L10).

**Kết quả và giới hạn:** Không texture/normal map/PBR; Metal dùng specular response nhưng không nhánh procedural riêng.


<a id="a17"></a>

## A17 — Blinn–Phong wet highlights

**Ý tưởng/Input→Output:** N, L, V, wetness, material →specular RGB.

**Quy trình thực tế:** H=normalize(L+V), pow(max dot(N, H), shininess); hai nguồn directional/flash. Strength theo surface/materialWetGloss; cap 0.45.

**Công thức/chi phí:** S=(max(N·H, 0))^n, n ground 18..42 và water 72 theo localWet/interior.

**Implementation:** [main (shaders/object.frag:65)](../shaders/object.frag#L65).

**Kết quả và giới hạn:** Không energy conservation; không Phong reflection-vector specular, nguồn directional RGB chỉ scale sunSpec theo max component.


<a id="a18"></a>

## A18 — Schlick và fake sky reflection

**Ý tưởng/Input→Output:** N, V, sky colors, water interior →reflected tint.

**Quy trình thực tế:** F0.02 fifth-power Fresnel; reflect view lấy y để mix horizon/top; mix vào water tối đa 0.65 interior, thêm broad flash glint.

**Công thức/chi phí:** F=.02+.98(1−max(N·V, 0))⁵.

**Implementation:** [main (shaders/object.frag:65)](../shaders/object.frag#L65).

**Kết quả và giới hạn:** Phản xạ màu trời theo góc, không phản chiếu hình học nhà/cây; không SSR/cubemap/planar reflection.


<a id="a19"></a>

## A19 — Ground moisture/path/contact

**Ý tưởng/Input→Output:** World XZ, uWetness, contacts →soil color/shading normal.

**Quy trình thực tế:** Field sin/cos continuous, localWet mask; path cong theo sin, footprint nhà và 7 anchor contact dùng max; perturb normal slopes nhỏ không displacement.

**Công thức/chi phí:** localWet=W·mix(.42, 1, smoothstep(.15, .88, field)).

**Implementation:** [moistureField (shaders/object.frag:34)](../shaders/object.frag#L34), [soilColor (shaders/object.frag:39)](../shaders/object.frag#L39), [main (shaders/object.frag:65)](../shaders/object.frag#L65).

**Kết quả và giới hạn:** Contact không phải bóng đổ/AO computed; collision ground vẫn y 0, soiling không thay geometry.


<a id="a20"></a>

## A20 — Sky gradient và exponential fog

**Ý tưởng/Input→Output:** UV/lighting/distance →final RGB.

**Quy trình thực tế:** Sky fullscreen mix horizon/top; object/effect lấy distance world, bỏ 6 đơn vị gần, exp falloff rồi mix fogColor. Cloud scale.65, rain.35, bolt.2, khác 1.

**Công thức/chi phí:** T=exp(−ρkmax(d−6, 0)); C=mix(Cfog, Clit, T).

**Implementation:** [main (shaders/sky.frag:6)](../shaders/sky.frag#L6), [main (shaders/sky.vert:4)](../shaders/sky.vert#L4), [main (shaders/effects.frag:13)](../shaders/effects.frag#L13), [main (shaders/object.frag:65)](../shaders/object.frag#L65), [LightingSystem::fogFactor (src/LightingSystem.cpp:71)](../src/LightingSystem.cpp#L71).

**Kết quả và giới hạn:** Sky screen-space, không khí quyển; fog không volumetric height fog hoặc light scattering.


<a id="a21"></a>

## A21 — Fixed timestep accumulator

**Ý tưởng/Input→Output:** Frame dt từ steady clock →n substep 1/120s.

**Quy trình thực tế:** Clamp frame≤.1; accumulate; while accumulator+1e−7≥step cập nhật modules theo thứ tự cố định rồi trừ step/max 0. Pause không tích lũy dt.

**Công thức/chi phí:** a←a+min(dt, .1); step=1/120; O(n×simulation cost).

**Implementation:** [Application::tick (src/Application.cpp:170)](../src/Application.cpp#L170), [WeatherSystem::update (src/WeatherSystem.cpp:30)](../src/WeatherSystem.cpp#L30).

**Kết quả và giới hạn:** Không render interpolation; stall mất wall time, giờ mô phỏng không tiến bù. Tolerance có thể cho chạy step sớm rất nhỏ.


<a id="a22"></a>

## A22 — Mưa semi-implicit Euler và recycling

**Ý tưởng/Input→Output:** 1000 drop, wind, dt →positions/velocities/streaks.

**Quy trình thực tế:** Init deterministic distributed; gravity/cap terminal−17; drag xz tới wind; position dùng v mới; age tăng. Chạm đất/ra XZ respawn cùng slot; length theo exposure.

**Công thức/chi phí:** v←v+gdt; vy=max(vy, −17); p←p+vdt; length=clamp(|v|·.024s, .12, .42).

**Implementation:** [RainSystem::respawn (src/RainSystem.cpp:10)](../src/RainSystem.cpp#L10), [RainSystem::reset (src/RainSystem.cpp:20)](../src/RainSystem.cpp#L20), [RainSystem::update (src/RainSystem.cpp:28)](../src/RainSystem.cpp#L28), [main (shaders/effects.vert:12)](../shaders/effects.vert#L12).

**Kết quả và giới hạn:** Wind là relaxation tuyến tính, terminal hard cap, không drag vật lý theo kích thước; 1000 lines trong 1 draw. Distance alpha/tail dựng GPU, không per-drop draw.


<a id="a23"></a>

## A23 — Impact plane và water containment

**Ý tưởng/Input→Output:** Previous/new rain position và polygon →hit/margin.

**Quy trình thực tế:** Nếu y≤0 dùng t=prevY/(prevY−newY), nội suy hit; lọc XZ ground. Water dùng AABB reject/ray crossing và khoảng cách ngắn nhất tới segment.

**Công thức/chi phí:** hit=(1−t)p0+tp1; edge t=clamp((p−a)·e/(e·e), 0, 1).

**Implementation:** [RainSystem::update (src/RainSystem.cpp:28)](../src/RainSystem.cpp#L28), [waterMargin (src/Surface.cpp:52)](../src/Surface.cpp#L52), [waterMargin::bounds@53 (src/Surface.cpp:53)](../src/Surface.cpp#L53).

**Kết quả và giới hạn:** Chỉ plane Y 0; không ray-triangle mái/cây/đá. waterMargin bỏ qua y; chỉ valid polygon inputs nội bộ.


<a id="a24"></a>

## A24 — Splash pool và lifetime

**Ý tưởng/Input→Output:** Impact requests →tối đa 512 slot.

**Quy trình thực tế:** Đếm request, validate/sample 45%, cần 2 slot; cursor scan slot chết, emit 2 radial/upward; gravity/Euler và remaining lifetime/y expire.

**Công thức/chi phí:** alpha=.85·remaining/initial; O(capacity)update, scan≤capacity mỗi allocation.

**Implementation:** [SplashSystem::reset (src/SplashSystem.cpp:10)](../src/SplashSystem.cpp#L10), [SplashSystem::emit (src/SplashSystem.cpp:18)](../src/SplashSystem.cpp#L18), [SplashSystem::update (src/SplashSystem.cpp:38)](../src/SplashSystem.cpp#L38), [Renderer::draw (src/Renderer.cpp:84)](../src/Renderer.cpp#L84).

**Kết quả và giới hạn:** Filtered khác rejected; không overwrite sống. Full pool có thể hợp lệ trong stress test; vệtGL_LINES1pixel.


<a id="a25"></a>

## A25 — Ripple pool/containment/expansion

**Ý tưởng/Input→Output:** Hit, waterMargin →tối đa 48 vòng 12 cạnh.

**Quy trình thực tế:** Sample 35%nước/5.5%đất; radius cap.45 hoặc.9 margin, bỏ cap<.08; slot chết với lifetime.55; update elapsed/radius/alpha, release hết đời.

**Công thức/chi phí:** r=mix(.04, rmax, t); α=.5(1−t)².

**Implementation:** [RippleSystem::reset (src/RippleSystem.cpp:5)](../src/RippleSystem.cpp#L5), [RippleSystem::emit (src/RippleSystem.cpp:13)](../src/RippleSystem.cpp#L13), [RippleSystem::update (src/RippleSystem.cpp:27)](../src/RippleSystem.cpp#L27), [Renderer::draw (src/Renderer.cpp:84)](../src/Renderer.cpp#L84).

**Kết quả và giới hạn:** Ripple đất vẫn emit dù W=0 vì không xét wetness; đây là animation line, không phải sóng vật lý. Rejected chỉ count full pool.


<a id="a26"></a>

## A26 — Cloud advection/wrap/fade

**Ý tưởng/Input→Output:** Wind, cluster speed, dt →position periodic/coverage.

**Quy trình thực tế:** Move XZ, speed.20...275; wrap 20 unit ở±10; fade 2.5 unit mỗi seam với smoothstep.

**Công thức/chi phí:** x′=x−20 floor((x+10)/20); α=t²(3−2t).

**Implementation:** [CloudSystem::reset (src/CloudSystem.cpp:12)](../src/CloudSystem.cpp#L12), [CloudSystem::update (src/CloudSystem.cpp:34)](../src/CloudSystem.cpp#L34), [wrap (src/CloudSystem.cpp:7)](../src/CloudSystem.cpp#L7), [CloudSystem::coverage (src/CloudSystem.cpp:44)](../src/CloudSystem.cpp#L44).

**Kết quả và giới hạn:** Không volumetric dynamics; cloud toggle freeze/hide nhưng Lightning vẫn dùng dữ liệu cloud kể cả cloud bị tắt.


<a id="a27"></a>

## A27 — Midpoint displacement ràng buộc

**Ý tưởng/Input→Output:** A/B, depth, displacement, width →segments.

**Quy trình thực tế:** Basis vuông góc, random offset 2 chiều; cap.12|AB|và scale chung để ở trong hộp; đệ quy 2 nửa, displacement*.4.

**Công thức/chi phí:** M=(A+B)/2+δ; N=2^depth; O(2^depth).

**Implementation:** [LightningSystem::subdivide (src/LightningSystem.cpp:26)](../src/LightningSystem.cpp#L26).

**Kết quả và giới hạn:** Fractal-looking polyline, không mô phỏng electrical breakdown. Không phải random walk đơn giản; geometry endpoints nằm trong ngân sách/test bounds.


<a id="a28"></a>

## A28 — Sét chọn nguồn/branch/taper

**Ý tưởng/Input→Output:** Clouds, observer, seed →32 trunk+48 branch segments.

**Quy trình thực tế:** Ưu tiên coverage>.65 và XZ distance>4.5, else farthest; root branch là vertex chung segments[5+4i].end; sáu nhánh sâu 3, taper đều.

**Công thức/chi phí:** wtrunk=.095(1−.35i/31), wbranch=.032(1−.6j/7).

**Implementation:** [LightningSystem::trigger (src/LightningSystem.cpp:55)](../src/LightningSystem.cpp#L55).

**Kết quả và giới hạn:** MAX 128 nhưng thực 80; tránh camera bằng heuristic không bảo đảm mọi observer; auto có thể strike khi rain/cloud toggle off.


<a id="a29"></a>

## A29 — Pulse/billboard/core-halo/cache

**Ý tưởng/Input→Output:** Segments, pulses, camera →transient flash/batch 320 triangles.

**Quy trình thực tế:** Duration.28...42s, 1..3 pulse với khoảng tối; exp decay. Renderer cross segment/view→side, 2 quad mỗi đoạn, edge varyings mềm. Re build theo revision/camera, intensity uniform.

**Công thức/chi phí:** flash=peak exp(−32τ)clamp((duration−τ)/.012, 0, 1).

**Implementation:** [LightningSystem::trigger (src/LightningSystem.cpp:55)](../src/LightningSystem.cpp#L55), [LightningSystem::update (src/LightningSystem.cpp:102)](../src/LightningSystem.cpp#L102), [LightningSystem::flashIntensity (src/LightningSystem.cpp:115)](../src/LightningSystem.cpp#L115), [Renderer::draw (src/Renderer.cpp:84)](../src/Renderer.cpp#L84), [main (shaders/effects.frag:13)](../shaders/effects.frag#L13).

**Kết quả và giới hạn:** Halo là lớp quad alpha, không bloom; cache vẫn upload khi camera đổi; sét không cull faces nhưng giữ depth test.


<a id="a30"></a>

## A30 — Day/night clock và ánh sáng

**Ý tưởng/Input→Output:** Hour/dayDuration/auto, storm →clock/RGB/sun/moon/windows.

**Quy trình thực tế:** Wrap fmod, từ chối NaN/Inf; auto hoạt động trong fixed step. smooth dawn 5..7, dusk 17..19→daylight/twilight, blend hướng/màu, bật window đêm.

**Công thức/chi phí:** Δh=24·fmod(dt, D)/D; daylight=dawn(1−dusk).

**Implementation:** [TimeOfDaySystem::setHour (src/TimeOfDaySystem.cpp:6)](../src/TimeOfDaySystem.cpp#L6), [TimeOfDaySystem::setDayDuration (src/TimeOfDaySystem.cpp:12)](../src/TimeOfDaySystem.cpp#L12), [TimeOfDaySystem::update (src/TimeOfDaySystem.cpp:16)](../src/TimeOfDaySystem.cpp#L16), [TimeOfDaySystem::label (src/TimeOfDaySystem.cpp:21)](../src/TimeOfDaySystem.cpp#L21), [LightingSystem::setEnvironment (src/LightingSystem.cpp:14)](../src/LightingSystem.cpp#L14), [shiftHours (include/TimeOfDaySystem.hpp:11)](../include/TimeOfDaySystem.hpp#L11), [toggleDayNight (include/TimeOfDaySystem.hpp:12)](../include/TimeOfDaySystem.hpp#L12), [setAutoAdvance (include/TimeOfDaySystem.hpp:13)](../include/TimeOfDaySystem.hpp#L13).

**Kết quả và giới hạn:** Đã triển khai thật; không system clock/astronomy. Space pause, manual hour vẫn đổi; R về 18:00/OFF/D=600.


<a id="a31"></a>

## A31 — Wetness tăng/khô tuyến tính

**Ý tưởng/Input→Output:** Rain enabled, dt →wetness[0, 1].

**Quy trình thực tế:** Default.65, tăng.06/s, mất.012/s, min(dt, .1)rồi clamp; Shader dùng W và local moisture.

**Công thức/chi phí:** W′=clamp(W+rate·dt, 0, 1).

**Implementation:** [LightingSystem::update (src/LightingSystem.cpp:46)](../src/LightingSystem.cpp#L46), [WeatherSystem::update (src/WeatherSystem.cpp:30)](../src/WeatherSystem.cpp#L30), [main (shaders/object.frag:65)](../shaders/object.frag#L65).

**Kết quả và giới hạn:** Không có tích lũy chất lỏng/evaporation model; area nước không thay theo W. CPU groundMultiplier là helper test, không công thức đất shader hiện tại.


<a id="a32"></a>

## A32 — Reuse/pool/batch/state management

**Ý tưởng/Input→Output:** Static meshes, dynamic staging/revisions →giảm allocation/draw/upload.

**Quy trình thực tế:** Cấp VAO/VBO một lần; glBufferSubData; cached unit box, polygons, cloud mesh, ring; pool std::array; ngừng upload khi revision không đổi. Scene gộp cỏ/vũng/nhà theo material.

**Công thức/chi phí:** O(active vertices)upload; fixed caps bound memory.

**Implementation:** [Mesh::Mesh (src/Mesh.cpp:8)](../src/Mesh.cpp#L8), [DynamicBatch::DynamicBatch (src/DynamicBatch.cpp:7)](../src/DynamicBatch.cpp#L7), [DynamicBatch::upload (src/DynamicBatch.cpp:36)](../src/DynamicBatch.cpp#L36), [Renderer::Renderer (src/Renderer.cpp:12)](../src/Renderer.cpp#L12), [Renderer::draw (src/Renderer.cpp:84)](../src/Renderer.cpp#L84), [box (src/ModelGeometry.cpp:18)](../src/ModelGeometry.cpp#L18).

**Kết quả và giới hạn:** Không GPU instancing, frustum/occlusion culling, compute/transform feed back. Back face culling và coverage 0 skip có thực; không gọi những kỹ thuật chưa có là optimization.


<a id="a33"></a>

## A33 — Đo hiệu năng và nonblocking query

**Ý tưởng/Input→Output:** Frame clock/rusage/GL queries/proc →FPS/percentiles/RSS.

**Quy trình thực tế:** Ring 65536 interval; 4GL_TIME_ELAPSED queries poll AVAILABLE; CPU submit wall trước Swap; sample pool/RSS mỗi giây, trace phút; timer 16 ms/unlocked idle.

**Công thức/chi phí:** FPS=frames/seconds; quantile=sorted[floor((N−1)p)].

**Implementation:** [Benchmark::beginFrame (src/Benchmark.cpp:64)](../src/Benchmark.cpp#L64), [Benchmark::endFrame (src/Benchmark.cpp:89)](../src/Benchmark.cpp#L89), [Benchmark::sample (src/Benchmark.cpp:99)](../src/Benchmark.cpp#L99), [Benchmark::report (src/Benchmark.cpp:114)](../src/Benchmark.cpp#L114), [Application::tick (src/Application.cpp:170)](../src/Application.cpp#L170).

**Kết quả và giới hạn:** GPU command ms không utilization/presentation; RSS không VRAM; pool peaks chỉ sampled; thiếu mẫu GPU pending không chặn report.


## Những kỹ thuật không có trong implementation

Không có EBO/indexed rendering, frustum/occlusion culling, OIT/per-particle depth sort, shadow mapping, ray tracing, texture sampling, PBR BRDF, true reflections/refraction, volumetric raymarching, physics water, L-system hoặc audio. GLSL không có discard/gl_FragCoord; anti-alias grid dùng fwidth không phải stochastic dither. Không dùng thuật toán DDA/Bresenham tự viết: rasterization triangle/line do OpenGL. Không có GPU compute particle simulation; CPU update và GPU transform/shading được phân biệt trong 05.
