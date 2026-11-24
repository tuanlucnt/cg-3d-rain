# 08 — Ưu điểm, hạn chế và hướng cải thiện

[Tổng quan](01_PROJECT_OVERVIEW.md) · [Bằng chứng test/hiệu năng](07_TESTING_AND_PERFORMANCE.md)

Đây là review, không phải thay đổi implementation. Các đề xuất dưới đây **chưa được triển khai**. Ưu tiên P1 ảnh hưởng cơ chế/khả năng xác minh; P2 cải thiện chất lượng và tính bền vững; P3 mở rộng khi có nhu cầu. Không có căn cứ từ lần chạy 5 binary CPU để báo một lỗi crash tái hiện được trong luồng mặc định.

## Mục lục

- [Điểm mạnh có bằng chứng](#s01)
- [Hạn chế thực tế và đề xuất](#s02)
- [Những gì đã hoàn thành trong code](#s03)
- [Những gì chưa có](#s04)
- [Thứ tự nâng cấp hợp lý cho bài học](#s05)

<a id="s01"></a>

## Điểm mạnh có bằng chứng

| Mặt | Implementation/bằng chứng | Giá trị khi báo cáo |
|---|---|---|
| Tách CPU/OpenGL | [CMakeLists.txt:13](../CMakeLists.txt#L13), scene_core chỉ link GLM |5 CPU test chạy không cần display; học thuật toán riêng khỏi driver |
| Geometry procedural | [house:79](../src/ModelGeometry.cpp#L79), [tree:140](../src/ModelGeometry.cpp#L140), [environment cloud:121](../src/EnvironmentGeometry.cpp#L121) | Có triangle mesh/winding/normal, bake model, seed và density field cụ thể |
| Shared geometry/collision | [Surface::polygon:32](../src/Surface.cpp#L32), [waterMargin:52](../src/Surface.cpp#L52) | Polygon nước dùng chung tránh lệch mesh và ripple containment |
| State ownership | Mesh/Shader/DynamicBatch destructors; [Application cleanup:327](../src/Application.cpp#L327) | GL IDs thuộc owner rõ; hủy trước context; copy cấm |
| Thời tiết liên kết | [WeatherSystem::update:30](../src/WeatherSystem.cpp#L30) | Gió chung, impact→splash/ripple, bolt→lighting, clock→environment |
| Tính ổn định simulation | Fixed 1/120, dt clamp, terminal/pool bounds, RNG seed | Test so cùng thời gian với step grouping khác; không tạo particle vô hạn |
| Lighting/render | [object.frag:65](../shaders/object.frag#L65), [Renderer::draw:84](../src/Renderer.cpp#L84) | Có RGB day/night, Lambert/Blinn/Schlick, normal matrix, depth/blend state rõ |
| Chi phí bị chặn | Persistent VBO, batch draw, revision cache, pool array | Không per-particle allocation/draw; 80 bolt segments/1000 rain có cap |
| Validation |5 test suites, GUI pixel smoke, weather.valid và GL error checks | Kiểm cả CPU invariants và ảnh/state khi có GUI; không chỉ đọc README |
| Đo lường | Async query 4 slot, frame interval ring 65536, RSS/rusage | Chỉ số có phạm vi rõ, không phải đo GPU utilization |

<a id="s02"></a>

## Hạn chế thực tế và đề xuất

| ID/ưu tiên | Nguyên nhân và vị trí | Ảnh hưởng | Phương án đề xuất, chưa làm |
|---|---|---|---|
| L01/P1 | Rain collision chỉ kiểm newY≤0 ở [RainSystem.cpp:43](../src/RainSystem.cpp#L43) | Giọt có thể đi xuyên mái/tán/đá và phát splash dưới object | Thêm collision proxies hoặc mesh/BVH; giữ ground interpolation làm fallback; test roof-hit/no-underground-splash |
| L02/P2 | Camera::move chỉ clamp hộp [Camera.cpp:41](../src/Camera.cpp#L41) | Tự đi xuyên nhà/cây/đá dù preset được kiểm ngoài object | Sphere/capsule camera với collider đơn giản, slide collision và test không xuyên |
| L03/P1 | [Renderer.cpp:148–188](../src/Renderer.cpp#L148) chỉ sort cluster fade; [190–342](../src/Renderer.cpp#L190) các effect không sort chung | Alpha ở geometry giao nhau/particle có thể sai thứ tự; back face cull cũng không mô hình thể tích mây | Nếu cần chất lượng, đánh giá global transparent sort hoặc weighted blended OIT; test chéo cloud/particle tại nhiều camera |
| L04/P2 | [environment::cloud:121](../src/EnvironmentGeometry.cpp#L121) bisection theo ray 0..6 và iso.18, không kiểm crossing | Cấu hình lobe tùy ý có thể không star-shaped hoặc field gốc dưới iso; chỉ default layouts được kiểm | Validate origin/bracket/monotonic assumptions hoặc dùng trích iso surface tổng quát; không tự gọi hiện tại là Marching Cubes |
| L05/P2 | Gaussian skin và normal blend, cloud move chỉ translation [EnvironmentGeometry.cpp:129](../src/EnvironmentGeometry.cpp#L129), [CloudSystem.cpp:34](../src/CloudSystem.cpp#L34) | Mây không tán xạ/tiến hóa mật độ, hình dạng cố định | Cân nhắc density animation/volume khi có budget; đánh giá sorting và sampling trước |
| L06/P2 | Gravity/Euler, drag relaxation và hard cap [RainSystem.cpp:28](../src/RainSystem.cpp#L28) | Vật lý xấp xỉ, không size-dependent terminal/turbulence/air interaction | Nếu yêu cầu vật lý, định nghĩa đơn vị/thông số hạt và mô hình drag; test analytical/simple cases |
| L07/P2 | GL_LINES rain/splash/ripple [Renderer.cpp:190](../src/Renderer.cpp#L190) | Mưa rộng 1 pixel, vệt/ring alias, không có giọt 3D/DOF | Billboard quad có width, analytic coverage hoặc MSAA nếu hỗ trợ; giữ một batch |
| L08/P2 | Sampling và fixed caps [SplashSystem.cpp:18](../src/SplashSystem.cpp#L18), [RippleSystem.cpp:13](../src/RippleSystem.cpp#L13) | Phát thiếu khi full; bỏ phần lớn impacts để giữ budget; ring là animation | Tuning theo quality hoặc emission budget có log; không tự tăng capacity khi chưa profile |
| L09/P2 | Ripple emission trên đất không xét wetness [WeatherSystem.cpp:43](../src/WeatherSystem.cpp#L43), [RippleSystem.cpp:13](../src/RippleSystem.cpp#L13) | Khi W=0 mà rain được bật lại, ripple đất có thể xuất hiện ngay; không mô hình puddle forming | Nếu muốn liên hệ vật lý, cho emission probability phụ thuộc local wetness; test case khô/bật mưa |
| L10/P2 | Wetness global, tuyến tính và static water [LightingSystem.cpp:46](../src/LightingSystem.cpp#L46), [Surface.cpp:7](../src/Surface.cpp#L7) | Vũng không co/giãn, lượng mưa/thoát nước không bảo toàn; đất không có height | Wetness field/spatial water balance hoặc height field, nêu rõ cost/collision nếu đổi |
| L11/P1 | Không shadow/visibility sampling trong [object.frag](../shaders/object.frag#L65) | Đèn chớp/sun/lamp chiếu xuyên vật; contact darkening không phải bóng thật | Shadow mapping có budget, kiểm bias/acne/peterpanning; không gọi contact shade là shadow map |
| L12/P2 | Blinn–Phong/capped flash/fake Fresnel [object.frag:90–150](../shaders/object.frag#L90) | Không bảo toàn năng lượng/true reflection; độ bóng gần đúng, không có refraction | Nếu nâng cấp, chuẩn hóa linear color và BRDF; reflection probe/planar/SSR là những lựa chọn khác nhau chưa có |
| L13/P2 | Fragment output trực tiếp, không có cấu hình sRGB/HDR/tone-map setup trong [Renderer.cpp](../src/Renderer.cpp#L12) | Không bảo đảm pipeline gamma/linear physically correct trên mọi framebuffer | Kiểm tra framebuffer encoding, thiết kế linear workflow phù hợp rồi test; không khẳng định đã có gamma correction |
| L14/P2 | Sky UV gradient [sky.frag:6](../shaders/sky.frag#L6), sun/moon artistic [LightingSystem.cpp:14](../src/LightingSystem.cpp#L14) | Bầu trời không xoay theo camera, mặt trời không có mesh, không thiên văn | Gradient theo world view ray/analytic sky nếu phù hợp, không cần thêm trong review |
| L15/P2 | Box/tube/seeded low poly [ModelGeometry.cpp](../src/ModelGeometry.cpp#L79) | Nhà có khối chồng, cửa không khoét, kính opaque; thân/tán cây facet, cỏ tĩnh | Nếu mục tiêu realism, kiểm topology/intersection và material trước tăng poly/assets |
| L16/P1 | Frame dt clamp và không interpolation [WeatherSystem.cpp:30](../src/WeatherSystem.cpp#L30) | Khi stall mô phỏng chậm wall time; render cao FPS có thể lặp lại state 120 Hz | Chọn rõ chính sách wall time/catch-up và interpolation nếu cần; thêm test drop-time/slow frame |
| L17/P2 | Scene animationTime float [Scene.cpp:75](../src/Scene.cpp#L75), clock double | Sway và weather không dùng cùng accumulator; độ chính xác float giảm sau thời gian chạy dài | Dùng bounded phase/double hoặc cùng sim clock nếu có yêu cầu đồng bộ; hiện pause/reset đã phối hợp |
| L18/P1 | Benchmark report [Benchmark.cpp:114](../src/Benchmark.cpp#L114) | Average cả lượt, P95/P99 chỉ dùng interval còn lưu; query cuối chưa sẵn thiếu mẫu, pool peak chỉ sampled | Báo measurement window/samples, chọn flush cuối có kiểm soát hoặc trì hoãn báo cáo; không block per frame |
| L19/P1 | Linux/GLX, /proc [Benchmark.cpp:1](../src/Benchmark.cpp#L1) | Benchmark không portable Win/mac/Wayland native; mở/đọc/proc lỗi có thể báo 0 | Tách phép đo theo nền tảng và availability status; nêu unsupported thay vì 0; không thêm dependency trong review |
| L20/P1 | Log soak cũ 53 calls/2675 triangles [soak-600.log](../build/reports/soak-600.log), bản mới 32/6497 | Chưa có bằng chứng throughput 600 giây bản mới, cửa sổ nền bị≈1 FPS trong log cũ | Đo lại giữ visible, cùng mode/build/driver; ghi wall time, sim speed, scheduling/presentation |
| L21/P1 |5 CPU binary hiện có +log cũ; chưa rebuild/GUI trong review | Không chứng minh binaries được dựng từ exact snapshot hoặc ảnh/cảm giác input hiện phiên | Tái hiện build/test/GUI theo 07 khi có phiên đồ họa; dùng hash/build metadata nếu cần provenance |
| L22/P2 | [WeatherSystem::valid:67](../src/WeatherSystem.cpp#L67) kiểm chọn lọc | Không kiểm hết wind/pulse range/width/material/GPU state; validation=ok chỉ cho những invariant đã được viết | Bổ sung invariants theo public API cần hỗ trợ; negative parameter/property tests; không hiểu là chứng minh đúng mọi state |
| L23/P3 | Không frustum/occlusion culling hoặc instancing [Renderer::draw](../src/Renderer.cpp#L84) | Mọi SceneObject submit kể cả ngoài view; ở quy mô lớn cost tăng | Profile trước, thêm frustum culling/instancing khi workload cần; back face culling hiện đã có |
| L24/P2 | API helper tin preconditions: [beam:24](../src/ModelGeometry.cpp#L24), [DynamicBatch::upload:36](../src/DynamicBatch.cpp#L36) | beam axis song song Z tạo zero cross; upload null+count>0 hoặc scale zero không được chặn, caller hiện hợp lệ | Nếu mở public/general-use, validate/fallback và test invalid; không gọi là bug đang tái hiện trong scene mặc định |
| L25/P3 | [LightingSystem.hpp:9](../include/LightingSystem.hpp#L9), [CloudLobe.shade](../include/CloudSystem.hpp#L9), [groundMultiplier](../include/LightingSystem.hpp#L34) | AMBIENT/DIFFUSE/shade không được pipeline đọc hiện tại; groundMultiplier chỉ test; dễ giải thích nhầm | Dọn/deprecate hoặc ghi rõ legacy trong development riêng; review hiện giữ nguyên |
| L26/P2 | README chứa nhiều báo cáo lịch sử | Số cloud 100 triangles, puddle 16 triangles, số test 2 suite có thể bị đọc thành hiện tại | Tách lịch sử/trạng thái hiện tại trong lần chỉnh README được ủy quyền riêng; 10 docs mới chọn source làm chuẩn |

<a id="s03"></a>

## Những gì đã hoàn thành trong code

Có geometric triangle meshes, normals/winding, transformations/camera/perspective, depth/culling, ambient/Lambert/Blinn–Phong/Fresnel xấp xỉ, rain/pool/collision-plane, splash/ripple, Gaussian-cloud skin, cloud wrap/fade, midpoint/branch/pulse bolt, day/night, sky/fog, wet ground/water, batch/cache/RAII, CPU test/GUI smoke và benchmark. Tài liệu 03 có 33 nhóm kỹ thuật với file/hàm cụ thể; không phải lời hứa nâng cấp.

<a id="s04"></a>

## Những gì chưa có

Không shadow mapping, OIT, volumetric lighting/cloud/fog, audio/thunder sound, texture assets runtime, physically based weather/water, true reflection/refraction, camera/object collision, GPU particle compute, frustum/occlusion culling, HDR/bloom/tone mapping hoặc GUI settings overlay. assets/audio và assets/textures hiện rỗng, không có assimp/objloader. Không có số đo VRAM/utilization hoặc multi-GPU results trong source/log đã đối chiếu.

<a id="s05"></a>

## Thứ tự nâng cấp hợp lý cho bài học

Ưu tiên giải thích/đo lại những gì đã có và tránh dùng số liệu lịch sử sai phiên bản. Nếu mở development phase, collision plane→object và transparency/lighting visibility là các cải thiện có ảnh hưởng rõ; tiếp theo chất lượng rain lines, đồng bộ sim time/render và validation API. Các nâng cấp nặng như volumetric/PBR/reflection chỉ cân nhắc khi mục tiêu và budget rõ. Không đề xuất cài thêm thư viện hoặc sửa source trong nhiệm vụ này.
