# 09 — Câu hỏi và trả lời khi thuyết trình

[Tổng quan](01_PROJECT_OVERVIEW.md) · [Chỉ mục hàm](10_FUNCTION_INDEX.md) · [Công thức](04_MATHEMATICAL_FORMULAS.md)

Các câu trả lời mô tả implementation hiện tại. Thuật ngữ tiếng Anh giữ nguyên để dễ đối chiếu source.

## Mục lục

- [Nền tảng và kiến trúc](#s01)
- [Geometry, camera và xử lý mặt khuất](#s02)
- [Lighting và shader](#s03)
- [Particle và sét](#s04)
- [Hiệu năng, kiểm thử và giới hạn](#s05)
- [Gợi ý cấu trúc bài nói ngắn](#s06)

<a id="s01"></a>

## Nền tảng và kiến trúc

### 1. Vì sao project chọn OpenGL?

OpenGL cung cấp pipeline rasterization để tự học cách truyền mesh, viết shader, biến đổi tọa độ và điều khiển depth/blend. Project dùng Core 3.3, không fixed-function `glBegin/glEnd`; mọi draw đi qua VAO/VBO và shader. Đây là lý do phù hợp với mục tiêu môn học, không phải khẳng định OpenGL luôn nhanh hơn API khác. Nguồn: [Renderer.cpp:12](../src/Renderer.cpp#L12), [Mesh.cpp:8](../src/Mesh.cpp#L8).

### 2. FreeGLUT có nhiệm vụ gì?

Tạo cửa sổ/context, quản lý event loop và callback display/timer/input/resize/close; `glutSwapBuffers` trình bày back buffer. FreeGLUT không tự cập nhật particle hoặc tính lighting. [Application.cpp:91](../src/Application.cpp#L91).

### 3. GLEW dùng làm gì?

Khởi tạo khả năng truy cập OpenGL functions/extensions sau khi có context. Code bật glewExperimental, gọi glewInit, kiểm version/core/depth rồi mới dựng tài nguyên GPU. GLEW header đứng trước FreeGLUT để tránh xung đột include. [Application.cpp:105](../src/Application.cpp#L105), [Application.hpp:3](../include/Application.hpp#L3).

### 4. GLM dùng làm gì?

Cung cấp vec/mat, dot/cross/normalize, translate/rotate/scale/lookAt/perspective/inverse/transpose. GLM chạy trên CPU; GLSL có kiểu/toán tương tự trên GPU nhưng không gọi thư viện GLM. [Camera.cpp:55](../src/Camera.cpp#L55), [Scene.cpp:61](../src/Scene.cpp#L61).

### 5. Scene và Renderer khác nhau thế nào?

Scene sở hữu mesh/danh sách 23 SceneObject cùng transform/material và animationTime; Renderer chọn shader, upload uniforms/dynamic vertices và thực hiện các pass. Clouds/effects dùng dữ liệu Weather và tài nguyên riêng trong Renderer. [Scene.hpp:19](../include/Scene.hpp#L19), [Renderer.cpp:84](../src/Renderer.cpp#L84).

### 6. Phần nào chạy CPU, phần nào chạy GPU?

CPU tạo geometry, cập nhật weather/clock, collision/pool, tính ma trận và sort fade clouds. GPU làm tree sway, rain tail theo camera, transform, rasterization và fragment lighting/fog/Fresnel. Không có compute shader mô phỏng particle. [WeatherSystem.cpp:30](../src/WeatherSystem.cpp#L30), [object.vert:18](../shaders/object.vert#L18), [effects.vert:12](../shaders/effects.vert#L12).

<a id="s02"></a>

## Geometry, camera và xử lý mặt khuất

### 7. VAO/VBO là gì trong project?

VBO lưu Vertex/EffectVertex trên GPU; VAO lưu cách giải mã buffer thành attribute theo location/stride/offset. Mesh dùng attr0 position và 1 normal; DynamicBatch thêm color vec4 và edge float. VAO không chứa shader hoặc dữ liệu uniform. [Mesh.cpp:15](../src/Mesh.cpp#L15), [DynamicBatch.cpp:12](../src/DynamicBatch.cpp#L12).

### 8. Project có dùng EBO hoặc index buffer không?

Không. Mesh và DynamicBatch đều gọi glDrawArrays; vertex bị lặp theo triangle list. Test dùng map cạnh/quantize position để kiểm topo nhưng đó không phải EBO render. [Mesh.cpp:36](../src/Mesh.cpp#L36), [DynamicBatch.cpp:46](../src/DynamicBatch.cpp#L46).

### 9. Mô hình khung kết nối khác mô hình mặt đa giác thế nào?

Khung biểu diễn cạnh, còn mặt đa giác có diện tích để rasterize/che khuất/chiếu sáng. Project render nhà/cây/đá bằng tam giác, không có wireframe mode; GL_LINES ở đây dùng cho rain/splash/ripple, không phải khung của mesh nhà. [Geometry.hpp:12](../include/Geometry.hpp#L12), [Renderer.cpp:190](../src/Renderer.cpp#L190).

### 10. Normal được tính ra sao?

Helper lấy normalize(cross(b−a, c−a)); ba đỉnh một mặt nhận cùng normal để flat shading. Model/Environment helper còn đổi b/c nếu ngược outward. Cloud skin có gradient normal trộn85% với 15%face normal. [Geometry.cpp:8](../src/Geometry.cpp#L8), [EnvironmentGeometry.cpp:161](../src/EnvironmentGeometry.cpp#L161).

### 11. Tại sao cần Normal Matrix?

Khi scale không đều, nhân normal trực tiếp bằng model matrix có thể mất vuông góc với mặt. Scene tính transpose(inverse(mat3(model))), shader dùng cho normal; fragment normalize sau nội suy. Cây còn sửa normal theo Jacobian bend trước đó. [Scene.cpp:61](../src/Scene.cpp#L61), [object.vert:26](../shaders/object.vert#L26).

### 12. Thứ tự nhân ma trận là gì?

Vector cột: clip=P·V·M·local. Model=T·Ry·S nên local được scale trước, rotate sau rồi translate. `GL_FALSE` trong glUniformMatrix không đảo thứ tự ma trận. [Scene.cpp:61](../src/Scene.cpp#L61), [object.vert:34](../shaders/object.vert#L34).

### 13. Phép chiếu phối cảnh hoạt động ra sao?

Camera tạo perspective FOV 55°, aspect width/height, near.1, far 80. Sau phép nhân projection, pipeline chia xyz cho w; vật xa nhỏ hơn. Core test xác nhận đổi aspect chỉ đổi scale ngang và ma trận có thành phần perspective, không phải orthographic. [Camera.cpp:59](../src/Camera.cpp#L59), [core_tests.cpp:126](../tests/core_tests.cpp#L126).

### 14. Depth Test là gì?

Mỗi fragment so depth với depth buffer; GL_LESS chỉ nhận fragment gần hơn depth đã lưu. Scene ghi depth; transparent effects vẫn test depth để không xuyên vật opaque nhưng tắt depth write để không chặn hiệu ứng vẽ sau bằng depth không đầy đủ. [Renderer.cpp:55](../src/Renderer.cpp#L55), [Renderer.cpp:213](../src/Renderer.cpp#L213).

### 15. Back-face Culling là gì?

Loại mặt quay lưng theo winding: project đặt front CCW, cull BACK. Điều này tiết kiệm xử lý mặt kín quay vào trong nhưng đòi winding đúng. Cỏ có triangle đảo explicit cho hai mặt; bolt tắt culling tạm vì billboard. Không đồng nghĩa frustum culling. [Renderer.cpp:61](../src/Renderer.cpp#L61), [EnvironmentGeometry.cpp:50](../src/EnvironmentGeometry.cpp#L50).

### 16. Tại sao phải sort vật trong suốt?

Alpha blending phụ thuộc thứ tự. Hiện chỉ cloud đang fade được sort theo dot(center−eye, forward) giảm dần; mưa/splash/ripple/bolt có pass riêng và không sort từng hạt. Single-skin cloud giảm giao cắt lobe nội bộ nhưng không giải quyết mọi transparency intersection. [Renderer.cpp:155](../src/Renderer.cpp#L155).

### 17. Project tránh Z-fighting ở nước/ripple thế nào?

Đất y 0, nước y.012, ripple tối thiểu y.028; nâng các lớp giúp depth khác nhau. Không có glPolygonOffset. Shader perturb ground normal chỉ đổi shading, không đổi cao độ collision. [Surface.cpp:7](../src/Surface.cpp#L7), [RippleSystem.cpp:22](../src/RippleSystem.cpp#L22), [object.frag:76](../shaders/object.frag#L76).

<a id="s03"></a>

## Lighting và shader

### 18. Shader là gì và có bao nhiêu shader?

Chương trình chạy trên GPU. Có 6 file:object.vert/frag, effects.vert/frag, sky.vert/frag; chúng link thành 3 program. Shader class đọc file, compile/link, kiểm lỗi và xóa program theo RAII. [Shader.cpp:9](../src/Shader.cpp#L9), [Renderer.cpp:12](../src/Renderer.cpp#L12).

### 19. Lambert lighting hoạt động thế nào?

Diffuse dùng max(dot(N, normalize(−lightDirection)), 0). N/L song song cùng chiều thì 1, vuông góc thì 0. Object shader cộng RGB ambient và flash; cloud thêm fill Lambert .12+.88 Lambert. [object.frag:82](../shaders/object.frag#L82).

### 20. Ambient, directional và flash có tác dụng lên đâu?

Object shader áp cho Scene và cloud; rain/splash/ripple nhận tint particleColor và fog thay vì normal lighting. Flash nguồn điểm ở đầu bolt dùng attenuation và Lambert/fill; sky/fogcolor cũng sáng theo flash. Bolt giữ màu trắng-xanh bằng weatherLight=(1, 1, 1). [Renderer.cpp:102](../src/Renderer.cpp#L102), [object.frag:84](../shaders/object.frag#L84), [Renderer.cpp:325](../src/Renderer.cpp#L325).

### 21. Blinn–Phong và Fresnel khác nhau thế nào?

Blinn–Phong dùng half-vector để tạo highlight theo nguồn sáng: max(N·H, 0)^shininess. Fresnel dùng góc nhìn để điều khiển tỷ lệ phản xạ; nước dùng Schlick F0.02, power5 vàfake sky color. Hai thành phần cùng có trong shader nhưng không phải phản xạ ảnh thật hoặc PBR đầy đủ. [object.frag:99](../shaders/object.frag#L99), [object.frag:138](../shaders/object.frag#L138).

### 22. Fog được tính thế nào?

Transmission=exp(−density×scale×max(distance−6, 0)); final RGB=mix(fogColor, litColor, transmission). Default density.018; scale cloud.65, rain.35, bolt.2, khác 1. F đặt density GPU 0 khi tắt. Fog này không phải volumetric scattering. [object.frag:151](../shaders/object.frag#L151), [effects.frag:17](../shaders/effects.frag#L17).

### 23. Wetness hoạt động thế nào?

Global W bắt đầu.65, tăng.06/s khi rainEnabled, giảm.012/s khi tắt và clamp[0, 1]. Shader kết hợp W với moisture field world space, đường đi/contact, độ bóng; water polygon không lớn lên theo W. `groundMultiplier`1−.24W là helper CPU test, không phải toàn bộ công thức màu đất hiện tại. [LightingSystem.cpp:46](../src/LightingSystem.cpp#L46), [object.frag:34](../shaders/object.frag#L34).

### 24. Ngày/đêm đã có thật chưa?

Có module TimeOfDaySystem:hour double[0, 24), duration 600 giây, mặc định 18:00 và auto OFF. CPU pha dawn 5–7/dusk 17–19, ambient/directional/sky/window; N chọn noon/midnight, T auto, [ / ] đổi giờ. Không dùng giờ OS hoặc mô hình thiên văn. [TimeOfDaySystem.cpp:6](../src/TimeOfDaySystem.cpp#L6), [LightingSystem.cpp:14](../src/LightingSystem.cpp#L14).

### 25. Kính cửa sổ có trong suốt và phát sáng môi trường thật không?

Glass được vẽ opaque trong Scene, fragment cộng emission màu ấm theo windowStrength. Có một warm point-light xấp xỉ fixed position gần cửa áp cho object không cloud; không mô phỏng ánh sáng từ từng cửa hoặc global illumination. [Scene.cpp:24](../src/Scene.cpp#L24), [object.frag:131](../shaders/object.frag#L131).

<a id="s04"></a>

## Particle và sét

### 26. Particle System là gì trong project?

Tập các state đơn giản được update theo thời gian rồi vẽ theo batch. Rain có 1000 Raindrop, splash 512 slot, ripple 48 slot với quy tắc lifetime/emission riêng. Không mỗi hạt một Mesh/VAO hoặc draw call. [RainSystem.hpp:8](../include/RainSystem.hpp#L8), [DynamicBatch.hpp:14](../include/DynamicBatch.hpp#L14).

### 27. Tại sao tái sử dụng hạt?

Giữ bộ nhớ/cost bị chặn, tránh allocation và tạo buffer mỗi frame. Rain respawn chính slot chạm đất/ra biên; Splash/Ripple tìm slot đã chết, đầy thì reject thay vì overwrite hạt sống. [RainSystem.cpp:43](../src/RainSystem.cpp#L43), [SplashSystem.cpp:18](../src/SplashSystem.cpp#L18), [RippleSystem.cpp:13](../src/RippleSystem.cpp#L13).

### 28. Vì sao cần deltaTime và fixed timestep?

DeltaTime đổi tốc độ từ “mỗi frame” thành “mỗi giây”. Weather còn gom dt và chạy bước 1/120s để các frame grouping khác cho quỹ đạo tương đương. Clamp.1s sau stall giữ ổn định nhưng mất thời gian thực; không nói mô phỏng luôn đồng bộ wall clock. [Application.cpp:170](../src/Application.cpp#L170), [WeatherSystem.cpp:30](../src/WeatherSystem.cpp#L30).

### 29. Euler Integration nào được dùng?

Semi-implicit Euler: update velocity trước, rồi position dùng velocity mới. Rain thêm gravity−9.81, terminal vy≥−17 và wind relaxation; Splash gravity nhưng không terminal cap. Không dùng Runge–Kutta hoặc Verlet. [RainSystem.cpp:28](../src/RainSystem.cpp#L28), [SplashSystem.cpp:38](../src/SplashSystem.cpp#L38).

### 30. Rain va chạm mái nhà chưa?

Chưa. Chỉ newY≤0 dẫn đến nội suy previous/new để tìm hit plane Y 0, lọc ground XZ±10. Không có triangle intersection hoặc colliders của nhà/cây/đá. Vì vậy có thể phát splash dưới những vật này. [RainSystem.cpp:43](../src/RainSystem.cpp#L43).

### 31. capacity, active, filtered, rejected khác nhau thế nào?

Capacity là số slot được giữ:512 splash/48 ripple. Active là slot lifetime>0. Splash requests đếm yêu cầu; filtered do sampling 45%; rejected do invalid XZ hoặc không đủ 2 slot. Ripple rejected chỉ pool full, các return do sampling/shore/position không được count. Accepted splash request tạo 2 hạt, không phải 1. [SplashSystem.cpp:18](../src/SplashSystem.cpp#L18), [RippleSystem.cpp:13](../src/RippleSystem.cpp#L13).

### 32. Ripple tăng và mờ như thế nào, làm sao không vượt bờ?

t=elapsed/.55, radius=mix(.04, maxRadius, t), alpha=.5(1−t)². WaterMargin tính khoảng cách nhỏ nhất tới cạnh polygon khi inside, maxRadius≤.9 margin; ring được nâng cao hơn nước. Mỗi vòng 12 line edges, không mô phỏng phương trình sóng. [Surface.cpp:52](../src/Surface.cpp#L52), [RippleSystem.cpp:27](../src/RippleSystem.cpp#L27).

### 33. Midpoint Displacement sinh sét thế nào?

Lấy midpoint AB, thêm random offset trong plane vuông góc AB, cap offset≤12%length và scale khi sát biên; đệ quy hai nửa, displacement giảm.4 mỗi cấp. Depth 5 cho 32 đoạn chính. Đây là polyline fractal-looking, có seed, không mô phỏng điện thế không khí. [LightningSystem.cpp:26](../src/LightningSystem.cpp#L26).

### 34. Sét phân nhánh ra sao?

Sáu nhánh bắt đầu đúng shared vertex `segments[5+4i].end` của trunk, mỗi nhánh subdivide depth 3 thành 8 đoạn, nhỏ và taper mạnh hơn. Tổng 80 đoạn trong array 128 capacity. Không phát branch đệ quy không giới hạn. [LightningSystem.cpp:55](../src/LightningSystem.cpp#L55).

### 35. Tại sao sét có nhiều xung và không phải upload lại mỗi xung?

Trigger tạo 1–3 pulse có duration 55–70%interval để có dark gap. Flash=peak exp(−32 time)×end-envelope; geometry giữ nguyên trong strike. Renderer cache revision/camera, đổi intensity uniform mà không upload lại; camera đổi thì billboard cần dựng lại. [LightningSystem.cpp:115](../src/LightningSystem.cpp#L115), [Renderer.cpp:290](../src/Renderer.cpp#L290).

### 36. Core/halo có phải bloom không?

Không. Mỗi segment tạo hai quad hướng camera, halo rộng xanh alpha thấp và core trắng hẹp, mép mềm dựa edge varying. Cùng mộtbatchGL_TRIANGLES, không blur/post process/HDR. [Renderer.cpp:294](../src/Renderer.cpp#L294), [effects.frag:13](../shaders/effects.frag#L13).

### 37. Mây có phải năm sphere chồng nhau không?

Dữ liệu có 5 ellipsoid lobe, nhưng renderer hiện dựng tổng Gaussian field và tìm skin iso.18 theo ray bằng 24 bisection. Mỗi cụm là một mesh 728 triangles, không vẽ 5 sphere 100 triangles. Một số đoạn README cũ còn mô tả cách trước. [EnvironmentGeometry.cpp:121](../src/EnvironmentGeometry.cpp#L121), [Renderer.cpp:47](../src/Renderer.cpp#L47).

<a id="s05"></a>

## Hiệu năng, kiểm thử và giới hạn

### 38. Draw call là gì, project dùng batch ở đâu?

Một lệnh glDrawArrays submit tập primitive với state hiện hành. Mưa 1000 vệt trong 1 draw; splash/ripple/bolt mỗi loại 1 batch; ba vũng gộp 1 mesh; cỏ 64 tuft cũng 1 mesh. Cảnh tĩnh 23 draw; không phải một draw cho mọi vật liệu. [Renderer.cpp:135](../src/Renderer.cpp#L135), [DynamicBatch.cpp:46](../src/DynamicBatch.cpp#L46).

### 39. FPS và Frame Time khác nhau thế nào?

FPS=frames/seconds; avg_frame_ms=seconds×1000/frames. Frame interval trong ring là khoảng giữa hai beginFrame liên tiếp; P95/P99 mô tả tail latency, không thể thay bằng 1000/FPS để suy phân bố. [Benchmark.cpp:64](../src/Benchmark.cpp#L64), [Benchmark.cpp:114](../src/Benchmark.cpp#L114).

### 40. Vì sao FPS có thể bị timer cap?

Ứng dụng dùng timer 16 ms, nhịp danh nghĩa≈62.5Hz trước overhead. Swap/VSync/compositor còn có thể hạn chế. `--benchmark-unlocked` dùng idle và yêu cầu swap interval 0, nhưng không bảo đảm throughput tối đa GPU. [Application.cpp:71](../src/Application.cpp#L71), [Benchmark.cpp:36](../src/Benchmark.cpp#L36).

### 41. Có thể suy GPU utilization từ gpu_command_ms không?

Không. Đây là GL_TIME_ELAPSED cho vùng render lệnh, không thời gian presentation hoặc phần trăm GPU bận trên cả hệ thống. Query đọc bất đồng bộ, gpu_samples có thể nhỏ hơn số frame; RSS cũng không phải VRAM. [Benchmark.cpp:64](../src/Benchmark.cpp#L64), [07 — metrics](07_TESTING_AND_PERFORMANCE.md).

### 42. Project hiện có bao nhiêu tam giác?

Scene 3264; sky 1; 4 cloud×728=2912; 80 segment bolt×4=320. Khi đều có mặt tổng 6497 triangles và tối đa 32 draw theo pipeline hiện tại; rain/splash/ripple là lines đếm riêng. Không dùng 2675 triangles/53 draw của log soak cũ để mô tả bản này. [Scene.cpp:10](../src/Scene.cpp#L10), [Renderer.cpp:84](../src/Renderer.cpp#L84), [07 — số liệu](07_TESTING_AND_PERFORMANCE.md).

### 43. Test chứng minh được gì và chưa chứng minh gì?

5 CPU executables kiểm geometry/camera/weather/models/environment/time, 17 nhóm test; phiên review chạy 5 binary sẵn có đều exit 0. Có log GUI/sanitizer 5/5 lịch sử. Chưa rebuild hoặc chạy GUI/sanitizer/benchmark mới, không chứng minh ảnh/cảm giác input hiện tại, VRAM hoặc ổn định throughput 600 giây bản mới. [07 — phạm vi](07_TESTING_AND_PERFORMANCE.md).

### 44. Space có dừng camera, hoặc tắt auto lightning có cấm L không?

Không. Camera update trước Weather pause; Space freeze weather/clock/sway. L gọi trigger trực tiếp kể cả auto off/pause; khi pause, strike và flash có thể sáng cố định cho tới resume. R reset tất cả default chứ không khôi phục CLI start options. [Application.cpp:159](../src/Application.cpp#L159), [Application.cpp:231](../src/Application.cpp#L231), [WeatherSystem.cpp:55](../src/WeatherSystem.cpp#L55).

### 45. Những giới hạn quan trọng nhất cần thừa nhận?

Collision chỉ plane Y 0, camera xuyên object; transparency sort gần đúng; không shadow/volumetric/real reflection; weather và daynight là xấp xỉ; particle line 1 pixel; cloud skin giả định star-shaped; benchmark bị môi trường presentation ảnh hưởng và chưa đo VRAM. Đây là giới hạn có thể chỉ ra trong source, không nói chung chung“chưa tối ưu”. [08 — review chi tiết](08_LIMITATIONS_AND_IMPROVEMENTS.md).

### 46. Khi thoát có giải phóng OpenGL resource không?

Có đường cleanup rõ:Application reset Benchmark/Renderer/Scene trước context đóng; Mesh/DynamicBatch delete VBO/VAO, Shader delete program, Benchmark delete queries. closecallback và destructor còn xử lý window pointer; shader constructor có cleanup nếu compile/link fail. Không gọi destructor GL sau khi context đã bị hủy trong luồng thoát bình thường. [Application.cpp:312](../src/Application.cpp#L312), [Mesh.cpp:31](../src/Mesh.cpp#L31), [Shader.cpp:41](../src/Shader.cpp#L41).

<a id="s06"></a>

## Gợi ý cấu trúc bài nói ngắn

Giới thiệu mục tiêu và pipeline; trình bày triangle mesh/ma trận/normal/depth; chọn rain fixed step và lightning midpoint làm hai thuật toán chủ đạo; demonstrate ngày đêm/wet ground/fog; đưa test và số benchmark có điều kiện; cuối cùng nêu collision/transparency/shadow limits. Khi trả lời giảng viên, mở hàm tương ứng bằng [function index](10_FUNCTION_INDEX.md) để chỉ source thay vì chỉ dựa ảnh.
