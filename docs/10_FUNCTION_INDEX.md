# 10 — Chỉ mục hàm đầy đủ
[Tổng quan](01_PROJECT_OVERVIEW.md) · [Walkthrough](02_SOURCE_CODE_WALKTHROUGH.md)
Quy tắc: 257 định nghĩa có tên (gồm 9 hàm GLSL), 23 lambda = 280 thân callable; 10 khai báo =delete được ghi riêng, tổng 290 dòng chỉ mục. Prototype lặp cùng hàm không tăng số đếm. Lambda capture `[&]` không có `()` vẫn là callable. Không đếm static array initializer nhầm thành lambda.

| STT | File | Class/Namespace | Function Signature | Dòng | Nhóm chức năng | Đã giải thích tại |
|---:|---|---|---|---:|---|---|
| 1 | [include/Application.hpp:30](../include/Application.hpp#L30) | Application | `Application(const Application&) = delete` | 30 | deleted: Cửa sổ, vòng đời, input và GUI smoke | [f001](02_SOURCE_CODE_WALKTHROUGH.md#f001) |
| 2 | [include/Application.hpp:31](../include/Application.hpp#L31) | Application | `Application& operator=(const Application&) = delete` | 31 | deleted: Cửa sổ, vòng đời, input và GUI smoke | [f002](02_SOURCE_CODE_WALKTHROUGH.md#f002) |
| 3 | [include/Benchmark.hpp:13](../include/Benchmark.hpp#L13) | Benchmark | `Benchmark(const Benchmark&) = delete` | 13 | deleted: Đo CPU/GPU/RAM và thống kê frame | [f003](02_SOURCE_CODE_WALKTHROUGH.md#f003) |
| 4 | [include/Benchmark.hpp:14](../include/Benchmark.hpp#L14) | Benchmark | `Benchmark& operator=(const Benchmark&) = delete` | 14 | deleted: Đo CPU/GPU/RAM và thống kê frame | [f004](02_SOURCE_CODE_WALKTHROUGH.md#f004) |
| 5 | [include/Camera.hpp:15](../include/Camera.hpp#L15) | Camera | `const glm::vec3& position() const` | 15 | function: Camera tự do và phép chiếu | [f005](02_SOURCE_CODE_WALKTHROUGH.md#f005) |
| 6 | [include/Camera.hpp:16](../include/Camera.hpp#L16) | Camera | `const glm::vec3& forward() const` | 16 | function: Camera tự do và phép chiếu | [f006](02_SOURCE_CODE_WALKTHROUGH.md#f006) |
| 7 | [include/Camera.hpp:17](../include/Camera.hpp#L17) | Camera | `float pitch() const` | 17 | function: Camera tự do và phép chiếu | [f007](02_SOURCE_CODE_WALKTHROUGH.md#f007) |
| 8 | [include/Camera.hpp:18](../include/Camera.hpp#L18) | Camera | `float yaw() const` | 18 | function: Camera tự do và phép chiếu | [f008](02_SOURCE_CODE_WALKTHROUGH.md#f008) |
| 9 | [include/CloudSystem.hpp:22](../include/CloudSystem.hpp#L22) | CloudSystem | `CloudSystem()` | 22 | function: Dữ liệu và chuyển động bốn cụm mây | [f009](02_SOURCE_CODE_WALKTHROUGH.md#f009) |
| 10 | [include/CloudSystem.hpp:25](../include/CloudSystem.hpp#L25) | CloudSystem | `const std::array<CloudCluster, COUNT>& clusters() const` | 25 | function: Dữ liệu và chuyển động bốn cụm mây | [f010](02_SOURCE_CODE_WALKTHROUGH.md#f010) |
| 11 | [include/DynamicBatch.hpp:19](../include/DynamicBatch.hpp#L19) | DynamicBatch | `DynamicBatch(const DynamicBatch&) = delete` | 19 | deleted: VAO/VBO động cho các hiệu ứng | [f011](02_SOURCE_CODE_WALKTHROUGH.md#f011) |
| 12 | [include/DynamicBatch.hpp:20](../include/DynamicBatch.hpp#L20) | DynamicBatch | `DynamicBatch& operator=(const DynamicBatch&) = delete` | 20 | deleted: VAO/VBO động cho các hiệu ứng | [f012](02_SOURCE_CODE_WALKTHROUGH.md#f012) |
| 13 | [include/DynamicBatch.hpp:23](../include/DynamicBatch.hpp#L23) | DynamicBatch | `std::size_t vertexCount() const` | 23 | function: VAO/VBO động cho các hiệu ứng | [f013](02_SOURCE_CODE_WALKTHROUGH.md#f013) |
| 14 | [include/DynamicBatch.hpp:24](../include/DynamicBatch.hpp#L24) | DynamicBatch | `std::uint64_t uploadCount() const` | 24 | function: VAO/VBO động cho các hiệu ứng | [f014](02_SOURCE_CODE_WALKTHROUGH.md#f014) |
| 15 | [include/LightingSystem.hpp:13](../include/LightingSystem.hpp#L13) | LightingSystem | `LightingSystem()` | 13 | function: Ánh sáng màu, flash, fog và wetness | [f015](02_SOURCE_CODE_WALKTHROUGH.md#f015) |
| 16 | [include/LightingSystem.hpp:17](../include/LightingSystem.hpp#L17) | LightingSystem | `glm::vec3 ambientColor() const` | 17 | function: Ánh sáng màu, flash, fog và wetness | [f016](02_SOURCE_CODE_WALKTHROUGH.md#f016) |
| 17 | [include/LightingSystem.hpp:18](../include/LightingSystem.hpp#L18) | LightingSystem | `glm::vec3 directionalColor() const` | 18 | function: Ánh sáng màu, flash, fog và wetness | [f017](02_SOURCE_CODE_WALKTHROUGH.md#f017) |
| 18 | [include/LightingSystem.hpp:19](../include/LightingSystem.hpp#L19) | LightingSystem | `glm::vec3 lightDirection() const` | 19 | function: Ánh sáng màu, flash, fog và wetness | [f018](02_SOURCE_CODE_WALKTHROUGH.md#f018) |
| 19 | [include/LightingSystem.hpp:21](../include/LightingSystem.hpp#L21) | LightingSystem | `float windowStrength() const` | 21 | function: Ánh sáng màu, flash, fog và wetness | [f019](02_SOURCE_CODE_WALKTHROUGH.md#f019) |
| 20 | [include/LightingSystem.hpp:22](../include/LightingSystem.hpp#L22) | LightingSystem | `float daylight() const` | 22 | function: Ánh sáng màu, flash, fog và wetness | [f020](02_SOURCE_CODE_WALKTHROUGH.md#f020) |
| 21 | [include/LightingSystem.hpp:24](../include/LightingSystem.hpp#L24) | LightingSystem | `float flash() const` | 24 | function: Ánh sáng màu, flash, fog và wetness | [f021](02_SOURCE_CODE_WALKTHROUGH.md#f021) |
| 22 | [include/LightingSystem.hpp:25](../include/LightingSystem.hpp#L25) | LightingSystem | `void setFlashPosition(glm::vec3 position)` | 25 | function: Ánh sáng màu, flash, fog và wetness | [f022](02_SOURCE_CODE_WALKTHROUGH.md#f022) |
| 23 | [include/LightingSystem.hpp:26](../include/LightingSystem.hpp#L26) | LightingSystem | `glm::vec3 flashPosition() const` | 26 | function: Ánh sáng màu, flash, fog và wetness | [f023](02_SOURCE_CODE_WALKTHROUGH.md#f023) |
| 24 | [include/LightingSystem.hpp:33](../include/LightingSystem.hpp#L33) | LightingSystem | `float wetness() const` | 33 | function: Ánh sáng màu, flash, fog và wetness | [f024](02_SOURCE_CODE_WALKTHROUGH.md#f024) |
| 25 | [include/LightingSystem.hpp:34](../include/LightingSystem.hpp#L34) | LightingSystem | `float groundMultiplier() const` | 34 | function: Ánh sáng màu, flash, fog và wetness | [f025](02_SOURCE_CODE_WALKTHROUGH.md#f025) |
| 26 | [include/LightningSystem.hpp:27](../include/LightningSystem.hpp#L27) | LightningSystem | `LightningSystem()` | 27 | function: Polyline sét đệ quy, nhánh và xung | [f026](02_SOURCE_CODE_WALKTHROUGH.md#f026) |
| 27 | [include/LightningSystem.hpp:34](../include/LightningSystem.hpp#L34) | LightningSystem | `bool active() const` | 34 | function: Polyline sét đệ quy, nhánh và xung | [f027](02_SOURCE_CODE_WALKTHROUGH.md#f027) |
| 28 | [include/LightningSystem.hpp:35](../include/LightningSystem.hpp#L35) | LightningSystem | `float duration() const` | 35 | function: Polyline sét đệ quy, nhánh và xung | [f028](02_SOURCE_CODE_WALKTHROUGH.md#f028) |
| 29 | [include/LightningSystem.hpp:36](../include/LightningSystem.hpp#L36) | LightningSystem | `float elapsed() const` | 36 | function: Polyline sét đệ quy, nhánh và xung | [f029](02_SOURCE_CODE_WALKTHROUGH.md#f029) |
| 30 | [include/LightningSystem.hpp:37](../include/LightningSystem.hpp#L37) | LightningSystem | `float nextStrike() const` | 37 | function: Polyline sét đệ quy, nhánh và xung | [f030](02_SOURCE_CODE_WALKTHROUGH.md#f030) |
| 31 | [include/LightningSystem.hpp:38](../include/LightningSystem.hpp#L38) | LightningSystem | `std::uint64_t strikeCount() const` | 38 | function: Polyline sét đệ quy, nhánh và xung | [f031](02_SOURCE_CODE_WALKTHROUGH.md#f031) |
| 32 | [include/LightningSystem.hpp:39](../include/LightningSystem.hpp#L39) | LightningSystem | `std::uint64_t revision() const` | 39 | function: Polyline sét đệ quy, nhánh và xung | [f032](02_SOURCE_CODE_WALKTHROUGH.md#f032) |
| 33 | [include/LightningSystem.hpp:40](../include/LightningSystem.hpp#L40) | LightningSystem | `unsigned pulseCount() const` | 40 | function: Polyline sét đệ quy, nhánh và xung | [f033](02_SOURCE_CODE_WALKTHROUGH.md#f033) |
| 34 | [include/LightningSystem.hpp:41](../include/LightningSystem.hpp#L41) | LightningSystem | `const std::array<LightningPulse, 3>& pulses() const` | 41 | function: Polyline sét đệ quy, nhánh và xung | [f034](02_SOURCE_CODE_WALKTHROUGH.md#f034) |
| 35 | [include/LightningSystem.hpp:42](../include/LightningSystem.hpp#L42) | LightningSystem | `glm::vec3 lightPosition() const` | 42 | function: Polyline sét đệ quy, nhánh và xung | [f035](02_SOURCE_CODE_WALKTHROUGH.md#f035) |
| 36 | [include/LightningSystem.hpp:45](../include/LightningSystem.hpp#L45) | LightningSystem | `const std::array<LightningSegment, MAX_SEGMENTS>& segments() const` | 45 | function: Polyline sét đệ quy, nhánh và xung | [f036](02_SOURCE_CODE_WALKTHROUGH.md#f036) |
| 37 | [include/LightningSystem.hpp:46](../include/LightningSystem.hpp#L46) | LightningSystem | `std::size_t segmentCount() const` | 46 | function: Polyline sét đệ quy, nhánh và xung | [f037](02_SOURCE_CODE_WALKTHROUGH.md#f037) |
| 38 | [include/Mesh.hpp:10](../include/Mesh.hpp#L10) | Mesh | `Mesh(const Mesh&) = delete` | 10 | deleted: Tài nguyên GPU của mesh tĩnh | [f038](02_SOURCE_CODE_WALKTHROUGH.md#f038) |
| 39 | [include/Mesh.hpp:11](../include/Mesh.hpp#L11) | Mesh | `Mesh& operator=(const Mesh&) = delete` | 11 | deleted: Tài nguyên GPU của mesh tĩnh | [f039](02_SOURCE_CODE_WALKTHROUGH.md#f039) |
| 40 | [include/Mesh.hpp:13](../include/Mesh.hpp#L13) | Mesh | `std::size_t triangleCount() const` | 13 | function: Tài nguyên GPU của mesh tĩnh | [f040](02_SOURCE_CODE_WALKTHROUGH.md#f040) |
| 41 | [include/RainSystem.hpp:20](../include/RainSystem.hpp#L20) | RainSystem | `RainSystem()` | 20 | function: Pool 1000 giọt và ground impacts | [f041](02_SOURCE_CODE_WALKTHROUGH.md#f041) |
| 42 | [include/RainSystem.hpp:23](../include/RainSystem.hpp#L23) | RainSystem | `const std::array<Raindrop, RAIN_COUNT>& drops() const` | 23 | function: Pool 1000 giọt và ground impacts | [f042](02_SOURCE_CODE_WALKTHROUGH.md#f042) |
| 43 | [include/RainSystem.hpp:24](../include/RainSystem.hpp#L24) | RainSystem | `const std::array<glm::vec3, RAIN_COUNT>& impacts() const` | 24 | function: Pool 1000 giọt và ground impacts | [f043](02_SOURCE_CODE_WALKTHROUGH.md#f043) |
| 44 | [include/RainSystem.hpp:25](../include/RainSystem.hpp#L25) | RainSystem | `std::size_t impactCount() const` | 25 | function: Pool 1000 giọt và ground impacts | [f044](02_SOURCE_CODE_WALKTHROUGH.md#f044) |
| 45 | [include/RainSystem.hpp:26](../include/RainSystem.hpp#L26) | RainSystem | `std::uint64_t recycledCount() const` | 26 | function: Pool 1000 giọt và ground impacts | [f045](02_SOURCE_CODE_WALKTHROUGH.md#f045) |
| 46 | [include/RainSystem.hpp:27](../include/RainSystem.hpp#L27) | RainSystem | `std::uint64_t revision() const` | 27 | function: Pool 1000 giọt và ground impacts | [f046](02_SOURCE_CODE_WALKTHROUGH.md#f046) |
| 47 | [include/Renderer.hpp:14](../include/Renderer.hpp#L14) | Renderer | `std::size_t drawCalls() const` | 14 | function: Render passes, uniforms và dynamic staging | [f047](02_SOURCE_CODE_WALKTHROUGH.md#f047) |
| 48 | [include/Renderer.hpp:15](../include/Renderer.hpp#L15) | Renderer | `std::size_t renderedRainCount() const` | 15 | function: Render passes, uniforms và dynamic staging | [f048](02_SOURCE_CODE_WALKTHROUGH.md#f048) |
| 49 | [include/Renderer.hpp:16](../include/Renderer.hpp#L16) | Renderer | `std::size_t renderedSplashCount() const` | 16 | function: Render passes, uniforms và dynamic staging | [f049](02_SOURCE_CODE_WALKTHROUGH.md#f049) |
| 50 | [include/Renderer.hpp:17](../include/Renderer.hpp#L17) | Renderer | `std::uint64_t lightningUploadCount() const` | 17 | function: Render passes, uniforms và dynamic staging | [f050](02_SOURCE_CODE_WALKTHROUGH.md#f050) |
| 51 | [include/Renderer.hpp:18](../include/Renderer.hpp#L18) | Renderer | `std::uint64_t rainUploadCount() const` | 18 | function: Render passes, uniforms và dynamic staging | [f051](02_SOURCE_CODE_WALKTHROUGH.md#f051) |
| 52 | [include/Renderer.hpp:19](../include/Renderer.hpp#L19) | Renderer | `std::size_t triangles() const` | 19 | function: Render passes, uniforms và dynamic staging | [f052](02_SOURCE_CODE_WALKTHROUGH.md#f052) |
| 53 | [include/Renderer.hpp:20](../include/Renderer.hpp#L20) | Renderer | `std::size_t lines() const` | 20 | function: Render passes, uniforms và dynamic staging | [f053](02_SOURCE_CODE_WALKTHROUGH.md#f053) |
| 54 | [include/RippleSystem.hpp:19](../include/RippleSystem.hpp#L19) | RippleSystem | `RippleSystem()` | 19 | function: Pool 48 vòng gợn | [f054](02_SOURCE_CODE_WALKTHROUGH.md#f054) |
| 55 | [include/RippleSystem.hpp:23](../include/RippleSystem.hpp#L23) | RippleSystem | `const std::array<Ripple, CAPACITY>& particles() const` | 23 | function: Pool 48 vòng gợn | [f055](02_SOURCE_CODE_WALKTHROUGH.md#f055) |
| 56 | [include/RippleSystem.hpp:24](../include/RippleSystem.hpp#L24) | RippleSystem | `std::size_t activeCount() const` | 24 | function: Pool 48 vòng gợn | [f056](02_SOURCE_CODE_WALKTHROUGH.md#f056) |
| 57 | [include/RippleSystem.hpp:25](../include/RippleSystem.hpp#L25) | RippleSystem | `std::uint64_t revision() const` | 25 | function: Pool 48 vòng gợn | [f057](02_SOURCE_CODE_WALKTHROUGH.md#f057) |
| 58 | [include/RippleSystem.hpp:26](../include/RippleSystem.hpp#L26) | RippleSystem | `std::uint64_t rejected() const` | 26 | function: Pool 48 vòng gợn | [f058](02_SOURCE_CODE_WALKTHROUGH.md#f058) |
| 59 | [include/Scene.hpp:22](../include/Scene.hpp#L22) | Scene | `const std::vector<SceneObject>& objects() const` | 22 | function: Sở hữu mesh và danh sách vật thể | [f059](02_SOURCE_CODE_WALKTHROUGH.md#f059) |
| 60 | [include/Scene.hpp:25](../include/Scene.hpp#L25) | Scene | `void resetAnimation()` | 25 | function: Sở hữu mesh và danh sách vật thể | [f060](02_SOURCE_CODE_WALKTHROUGH.md#f060) |
| 61 | [include/Scene.hpp:26](../include/Scene.hpp#L26) | Scene | `float animationTime() const` | 26 | function: Sở hữu mesh và danh sách vật thể | [f061](02_SOURCE_CODE_WALKTHROUGH.md#f061) |
| 62 | [include/Shader.hpp:10](../include/Shader.hpp#L10) | Shader | `Shader(const Shader&) = delete` | 10 | deleted: Đọc/compile/link GLSL | [f062](02_SOURCE_CODE_WALKTHROUGH.md#f062) |
| 63 | [include/Shader.hpp:11](../include/Shader.hpp#L11) | Shader | `Shader& operator=(const Shader&) = delete` | 11 | deleted: Đọc/compile/link GLSL | [f063](02_SOURCE_CODE_WALKTHROUGH.md#f063) |
| 64 | [include/Shader.hpp:12](../include/Shader.hpp#L12) | Shader | `void use() const` | 12 | function: Đọc/compile/link GLSL | [f064](02_SOURCE_CODE_WALKTHROUGH.md#f064) |
| 65 | [include/SplashSystem.hpp:19](../include/SplashSystem.hpp#L19) | SplashSystem | `SplashSystem()` | 19 | function: Pool 512 hạt bắn | [f065](02_SOURCE_CODE_WALKTHROUGH.md#f065) |
| 66 | [include/SplashSystem.hpp:23](../include/SplashSystem.hpp#L23) | SplashSystem | `std::size_t activeCount() const` | 23 | function: Pool 512 hạt bắn | [f066](02_SOURCE_CODE_WALKTHROUGH.md#f066) |
| 67 | [include/SplashSystem.hpp:24](../include/SplashSystem.hpp#L24) | SplashSystem | `const std::array<SplashParticle, MAX_SPLASHES>& particles() const` | 24 | function: Pool 512 hạt bắn | [f067](02_SOURCE_CODE_WALKTHROUGH.md#f067) |
| 68 | [include/SplashSystem.hpp:25](../include/SplashSystem.hpp#L25) | SplashSystem | `std::uint64_t revision() const` | 25 | function: Pool 512 hạt bắn | [f068](02_SOURCE_CODE_WALKTHROUGH.md#f068) |
| 69 | [include/SplashSystem.hpp:26](../include/SplashSystem.hpp#L26) | SplashSystem | `std::uint64_t requests() const` | 26 | function: Pool 512 hạt bắn | [f069](02_SOURCE_CODE_WALKTHROUGH.md#f069) |
| 70 | [include/SplashSystem.hpp:27](../include/SplashSystem.hpp#L27) | SplashSystem | `std::uint64_t rejected() const` | 27 | function: Pool 512 hạt bắn | [f070](02_SOURCE_CODE_WALKTHROUGH.md#f070) |
| 71 | [include/SplashSystem.hpp:28](../include/SplashSystem.hpp#L28) | SplashSystem | `std::uint64_t filtered() const` | 28 | function: Pool 512 hạt bắn | [f071](02_SOURCE_CODE_WALKTHROUGH.md#f071) |
| 72 | [include/TimeOfDaySystem.hpp:11](../include/TimeOfDaySystem.hpp#L11) | TimeOfDaySystem | `void shiftHours(double hours)` | 11 | function: Đồng hồ giờ mô phỏng | [f072](02_SOURCE_CODE_WALKTHROUGH.md#f072) |
| 73 | [include/TimeOfDaySystem.hpp:12](../include/TimeOfDaySystem.hpp#L12) | TimeOfDaySystem | `void toggleDayNight()` | 12 | function: Đồng hồ giờ mô phỏng | [f073](02_SOURCE_CODE_WALKTHROUGH.md#f073) |
| 74 | [include/TimeOfDaySystem.hpp:13](../include/TimeOfDaySystem.hpp#L13) | TimeOfDaySystem | `void setAutoAdvance(bool enabled)` | 13 | function: Đồng hồ giờ mô phỏng | [f074](02_SOURCE_CODE_WALKTHROUGH.md#f074) |
| 75 | [include/TimeOfDaySystem.hpp:15](../include/TimeOfDaySystem.hpp#L15) | TimeOfDaySystem | `double hour() const` | 15 | function: Đồng hồ giờ mô phỏng | [f075](02_SOURCE_CODE_WALKTHROUGH.md#f075) |
| 76 | [include/TimeOfDaySystem.hpp:16](../include/TimeOfDaySystem.hpp#L16) | TimeOfDaySystem | `double dayDuration() const` | 16 | function: Đồng hồ giờ mô phỏng | [f076](02_SOURCE_CODE_WALKTHROUGH.md#f076) |
| 77 | [include/TimeOfDaySystem.hpp:17](../include/TimeOfDaySystem.hpp#L17) | TimeOfDaySystem | `bool autoAdvance() const` | 17 | function: Đồng hồ giờ mô phỏng | [f077](02_SOURCE_CODE_WALKTHROUGH.md#f077) |
| 78 | [shaders/effects.frag:13](../shaders/effects.frag#L13) | GLSL global — effects.frag | `void main()` | 13 | function: GLSL | [f078](02_SOURCE_CODE_WALKTHROUGH.md#f078) |
| 79 | [shaders/effects.vert:12](../shaders/effects.vert#L12) | GLSL global — effects.vert | `void main()` | 12 | function: GLSL | [f079](02_SOURCE_CODE_WALKTHROUGH.md#f079) |
| 80 | [shaders/object.frag:34](../shaders/object.frag#L34) | GLSL global — object.frag | `float moistureField(vec2 p)` | 34 | function: GLSL | [f080](02_SOURCE_CODE_WALKTHROUGH.md#f080) |
| 81 | [shaders/object.frag:39](../shaders/object.frag#L39) | GLSL global — object.frag | `vec3 soilColor(float field, float wet)` | 39 | function: GLSL | [f081](02_SOURCE_CODE_WALKTHROUGH.md#f081) |
| 82 | [shaders/object.frag:50](../shaders/object.frag#L50) | GLSL global — object.frag | `float waterInterior(vec2 world)` | 50 | function: GLSL | [f082](02_SOURCE_CODE_WALKTHROUGH.md#f082) |
| 83 | [shaders/object.frag:65](../shaders/object.frag#L65) | GLSL global — object.frag | `void main()` | 65 | function: GLSL | [f083](02_SOURCE_CODE_WALKTHROUGH.md#f083) |
| 84 | [shaders/object.vert:18](../shaders/object.vert#L18) | GLSL global — object.vert | `void main()` | 18 | function: GLSL | [f084](02_SOURCE_CODE_WALKTHROUGH.md#f084) |
| 85 | [shaders/sky.frag:6](../shaders/sky.frag#L6) | GLSL global — sky.frag | `void main()` | 6 | function: GLSL | [f085](02_SOURCE_CODE_WALKTHROUGH.md#f085) |
| 86 | [shaders/sky.vert:4](../shaders/sky.vert#L4) | GLSL global — sky.vert | `void main()` | 4 | function: GLSL | [f086](02_SOURCE_CODE_WALKTHROUGH.md#f086) |
| 87 | [src/Application.cpp:17](../src/Application.cpp#L17) | namespace ẩn | `void checkOpenGL(const char* where)` | 17 | function: Cửa sổ, vòng đời, input và GUI smoke | [f087](02_SOURCE_CODE_WALKTHROUGH.md#f087) |
| 88 | [src/Application.cpp:27](../src/Application.cpp#L27) | namespace ẩn | `void require(bool condition, const char* message)` | 27 | function: Cửa sổ, vòng đời, input và GUI smoke | [f088](02_SOURCE_CODE_WALKTHROUGH.md#f088) |
| 89 | [src/Application.cpp:31](../src/Application.cpp#L31) | namespace ẩn | `std::vector<unsigned char> readColor(int width, int height)` | 31 | function: Cửa sổ, vòng đời, input và GUI smoke | [f089](02_SOURCE_CODE_WALKTHROUGH.md#f089) |
| 90 | [src/Application.cpp:39](../src/Application.cpp#L39) | namespace ẩn | `std::size_t changedPixels(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b)` | 39 | function: Cửa sổ, vòng đời, input và GUI smoke | [f090](02_SOURCE_CODE_WALKTHROUGH.md#f090) |
| 91 | [src/Application.cpp:54](../src/Application.cpp#L54) | Application | `Application::Application(RunOptions options)` | 54 | function: Cửa sổ, vòng đời, input và GUI smoke | [f091](02_SOURCE_CODE_WALKTHROUGH.md#f091) |
| 92 | [src/Application.cpp:56](../src/Application.cpp#L56) | Application | `Application::~Application()` | 56 | function: Cửa sổ, vòng đời, input và GUI smoke | [f092](02_SOURCE_CODE_WALKTHROUGH.md#f092) |
| 93 | [src/Application.cpp:71](../src/Application.cpp#L71) | Application | `int Application::run(char* executableName)` | 71 | function: Cửa sổ, vòng đời, input và GUI smoke | [f093](02_SOURCE_CODE_WALKTHROUGH.md#f093) |
| 94 | [src/Application.cpp:91](../src/Application.cpp#L91) | Application | `void Application::initialize(char* executableName)` | 91 | function: Cửa sổ, vòng đời, input và GUI smoke | [f094](02_SOURCE_CODE_WALKTHROUGH.md#f094) |
| 95 | [src/Application.cpp:159](../src/Application.cpp#L159) | Application | `void Application::update(float deltaTime)` | 159 | function: Cửa sổ, vòng đời, input và GUI smoke | [f095](02_SOURCE_CODE_WALKTHROUGH.md#f095) |
| 96 | [src/Application.cpp:170](../src/Application.cpp#L170) | Application | `void Application::tick()` | 170 | function: Cửa sổ, vòng đời, input và GUI smoke | [f096](02_SOURCE_CODE_WALKTHROUGH.md#f096) |
| 97 | [src/Application.cpp:206](../src/Application.cpp#L206) | Application | `void Application::display()` | 206 | function: Cửa sổ, vòng đời, input và GUI smoke | [f097](02_SOURCE_CODE_WALKTHROUGH.md#f097) |
| 98 | [src/Application.cpp:225](../src/Application.cpp#L225) | Application | `void Application::reshape(int width, int height)` | 225 | function: Cửa sổ, vòng đời, input và GUI smoke | [f098](02_SOURCE_CODE_WALKTHROUGH.md#f098) |
| 99 | [src/Application.cpp:231](../src/Application.cpp#L231) | Application | `void Application::key(unsigned char value, bool pressed)` | 231 | function: Cửa sổ, vòng đời, input và GUI smoke | [f099](02_SOURCE_CODE_WALKTHROUGH.md#f099) |
| 100 | [src/Application.cpp:264](../src/Application.cpp#L264) | Application | `void Application::printTimeHelp() const` | 264 | function: Cửa sổ, vòng đời, input và GUI smoke | [f100](02_SOURCE_CODE_WALKTHROUGH.md#f100) |
| 101 | [src/Application.cpp:270](../src/Application.cpp#L270) | Application | `void Application::updateStatistics(Clock::time_point now)` | 270 | function: Cửa sổ, vòng đời, input và GUI smoke | [f101](02_SOURCE_CODE_WALKTHROUGH.md#f101) |
| 102 | [src/Application.cpp:291](../src/Application.cpp#L291) | Application | `void Application::mouseButton(int button, int state, int x, int y)` | 291 | function: Cửa sổ, vòng đời, input và GUI smoke | [f102](02_SOURCE_CODE_WALKTHROUGH.md#f102) |
| 103 | [src/Application.cpp:299](../src/Application.cpp#L299) | Application | `void Application::mouseMotion(int x, int y)` | 299 | function: Cửa sổ, vòng đời, input và GUI smoke | [f103](02_SOURCE_CODE_WALKTHROUGH.md#f103) |
| 104 | [src/Application.cpp:307](../src/Application.cpp#L307) | Application | `void Application::releaseInput()` | 307 | function: Cửa sổ, vòng đời, input và GUI smoke | [f104](02_SOURCE_CODE_WALKTHROUGH.md#f104) |
| 105 | [src/Application.cpp:312](../src/Application.cpp#L312) | Application | `void Application::close()` | 312 | function: Cửa sổ, vòng đời, input và GUI smoke | [f105](02_SOURCE_CODE_WALKTHROUGH.md#f105) |
| 106 | [src/Application.cpp:327](../src/Application.cpp#L327) | Application | `void Application::requestExit()` | 327 | function: Cửa sổ, vòng đời, input và GUI smoke | [f106](02_SOURCE_CODE_WALKTHROUGH.md#f106) |
| 107 | [src/Application.cpp:339](../src/Application.cpp#L339) | Application | `void Application::fail(const char* message)` | 339 | function: Cửa sổ, vòng đời, input và GUI smoke | [f107](02_SOURCE_CODE_WALKTHROUGH.md#f107) |
| 108 | [src/Application.cpp:352](../src/Application.cpp#L352) | Application | `void Application::inspectFramebuffer(const std::string& capturePath) const` | 352 | function: Cửa sổ, vòng đời, input và GUI smoke | [f108](02_SOURCE_CODE_WALKTHROUGH.md#f108) |
| 109 | [src/Application.cpp:394](../src/Application.cpp#L394) | Application | `void Application::smokeStep()` | 394 | function: Cửa sổ, vòng đời, input và GUI smoke | [f109](02_SOURCE_CODE_WALKTHROUGH.md#f109) |
| 110 | [src/Application.cpp:406](../src/Application.cpp#L406) | Application::smokeStep | `[&]` | 406 | lambda: Cửa sổ, vòng đời, input và GUI smoke | [f110](02_SOURCE_CODE_WALKTHROUGH.md#f110) |
| 111 | [src/Application.cpp:545](../src/Application.cpp#L545) | Application::smokeStep | `[&](const std::vector<unsigned char>& pixels,glm::vec3 world)` | 545 | lambda: Cửa sổ, vòng đời, input và GUI smoke | [f111](02_SOURCE_CODE_WALKTHROUGH.md#f111) |
| 112 | [src/Application.cpp:607](../src/Application.cpp#L607) | Application::smokeStep | `[&](const char* name)` | 607 | lambda: Cửa sổ, vòng đời, input và GUI smoke | [f112](02_SOURCE_CODE_WALKTHROUGH.md#f112) |
| 113 | [src/Application.cpp:705](../src/Application.cpp#L705) | Application | `void Application::displayCallback()` | 705 | function: Cửa sổ, vòng đời, input và GUI smoke | [f113](02_SOURCE_CODE_WALKTHROUGH.md#f113) |
| 114 | [src/Application.cpp:709](../src/Application.cpp#L709) | Application | `void Application::timerCallback(int)` | 709 | function: Cửa sổ, vòng đời, input và GUI smoke | [f114](02_SOURCE_CODE_WALKTHROUGH.md#f114) |
| 115 | [src/Application.cpp:713](../src/Application.cpp#L713) | Application | `void Application::idleCallback()` | 713 | function: Cửa sổ, vòng đời, input và GUI smoke | [f115](02_SOURCE_CODE_WALKTHROUGH.md#f115) |
| 116 | [src/Application.cpp:714](../src/Application.cpp#L714) | Application | `void Application::reshapeCallback(int w, int h)` | 714 | function: Cửa sổ, vòng đời, input và GUI smoke | [f116](02_SOURCE_CODE_WALKTHROUGH.md#f116) |
| 117 | [src/Application.cpp:715](../src/Application.cpp#L715) | Application | `void Application::keyDownCallback(unsigned char k, int, int)` | 715 | function: Cửa sổ, vòng đời, input và GUI smoke | [f117](02_SOURCE_CODE_WALKTHROUGH.md#f117) |
| 118 | [src/Application.cpp:719](../src/Application.cpp#L719) | Application | `void Application::keyUpCallback(unsigned char k, int, int)` | 719 | function: Cửa sổ, vòng đời, input và GUI smoke | [f118](02_SOURCE_CODE_WALKTHROUGH.md#f118) |
| 119 | [src/Application.cpp:720](../src/Application.cpp#L720) | Application | `void Application::mouseButtonCallback(int b, int s, int x, int y)` | 720 | function: Cửa sổ, vòng đời, input và GUI smoke | [f119](02_SOURCE_CODE_WALKTHROUGH.md#f119) |
| 120 | [src/Application.cpp:721](../src/Application.cpp#L721) | Application | `void Application::mouseMotionCallback(int x, int y)` | 721 | function: Cửa sổ, vòng đời, input và GUI smoke | [f120](02_SOURCE_CODE_WALKTHROUGH.md#f120) |
| 121 | [src/Application.cpp:722](../src/Application.cpp#L722) | Application | `void Application::entryCallback(int state)` | 722 | function: Cửa sổ, vòng đời, input và GUI smoke | [f121](02_SOURCE_CODE_WALKTHROUGH.md#f121) |
| 122 | [src/Application.cpp:723](../src/Application.cpp#L723) | Application | `void Application::visibilityCallback(int state)` | 723 | function: Cửa sổ, vòng đời, input và GUI smoke | [f122](02_SOURCE_CODE_WALKTHROUGH.md#f122) |
| 123 | [src/Application.cpp:727](../src/Application.cpp#L727) | Application | `void Application::closeCallback()` | 727 | function: Cửa sổ, vòng đời, input và GUI smoke | [f123](02_SOURCE_CODE_WALKTHROUGH.md#f123) |
| 124 | [src/Benchmark.cpp:11](../src/Benchmark.cpp#L11) | namespace ẩn | `double cpuSeconds()` | 11 | function: Đo CPU/GPU/RAM và thống kê frame | [f124](02_SOURCE_CODE_WALKTHROUGH.md#f124) |
| 125 | [src/Benchmark.cpp:17](../src/Benchmark.cpp#L17) | namespace ẩn | `std::size_t residentBytes()` | 17 | function: Đo CPU/GPU/RAM và thống kê frame | [f125](02_SOURCE_CODE_WALKTHROUGH.md#f125) |
| 126 | [src/Benchmark.cpp:23](../src/Benchmark.cpp#L23) | namespace ẩn | `std::size_t highWaterBytes()` | 23 | function: Đo CPU/GPU/RAM và thống kê frame | [f126](02_SOURCE_CODE_WALKTHROUGH.md#f126) |
| 127 | [src/Benchmark.cpp:36](../src/Benchmark.cpp#L36) | Benchmark | `Benchmark::Benchmark(bool unlocked)` | 36 | function: Đo CPU/GPU/RAM và thống kê frame | [f127](02_SOURCE_CODE_WALKTHROUGH.md#f127) |
| 128 | [src/Benchmark.cpp:59](../src/Benchmark.cpp#L59) | Benchmark | `Benchmark::~Benchmark()` | 59 | function: Đo CPU/GPU/RAM và thống kê frame | [f128](02_SOURCE_CODE_WALKTHROUGH.md#f128) |
| 129 | [src/Benchmark.cpp:64](../src/Benchmark.cpp#L64) | Benchmark | `void Benchmark::beginFrame()` | 64 | function: Đo CPU/GPU/RAM và thống kê frame | [f129](02_SOURCE_CODE_WALKTHROUGH.md#f129) |
| 130 | [src/Benchmark.cpp:89](../src/Benchmark.cpp#L89) | Benchmark | `void Benchmark::endFrame()` | 89 | function: Đo CPU/GPU/RAM và thống kê frame | [f130](02_SOURCE_CODE_WALKTHROUGH.md#f130) |
| 131 | [src/Benchmark.cpp:99](../src/Benchmark.cpp#L99) | Benchmark | `void Benchmark::sample(const WeatherSystem& weather, double seconds, double fps)` | 99 | function: Đo CPU/GPU/RAM và thống kê frame | [f131](02_SOURCE_CODE_WALKTHROUGH.md#f131) |
| 132 | [src/Benchmark.cpp:114](../src/Benchmark.cpp#L114) | Benchmark | `void Benchmark::report(std::ostream& output, double seconds, int frames) const` | 114 | function: Đo CPU/GPU/RAM và thống kê frame | [f132](02_SOURCE_CODE_WALKTHROUGH.md#f132) |
| 133 | [src/Benchmark.cpp:118](../src/Benchmark.cpp#L118) | Benchmark::report | `[&](double p)` | 118 | lambda: Đo CPU/GPU/RAM và thống kê frame | [f133](02_SOURCE_CODE_WALKTHROUGH.md#f133) |
| 134 | [src/Camera.cpp:7](../src/Camera.cpp#L7) | Camera | `Camera::Camera()` | 7 | function: Camera tự do và phép chiếu | [f134](02_SOURCE_CODE_WALKTHROUGH.md#f134) |
| 135 | [src/Camera.cpp:9](../src/Camera.cpp#L9) | Camera | `void Camera::reset()` | 9 | function: Camera tự do và phép chiếu | [f135](02_SOURCE_CODE_WALKTHROUGH.md#f135) |
| 136 | [src/Camera.cpp:16](../src/Camera.cpp#L16) | Camera | `void Camera::preset(unsigned index)` | 16 | function: Camera tự do và phép chiếu | [f136](02_SOURCE_CODE_WALKTHROUGH.md#f136) |
| 137 | [src/Camera.cpp:27](../src/Camera.cpp#L27) | Camera | `void Camera::updateDirection()` | 27 | function: Camera tự do và phép chiếu | [f137](02_SOURCE_CODE_WALKTHROUGH.md#f137) |
| 138 | [src/Camera.cpp:35](../src/Camera.cpp#L35) | Camera | `void Camera::rotate(float deltaX, float deltaY)` | 35 | function: Camera tự do và phép chiếu | [f138](02_SOURCE_CODE_WALKTHROUGH.md#f138) |
| 139 | [src/Camera.cpp:41](../src/Camera.cpp#L41) | Camera | `void Camera::move(const glm::vec3& localDirection, float deltaTime)` | 41 | function: Camera tự do và phép chiếu | [f139](02_SOURCE_CODE_WALKTHROUGH.md#f139) |
| 140 | [src/Camera.cpp:55](../src/Camera.cpp#L55) | Camera | `glm::mat4 Camera::view() const` | 55 | function: Camera tự do và phép chiếu | [f140](02_SOURCE_CODE_WALKTHROUGH.md#f140) |
| 141 | [src/Camera.cpp:59](../src/Camera.cpp#L59) | Camera | `glm::mat4 Camera::projection(float aspect) const` | 59 | function: Camera tự do và phép chiếu | [f141](02_SOURCE_CODE_WALKTHROUGH.md#f141) |
| 142 | [src/CloudSystem.cpp:7](../src/CloudSystem.cpp#L7) | namespace ẩn | `float wrap(float x)` | 7 | function: Dữ liệu và chuyển động bốn cụm mây | [f142](02_SOURCE_CODE_WALKTHROUGH.md#f142) |
| 143 | [src/CloudSystem.cpp:12](../src/CloudSystem.cpp#L12) | CloudSystem | `void CloudSystem::reset()` | 12 | function: Dữ liệu và chuyển động bốn cụm mây | [f143](02_SOURCE_CODE_WALKTHROUGH.md#f143) |
| 144 | [src/CloudSystem.cpp:34](../src/CloudSystem.cpp#L34) | CloudSystem | `void CloudSystem::update(float deltaTime, const glm::vec3& windVelocity)` | 34 | function: Dữ liệu và chuyển động bốn cụm mây | [f144](02_SOURCE_CODE_WALKTHROUGH.md#f144) |
| 145 | [src/CloudSystem.cpp:44](../src/CloudSystem.cpp#L44) | CloudSystem | `float CloudSystem::coverage(glm::vec3 center)` | 44 | function: Dữ liệu và chuyển động bốn cụm mây | [f145](02_SOURCE_CODE_WALKTHROUGH.md#f145) |
| 146 | [src/DynamicBatch.cpp:7](../src/DynamicBatch.cpp#L7) | DynamicBatch | `DynamicBatch::DynamicBatch(std::size_t capacity, GLenum primitive)` | 7 | function: VAO/VBO động cho các hiệu ứng | [f146](02_SOURCE_CODE_WALKTHROUGH.md#f146) |
| 147 | [src/DynamicBatch.cpp:31](../src/DynamicBatch.cpp#L31) | DynamicBatch | `DynamicBatch::~DynamicBatch()` | 31 | function: VAO/VBO động cho các hiệu ứng | [f147](02_SOURCE_CODE_WALKTHROUGH.md#f147) |
| 148 | [src/DynamicBatch.cpp:36](../src/DynamicBatch.cpp#L36) | DynamicBatch | `void DynamicBatch::upload(const EffectVertex* vertices, std::size_t count)` | 36 | function: VAO/VBO động cho các hiệu ứng | [f148](02_SOURCE_CODE_WALKTHROUGH.md#f148) |
| 149 | [src/DynamicBatch.cpp:46](../src/DynamicBatch.cpp#L46) | DynamicBatch | `void DynamicBatch::draw() const` | 46 | function: VAO/VBO động cho các hiệu ứng | [f149](02_SOURCE_CODE_WALKTHROUGH.md#f149) |
| 150 | [src/EnvironmentGeometry.cpp:11](../src/EnvironmentGeometry.cpp#L11) | namespace ẩn | `void triangle(std::vector<Vertex>& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 outward)` | 11 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f150](02_SOURCE_CODE_WALKTHROUGH.md#f150) |
| 151 | [src/EnvironmentGeometry.cpp:19](../src/EnvironmentGeometry.cpp#L19) | environment | `bool grassAllowed(glm::vec2 p)` | 19 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f151](02_SOURCE_CODE_WALKTHROUGH.md#f151) |
| 152 | [src/EnvironmentGeometry.cpp:33](../src/EnvironmentGeometry.cpp#L33) | environment | `const std::array<GrassTuft,64>& grassTufts()` | 33 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f152](02_SOURCE_CODE_WALKTHROUGH.md#f152) |
| 153 | [src/EnvironmentGeometry.cpp:34](../src/EnvironmentGeometry.cpp#L34) | grassTufts | `[]` | 34 | lambda: Sinh mesh nước/đá/mây/cỏ hiện tại | [f153](02_SOURCE_CODE_WALKTHROUGH.md#f153) |
| 154 | [src/EnvironmentGeometry.cpp:50](../src/EnvironmentGeometry.cpp#L50) | environment | `std::vector<Vertex> grass()` | 50 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f154](02_SOURCE_CODE_WALKTHROUGH.md#f154) |
| 155 | [src/EnvironmentGeometry.cpp:65](../src/EnvironmentGeometry.cpp#L65) | environment | `std::vector<Vertex> puddles()` | 65 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f155](02_SOURCE_CODE_WALKTHROUGH.md#f155) |
| 156 | [src/EnvironmentGeometry.cpp:79](../src/EnvironmentGeometry.cpp#L79) | environment | `const std::array<RockInstance,3>& rocks()` | 79 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f156](02_SOURCE_CODE_WALKTHROUGH.md#f156) |
| 157 | [src/EnvironmentGeometry.cpp:88](../src/EnvironmentGeometry.cpp#L88) | environment | `std::vector<Vertex> rock(std::uint32_t seed)` | 88 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f157](02_SOURCE_CODE_WALKTHROUGH.md#f157) |
| 158 | [src/EnvironmentGeometry.cpp:91](../src/EnvironmentGeometry.cpp#L91) | rock | `[&](float low,float high)` | 91 | lambda: Sinh mesh nước/đá/mây/cỏ hiện tại | [f158](02_SOURCE_CODE_WALKTHROUGH.md#f158) |
| 159 | [src/EnvironmentGeometry.cpp:108](../src/EnvironmentGeometry.cpp#L108) | rock | `[&](glm::vec3 a,glm::vec3 b,glm::vec3 c)` | 108 | lambda: Sinh mesh nước/đá/mây/cỏ hiện tại | [f159](02_SOURCE_CODE_WALKTHROUGH.md#f159) |
| 160 | [src/EnvironmentGeometry.cpp:121](../src/EnvironmentGeometry.cpp#L121) | environment | `std::vector<Vertex> cloud(const CloudCluster& cluster)` | 121 | function: Sinh mesh nước/đá/mây/cỏ hiện tại | [f160](02_SOURCE_CODE_WALKTHROUGH.md#f160) |
| 161 | [src/EnvironmentGeometry.cpp:129](../src/EnvironmentGeometry.cpp#L129) | cloud | `[&](glm::vec3 p,glm::vec3* normal=nullptr)` | 129 | lambda: Sinh mesh nước/đá/mây/cỏ hiện tại | [f161](02_SOURCE_CODE_WALKTHROUGH.md#f161) |
| 162 | [src/EnvironmentGeometry.cpp:161](../src/EnvironmentGeometry.cpp#L161) | cloud | `[&](Vertex a,Vertex b,Vertex c)` | 161 | lambda: Sinh mesh nước/đá/mây/cỏ hiện tại | [f162](02_SOURCE_CODE_WALKTHROUGH.md#f162) |
| 163 | [src/Geometry.cpp:8](../src/Geometry.cpp#L8) | namespace ẩn | `void triangle(std::vector<Vertex>& vertices, glm::vec3 a, glm::vec3 b, glm::vec3 c)` | 8 | function: Primitive triangle lists cơ sở | [f163](02_SOURCE_CODE_WALKTHROUGH.md#f163) |
| 164 | [src/Geometry.cpp:15](../src/Geometry.cpp#L15) | namespace ẩn | `void quad(std::vector<Vertex>& vertices, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)` | 15 | function: Primitive triangle lists cơ sở | [f164](02_SOURCE_CODE_WALKTHROUGH.md#f164) |
| 165 | [src/Geometry.cpp:23](../src/Geometry.cpp#L23) | geometry | `std::vector<Vertex> screenTriangle()` | 23 | function: Primitive triangle lists cơ sở | [f165](02_SOURCE_CODE_WALKTHROUGH.md#f165) |
| 166 | [src/Geometry.cpp:26](../src/Geometry.cpp#L26) | geometry | `std::vector<Vertex> puddle()` | 26 | function: Primitive triangle lists cơ sở | [f166](02_SOURCE_CODE_WALKTHROUGH.md#f166) |
| 167 | [src/Geometry.cpp:36](../src/Geometry.cpp#L36) | geometry | `std::vector<Vertex> ground()` | 36 | function: Primitive triangle lists cơ sở | [f167](02_SOURCE_CODE_WALKTHROUGH.md#f167) |
| 168 | [src/Geometry.cpp:43](../src/Geometry.cpp#L43) | geometry | `std::vector<Vertex> box()` | 43 | function: Primitive triangle lists cơ sở | [f168](02_SOURCE_CODE_WALKTHROUGH.md#f168) |
| 169 | [src/Geometry.cpp:56](../src/Geometry.cpp#L56) | geometry | `std::vector<Vertex> roof()` | 56 | function: Primitive triangle lists cơ sở | [f169](02_SOURCE_CODE_WALKTHROUGH.md#f169) |
| 170 | [src/Geometry.cpp:69](../src/Geometry.cpp#L69) | geometry | `std::vector<Vertex> cone(unsigned segments)` | 69 | function: Primitive triangle lists cơ sở | [f170](02_SOURCE_CODE_WALKTHROUGH.md#f170) |
| 171 | [src/Geometry.cpp:87](../src/Geometry.cpp#L87) | geometry | `std::vector<Vertex> rock()` | 87 | function: Primitive triangle lists cơ sở | [f171](02_SOURCE_CODE_WALKTHROUGH.md#f171) |
| 172 | [src/Geometry.cpp:98](../src/Geometry.cpp#L98) | geometry | `std::vector<Vertex> sphere(unsigned slices, unsigned stacks)` | 98 | function: Primitive triangle lists cơ sở | [f172](02_SOURCE_CODE_WALKTHROUGH.md#f172) |
| 173 | [src/Geometry.cpp:103](../src/Geometry.cpp#L103) | sphere | `[=](unsigned row, unsigned column)` | 103 | lambda: Primitive triangle lists cơ sở | [f173](02_SOURCE_CODE_WALKTHROUGH.md#f173) |
| 174 | [src/LightingSystem.cpp:6](../src/LightingSystem.cpp#L6) | LightingSystem | `void LightingSystem::reset()` | 6 | function: Ánh sáng màu, flash, fog và wetness | [f174](02_SOURCE_CODE_WALKTHROUGH.md#f174) |
| 175 | [src/LightingSystem.cpp:14](../src/LightingSystem.cpp#L14) | LightingSystem | `void LightingSystem::setEnvironment(float hour, float storminess)` | 14 | function: Ánh sáng màu, flash, fog và wetness | [f175](02_SOURCE_CODE_WALKTHROUGH.md#f175) |
| 176 | [src/LightingSystem.cpp:18](../src/LightingSystem.cpp#L18) | LightingSystem::setEnvironment | `[](float a,float b,float x)` | 18 | lambda: Ánh sáng màu, flash, fog và wetness | [f176](02_SOURCE_CODE_WALKTHROUGH.md#f176) |
| 177 | [src/LightingSystem.cpp:42](../src/LightingSystem.cpp#L42) | LightingSystem | `glm::vec3 LightingSystem::particleColor() const` | 42 | function: Ánh sáng màu, flash, fog và wetness | [f177](02_SOURCE_CODE_WALKTHROUGH.md#f177) |
| 178 | [src/LightingSystem.cpp:46](../src/LightingSystem.cpp#L46) | LightingSystem | `void LightingSystem::update(float flashIntensity, float deltaTime, bool raining)` | 46 | function: Ánh sáng màu, flash, fog và wetness | [f178](02_SOURCE_CODE_WALKTHROUGH.md#f178) |
| 179 | [src/LightingSystem.cpp:54](../src/LightingSystem.cpp#L54) | LightingSystem | `glm::vec3 LightingSystem::skyColor() const` | 54 | function: Ánh sáng màu, flash, fog và wetness | [f179](02_SOURCE_CODE_WALKTHROUGH.md#f179) |
| 180 | [src/LightingSystem.cpp:58](../src/LightingSystem.cpp#L58) | LightingSystem | `float LightingSystem::attenuationAt(glm::vec3 point) const` | 58 | function: Ánh sáng màu, flash, fog và wetness | [f180](02_SOURCE_CODE_WALKTHROUGH.md#f180) |
| 181 | [src/LightingSystem.cpp:63](../src/LightingSystem.cpp#L63) | LightingSystem | `glm::vec3 LightingSystem::skyTop() const` | 63 | function: Ánh sáng màu, flash, fog và wetness | [f181](02_SOURCE_CODE_WALKTHROUGH.md#f181) |
| 182 | [src/LightingSystem.cpp:67](../src/LightingSystem.cpp#L67) | LightingSystem | `glm::vec3 LightingSystem::fogColor() const` | 67 | function: Ánh sáng màu, flash, fog và wetness | [f182](02_SOURCE_CODE_WALKTHROUGH.md#f182) |
| 183 | [src/LightingSystem.cpp:71](../src/LightingSystem.cpp#L71) | LightingSystem | `float LightingSystem::fogFactor(float distance) const` | 71 | function: Ánh sáng màu, flash, fog và wetness | [f183](02_SOURCE_CODE_WALKTHROUGH.md#f183) |
| 184 | [src/LightningSystem.cpp:6](../src/LightningSystem.cpp#L6) | LightningSystem | `float LightningSystem::random(float low, float high)` | 6 | function: Polyline sét đệ quy, nhánh và xung | [f184](02_SOURCE_CODE_WALKTHROUGH.md#f184) |
| 185 | [src/LightningSystem.cpp:10](../src/LightningSystem.cpp#L10) | LightningSystem | `void LightningSystem::reset(std::uint32_t seed)` | 10 | function: Polyline sét đệ quy, nhánh và xung | [f185](02_SOURCE_CODE_WALKTHROUGH.md#f185) |
| 186 | [src/LightningSystem.cpp:20](../src/LightningSystem.cpp#L20) | LightningSystem | `void LightningSystem::cancel()` | 20 | function: Polyline sét đệ quy, nhánh và xung | [f186](02_SOURCE_CODE_WALKTHROUGH.md#f186) |
| 187 | [src/LightningSystem.cpp:26](../src/LightningSystem.cpp#L26) | LightningSystem | `void LightningSystem::subdivide(glm::vec3 a, glm::vec3 b, unsigned depth, float displacement, float width, bool branch)` | 26 | function: Polyline sét đệ quy, nhánh và xung | [f187](02_SOURCE_CODE_WALKTHROUGH.md#f187) |
| 188 | [src/LightningSystem.cpp:55](../src/LightningSystem.cpp#L55) | LightningSystem | `void LightningSystem::trigger(const CloudSystem& clouds, glm::vec3 observer)` | 55 | function: Polyline sét đệ quy, nhánh và xung | [f188](02_SOURCE_CODE_WALKTHROUGH.md#f188) |
| 189 | [src/LightningSystem.cpp:102](../src/LightningSystem.cpp#L102) | LightningSystem | `void LightningSystem::update(float deltaTime, const CloudSystem& clouds, bool randomEnabled, glm::vec3 observer)` | 102 | function: Polyline sét đệ quy, nhánh và xung | [f189](02_SOURCE_CODE_WALKTHROUGH.md#f189) |
| 190 | [src/LightningSystem.cpp:115](../src/LightningSystem.cpp#L115) | LightningSystem | `float LightningSystem::flashIntensity() const` | 115 | function: Polyline sét đệ quy, nhánh và xung | [f190](02_SOURCE_CODE_WALKTHROUGH.md#f190) |
| 191 | [src/Mesh.cpp:8](../src/Mesh.cpp#L8) | Mesh | `Mesh::Mesh(const std::vector<Vertex>& vertices)` | 8 | function: Tài nguyên GPU của mesh tĩnh | [f191](02_SOURCE_CODE_WALKTHROUGH.md#f191) |
| 192 | [src/Mesh.cpp:31](../src/Mesh.cpp#L31) | Mesh | `Mesh::~Mesh()` | 31 | function: Tài nguyên GPU của mesh tĩnh | [f192](02_SOURCE_CODE_WALKTHROUGH.md#f192) |
| 193 | [src/Mesh.cpp:36](../src/Mesh.cpp#L36) | Mesh | `void Mesh::draw() const` | 36 | function: Tài nguyên GPU của mesh tĩnh | [f193](02_SOURCE_CODE_WALKTHROUGH.md#f193) |
| 194 | [src/ModelGeometry.cpp:10](../src/ModelGeometry.cpp#L10) | namespace ẩn | `void tri(MeshData& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 outward)` | 10 | function: Mô hình nhà/cây procedural | [f194](02_SOURCE_CODE_WALKTHROUGH.md#f194) |
| 195 | [src/ModelGeometry.cpp:15](../src/ModelGeometry.cpp#L15) | namespace ẩn | `void quad(MeshData& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 outward)` | 15 | function: Mô hình nhà/cây procedural | [f195](02_SOURCE_CODE_WALKTHROUGH.md#f195) |
| 196 | [src/ModelGeometry.cpp:18](../src/ModelGeometry.cpp#L18) | namespace ẩn | `void box(MeshData& v, glm::vec3 position, glm::vec3 scale, const glm::mat4& parent = glm::mat4(1))` | 18 | function: Mô hình nhà/cây procedural | [f196](02_SOURCE_CODE_WALKTHROUGH.md#f196) |
| 197 | [src/ModelGeometry.cpp:24](../src/ModelGeometry.cpp#L24) | namespace ẩn | `void beam(MeshData& v, glm::vec3 a, glm::vec3 b, float width, float depth)` | 24 | function: Mô hình nhà/cây procedural | [f197](02_SOURCE_CODE_WALKTHROUGH.md#f197) |
| 198 | [src/ModelGeometry.cpp:31](../src/ModelGeometry.cpp#L31) | namespace ẩn | `void tube(MeshData& v, glm::vec3 a, glm::vec3 b, float ra, float rb, unsigned sides)` | 31 | function: Mô hình nhà/cây procedural | [f198](02_SOURCE_CODE_WALKTHROUGH.md#f198) |
| 199 | [src/ModelGeometry.cpp:46](../src/ModelGeometry.cpp#L46) | namespace ẩn | `void roofShell(MeshData& v)` | 46 | function: Mô hình nhà/cây procedural | [f199](02_SOURCE_CODE_WALKTHROUGH.md#f199) |
| 200 | [src/ModelGeometry.cpp:50](../src/ModelGeometry.cpp#L50) | roofShell | `[](glm::vec3 p)` | 50 | lambda: Mô hình nhà/cây procedural | [f200](02_SOURCE_CODE_WALKTHROUGH.md#f200) |
| 201 | [src/ModelGeometry.cpp:62](../src/ModelGeometry.cpp#L62) | namespace ẩn | `void gable(MeshData& v)` | 62 | function: Mô hình nhà/cây procedural | [f201](02_SOURCE_CODE_WALKTHROUGH.md#f201) |
| 202 | [src/ModelGeometry.cpp:65](../src/ModelGeometry.cpp#L65) | gable | `[&](std::size_t i, float z)` | 65 | lambda: Mô hình nhà/cây procedural | [f202](02_SOURCE_CODE_WALKTHROUGH.md#f202) |
| 203 | [src/ModelGeometry.cpp:79](../src/ModelGeometry.cpp#L79) | models | `House house()` | 79 | function: Mô hình nhà/cây procedural | [f203](02_SOURCE_CODE_WALKTHROUGH.md#f203) |
| 204 | [src/ModelGeometry.cpp:82](../src/ModelGeometry.cpp#L82) | house | `[&](HousePart p) -> MeshData&` | 82 | lambda: Mô hình nhà/cây procedural | [f204](02_SOURCE_CODE_WALKTHROUGH.md#f204) |
| 205 | [src/ModelGeometry.cpp:109](../src/ModelGeometry.cpp#L109) | house | `[&](glm::vec3 center, float yaw)` | 109 | lambda: Mô hình nhà/cây procedural | [f205](02_SOURCE_CODE_WALKTHROUGH.md#f205) |
| 206 | [src/ModelGeometry.cpp:130](../src/ModelGeometry.cpp#L130) | models | `const std::array<TreeInstance, 4>& treeLayout()` | 130 | function: Mô hình nhà/cây procedural | [f206](02_SOURCE_CODE_WALKTHROUGH.md#f206) |
| 207 | [src/ModelGeometry.cpp:140](../src/ModelGeometry.cpp#L140) | models | `Tree tree(const TreeParameters& p)` | 140 | function: Mô hình nhà/cây procedural | [f207](02_SOURCE_CODE_WALKTHROUGH.md#f207) |
| 208 | [src/ModelGeometry.cpp:149](../src/ModelGeometry.cpp#L149) | tree | `[&](float low, float high)` | 149 | lambda: Mô hình nhà/cây procedural | [f208](02_SOURCE_CODE_WALKTHROUGH.md#f208) |
| 209 | [src/RainSystem.cpp:6](../src/RainSystem.cpp#L6) | RainSystem | `float RainSystem::random(float low, float high)` | 6 | function: Pool 1000 giọt và ground impacts | [f209](02_SOURCE_CODE_WALKTHROUGH.md#f209) |
| 210 | [src/RainSystem.cpp:10](../src/RainSystem.cpp#L10) | RainSystem | `void RainSystem::respawn(Raindrop& drop, const glm::vec3& wind, bool initial)` | 10 | function: Pool 1000 giọt và ground impacts | [f210](02_SOURCE_CODE_WALKTHROUGH.md#f210) |
| 211 | [src/RainSystem.cpp:20](../src/RainSystem.cpp#L20) | RainSystem | `void RainSystem::reset(const glm::vec3& wind)` | 20 | function: Pool 1000 giọt và ground impacts | [f211](02_SOURCE_CODE_WALKTHROUGH.md#f211) |
| 212 | [src/RainSystem.cpp:28](../src/RainSystem.cpp#L28) | RainSystem | `void RainSystem::update(float deltaTime, const glm::vec3& wind)` | 28 | function: Pool 1000 giọt và ground impacts | [f212](02_SOURCE_CODE_WALKTHROUGH.md#f212) |
| 213 | [src/Renderer.cpp:12](../src/Renderer.cpp#L12) | Renderer | `Renderer::Renderer()` | 12 | function: Render passes, uniforms và dynamic staging | [f213](02_SOURCE_CODE_WALKTHROUGH.md#f213) |
| 214 | [src/Renderer.cpp:76](../src/Renderer.cpp#L76) | Renderer::Renderer | `[&](glm::vec3 p,float radius)` | 76 | lambda: Render passes, uniforms và dynamic staging | [f214](02_SOURCE_CODE_WALKTHROUGH.md#f214) |
| 215 | [src/Renderer.cpp:84](../src/Renderer.cpp#L84) | Renderer | `void Renderer::draw(const Scene& scene, const Camera& camera, const WeatherSystem& weather, int width, int height)` | 84 | function: Render passes, uniforms và dynamic staging | [f215](02_SOURCE_CODE_WALKTHROUGH.md#f215) |
| 216 | [src/Renderer.cpp:158](../src/Renderer.cpp#L158) | Renderer::draw | `[&](std::size_t i)` | 158 | lambda: Render passes, uniforms và dynamic staging | [f216](02_SOURCE_CODE_WALKTHROUGH.md#f216) |
| 217 | [src/Renderer.cpp:180](../src/Renderer.cpp#L180) | Renderer::draw | `[](const auto& a,const auto& b)` | 180 | lambda: Render passes, uniforms và dynamic staging | [f217](02_SOURCE_CODE_WALKTHROUGH.md#f217) |
| 218 | [src/RippleSystem.cpp:5](../src/RippleSystem.cpp#L5) | RippleSystem | `void RippleSystem::reset()` | 5 | function: Pool 48 vòng gợn | [f218](02_SOURCE_CODE_WALKTHROUGH.md#f218) |
| 219 | [src/RippleSystem.cpp:13](../src/RippleSystem.cpp#L13) | RippleSystem | `void RippleSystem::emit(glm::vec3 hit, bool sampled, float waterMargin)` | 13 | function: Pool 48 vòng gợn | [f219](02_SOURCE_CODE_WALKTHROUGH.md#f219) |
| 220 | [src/RippleSystem.cpp:27](../src/RippleSystem.cpp#L27) | RippleSystem | `void RippleSystem::update(float dt)` | 27 | function: Pool 48 vòng gợn | [f220](02_SOURCE_CODE_WALKTHROUGH.md#f220) |
| 221 | [src/Scene.cpp:10](../src/Scene.cpp#L10) | Scene | `Scene::Scene()` | 10 | function: Sở hữu mesh và danh sách vật thể | [f221](02_SOURCE_CODE_WALKTHROUGH.md#f221) |
| 222 | [src/Scene.cpp:53](../src/Scene.cpp#L53) | Scene | `void Scene::modelPart(const std::vector<Vertex>& vertices, glm::vec3 position, glm::vec3 color, Material material, float treeHeight)` | 53 | function: Sở hữu mesh và danh sách vật thể | [f222](02_SOURCE_CODE_WALKTHROUGH.md#f222) |
| 223 | [src/Scene.cpp:61](../src/Scene.cpp#L61) | Scene | `void Scene::add(const Mesh& mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 color, float yaw)` | 61 | function: Sở hữu mesh và danh sách vật thể | [f223](02_SOURCE_CODE_WALKTHROUGH.md#f223) |
| 224 | [src/Scene.cpp:69](../src/Scene.cpp#L69) | Scene | `std::size_t Scene::triangleCount() const` | 69 | function: Sở hữu mesh và danh sách vật thể | [f224](02_SOURCE_CODE_WALKTHROUGH.md#f224) |
| 225 | [src/Scene.cpp:75](../src/Scene.cpp#L75) | Scene | `void Scene::update(float deltaTime, bool paused)` | 75 | function: Sở hữu mesh và danh sách vật thể | [f225](02_SOURCE_CODE_WALKTHROUGH.md#f225) |
| 226 | [src/Shader.cpp:9](../src/Shader.cpp#L9) | namespace ẩn | `GLuint compile(GLenum type, const std::string& path)` | 9 | function: Đọc/compile/link GLSL | [f226](02_SOURCE_CODE_WALKTHROUGH.md#f226) |
| 227 | [src/Shader.cpp:41](../src/Shader.cpp#L41) | Shader | `Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)` | 41 | function: Đọc/compile/link GLSL | [f227](02_SOURCE_CODE_WALKTHROUGH.md#f227) |
| 228 | [src/Shader.cpp:75](../src/Shader.cpp#L75) | Shader | `Shader::~Shader()` | 75 | function: Đọc/compile/link GLSL | [f228](02_SOURCE_CODE_WALKTHROUGH.md#f228) |
| 229 | [src/Shader.cpp:77](../src/Shader.cpp#L77) | Shader | `GLint Shader::uniform(const char* name) const` | 77 | function: Đọc/compile/link GLSL | [f229](02_SOURCE_CODE_WALKTHROUGH.md#f229) |
| 230 | [src/SplashSystem.cpp:6](../src/SplashSystem.cpp#L6) | SplashSystem | `float SplashSystem::random(float low, float high)` | 6 | function: Pool 512 hạt bắn | [f230](02_SOURCE_CODE_WALKTHROUGH.md#f230) |
| 231 | [src/SplashSystem.cpp:10](../src/SplashSystem.cpp#L10) | SplashSystem | `void SplashSystem::reset()` | 10 | function: Pool 512 hạt bắn | [f231](02_SOURCE_CODE_WALKTHROUGH.md#f231) |
| 232 | [src/SplashSystem.cpp:18](../src/SplashSystem.cpp#L18) | SplashSystem | `void SplashSystem::emit(glm::vec3 hit, bool sampled)` | 18 | function: Pool 512 hạt bắn | [f232](02_SOURCE_CODE_WALKTHROUGH.md#f232) |
| 233 | [src/SplashSystem.cpp:38](../src/SplashSystem.cpp#L38) | SplashSystem | `void SplashSystem::update(float deltaTime)` | 38 | function: Pool 512 hạt bắn | [f233](02_SOURCE_CODE_WALKTHROUGH.md#f233) |
| 234 | [src/Surface.cpp:7](../src/Surface.cpp#L7) | surface | `const std::array<Puddle, 3>& puddles()` | 7 | function: Polygon nước dùng chung | [f234](02_SOURCE_CODE_WALKTHROUGH.md#f234) |
| 235 | [src/Surface.cpp:15](../src/Surface.cpp#L15) | surface | `float outlineRadius(float a)` | 15 | function: Polygon nước dùng chung | [f235](02_SOURCE_CODE_WALKTHROUGH.md#f235) |
| 236 | [src/Surface.cpp:19](../src/Surface.cpp#L19) | surface | `const std::array<glm::vec2, OUTLINE_POINTS>& outline()` | 19 | function: Polygon nước dùng chung | [f236](02_SOURCE_CODE_WALKTHROUGH.md#f236) |
| 237 | [src/Surface.cpp:20](../src/Surface.cpp#L20) | outline | `[]` | 20 | lambda: Polygon nước dùng chung | [f237](02_SOURCE_CODE_WALKTHROUGH.md#f237) |
| 238 | [src/Surface.cpp:32](../src/Surface.cpp#L32) | surface | `const std::array<glm::vec2, OUTLINE_POINTS>& polygon(std::size_t index)` | 32 | function: Polygon nước dùng chung | [f238](02_SOURCE_CODE_WALKTHROUGH.md#f238) |
| 239 | [src/Surface.cpp:35](../src/Surface.cpp#L35) | polygon | `[]` | 35 | lambda: Polygon nước dùng chung | [f239](02_SOURCE_CODE_WALKTHROUGH.md#f239) |
| 240 | [src/Surface.cpp:52](../src/Surface.cpp#L52) | surface | `float waterMargin(glm::vec3 hit)` | 52 | function: Polygon nước dùng chung | [f240](02_SOURCE_CODE_WALKTHROUGH.md#f240) |
| 241 | [src/Surface.cpp:53](../src/Surface.cpp#L53) | waterMargin | `[]` | 53 | lambda: Polygon nước dùng chung | [f241](02_SOURCE_CODE_WALKTHROUGH.md#f241) |
| 242 | [src/TimeOfDaySystem.cpp:5](../src/TimeOfDaySystem.cpp#L5) | TimeOfDaySystem | `void TimeOfDaySystem::reset()` | 5 | function: Đồng hồ giờ mô phỏng | [f242](02_SOURCE_CODE_WALKTHROUGH.md#f242) |
| 243 | [src/TimeOfDaySystem.cpp:6](../src/TimeOfDaySystem.cpp#L6) | TimeOfDaySystem | `bool TimeOfDaySystem::setHour(double hour)` | 6 | function: Đồng hồ giờ mô phỏng | [f243](02_SOURCE_CODE_WALKTHROUGH.md#f243) |
| 244 | [src/TimeOfDaySystem.cpp:12](../src/TimeOfDaySystem.cpp#L12) | TimeOfDaySystem | `bool TimeOfDaySystem::setDayDuration(double seconds)` | 12 | function: Đồng hồ giờ mô phỏng | [f244](02_SOURCE_CODE_WALKTHROUGH.md#f244) |
| 245 | [src/TimeOfDaySystem.cpp:16](../src/TimeOfDaySystem.cpp#L16) | TimeOfDaySystem | `void TimeOfDaySystem::update(double seconds,bool paused)` | 16 | function: Đồng hồ giờ mô phỏng | [f245](02_SOURCE_CODE_WALKTHROUGH.md#f245) |
| 246 | [src/TimeOfDaySystem.cpp:21](../src/TimeOfDaySystem.cpp#L21) | TimeOfDaySystem | `std::string TimeOfDaySystem::label() const` | 21 | function: Đồng hồ giờ mô phỏng | [f246](02_SOURCE_CODE_WALKTHROUGH.md#f246) |
| 247 | [src/WeatherSystem.cpp:7](../src/WeatherSystem.cpp#L7) | WeatherSystem | `void WeatherSystem::reset()` | 7 | function: Điều phối fixed timestep | [f247](02_SOURCE_CODE_WALKTHROUGH.md#f247) |
| 248 | [src/WeatherSystem.cpp:25](../src/WeatherSystem.cpp#L25) | WeatherSystem | `void WeatherSystem::refreshLighting()` | 25 | function: Điều phối fixed timestep | [f248](02_SOURCE_CODE_WALKTHROUGH.md#f248) |
| 249 | [src/WeatherSystem.cpp:30](../src/WeatherSystem.cpp#L30) | WeatherSystem | `void WeatherSystem::update(float deltaTime)` | 30 | function: Điều phối fixed timestep | [f249](02_SOURCE_CODE_WALKTHROUGH.md#f249) |
| 250 | [src/WeatherSystem.cpp:55](../src/WeatherSystem.cpp#L55) | WeatherSystem | `void WeatherSystem::triggerLightning()` | 55 | function: Điều phối fixed timestep | [f250](02_SOURCE_CODE_WALKTHROUGH.md#f250) |
| 251 | [src/WeatherSystem.cpp:61](../src/WeatherSystem.cpp#L61) | WeatherSystem | `void WeatherSystem::toggleLightning()` | 61 | function: Điều phối fixed timestep | [f251](02_SOURCE_CODE_WALKTHROUGH.md#f251) |
| 252 | [src/WeatherSystem.cpp:67](../src/WeatherSystem.cpp#L67) | WeatherSystem | `bool WeatherSystem::valid() const` | 67 | function: Điều phối fixed timestep | [f252](02_SOURCE_CODE_WALKTHROUGH.md#f252) |
| 253 | [src/WeatherSystem.cpp:69](../src/WeatherSystem.cpp#L69) | WeatherSystem::valid | `[](glm::vec3 p)` | 69 | lambda: Điều phối fixed timestep | [f253](02_SOURCE_CODE_WALKTHROUGH.md#f253) |
| 254 | [src/main.cpp:10](../src/main.cpp#L10) | global | `int main(int argc, char** argv)` | 10 | function: Parse CLI và entry point | [f254](02_SOURCE_CODE_WALKTHROUGH.md#f254) |
| 255 | [tests/core_tests.cpp:15](../tests/core_tests.cpp#L15) | namespace ẩn | `void require(bool condition, const std::string& message)` | 15 | function: Test | [f255](02_SOURCE_CODE_WALKTHROUGH.md#f255) |
| 256 | [tests/core_tests.cpp:19](../tests/core_tests.cpp#L19) | namespace ẩn | `bool near(float a, float b, float epsilon = 0.0001f)` | 19 | function: Test | [f256](02_SOURCE_CODE_WALKTHROUGH.md#f256) |
| 257 | [tests/core_tests.cpp:22](../tests/core_tests.cpp#L22) | namespace ẩn | `Point point(glm::vec3 p)` | 22 | function: Test | [f257](02_SOURCE_CODE_WALKTHROUGH.md#f257) |
| 258 | [tests/core_tests.cpp:26](../tests/core_tests.cpp#L26) | namespace ẩn | `void mesh(const std::string& name, const std::vector<Vertex>& vertices, glm::vec3 interior, bool closed = true)` | 26 | function: Test | [f258](02_SOURCE_CODE_WALKTHROUGH.md#f258) |
| 259 | [tests/core_tests.cpp:66](../tests/core_tests.cpp#L66) | namespace ẩn | `void geometryTests()` | 66 | function: Test | [f259](02_SOURCE_CODE_WALKTHROUGH.md#f259) |
| 260 | [tests/core_tests.cpp:90](../tests/core_tests.cpp#L90) | namespace ẩn | `void cameraTests()` | 90 | function: Test | [f260](02_SOURCE_CODE_WALKTHROUGH.md#f260) |
| 261 | [tests/core_tests.cpp:151](../tests/core_tests.cpp#L151) | global | `int main()` | 151 | function: Test | [f261](02_SOURCE_CODE_WALKTHROUGH.md#f261) |
| 262 | [tests/environment_tests.cpp:12](../tests/environment_tests.cpp#L12) | namespace ẩn | `void require(bool value, const char* message)` | 12 | function: Test | [f262](02_SOURCE_CODE_WALKTHROUGH.md#f262) |
| 263 | [tests/environment_tests.cpp:14](../tests/environment_tests.cpp#L14) | namespace ẩn | `Point point(glm::vec3 p)` | 14 | function: Test | [f263](02_SOURCE_CODE_WALKTHROUGH.md#f263) |
| 264 | [tests/environment_tests.cpp:15](../tests/environment_tests.cpp#L15) | namespace ẩn | `void valid(const std::vector<Vertex>& mesh, bool closed, bool smooth=false)` | 15 | function: Test | [f264](02_SOURCE_CODE_WALKTHROUGH.md#f264) |
| 265 | [tests/environment_tests.cpp:35](../tests/environment_tests.cpp#L35) | namespace ẩn | `void waterTests()` | 35 | function: Test | [f265](02_SOURCE_CODE_WALKTHROUGH.md#f265) |
| 266 | [tests/environment_tests.cpp:69](../tests/environment_tests.cpp#L69) | namespace ẩn | `void rockTests()` | 69 | function: Test | [f266](02_SOURCE_CODE_WALKTHROUGH.md#f266) |
| 267 | [tests/environment_tests.cpp:85](../tests/environment_tests.cpp#L85) | namespace ẩn | `void cloudTests()` | 85 | function: Test | [f267](02_SOURCE_CODE_WALKTHROUGH.md#f267) |
| 268 | [tests/environment_tests.cpp:111](../tests/environment_tests.cpp#L111) | namespace ẩn | `void grassTests()` | 111 | function: Test | [f268](02_SOURCE_CODE_WALKTHROUGH.md#f268) |
| 269 | [tests/environment_tests.cpp:126](../tests/environment_tests.cpp#L126) | global | `int main()` | 126 | function: Test | [f269](02_SOURCE_CODE_WALKTHROUGH.md#f269) |
| 270 | [tests/lighting_time_tests.cpp:9](../tests/lighting_time_tests.cpp#L9) | namespace ẩn | `void require(bool value,const char* message)` | 9 | function: Test | [f270](02_SOURCE_CODE_WALKTHROUGH.md#f270) |
| 271 | [tests/lighting_time_tests.cpp:10](../tests/lighting_time_tests.cpp#L10) | namespace ẩn | `void lightingTests()` | 10 | function: Test | [f271](02_SOURCE_CODE_WALKTHROUGH.md#f271) |
| 272 | [tests/lighting_time_tests.cpp:39](../tests/lighting_time_tests.cpp#L39) | namespace ẩn | `void clockTests()` | 39 | function: Test | [f272](02_SOURCE_CODE_WALKTHROUGH.md#f272) |
| 273 | [tests/lighting_time_tests.cpp:79](../tests/lighting_time_tests.cpp#L79) | global | `int main()` | 79 | function: Test | [f273](02_SOURCE_CODE_WALKTHROUGH.md#f273) |
| 274 | [tests/model_tests.cpp:12](../tests/model_tests.cpp#L12) | namespace ẩn | `void require(bool ok, const char* text)` | 12 | function: Test | [f274](02_SOURCE_CODE_WALKTHROUGH.md#f274) |
| 275 | [tests/model_tests.cpp:13](../tests/model_tests.cpp#L13) | namespace ẩn | `void vertices(const std::vector<Vertex>& mesh)` | 13 | function: Test | [f275](02_SOURCE_CODE_WALKTHROUGH.md#f275) |
| 276 | [tests/model_tests.cpp:28](../tests/model_tests.cpp#L28) | namespace ẩn | `Point key(glm::vec3 p)` | 28 | function: Test | [f276](02_SOURCE_CODE_WALKTHROUGH.md#f276) |
| 277 | [tests/model_tests.cpp:29](../tests/model_tests.cpp#L29) | namespace ẩn | `void closed(const std::vector<Vertex>& mesh)` | 29 | function: Test | [f277](02_SOURCE_CODE_WALKTHROUGH.md#f277) |
| 278 | [tests/model_tests.cpp:40](../tests/model_tests.cpp#L40) | namespace ẩn | `float undersideAt(const std::vector<Vertex>& mesh, float x, float z)` | 40 | function: Test | [f278](02_SOURCE_CODE_WALKTHROUGH.md#f278) |
| 279 | [tests/model_tests.cpp:55](../tests/model_tests.cpp#L55) | namespace ẩn | `void houseTests()` | 55 | function: Test | [f279](02_SOURCE_CODE_WALKTHROUGH.md#f279) |
| 280 | [tests/model_tests.cpp:76](../tests/model_tests.cpp#L76) | namespace ẩn | `void treeTests()` | 76 | function: Test | [f280](02_SOURCE_CODE_WALKTHROUGH.md#f280) |
| 281 | [tests/model_tests.cpp:106](../tests/model_tests.cpp#L106) | global | `int main()` | 106 | function: Test | [f281](02_SOURCE_CODE_WALKTHROUGH.md#f281) |
| 282 | [tests/weather_tests.cpp:11](../tests/weather_tests.cpp#L11) | namespace ẩn | `void require(bool condition, const char* message)` | 11 | function: Test | [f282](02_SOURCE_CODE_WALKTHROUGH.md#f282) |
| 283 | [tests/weather_tests.cpp:15](../tests/weather_tests.cpp#L15) | namespace ẩn | `void cloudTests()` | 15 | function: Test | [f283](02_SOURCE_CODE_WALKTHROUGH.md#f283) |
| 284 | [tests/weather_tests.cpp:65](../tests/weather_tests.cpp#L65) | namespace ẩn | `void rainTests()` | 65 | function: Test | [f284](02_SOURCE_CODE_WALKTHROUGH.md#f284) |
| 285 | [tests/weather_tests.cpp:132](../tests/weather_tests.cpp#L132) | namespace ẩn | `void lightningTests()` | 132 | function: Test | [f285](02_SOURCE_CODE_WALKTHROUGH.md#f285) |
| 286 | [tests/weather_tests.cpp:256](../tests/weather_tests.cpp#L256) | namespace ẩn | `void splashTests()` | 256 | function: Test | [f286](02_SOURCE_CODE_WALKTHROUGH.md#f286) |
| 287 | [tests/weather_tests.cpp:298](../tests/weather_tests.cpp#L298) | namespace ẩn | `void integrationTests()` | 298 | function: Test | [f287](02_SOURCE_CODE_WALKTHROUGH.md#f287) |
| 288 | [tests/weather_tests.cpp:362](../tests/weather_tests.cpp#L362) | namespace ẩn | `void rippleTests()` | 362 | function: Test | [f288](02_SOURCE_CODE_WALKTHROUGH.md#f288) |
| 289 | [tests/weather_tests.cpp:382](../tests/weather_tests.cpp#L382) | namespace ẩn | `void surfaceTests()` | 382 | function: Test | [f289](02_SOURCE_CODE_WALKTHROUGH.md#f289) |
| 290 | [tests/weather_tests.cpp:407](../tests/weather_tests.cpp#L407) | global | `int main()` | 407 | function: Test | [f290](02_SOURCE_CODE_WALKTHROUGH.md#f290) |

## Đối chiếu theo file

| File | Định nghĩa có tên | Lambda | = delete |
|---|---:|---:|---:|
| `include/Application.hpp` | 0 | 0 | 2 |
| `include/Benchmark.hpp` | 0 | 0 | 2 |
| `include/Camera.hpp` | 4 | 0 | 0 |
| `include/CloudSystem.hpp` | 2 | 0 | 0 |
| `include/DynamicBatch.hpp` | 2 | 0 | 2 |
| `include/EnvironmentGeometry.hpp` | 0 | 0 | 0 |
| `include/Geometry.hpp` | 0 | 0 | 0 |
| `include/LightingSystem.hpp` | 11 | 0 | 0 |
| `include/LightningSystem.hpp` | 12 | 0 | 0 |
| `include/Material.hpp` | 0 | 0 | 0 |
| `include/Mesh.hpp` | 1 | 0 | 2 |
| `include/ModelGeometry.hpp` | 0 | 0 | 0 |
| `include/RainSystem.hpp` | 6 | 0 | 0 |
| `include/Renderer.hpp` | 7 | 0 | 0 |
| `include/RippleSystem.hpp` | 5 | 0 | 0 |
| `include/Scene.hpp` | 3 | 0 | 0 |
| `include/Shader.hpp` | 1 | 0 | 2 |
| `include/SplashSystem.hpp` | 7 | 0 | 0 |
| `include/Surface.hpp` | 0 | 0 | 0 |
| `include/TimeOfDaySystem.hpp` | 6 | 0 | 0 |
| `include/WeatherSystem.hpp` | 0 | 0 | 0 |
| `shaders/effects.frag` | 1 | 0 | 0 |
| `shaders/effects.vert` | 1 | 0 | 0 |
| `shaders/object.frag` | 4 | 0 | 0 |
| `shaders/object.vert` | 1 | 0 | 0 |
| `shaders/sky.frag` | 1 | 0 | 0 |
| `shaders/sky.vert` | 1 | 0 | 0 |
| `src/Application.cpp` | 34 | 3 | 0 |
| `src/Benchmark.cpp` | 9 | 1 | 0 |
| `src/Camera.cpp` | 8 | 0 | 0 |
| `src/CloudSystem.cpp` | 4 | 0 | 0 |
| `src/DynamicBatch.cpp` | 4 | 0 | 0 |
| `src/EnvironmentGeometry.cpp` | 8 | 5 | 0 |
| `src/Geometry.cpp` | 10 | 1 | 0 |
| `src/LightingSystem.cpp` | 9 | 1 | 0 |
| `src/LightningSystem.cpp` | 7 | 0 | 0 |
| `src/Mesh.cpp` | 3 | 0 | 0 |
| `src/ModelGeometry.cpp` | 10 | 5 | 0 |
| `src/RainSystem.cpp` | 4 | 0 | 0 |
| `src/Renderer.cpp` | 2 | 3 | 0 |
| `src/RippleSystem.cpp` | 3 | 0 | 0 |
| `src/Scene.cpp` | 5 | 0 | 0 |
| `src/Shader.cpp` | 4 | 0 | 0 |
| `src/SplashSystem.cpp` | 4 | 0 | 0 |
| `src/Surface.cpp` | 5 | 3 | 0 |
| `src/TimeOfDaySystem.cpp` | 5 | 0 | 0 |
| `src/WeatherSystem.cpp` | 6 | 1 | 0 |
| `src/main.cpp` | 1 | 0 | 0 |
| `tests/core_tests.cpp` | 7 | 0 | 0 |
| `tests/environment_tests.cpp` | 8 | 0 | 0 |
| `tests/lighting_time_tests.cpp` | 4 | 0 | 0 |
| `tests/model_tests.cpp` | 8 | 0 | 0 |
| `tests/weather_tests.cpp` | 9 | 0 | 0 |
