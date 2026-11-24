# 02 — Đọc mã nguồn theo file và theo hàm

[Mục lục bộ tài liệu](01_PROJECT_OVERVIEW.md) · [Chỉ mục toàn bộ hàm](10_FUNCTION_INDEX.md)

Đối chiếu source ngày 23/11/2026. Số dòng là dòng bắt đầu định nghĩa trong snapshot hiện tại. Mỗi định nghĩa có một anchor ổn định; prototype ở header liên kết về implementation, không đếm hai lần. Có cả 23 lambda (kể cả static initializer và comparator generic) và 10 khai báo copy bị delete, được ghi riêng. Không đếm constructor/destructor ngầm do compiler sinh là hàm do project viết.

## Mục lục theo file

- [include/Application.hpp](#file-01)

- [include/Benchmark.hpp](#file-02)

- [include/Camera.hpp](#file-03)

- [include/CloudSystem.hpp](#file-04)

- [include/DynamicBatch.hpp](#file-05)

- [include/EnvironmentGeometry.hpp](#file-06)

- [include/Geometry.hpp](#file-07)

- [include/LightingSystem.hpp](#file-08)

- [include/LightningSystem.hpp](#file-09)

- [include/Material.hpp](#file-10)

- [include/Mesh.hpp](#file-11)

- [include/ModelGeometry.hpp](#file-12)

- [include/RainSystem.hpp](#file-13)

- [include/Renderer.hpp](#file-14)

- [include/RippleSystem.hpp](#file-15)

- [include/Scene.hpp](#file-16)

- [include/Shader.hpp](#file-17)

- [include/SplashSystem.hpp](#file-18)

- [include/Surface.hpp](#file-19)

- [include/TimeOfDaySystem.hpp](#file-20)

- [include/WeatherSystem.hpp](#file-21)

- [shaders/effects.frag](#file-22)

- [shaders/effects.vert](#file-23)

- [shaders/object.frag](#file-24)

- [shaders/object.vert](#file-25)

- [shaders/sky.frag](#file-26)

- [shaders/sky.vert](#file-27)

- [src/Application.cpp](#file-28)

- [src/Benchmark.cpp](#file-29)

- [src/Camera.cpp](#file-30)

- [src/CloudSystem.cpp](#file-31)

- [src/DynamicBatch.cpp](#file-32)

- [src/EnvironmentGeometry.cpp](#file-33)

- [src/Geometry.cpp](#file-34)

- [src/LightingSystem.cpp](#file-35)

- [src/LightningSystem.cpp](#file-36)

- [src/Mesh.cpp](#file-37)

- [src/ModelGeometry.cpp](#file-38)

- [src/RainSystem.cpp](#file-39)

- [src/Renderer.cpp](#file-40)

- [src/RippleSystem.cpp](#file-41)

- [src/Scene.cpp](#file-42)

- [src/Shader.cpp](#file-43)

- [src/SplashSystem.cpp](#file-44)

- [src/Surface.cpp](#file-45)

- [src/TimeOfDaySystem.cpp](#file-46)

- [src/WeatherSystem.cpp](#file-47)

- [src/main.cpp](#file-48)

- [tests/core_tests.cpp](#file-49)

- [tests/environment_tests.cpp](#file-50)

- [tests/lighting_time_tests.cpp](#file-51)

- [tests/model_tests.cpp](#file-52)

- [tests/weather_tests.cpp](#file-53)


## Quy ước dữ liệu và lỗi

Các vị trí/hướng dùng vec3; đơn vị thế giới không được code gắn đơn vị SI bắt buộc. Gravity 9.81 gợi liên hệ m/s² nhưng quy mô cảnh là quy ước. Vector cột và hệ +Y hướng lên, ground thuộc XZ. Mesh tĩnh lưu triangle list không index; array particle có capacity cố định, lifetime>0 đánh dấu splash/ripple sống. Không suy từ số vertex submit thành pixel nhìn thấy.

## Danh mục kiểu và biến quan trọng

| Kiểu | Loại/vị trí | Ý nghĩa, trạng thái ban đầu và nơi đọc/ghi |
|---|---|---|
| `RunOptions` | struct, [include/Application.hpp:11](../include/Application.hpp#L11) | State đầu vào CLI; smoke=false, paths rỗng, benchmarkSeconds=0, noFog=false, density 0.018, unlocked=false, hour 18, day 600, autoTime=false. main ghi; initialize/run/tick/smokeStep đọc. |
| `Application` | class, [include/Application.hpp:26](../include/Application.hpp#L26) | Owner cửa sổ và unique_ptr Renderer/Scene/Benchmark; singleton active_ route callback C. Member modules CPU có lifetime bằng app, GL resources chỉ được tạo sau context. |
| `Benchmark` | class, [include/Benchmark.hpp:9](../include/Benchmark.hpp#L9) | Bốn query IDs/pending flags, active_=-1; array 65536 interval, counts/sums=0; Clock points{}, RSS/pool counters 0, minute_=-1. beginFrame/endFrame ghi frame/query data, sample ghi RAM/pool, report đọc. |
| `Camera` | class, [include/Camera.hpp:5](../include/Camera.hpp#L5) | position/forward{} được constructor reset; up const(0, 1, 0), yaw−128°, pitch−24°, speed 5 world-unit/s, FOV 55°. reset/preset/move/rotate ghi, view/projection/getters đọc. |
| `CloudLobe` | struct, [include/CloudSystem.hpp:6](../include/CloudSystem.hpp#L6) | offset vec3 local; scale vec3 bán trục ellipsoid; shade float. Aggregate không có default initializer, Cloud reset gán hết. environment::cloud đọc offset/scale; shade hiện chưa được sử dụng. |
| `CloudCluster` | struct, [include/CloudSystem.hpp:12](../include/CloudSystem.hpp#L12) | position vec3 world, speed float hệ số gió, lobes array 5. Cloud reset gán bố cục, update chỉ ghi position; Lightning và Renderer đọc. |
| `CloudSystem` | class, [include/CloudSystem.hpp:19](../include/CloudSystem.hpp#L19) | COUNT=4; clusters_{} được reset trong constructor. CPU simulation không giữ Mesh hoặc OpenGL ID. |
| `EffectVertex` | struct, [include/DynamicBatch.hpp:8](../include/DynamicBatch.hpp#L8) | position vec3, color vec4, edge float default 0. Trong rain color.xyz là velocity, color.a opacity, edge tail length; các effect khác color là RGBA và bolt edge±1. Renderer ghi staging; DynamicBatch mô tả layout GPU. |
| `DynamicBatch` | class, [include/DynamicBatch.hpp:15](../include/DynamicBatch.hpp#L15) | vao_/vbo_=0, capacity_/primitive_ từ constructor, count_=0, uploads_=0. upload ghi count/VBO, draw đọc, destructor xóa GL IDs. Sở hữu độc quyền, copy bị delete. |
| `RockInstance` | struct, [include/EnvironmentGeometry.hpp:10](../include/EnvironmentGeometry.hpp#L10) | position/scale/color vec3, yaw float độ, seed uint32. rocks() gán ba aggregate constants; Scene đọc để tạo M, Renderer/grassAllowed dùng footprint. |
| `GrassTuft` | struct, [include/EnvironmentGeometry.hpp:15](../include/EnvironmentGeometry.hpp#L15) | position vec3 world, height float world-unit và angle float radian. grassTufts lambda gán seed 824; grass() đọc dựng lá. Không có animation state. |
| `Vertex` | struct, [include/Geometry.hpp:6](../include/Geometry.hpp#L6) | position vec3 local/world tùy builder, normal vec3. Aggregate không tự chuẩn hóa; helper geometry tạo normal. Mesh khai báo attr0/1 theo offsetof, shader biến đổi normal. |
| `LightingSystem` | class, [include/LightingSystem.hpp:6](../include/LightingSystem.hpp#L6) | flash 0, wetness 0.65, flashPosition(0, 8, 0); ambient/directional/direction/top/horizon{} rồi reset tính nền; daylight/windows 0; fogDensity public 0.018. setEnvironment ghi nền, update ghi flash/wetness, Renderer/getters đọc. AMBIENT 0.27/DIFFUSE 0.53 là constants legacy không được render hiện dùng; attenuation 0.025/0.018 đang upload. |
| `LightningSegment` | struct, [include/LightningSystem.hpp:8](../include/LightningSystem.hpp#L8) | start/end vec3 world, width float world-unit, branch bool. subdivide gán, trigger taper width, Renderer chuyển thành quads. |
| `LightningPulse` | struct, [include/LightningSystem.hpp:15](../include/LightningSystem.hpp#L15) | start/duration/peak float default 0; trigger ghi 1..3 pulse, flashIntensity đọc theo elapsed. start/duration tính giây, peak vô thứ nguyên. |
| `LightningSystem` | class, [include/LightningSystem.hpp:21](../include/LightningSystem.hpp#L21) | MAIN_DEPTH=5, MAIN_SEGMENTS=32, MAX_BRANCHES=6, MAX_SEGMENTS=128; segments{} RNG 6174, count/strike/revision 0, times 0, pulseCount 1, pulses{}, active false. constructor reset tăng revision và chọn countdown. trigger/cancel/update ghi; Renderer và tests đọc. |
| `MaterialKind` | enum class, [include/Material.hpp:4](../include/Material.hpp#L4) | enum class Plain 0, Plaster 1, RoofTile 2, Wood 3, Glass 4, Stone 5, Foliage 6, Brick 7, Metal 8, Rock 9, Grass 10. Renderer cast int vào uMaterialKind; object.frag dựa đúng các giá trị này. |
| `Material` | struct, [include/Material.hpp:6](../include/Material.hpp#L6) | kind=Plain; response vec4(1, 1, 0, 32) lần lượt ambient/diffuse multiplier, specular strength, shininess exponent; wetGloss 0. Scene gán theo part, Renderer upload. Không phải texture/PBR material. |
| `Mesh` | class, [include/Mesh.hpp:6](../include/Mesh.hpp#L6) | VAO/VBO IDs=0 và count 0 trước constructor; upload static vertex một lần. Không giữ EBO, indices hoặc vector CPU. Destructor do owner Scene/Renderer tự động gọi. |
| `HousePart` | enum class, [include/ModelGeometry.hpp:7](../include/ModelGeometry.hpp#L7) | enum class size_t: Walls 0, Stone 1, Roof 2, Trim 3, Door 4, Glass 5, Metal 6, Chimney 7, Cavity 8, Count 9. Chỉ số nhóm mesh nhà, khác MaterialKind; Scene ánh xạ sang material riêng. |
| `TreeParameters` | struct, [include/ModelGeometry.hpp:12](../include/ModelGeometry.hpp#L12) | height 3.8, trunkRadius 0.18, canopyWidth 1.25 world-unit, branchCount 8, seed 1 defaults. tree validate/rng đọc; treeLayout ghi các cấu hình riêng. |
| `Tree` | struct, [include/ModelGeometry.hpp:19](../include/ModelGeometry.hpp#L19) | bark và foliage vector<Vertex> ban đầu rỗng, tree() reserve/fill; Scene::modelPart upload thành hai Mesh/material; vector CPU giải phóng sau constructor Scene. |
| `TreeInstance` | struct, [include/ModelGeometry.hpp:23](../include/ModelGeometry.hpp#L23) | position vec3 world, shade float chỉ nhân màu foliage, geometry TreeParameters. treeLayout constants được Scene và placement/shading đọc. |
| `Raindrop` | struct, [include/RainSystem.hpp:8](../include/RainSystem.hpp#L8) | position/velocity vec3, length/lifetime/streakScale/opacity float; respawn gán hết. lifetime là tuổi đã sống giây, không phải thời gian còn lại; Rain update ghi; Renderer đọc position/velocity/length/opacity, không dùng tuổi để kill. |
| `RainSystem` | class, [include/RainSystem.hpp:17](../include/RainSystem.hpp#L17) | RAIN_COUNT=1000; drops/impacts arrays{}, RNG 1337, impact/recycled/revision 0. reset/respawn/update ghi; Weather đọc impactCount hợp lệ và renderer dùng revision để upload. |
| `Renderer` | class, [include/Renderer.hpp:9](../include/Renderer.hpp#L9) | Sở hữu ba Shader, sky Mesh, bốn cloud Mesh và bốn batch. Staging arrays: rain 2000, bolt 1536, splash 1024, ripple 1152 vertex; ring 12; revisions/counters 0. Uniform GLint được constructor lấy, không thể suy nghĩa 0 là location thiếu. draw ghi counters/cache, Application và smoke đọc. |
| `Ripple` | struct, [include/RippleSystem.hpp:7](../include/RippleSystem.hpp#L7) | center{}, radius 0.04, maxRadius 0.45, elapsed 0, lifetime 0, alpha 0 defaults. emit đặt lifetime 0.55; update tăng elapsed và tính alpha/radius, lifetime 0 đánh dấu chết. Khác Splash lifetime đếm lùi. |
| `RippleSystem` | class, [include/RippleSystem.hpp:16](../include/RippleSystem.hpp#L16) | CAPACITY 48, particles{}, RNG 76, active/cursor/revision/rejected 0. reset tăng revision; emit tìm slot chết, update giải phóng; Renderer chỉ đọc slot lifetime>0. |
| `SceneObject` | struct, [include/Scene.hpp:8](../include/Scene.hpp#L8) | mesh const Mesh* không sở hữu, model mat4, normal mat3, color vec3 do Scene::add gán. ground/puddle=false, material default, treeHeight 0; Scene constructor sửa metadata sau add. Renderer chỉ đọc; không có collider. |
| `Scene` | class, [include/Scene.hpp:19](../include/Scene.hpp#L19) | ground_/puddle_ Mesh owner trực tiếp; modelMeshes vector<unique_ptr<Mesh>> owner phần còn lại, objects vector SceneObject tham chiếu; animationTime 0. constructor/add/modelPart ghi danh sách, update/resetAnimation ghi thời gian. |
| `Shader` | class, [include/Shader.hpp:6](../include/Shader.hpp#L6) | program_ GLuint 0 trước link; constructor tạo program, uniform/use đọc, destructor xóa. Xóa copy tránh duplicate ownership; chưa có move API/hot reload. |
| `SplashParticle` | struct, [include/SplashSystem.hpp:8](../include/SplashSystem.hpp#L8) | position/velocity{}, lifetime 0, initialLifetime 1, length 0.07. emit gán position/velocity/remaining và initial lifetime; update giảm lifetime; renderer dùng remaining/initial làm alpha. |
| `SplashSystem` | class, [include/SplashSystem.hpp:16](../include/SplashSystem.hpp#L16) | MAX_SPLASHES=512, particles{}, RNG 2026, cursor/activeCount/revision/requests/rejected/filtered 0. reset tăng revision; emit ghi slot chết và thống kê, update giải phóng; Renderer/getters đọc. |
| `Puddle` | struct, [include/Surface.hpp:8](../include/Surface.hpp#L8) | center vec3 world với y 0.012, scale vec2 XZ, yaw float độ. surface::puddles constants gán; polygon đổi sang world, CPU waterMargin chỉ dùng XZ. |
| `TimeOfDaySystem` | class, [include/TimeOfDaySystem.hpp:4](../include/TimeOfDaySystem.hpp#L4) | DEFAULT_HOUR18 và DEFAULT_DAY_SECONDS600; hour/dayDuration double defaults, autoAdvance false. Không có constructor tường minh; default member initialization là đủ. setHour/duration/auto/update/reset ghi, Lighting đọc hour. |
| `WeatherSystem` | class, [include/WeatherSystem.hpp:10](../include/WeatherSystem.hpp#L10) | wind(0.65, 0, 0.2), observer(14, 10, 18); các module CPU là value members; rain/cloud/lightning/fog enabled=true, paused false, accumulator double 0. reset không ghi observer; Application cập nhật observer trước Weather update. |
| `FadingCloud` | struct, [src/Renderer.cpp:155](../src/Renderer.cpp#L155) | Struct local Renderer::draw: index size_t chỉ cached cloud mesh; depth float dot(center−camera, forward), tạo chỉ khi coverage ở(0, 1), comparator sort giảm depth. |


### Type alias và constants

| Alias/hằng | Vị trí | Cách dùng |
|---|---|---|
| Application::Clock | [include/Application.hpp:35](../include/Application.hpp#L35) | steady_clock dùng dt/timer/statistics, không phải giờ hệ điều hành |
| Benchmark::Clock | [include/Benchmark.hpp:20](../include/Benchmark.hpp#L20) | steady_clock đo interval/submit |
| models::House | [include/ModelGeometry.hpp:8](../include/ModelGeometry.hpp#L8) | array 9 vector<Vertex>, mỗi nhóm HousePart một batch |
| MeshData | [src/ModelGeometry.cpp:9](../src/ModelGeometry.cpp#L9) | vector<Vertex> alias nội bộ giúp ghép mesh nhà/cây |
| Point trong ba test | [tests/core_tests.cpp:21](../tests/core_tests.cpp#L21), [tests/model_tests.cpp:27](../tests/model_tests.cpp#L27), [tests/environment_tests.cpp:13](../tests/environment_tests.cpp#L13) | array<long, 3> quantize topology; ba alias độc lập theo translation unit |
| surface::OUTLINE_POINTS/GROUND_COLOR | [include/Surface.hpp:6](../include/Surface.hpp#L6) |32 mẫu polygon; RGB(0.23, 0.30, 0.26) thống nhất CPU/GPU |

Các capacity/seeds/lighting/time constants khác được ghi ở bảng kiểu phía trên và phần hàm ghi chúng. `constexpr step=1/120` tại WeatherSystem.cpp:33; `1e-7` là tolerance accumulator; field cloud dùng iso 0.18, slices 28, stacks 14 và 24 vòng bisection tại EnvironmentGeometry.cpp:143.

### Vòng đời state Application/Renderer

Application.hpp:64–89: keys_ array<bool, 256>{}; Clock points{}, statisticsFrames/max counters/window/mouse/frames/exit Code 0; width 800, height 600; dragging/exiting/smokeCompleted false, visible true. key/releaseInput ghi keys_; mouse handlers ghi mouseX/Y/dragging; initialize/close/destructor ghi window_; tick ghi lastTick_, updateStatistics ghi statistics mốc, display ghi frames/max primitives, fail/run ghi exit Code. RunOptions đã ghi đầy đủ trong bảng kiểu.

Renderer.hpp:23–58: mỗi uniform GLint lấy từ Shader::uniform trong constructor (không dùng giá trị mặc định); mỗi staging/revision/camera cache được draw ghi. Có ba shader, một sky Mesh, bốn cloud Mesh và bốn DynamicBatch persistent. Không có EBO hay render-to-texture FBO trong module. Các thành viên được hủy theo thứ tự ngược khai báo của C++; Application chủ động reset owner khi context sống.


<a id="file-01"></a>
## include/Application.hpp

**Module/chức năng:** Cửa sổ, vòng đời, input và GUI smoke.

**Headers/phụ thuộc trực tiếp:** `Renderer.hpp`, `Benchmark.hpp`, `array`, `chrono`, `memory`, `string`.

**Kiểu/dữ liệu quản lý:** `RunOptions`, `Application`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Application](#f001) | `const Application&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [Application& operator=](#f002) | `const Application&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |


<a id="f001"></a>
### Application

**Khai báo chính xác tại định nghĩa:**

```cpp
Application(const Application&) = delete
```

**Vị trí mã nguồn:** [include/Application.hpp:30](../include/Application.hpp#L30).

**Tham số:** `const Application&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f002"></a>
### Application& operator=

**Khai báo chính xác tại định nghĩa:**

```cpp
Application& operator=(const Application&) = delete
```

**Vị trí mã nguồn:** [include/Application.hpp:31](../include/Application.hpp#L31).

**Tham số:** `const Application&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 28 | `explicit Application(RunOptions options)` | [Application::Application](#f091) — [source src/Application.cpp:54](../src/Application.cpp#L54) |
| 32 | `int run(char* executableName)` | [Application::run](#f093) — [source src/Application.cpp:71](../src/Application.cpp#L71) |
| 36 | `void initialize(char* executableName)` | [Application::initialize](#f094) — [source src/Application.cpp:91](../src/Application.cpp#L91) |
| 39 | `void update(float deltaTime)` | [Application::update](#f095) — [source src/Application.cpp:159](../src/Application.cpp#L159) |
| 38 | `void tick()` | [Application::tick](#f096) — [source src/Application.cpp:170](../src/Application.cpp#L170) |
| 37 | `void display()` | [Application::display](#f097) — [source src/Application.cpp:206](../src/Application.cpp#L206) |
| 41 | `void reshape(int width, int height)` | [Application::reshape](#f098) — [source src/Application.cpp:225](../src/Application.cpp#L225) |
| 42 | `void key(unsigned char value, bool pressed)` | [Application::key](#f099) — [source src/Application.cpp:231](../src/Application.cpp#L231) |
| 51 | `void printTimeHelp() const` | [Application::printTimeHelp](#f100) — [source src/Application.cpp:264](../src/Application.cpp#L264) |
| 40 | `void updateStatistics(Clock::time_point now)` | [Application::updateStatistics](#f101) — [source src/Application.cpp:270](../src/Application.cpp#L270) |
| 43 | `void mouseButton(int button, int state, int x, int y)` | [Application::mouseButton](#f102) — [source src/Application.cpp:291](../src/Application.cpp#L291) |
| 44 | `void mouseMotion(int x, int y)` | [Application::mouseMotion](#f103) — [source src/Application.cpp:299](../src/Application.cpp#L299) |
| 45 | `void releaseInput()` | [Application::releaseInput](#f104) — [source src/Application.cpp:307](../src/Application.cpp#L307) |
| 46 | `void close()` | [Application::close](#f105) — [source src/Application.cpp:312](../src/Application.cpp#L312) |
| 47 | `void requestExit()` | [Application::requestExit](#f106) — [source src/Application.cpp:327](../src/Application.cpp#L327) |
| 48 | `void fail(const char* message)` | [Application::fail](#f107) — [source src/Application.cpp:339](../src/Application.cpp#L339) |
| 49 | `void smokeStep()` | [Application::smokeStep](#f109) — [source src/Application.cpp:394](../src/Application.cpp#L394) |
| 55 | `static void displayCallback()` | [Application::displayCallback](#f113) — [source src/Application.cpp:705](../src/Application.cpp#L705) |
| 56 | `static void timerCallback(int)` | [Application::timerCallback](#f114) — [source src/Application.cpp:709](../src/Application.cpp#L709) |
| 57 | `static void idleCallback()` | [Application::idleCallback](#f115) — [source src/Application.cpp:713](../src/Application.cpp#L713) |
| 58 | `static void reshapeCallback(int width, int height)` | [Application::reshapeCallback](#f116) — [source src/Application.cpp:714](../src/Application.cpp#L714) |
| 59 | `static void keyDownCallback(unsigned char key, int, int)` | [Application::keyDownCallback](#f117) — [source src/Application.cpp:715](../src/Application.cpp#L715) |
| 60 | `static void keyUpCallback(unsigned char key, int, int)` | [Application::keyUpCallback](#f118) — [source src/Application.cpp:719](../src/Application.cpp#L719) |
| 61 | `static void mouseButtonCallback(int button, int state, int x, int y)` | [Application::mouseButtonCallback](#f119) — [source src/Application.cpp:720](../src/Application.cpp#L720) |
| 62 | `static void mouseMotionCallback(int x, int y)` | [Application::mouseMotionCallback](#f120) — [source src/Application.cpp:721](../src/Application.cpp#L721) |
| 63 | `static void entryCallback(int state)` | [Application::entryCallback](#f121) — [source src/Application.cpp:722](../src/Application.cpp#L722) |
| 64 | `static void visibilityCallback(int state)` | [Application::visibilityCallback](#f122) — [source src/Application.cpp:723](../src/Application.cpp#L723) |
| 65 | `static void closeCallback()` | [Application::closeCallback](#f123) — [source src/Application.cpp:727](../src/Application.cpp#L727) |

<a id="file-02"></a>
## include/Benchmark.hpp

**Module/chức năng:** Đo CPU/GPU/RAM và thống kê frame.

**Headers/phụ thuộc trực tiếp:** `GL/glew.h`, `WeatherSystem.hpp`, `array`, `chrono`, `iosfwd`.

**Kiểu/dữ liệu quản lý:** `Benchmark`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Benchmark](#f003) | `const Benchmark&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [Benchmark& operator=](#f004) | `const Benchmark&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |


<a id="f003"></a>
### Benchmark

**Khai báo chính xác tại định nghĩa:**

```cpp
Benchmark(const Benchmark&) = delete
```

**Vị trí mã nguồn:** [include/Benchmark.hpp:13](../include/Benchmark.hpp#L13).

**Tham số:** `const Benchmark&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f004"></a>
### Benchmark& operator=

**Khai báo chính xác tại định nghĩa:**

```cpp
Benchmark& operator=(const Benchmark&) = delete
```

**Vị trí mã nguồn:** [include/Benchmark.hpp:14](../include/Benchmark.hpp#L14).

**Tham số:** `const Benchmark&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 11 | `explicit Benchmark(bool unlocked)` | [Benchmark::Benchmark](#f127) — [source src/Benchmark.cpp:36](../src/Benchmark.cpp#L36) |
| 15 | `void beginFrame()` | [Benchmark::beginFrame](#f129) — [source src/Benchmark.cpp:64](../src/Benchmark.cpp#L64) |
| 16 | `void endFrame()` | [Benchmark::endFrame](#f130) — [source src/Benchmark.cpp:89](../src/Benchmark.cpp#L89) |
| 17 | `void sample(const WeatherSystem& weather, double seconds, double fps)` | [Benchmark::sample](#f131) — [source src/Benchmark.cpp:99](../src/Benchmark.cpp#L99) |
| 18 | `void report(std::ostream& output, double seconds, int frames) const` | [Benchmark::report](#f132) — [source src/Benchmark.cpp:114](../src/Benchmark.cpp#L114) |

<a id="file-03"></a>
## include/Camera.hpp

**Module/chức năng:** Camera tự do và phép chiếu.

**Headers/phụ thuộc trực tiếp:** `glm/glm.hpp`.

**Kiểu/dữ liệu quản lý:** `Camera`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [position](#f005) | Không có. | `const glm::vec3&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả vị trí camera world-space; Application/Renderer và test đọc, chỉ Camera reset/preset/move ghi. | Application input/update, Renderer::draw, smoke và core_tests. |
| [forward](#f006) | Không có. | `const glm::vec3&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả hướng nhìn đơn vị; view/move/Renderer dùng, updateDirection ghi. | Application input/update, Renderer::draw, smoke và core_tests. |
| [pitch](#f007) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả góc ngẩng/cúi độ, clamp ±89; reset/preset/rotate ghi. | Application input/update, Renderer::draw, smoke và core_tests. |
| [yaw](#f008) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả góc quay ngang độ; reset/preset/rotate ghi. | Application input/update, Renderer::draw, smoke và core_tests. |


<a id="f005"></a>
### position

**Khai báo chính xác tại định nghĩa:**

```cpp
const glm::vec3& position() const
```

**Vị trí mã nguồn:** [include/Camera.hpp:15](../include/Camera.hpp#L15).

**Tham số:** Không có.

**Giá trị trả về:** `const glm::vec3&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả vị trí camera world-space; Application/Renderer và test đọc, chỉ Camera reset/preset/move ghi. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f006"></a>
### forward

**Khai báo chính xác tại định nghĩa:**

```cpp
const glm::vec3& forward() const
```

**Vị trí mã nguồn:** [include/Camera.hpp:16](../include/Camera.hpp#L16).

**Tham số:** Không có.

**Giá trị trả về:** `const glm::vec3&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả hướng nhìn đơn vị; view/move/Renderer dùng, updateDirection ghi. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f007"></a>
### pitch

**Khai báo chính xác tại định nghĩa:**

```cpp
float pitch() const
```

**Vị trí mã nguồn:** [include/Camera.hpp:17](../include/Camera.hpp#L17).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả góc ngẩng/cúi độ, clamp ±89; reset/preset/rotate ghi. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f008"></a>
### yaw

**Khai báo chính xác tại định nghĩa:**

```cpp
float yaw() const
```

**Vị trí mã nguồn:** [include/Camera.hpp:18](../include/Camera.hpp#L18).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả góc quay ngang độ; reset/preset/rotate ghi. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 7 | `Camera()` | [Camera::Camera](#f134) — [source src/Camera.cpp:7](../src/Camera.cpp#L7) |
| 8 | `void reset()` | [Camera::reset](#f135) — [source src/Camera.cpp:9](../src/Camera.cpp#L9) |
| 9 | `void preset(unsigned index)` | [Camera::preset](#f136) — [source src/Camera.cpp:16](../src/Camera.cpp#L16) |
| 21 | `void updateDirection()` | [Camera::updateDirection](#f137) — [source src/Camera.cpp:27](../src/Camera.cpp#L27) |
| 12 | `void rotate(float deltaX, float deltaY)` | [Camera::rotate](#f138) — [source src/Camera.cpp:35](../src/Camera.cpp#L35) |
| 11 | `void move(const glm::vec3& localDirection, float deltaTime)` | [Camera::move](#f139) — [source src/Camera.cpp:41](../src/Camera.cpp#L41) |
| 13 | `glm::mat4 view() const` | [Camera::view](#f140) — [source src/Camera.cpp:55](../src/Camera.cpp#L55) |
| 14 | `glm::mat4 projection(float aspect) const` | [Camera::projection](#f141) — [source src/Camera.cpp:59](../src/Camera.cpp#L59) |

<a id="file-04"></a>
## include/CloudSystem.hpp

**Module/chức năng:** Dữ liệu và chuyển động bốn cụm mây.

**Headers/phụ thuộc trực tiếp:** `array`, `glm/glm.hpp`.

**Kiểu/dữ liệu quản lý:** `CloudLobe`, `CloudCluster`, `CloudSystem`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [CloudSystem](#f009) | Không có. | Constructor/destructor: không có kiểu trả về. | Constructor gọi reset để seed/pool/state của module nhất quán. | WeatherSystem, LightningSystem, Renderer và CPU tests. |
| [clusters](#f010) | Không có. | `const std::array<CloudCluster, COUNT>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả array 4 cluster/lobes; Cloud reset/update ghi, Renderer/Lightning/geometry/test đọc. | WeatherSystem, LightningSystem, Renderer và CPU tests. |


<a id="f009"></a>
### CloudSystem

**Khai báo chính xác tại định nghĩa:**

```cpp
CloudSystem()
```

**Vị trí mã nguồn:** [include/CloudSystem.hpp:22](../include/CloudSystem.hpp#L22).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Constructor gọi reset để seed/pool/state của module nhất quán. Các default member initializer chạy trước thân constructor; chưa tạo tài nguyên GPU trong module mô phỏng này.

**Quan hệ — nơi gọi:** WeatherSystem, LightningSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `reset`.


<a id="f010"></a>
### clusters

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<CloudCluster, COUNT>& clusters() const
```

**Vị trí mã nguồn:** [include/CloudSystem.hpp:25](../include/CloudSystem.hpp#L25).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<CloudCluster, COUNT>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả array 4 cluster/lobes; Cloud reset/update ghi, Renderer/Lightning/geometry/test đọc. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** WeatherSystem, LightningSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 23 | `void reset()` | [CloudSystem::reset](#f143) — [source src/CloudSystem.cpp:12](../src/CloudSystem.cpp#L12) |
| 24 | `void update(float deltaTime, const glm::vec3& windVelocity)` | [CloudSystem::update](#f144) — [source src/CloudSystem.cpp:34](../src/CloudSystem.cpp#L34) |
| 26 | `static float coverage(glm::vec3 center)` | [CloudSystem::coverage](#f145) — [source src/CloudSystem.cpp:44](../src/CloudSystem.cpp#L44) |

<a id="file-05"></a>
## include/DynamicBatch.hpp

**Module/chức năng:** VAO/VBO động cho các hiệu ứng.

**Headers/phụ thuộc trực tiếp:** `GL/glew.h`, `glm/glm.hpp`, `cstddef`, `cstdint`.

**Kiểu/dữ liệu quản lý:** `EffectVertex`, `DynamicBatch`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [DynamicBatch](#f011) | `const DynamicBatch&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [DynamicBatch& operator=](#f012) | `const DynamicBatch&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [vertexCount](#f013) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả count_ đổi GLsizei sang size_t; là số vertex upload gần nhất của DynamicBatch. | Renderer constructor/draw; destructor khi Renderer bị hủy. |
| [uploadCount](#f014) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả uploads_ là số lần glBufferSubData có count>0 từ khi dựng batch. | Renderer constructor/draw; destructor khi Renderer bị hủy. |


<a id="f011"></a>
### DynamicBatch

**Khai báo chính xác tại định nghĩa:**

```cpp
DynamicBatch(const DynamicBatch&) = delete
```

**Vị trí mã nguồn:** [include/DynamicBatch.hpp:19](../include/DynamicBatch.hpp#L19).

**Tham số:** `const DynamicBatch&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f012"></a>
### DynamicBatch& operator=

**Khai báo chính xác tại định nghĩa:**

```cpp
DynamicBatch& operator=(const DynamicBatch&) = delete
```

**Vị trí mã nguồn:** [include/DynamicBatch.hpp:20](../include/DynamicBatch.hpp#L20).

**Tham số:** `const DynamicBatch&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f013"></a>
### vertexCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t vertexCount() const
```

**Vị trí mã nguồn:** [include/DynamicBatch.hpp:23](../include/DynamicBatch.hpp#L23).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả count_ đổi GLsizei sang size_t; là số vertex upload gần nhất của DynamicBatch. GL_LINES chia 2 mới ra số hạt/segment; GL_TRIANGLES chia 3 mới ra tam giác.

**Quan hệ — nơi gọi:** Renderer constructor/draw; destructor khi Renderer bị hủy.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f014"></a>
### uploadCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t uploadCount() const
```

**Vị trí mã nguồn:** [include/DynamicBatch.hpp:24](../include/DynamicBatch.hpp#L24).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả uploads_ là số lần glBufferSubData có count>0 từ khi dựng batch. upload 0 không tăng; dùng test cache, không phải byte count.

**Quan hệ — nơi gọi:** Renderer constructor/draw; destructor khi Renderer bị hủy.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 17 | `DynamicBatch(std::size_t capacity, GLenum primitive)` | [DynamicBatch::DynamicBatch](#f146) — [source src/DynamicBatch.cpp:7](../src/DynamicBatch.cpp#L7) |
| 21 | `void upload(const EffectVertex* vertices, std::size_t count)` | [DynamicBatch::upload](#f148) — [source src/DynamicBatch.cpp:36](../src/DynamicBatch.cpp#L36) |
| 22 | `void draw() const` | [DynamicBatch::draw](#f149) — [source src/DynamicBatch.cpp:46](../src/DynamicBatch.cpp#L46) |

<a id="file-06"></a>
## include/EnvironmentGeometry.hpp

**Module/chức năng:** Sinh mesh nước/đá/mây/cỏ hiện tại.

**Headers/phụ thuộc trực tiếp:** `Geometry.hpp`, `CloudSystem.hpp`, `cstdint`.

**Kiểu/dữ liệu quản lý:** `RockInstance`, `GrassTuft`; xem bảng trạng thái phía trên.

Không có thân hàm trong file này. Các prototype được thực thi ở file .cpp cùng module; riêng Material chỉ có enum/aggregate.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 16 | `bool grassAllowed(glm::vec2 position)` | [grassAllowed](#f151) — [source src/EnvironmentGeometry.cpp:19](../src/EnvironmentGeometry.cpp#L19) |
| 17 | `const std::array<GrassTuft,64>& grassTufts()` | [grassTufts](#f152) — [source src/EnvironmentGeometry.cpp:33](../src/EnvironmentGeometry.cpp#L33) |
| 18 | `std::vector<Vertex> grass()` | [grass](#f154) — [source src/EnvironmentGeometry.cpp:50](../src/EnvironmentGeometry.cpp#L50) |
| 8 | `std::vector<Vertex> puddles()` | [puddles](#f155) — [source src/EnvironmentGeometry.cpp:65](../src/EnvironmentGeometry.cpp#L65) |
| 11 | `const std::array<RockInstance,3>& rocks()` | [rocks](#f156) — [source src/EnvironmentGeometry.cpp:79](../src/EnvironmentGeometry.cpp#L79) |
| 9 | `std::vector<Vertex> rock(std::uint32_t seed)` | [rock](#f157) — [source src/EnvironmentGeometry.cpp:88](../src/EnvironmentGeometry.cpp#L88) |
| 14 | `std::vector<Vertex> cloud(const CloudCluster& cluster)` | [cloud](#f160) — [source src/EnvironmentGeometry.cpp:121](../src/EnvironmentGeometry.cpp#L121) |

<a id="file-07"></a>
## include/Geometry.hpp

**Module/chức năng:** Primitive triangle lists cơ sở.

**Headers/phụ thuộc trực tiếp:** `glm/glm.hpp`, `vector`.

**Kiểu/dữ liệu quản lý:** `Vertex`; xem bảng trạng thái phía trên.

Không có thân hàm trong file này. Các prototype được thực thi ở file .cpp cùng module; riêng Material chỉ có enum/aggregate.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 20 | `std::vector<Vertex> screenTriangle()` | [screenTriangle](#f165) — [source src/Geometry.cpp:23](../src/Geometry.cpp#L23) |
| 19 | `std::vector<Vertex> puddle()` | [puddle](#f166) — [source src/Geometry.cpp:26](../src/Geometry.cpp#L26) |
| 13 | `std::vector<Vertex> ground()` | [ground](#f167) — [source src/Geometry.cpp:36](../src/Geometry.cpp#L36) |
| 14 | `std::vector<Vertex> box()` | [box](#f168) — [source src/Geometry.cpp:43](../src/Geometry.cpp#L43) |
| 15 | `std::vector<Vertex> roof()` | [roof](#f169) — [source src/Geometry.cpp:56](../src/Geometry.cpp#L56) |
| 16 | `std::vector<Vertex> cone(unsigned segments = 8)` | [cone](#f170) — [source src/Geometry.cpp:69](../src/Geometry.cpp#L69) |
| 17 | `std::vector<Vertex> rock()` | [rock](#f171) — [source src/Geometry.cpp:87](../src/Geometry.cpp#L87) |
| 18 | `std::vector<Vertex> sphere(unsigned slices = 10, unsigned stacks = 6)` | [sphere](#f172) — [source src/Geometry.cpp:98](../src/Geometry.cpp#L98) |

<a id="file-08"></a>
## include/LightingSystem.hpp

**Module/chức năng:** Ánh sáng màu, flash, fog và wetness.

**Headers/phụ thuộc trực tiếp:** `glm/glm.hpp`, `TimeOfDaySystem.hpp`.

**Kiểu/dữ liệu quản lý:** `LightingSystem`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [LightingSystem](#f015) | Không có. | Constructor/destructor: không có kiểu trả về. | Constructor gọi reset để seed/pool/state của module nhất quán. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [ambientColor](#f016) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả RGB ambient nền đã pha ngày/đêm/storm bởi setEnvironment. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [directionalColor](#f017) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả RGB nguồn định hướng nền từ setEnvironment. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [lightDirection](#f018) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả hướng truyền sáng đơn vị; shader dùng normalize(−direction) để lấy hướng từ mặt tới nguồn. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [windowStrength](#f019) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả cường độ emission cửa sổ [0, 1] từ daylight. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [daylight](#f020) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả hệ số ban ngày từ dawn/dusk. | Không tìm thấy lời gọi daylight() trong src/ hoặc tests/ hiện tại; đây là accessor dự phòng. |
| [flash](#f021) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả cường độ xung [0, 1] được Lighting update ghi. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [setFlashPosition](#f022) | `glm::vec3 position`: tọa độ world hoặc local theo caller. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Ghi position vào flashPosition_ theo world-space; không clamp hoặc kiểm tra finite. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [flashPosition](#f023) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả world position đầu bolt làm nguồn điểm, Weather setFlashPosition ghi. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [wetness](#f024) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả độ ướt toàn cảnh [0, 1], reset 0.65, update tăng/giảm theo rain. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [groundMultiplier](#f025) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả 1−wetness_×0.24 (0.844 khi wetness 0.65). | weather_tests.cpp::integrationTests; pipeline GPU không gọi helper groundMultiplier. |


<a id="f015"></a>
### LightingSystem

**Khai báo chính xác tại định nghĩa:**

```cpp
LightingSystem()
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:13](../include/LightingSystem.hpp#L13).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Constructor gọi reset để seed/pool/state của module nhất quán. Các default member initializer chạy trước thân constructor; chưa tạo tài nguyên GPU trong module mô phỏng này.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `reset`.


<a id="f016"></a>
### ambientColor

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 ambientColor() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:17](../include/LightingSystem.hpp#L17).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả RGB ambient nền đã pha ngày/đêm/storm bởi setEnvironment. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f017"></a>
### directionalColor

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 directionalColor() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:18](../include/LightingSystem.hpp#L18).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả RGB nguồn định hướng nền từ setEnvironment. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f018"></a>
### lightDirection

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 lightDirection() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:19](../include/LightingSystem.hpp#L19).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả hướng truyền sáng đơn vị; shader dùng normalize(−direction) để lấy hướng từ mặt tới nguồn. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f019"></a>
### windowStrength

**Khai báo chính xác tại định nghĩa:**

```cpp
float windowStrength() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:21](../include/LightingSystem.hpp#L21).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả cường độ emission cửa sổ [0, 1] từ daylight. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f020"></a>
### daylight

**Khai báo chính xác tại định nghĩa:**

```cpp
float daylight() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:22](../include/LightingSystem.hpp#L22).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả hệ số ban ngày từ dawn/dusk. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Không tìm thấy lời gọi daylight() trong src/ hoặc tests/ hiện tại; đây là accessor dự phòng.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f021"></a>
### flash

**Khai báo chính xác tại định nghĩa:**

```cpp
float flash() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:24](../include/LightingSystem.hpp#L24).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả cường độ xung [0, 1] được Lighting update ghi. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f022"></a>
### setFlashPosition

**Khai báo chính xác tại định nghĩa:**

```cpp
void setFlashPosition(glm::vec3 position)
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:25](../include/LightingSystem.hpp#L25).

**Tham số:** `glm::vec3 position`: tọa độ world hoặc local theo caller.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Ghi position vào flashPosition_ theo world-space; không clamp hoặc kiểm tra finite. Weather truyền đầu lightning segment, shader dùng làm nguồn điểm.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f023"></a>
### flashPosition

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 flashPosition() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:26](../include/LightingSystem.hpp#L26).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả world position đầu bolt làm nguồn điểm, Weather setFlashPosition ghi. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f024"></a>
### wetness

**Khai báo chính xác tại định nghĩa:**

```cpp
float wetness() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:33](../include/LightingSystem.hpp#L33).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả độ ướt toàn cảnh [0, 1], reset 0.65, update tăng/giảm theo rain. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f025"></a>
### groundMultiplier

**Khai báo chính xác tại định nghĩa:**

```cpp
float groundMultiplier() const
```

**Vị trí mã nguồn:** [include/LightingSystem.hpp:34](../include/LightingSystem.hpp#L34).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả 1−wetness_×0.24 (0.844 khi wetness 0.65). Helper CPU được weather test dùng; object shader hiện có moisture/path/localWet riêng, không gọi trực tiếp helper này.

**Quan hệ — nơi gọi:** weather_tests.cpp::integrationTests; pipeline GPU không gọi helper groundMultiplier.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 14 | `void reset()` | [LightingSystem::reset](#f174) — [source src/LightingSystem.cpp:6](../src/LightingSystem.cpp#L6) |
| 16 | `void setEnvironment(float hour, float storminess)` | [LightingSystem::setEnvironment](#f175) — [source src/LightingSystem.cpp:14](../src/LightingSystem.cpp#L14) |
| 20 | `glm::vec3 particleColor() const` | [LightingSystem::particleColor](#f177) — [source src/LightingSystem.cpp:42](../src/LightingSystem.cpp#L42) |
| 23 | `void update(float flashIntensity, float deltaTime = 0, bool raining = true)` | [LightingSystem::update](#f178) — [source src/LightingSystem.cpp:46](../src/LightingSystem.cpp#L46) |
| 28 | `glm::vec3 skyColor() const` | [LightingSystem::skyColor](#f179) — [source src/LightingSystem.cpp:54](../src/LightingSystem.cpp#L54) |
| 27 | `float attenuationAt(glm::vec3 point) const` | [LightingSystem::attenuationAt](#f180) — [source src/LightingSystem.cpp:58](../src/LightingSystem.cpp#L58) |
| 29 | `glm::vec3 skyTop() const` | [LightingSystem::skyTop](#f181) — [source src/LightingSystem.cpp:63](../src/LightingSystem.cpp#L63) |
| 30 | `glm::vec3 fogColor() const` | [LightingSystem::fogColor](#f182) — [source src/LightingSystem.cpp:67](../src/LightingSystem.cpp#L67) |
| 31 | `float fogFactor(float distance) const` | [LightingSystem::fogFactor](#f183) — [source src/LightingSystem.cpp:71](../src/LightingSystem.cpp#L71) |

<a id="file-09"></a>
## include/LightningSystem.hpp

**Module/chức năng:** Polyline sét đệ quy, nhánh và xung.

**Headers/phụ thuộc trực tiếp:** `CloudSystem.hpp`, `array`, `cstdint`, `random`.

**Kiểu/dữ liệu quản lý:** `LightningSegment`, `LightningPulse`, `LightningSystem`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [LightningSystem](#f026) | Không có. | Constructor/destructor: không có kiểu trả về. | Constructor gọi reset để seed/pool/state của module nhất quán. | WeatherSystem, Renderer, smoke và weather_tests. |
| [active](#f027) | Không có. | `bool`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả trạng thái strike active (Lightning) hoặc số ripple đang sống (Ripple), phân biệt theo kiểu trong signature. | WeatherSystem, Renderer, smoke và weather_tests. |
| [duration](#f028) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả tổng đời strike theo giây 0.28..0.42. | Không tìm thấy lời gọi duration() trong src/ hoặc tests/ hiện tại; duration_ vẫn được update/flashIntensity sử dụng nội bộ. |
| [elapsed](#f029) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả tuổi strike theo giây, update ghi. | WeatherSystem, Renderer, smoke và weather_tests. |
| [nextStrike](#f030) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả thời gian chờ strike tiếp theo 2..6 giây, trigger/reset/update ghi. | WeatherSystem, Renderer, smoke và weather_tests. |
| [strikeCount](#f031) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả tổng strike từ reset, trigger tăng. | WeatherSystem, Renderer, smoke và weather_tests. |
| [revision](#f032) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. | WeatherSystem, Renderer, smoke và weather_tests. |
| [pulseCount](#f033) | Không có. | `unsigned`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số pulse 1..3 trong strike gần nhất. | WeatherSystem, Renderer, smoke và weather_tests. |
| [pulses](#f034) | Không có. | `const std::array<LightningPulse, 3>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả array 3 pulse, chỉ pulseCount phần tử đầu có hiệu lực. | weather_tests.cpp::lightningTests (xem pulse gap và peak); Renderer đọc flashIntensity chứ không đọc pulses trực tiếp. |
| [lightPosition](#f035) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Nếu segmentCount_>0 trả segments_[0].start, ngược lại (0, 8, 0). | WeatherSystem, Renderer, smoke và weather_tests. |
| [segments](#f036) | Không có. | `const std::array<LightningSegment, MAX_SEGMENTS>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả array 128 segment, chỉ segmentCount phần tử đầu hợp lệ. | WeatherSystem, Renderer, smoke và weather_tests. |
| [segmentCount](#f037) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số đoạn đang lưu; trigger hiện tạo 80, cancel/reset đặt 0. | WeatherSystem, Renderer, smoke và weather_tests. |


<a id="f026"></a>
### LightningSystem

**Khai báo chính xác tại định nghĩa:**

```cpp
LightningSystem()
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:27](../include/LightningSystem.hpp#L27).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Constructor gọi reset để seed/pool/state của module nhất quán. Các default member initializer chạy trước thân constructor; chưa tạo tài nguyên GPU trong module mô phỏng này.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `reset`.


<a id="f027"></a>
### active

**Khai báo chính xác tại định nghĩa:**

```cpp
bool active() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:34](../include/LightningSystem.hpp#L34).

**Tham số:** Không có.

**Giá trị trả về:** `bool`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả trạng thái strike active (Lightning) hoặc số ripple đang sống (Ripple), phân biệt theo kiểu trong signature. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f028"></a>
### duration

**Khai báo chính xác tại định nghĩa:**

```cpp
float duration() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:35](../include/LightningSystem.hpp#L35).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả tổng đời strike theo giây 0.28..0.42. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Không tìm thấy lời gọi duration() trong src/ hoặc tests/ hiện tại; duration_ vẫn được update/flashIntensity sử dụng nội bộ.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f029"></a>
### elapsed

**Khai báo chính xác tại định nghĩa:**

```cpp
float elapsed() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:36](../include/LightningSystem.hpp#L36).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả tuổi strike theo giây, update ghi. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f030"></a>
### nextStrike

**Khai báo chính xác tại định nghĩa:**

```cpp
float nextStrike() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:37](../include/LightningSystem.hpp#L37).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả thời gian chờ strike tiếp theo 2..6 giây, trigger/reset/update ghi. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f031"></a>
### strikeCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t strikeCount() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:38](../include/LightningSystem.hpp#L38).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả tổng strike từ reset, trigger tăng. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f032"></a>
### revision

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t revision() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:39](../include/LightningSystem.hpp#L39).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f033"></a>
### pulseCount

**Khai báo chính xác tại định nghĩa:**

```cpp
unsigned pulseCount() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:40](../include/LightningSystem.hpp#L40).

**Tham số:** Không có.

**Giá trị trả về:** `unsigned`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số pulse 1..3 trong strike gần nhất. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f034"></a>
### pulses

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<LightningPulse, 3>& pulses() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:41](../include/LightningSystem.hpp#L41).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<LightningPulse, 3>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả array 3 pulse, chỉ pulseCount phần tử đầu có hiệu lực. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** weather_tests.cpp::lightningTests (xem pulse gap và peak); Renderer đọc flashIntensity chứ không đọc pulses trực tiếp.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f035"></a>
### lightPosition

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 lightPosition() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:42](../include/LightningSystem.hpp#L42).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu segmentCount_>0 trả segments_[0].start, ngược lại (0, 8, 0). Đây là vị trí nguồn điểm đại diện cả bolt; không tích phân ánh sáng dọc các đoạn.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `glm::vec3`.


<a id="f036"></a>
### segments

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<LightningSegment, MAX_SEGMENTS>& segments() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:45](../include/LightningSystem.hpp#L45).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<LightningSegment, MAX_SEGMENTS>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả array 128 segment, chỉ segmentCount phần tử đầu hợp lệ. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f037"></a>
### segmentCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t segmentCount() const
```

**Vị trí mã nguồn:** [include/LightningSystem.hpp:46](../include/LightningSystem.hpp#L46).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số đoạn đang lưu; trigger hiện tạo 80, cancel/reset đặt 0. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 49 | `float random(float low, float high)` | [LightningSystem::random](#f184) — [source src/LightningSystem.cpp:6](../src/LightningSystem.cpp#L6) |
| 28 | `void reset(std::uint32_t seed = 6174)` | [LightningSystem::reset](#f185) — [source src/LightningSystem.cpp:10](../src/LightningSystem.cpp#L10) |
| 30 | `void cancel()` | [LightningSystem::cancel](#f186) — [source src/LightningSystem.cpp:20](../src/LightningSystem.cpp#L20) |
| 50 | `void subdivide(glm::vec3 start, glm::vec3 end, unsigned depth, float displacement, float width, bool branch)` | [LightningSystem::subdivide](#f187) — [source src/LightningSystem.cpp:26](../src/LightningSystem.cpp#L26) |
| 33 | `float flashIntensity() const` | [LightningSystem::flashIntensity](#f190) — [source src/LightningSystem.cpp:115](../src/LightningSystem.cpp#L115) |

<a id="file-10"></a>
## include/Material.hpp

**Module/chức năng:** Phân loại và hệ số vật liệu.

**Headers/phụ thuộc trực tiếp:** `glm/glm.hpp`.

**Kiểu/dữ liệu quản lý:** `MaterialKind`, `Material`; xem bảng trạng thái phía trên.

Không có thân hàm trong file này. Các prototype được thực thi ở file .cpp cùng module; riêng Material chỉ có enum/aggregate.


<a id="file-11"></a>
## include/Mesh.hpp

**Module/chức năng:** Tài nguyên GPU của mesh tĩnh.

**Headers/phụ thuộc trực tiếp:** `Geometry.hpp`, `GL/glew.h`.

**Kiểu/dữ liệu quản lý:** `Mesh`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Mesh](#f038) | `const Mesh&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [Mesh& operator=](#f039) | `const Mesh&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [triangleCount](#f040) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả static_cast<size_t>(count_)/3 vì Mesh lưu triangle list với 3 vertex/tam giác. | Scene/Renderer constructors/draw và RAII cleanup. |


<a id="f038"></a>
### Mesh

**Khai báo chính xác tại định nghĩa:**

```cpp
Mesh(const Mesh&) = delete
```

**Vị trí mã nguồn:** [include/Mesh.hpp:10](../include/Mesh.hpp#L10).

**Tham số:** `const Mesh&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f039"></a>
### Mesh& operator=

**Khai báo chính xác tại định nghĩa:**

```cpp
Mesh& operator=(const Mesh&) = delete
```

**Vị trí mã nguồn:** [include/Mesh.hpp:11](../include/Mesh.hpp#L11).

**Tham số:** `const Mesh&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f040"></a>
### triangleCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t triangleCount() const
```

**Vị trí mã nguồn:** [include/Mesh.hpp:13](../include/Mesh.hpp#L13).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả static_cast<size_t>(count_)/3 vì Mesh lưu triangle list với 3 vertex/tam giác. Không đếm unique vertices hoặc triangles thấy trên màn hình.

**Quan hệ — nơi gọi:** Scene/Renderer constructors/draw và RAII cleanup.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 8 | `explicit Mesh(const std::vector<Vertex>& vertices)` | [Mesh::Mesh](#f191) — [source src/Mesh.cpp:8](../src/Mesh.cpp#L8) |
| 12 | `void draw() const` | [Mesh::draw](#f193) — [source src/Mesh.cpp:36](../src/Mesh.cpp#L36) |

<a id="file-12"></a>
## include/ModelGeometry.hpp

**Module/chức năng:** Mô hình nhà/cây procedural.

**Headers/phụ thuộc trực tiếp:** `Geometry.hpp`, `array`, `cstdint`.

**Kiểu/dữ liệu quản lý:** `HousePart`, `TreeParameters`, `Tree`, `TreeInstance`; xem bảng trạng thái phía trên.

Không có thân hàm trong file này. Các prototype được thực thi ở file .cpp cùng module; riêng Material chỉ có enum/aggregate.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 10 | `House house()` | [house](#f203) — [source src/ModelGeometry.cpp:79](../src/ModelGeometry.cpp#L79) |
| 24 | `const std::array<TreeInstance, 4>& treeLayout()` | [treeLayout](#f206) — [source src/ModelGeometry.cpp:130](../src/ModelGeometry.cpp#L130) |
| 25 | `Tree tree(const TreeParameters& parameters)` | [tree](#f207) — [source src/ModelGeometry.cpp:140](../src/ModelGeometry.cpp#L140) |

<a id="file-13"></a>
## include/RainSystem.hpp

**Module/chức năng:** Pool 1000 giọt và ground impacts.

**Headers/phụ thuộc trực tiếp:** `array`, `cstdint`, `glm/glm.hpp`, `random`.

**Kiểu/dữ liệu quản lý:** `Raindrop`, `RainSystem`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [RainSystem](#f041) | Không có. | Constructor/destructor: không có kiểu trả về. | Constructor gọi reset để seed/pool/state của module nhất quán. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [drops](#f042) | Không có. | `const std::array<Raindrop, RAIN_COUNT>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả array 1000 giọt sống, Rain reset/update ghi. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [impacts](#f043) | Không có. | `const std::array<glm::vec3, RAIN_COUNT>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả array 1000 hit; chỉ impactCount phần tử đầu thuộc lần Rain update mới nhất. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [impactCount](#f044) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số impact hợp lệ lần update rain gần nhất; bị xóa đầu mỗi update. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [recycledCount](#f045) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số drop tái sinh vì đất/biên từ reset. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [revision](#f046) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. | WeatherSystem::reset/update, Renderer và weather_tests. |


<a id="f041"></a>
### RainSystem

**Khai báo chính xác tại định nghĩa:**

```cpp
RainSystem()
```

**Vị trí mã nguồn:** [include/RainSystem.hpp:20](../include/RainSystem.hpp#L20).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Constructor gọi reset để seed/pool/state của module nhất quán. Các default member initializer chạy trước thân constructor; chưa tạo tài nguyên GPU trong module mô phỏng này.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `reset`.


<a id="f042"></a>
### drops

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<Raindrop, RAIN_COUNT>& drops() const
```

**Vị trí mã nguồn:** [include/RainSystem.hpp:23](../include/RainSystem.hpp#L23).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<Raindrop, RAIN_COUNT>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả array 1000 giọt sống, Rain reset/update ghi. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f043"></a>
### impacts

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<glm::vec3, RAIN_COUNT>& impacts() const
```

**Vị trí mã nguồn:** [include/RainSystem.hpp:24](../include/RainSystem.hpp#L24).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<glm::vec3, RAIN_COUNT>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả array 1000 hit; chỉ impactCount phần tử đầu thuộc lần Rain update mới nhất. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f044"></a>
### impactCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t impactCount() const
```

**Vị trí mã nguồn:** [include/RainSystem.hpp:25](../include/RainSystem.hpp#L25).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số impact hợp lệ lần update rain gần nhất; bị xóa đầu mỗi update. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f045"></a>
### recycledCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t recycledCount() const
```

**Vị trí mã nguồn:** [include/RainSystem.hpp:26](../include/RainSystem.hpp#L26).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số drop tái sinh vì đất/biên từ reset. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f046"></a>
### revision

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t revision() const
```

**Vị trí mã nguồn:** [include/RainSystem.hpp:27](../include/RainSystem.hpp#L27).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 30 | `float random(float low, float high)` | [RainSystem::random](#f209) — [source src/RainSystem.cpp:6](../src/RainSystem.cpp#L6) |
| 31 | `void respawn(Raindrop& drop, const glm::vec3& windVelocity, bool initial)` | [RainSystem::respawn](#f210) — [source src/RainSystem.cpp:10](../src/RainSystem.cpp#L10) |
| 22 | `void update(float deltaTime, const glm::vec3& windVelocity)` | [RainSystem::update](#f212) — [source src/RainSystem.cpp:28](../src/RainSystem.cpp#L28) |

<a id="file-14"></a>
## include/Renderer.hpp

**Module/chức năng:** Render passes, uniforms và dynamic staging.

**Headers/phụ thuộc trực tiếp:** `Camera.hpp`, `Scene.hpp`, `Shader.hpp`, `WeatherSystem.hpp`, `DynamicBatch.hpp`.

**Kiểu/dữ liệu quản lý:** `Renderer`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [drawCalls](#f047) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số glDrawArrays submit trong lầnRenderer draw gần nhất. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [renderedRainCount](#f048) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số rain streak submit, chưa trừ clipping/depth occlusion. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [renderedSplashCount](#f049) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số splash streak submit từ slot sống. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [lightningUploadCount](#f050) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Ủy quyền tới lightningBatch_.uploadCount(); trả số upload thực tế để smoke xác nhận cache không ghi lại VBO khi chỉ đổi intensity/hour.. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [rainUploadCount](#f051) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Ủy quyền tới rainBatch_.uploadCount(); trả số upload thực tế để smoke xác nhận cache không ghi lại VBO khi chỉ đổi intensity/hour.. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [triangles](#f052) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số triangles submit, gồm sky/scene/cloud/bolt. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [lines](#f053) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số line segments submit, tính riêng triangles. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |


<a id="f047"></a>
### drawCalls

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t drawCalls() const
```

**Vị trí mã nguồn:** [include/Renderer.hpp:14](../include/Renderer.hpp#L14).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số glDrawArrays submit trong lầnRenderer draw gần nhất. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f048"></a>
### renderedRainCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t renderedRainCount() const
```

**Vị trí mã nguồn:** [include/Renderer.hpp:15](../include/Renderer.hpp#L15).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số rain streak submit, chưa trừ clipping/depth occlusion. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f049"></a>
### renderedSplashCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t renderedSplashCount() const
```

**Vị trí mã nguồn:** [include/Renderer.hpp:16](../include/Renderer.hpp#L16).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số splash streak submit từ slot sống. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f050"></a>
### lightningUploadCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t lightningUploadCount() const
```

**Vị trí mã nguồn:** [include/Renderer.hpp:17](../include/Renderer.hpp#L17).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Ủy quyền tới lightningBatch_.uploadCount(); trả số upload thực tế để smoke xác nhận cache không ghi lại VBO khi chỉ đổi intensity/hour.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** `uploadCount`.


<a id="f051"></a>
### rainUploadCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t rainUploadCount() const
```

**Vị trí mã nguồn:** [include/Renderer.hpp:18](../include/Renderer.hpp#L18).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Ủy quyền tới rainBatch_.uploadCount(); trả số upload thực tế để smoke xác nhận cache không ghi lại VBO khi chỉ đổi intensity/hour.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** `uploadCount`.


<a id="f052"></a>
### triangles

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t triangles() const
```

**Vị trí mã nguồn:** [include/Renderer.hpp:19](../include/Renderer.hpp#L19).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số triangles submit, gồm sky/scene/cloud/bolt. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f053"></a>
### lines

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t lines() const
```

**Vị trí mã nguồn:** [include/Renderer.hpp:20](../include/Renderer.hpp#L20).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số line segments submit, tính riêng triangles. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 11 | `Renderer()` | [Renderer::Renderer](#f213) — [source src/Renderer.cpp:12](../src/Renderer.cpp#L12) |
| 12 | `void draw(const Scene& scene, const Camera& camera, const WeatherSystem& weather, int width, int height)` | [Renderer::draw](#f215) — [source src/Renderer.cpp:84](../src/Renderer.cpp#L84) |

<a id="file-15"></a>
## include/RippleSystem.hpp

**Module/chức năng:** Pool 48 vòng gợn.

**Headers/phụ thuộc trực tiếp:** `array`, `cstdint`, `glm/glm.hpp`, `random`.

**Kiểu/dữ liệu quản lý:** `Ripple`, `RippleSystem`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [RippleSystem](#f054) | Không có. | Constructor/destructor: không có kiểu trả về. | Constructor gọi reset để seed/pool/state của module nhất quán. | WeatherSystem impact/update, Renderer và weather/environment_tests. |
| [particles](#f055) | Không có. | `const std::array<Ripple, CAPACITY>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả pool cố định của Splash hoặc Ripple; caller lọc lifetime>0 để lấy slot sống. | WeatherSystem impact/update, Renderer và weather/environment_tests. |
| [activeCount](#f056) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả trạng thái strike active (Lightning) hoặc số ripple đang sống (Ripple), phân biệt theo kiểu trong signature. | WeatherSystem impact/update, Renderer và weather/environment_tests. |
| [revision](#f057) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. | WeatherSystem impact/update, Renderer và weather/environment_tests. |
| [rejected](#f058) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số yêu cầu từ chối do invalid/full ở Splash, chỉ full ở Ripple. | WeatherSystem impact/update, Renderer và weather/environment_tests. |


<a id="f054"></a>
### RippleSystem

**Khai báo chính xác tại định nghĩa:**

```cpp
RippleSystem()
```

**Vị trí mã nguồn:** [include/RippleSystem.hpp:19](../include/RippleSystem.hpp#L19).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Constructor gọi reset để seed/pool/state của module nhất quán. Các default member initializer chạy trước thân constructor; chưa tạo tài nguyên GPU trong module mô phỏng này.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** `reset`.


<a id="f055"></a>
### particles

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<Ripple, CAPACITY>& particles() const
```

**Vị trí mã nguồn:** [include/RippleSystem.hpp:23](../include/RippleSystem.hpp#L23).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<Ripple, CAPACITY>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả pool cố định của Splash hoặc Ripple; caller lọc lifetime>0 để lấy slot sống. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f056"></a>
### activeCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t activeCount() const
```

**Vị trí mã nguồn:** [include/RippleSystem.hpp:24](../include/RippleSystem.hpp#L24).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả trạng thái strike active (Lightning) hoặc số ripple đang sống (Ripple), phân biệt theo kiểu trong signature. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f057"></a>
### revision

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t revision() const
```

**Vị trí mã nguồn:** [include/RippleSystem.hpp:25](../include/RippleSystem.hpp#L25).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f058"></a>
### rejected

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t rejected() const
```

**Vị trí mã nguồn:** [include/RippleSystem.hpp:26](../include/RippleSystem.hpp#L26).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số yêu cầu từ chối do invalid/full ở Splash, chỉ full ở Ripple. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 20 | `void reset()` | [RippleSystem::reset](#f218) — [source src/RippleSystem.cpp:5](../src/RippleSystem.cpp#L5) |
| 21 | `void emit(glm::vec3 hit, bool sampled = false, float waterMargin = 0)` | [RippleSystem::emit](#f219) — [source src/RippleSystem.cpp:13](../src/RippleSystem.cpp#L13) |
| 22 | `void update(float deltaTime)` | [RippleSystem::update](#f220) — [source src/RippleSystem.cpp:27](../src/RippleSystem.cpp#L27) |

<a id="file-16"></a>
## include/Scene.hpp

**Module/chức năng:** Sở hữu mesh và danh sách vật thể.

**Headers/phụ thuộc trực tiếp:** `Mesh.hpp`, `Material.hpp`, `memory`, `vector`.

**Kiểu/dữ liệu quản lý:** `SceneObject`, `Scene`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [objects](#f059) | Không có. | `const std::vector<SceneObject>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả vector SceneObject không sở hữu mesh; Mesh owner nằm trong Scene. | Application initialize/update/display/smoke và Renderer. |
| [resetAnimation](#f060) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đặt animationTime_=0; chỉ reset pha sway cây, không recreate mesh, không thay Weather hoặc Camera.. | Application initialize/update/display/smoke và Renderer. |
| [animationTime](#f061) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả thời gian sway cây tính giây, dừng theo pause. | Application initialize/update/display/smoke và Renderer. |


<a id="f059"></a>
### objects

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::vector<SceneObject>& objects() const
```

**Vị trí mã nguồn:** [include/Scene.hpp:22](../include/Scene.hpp#L22).

**Tham số:** Không có.

**Giá trị trả về:** `const std::vector<SceneObject>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả vector SceneObject không sở hữu mesh; Mesh owner nằm trong Scene. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f060"></a>
### resetAnimation

**Khai báo chính xác tại định nghĩa:**

```cpp
void resetAnimation()
```

**Vị trí mã nguồn:** [include/Scene.hpp:25](../include/Scene.hpp#L25).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đặt animationTime_=0; chỉ reset pha sway cây, không recreate mesh, không thay Weather hoặc Camera.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f061"></a>
### animationTime

**Khai báo chính xác tại định nghĩa:**

```cpp
float animationTime() const
```

**Vị trí mã nguồn:** [include/Scene.hpp:26](../include/Scene.hpp#L26).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả thời gian sway cây tính giây, dừng theo pause. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 21 | `Scene()` | [Scene::Scene](#f221) — [source src/Scene.cpp:10](../src/Scene.cpp#L10) |
| 31 | `void modelPart(const std::vector<Vertex>& vertices, glm::vec3 position, glm::vec3 color, Material material, float treeHeight = 0)` | [Scene::modelPart](#f222) — [source src/Scene.cpp:53](../src/Scene.cpp#L53) |
| 29 | `void add(const Mesh& mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 color, float yaw = 0.0f)` | [Scene::add](#f223) — [source src/Scene.cpp:61](../src/Scene.cpp#L61) |
| 23 | `std::size_t triangleCount() const` | [Scene::triangleCount](#f224) — [source src/Scene.cpp:69](../src/Scene.cpp#L69) |
| 24 | `void update(float deltaTime, bool paused)` | [Scene::update](#f225) — [source src/Scene.cpp:75](../src/Scene.cpp#L75) |

<a id="file-17"></a>
## include/Shader.hpp

**Module/chức năng:** Đọc/compile/link GLSL.

**Headers/phụ thuộc trực tiếp:** `GL/glew.h`, `string`.

**Kiểu/dữ liệu quản lý:** `Shader`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Shader](#f062) | `const Shader&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [Shader& operator=](#f063) | `const Shader&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | Không thể gọi; không có giá trị trả về ở runtime. | Cấm copy để bảo vệ ownership. | Không có lời gọi hợp lệ; compiler từ chối sao chép/gán. |
| [use](#f064) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Gọi glUseProgram(program_) để chọn executable GLSL hiện hành trước upload uniforms/draw. | Renderer constructor/draw; RAII cleanup. |


<a id="f062"></a>
### Shader

**Khai báo chính xác tại định nghĩa:**

```cpp
Shader(const Shader&) = delete
```

**Vị trí mã nguồn:** [include/Shader.hpp:10](../include/Shader.hpp#L10).

**Tham số:** `const Shader&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f063"></a>
### Shader& operator=

**Khai báo chính xác tại định nghĩa:**

```cpp
Shader& operator=(const Shader&) = delete
```

**Vị trí mã nguồn:** [include/Shader.hpp:11](../include/Shader.hpp#L11).

**Tham số:** `const Shader&`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** Không thể gọi; không có giá trị trả về ở runtime.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khai báo `= delete` khiến thao tác copy/assignment không hợp lệ lúc biên dịch. Không có thân hàm hoặc đường thực thi. Các GL owners cần sở hữu độc quyền để không xóa ID hai lần; Application cũng chỉ cho phép một active instance.

**Quan hệ — nơi gọi:** Không có lời gọi hợp lệ; compiler từ chối sao chép/gán.


<a id="f064"></a>
### use

**Khai báo chính xác tại định nghĩa:**

```cpp
void use() const
```

**Vị trí mã nguồn:** [include/Shader.hpp:12](../include/Shader.hpp#L12).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Gọi glUseProgram(program_) để chọn executable GLSL hiện hành trước upload uniforms/draw. Không tự bind VAO hoặc upload dữ liệu; caller cần GL context.

**Quan hệ — nơi gọi:** Renderer constructor/draw; RAII cleanup.

**Gọi tiếp/API nổi bật:** `glUseProgram`.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 8 | `Shader(const std::string& vertexPath, const std::string& fragmentPath)` | [Shader::Shader](#f227) — [source src/Shader.cpp:41](../src/Shader.cpp#L41) |
| 13 | `GLint uniform(const char* name) const` | [Shader::uniform](#f229) — [source src/Shader.cpp:77](../src/Shader.cpp#L77) |

<a id="file-18"></a>
## include/SplashSystem.hpp

**Module/chức năng:** Pool 512 hạt bắn.

**Headers/phụ thuộc trực tiếp:** `array`, `cstdint`, `glm/glm.hpp`, `random`.

**Kiểu/dữ liệu quản lý:** `SplashParticle`, `SplashSystem`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [SplashSystem](#f065) | Không có. | Constructor/destructor: không có kiểu trả về. | Constructor gọi reset để seed/pool/state của module nhất quán. | WeatherSystem impact/update, Renderer và weather_tests. |
| [activeCount](#f066) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số slot splash có lifetime>0, không phải capacity 512. | WeatherSystem impact/update, Renderer và weather_tests. |
| [particles](#f067) | Không có. | `const std::array<SplashParticle, MAX_SPLASHES>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả pool cố định của Splash hoặc Ripple; caller lọc lifetime>0 để lấy slot sống. | WeatherSystem impact/update, Renderer và weather_tests. |
| [revision](#f068) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. | WeatherSystem impact/update, Renderer và weather_tests. |
| [requests](#f069) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả tổng yêu cầu emit splash từ reset, gồm accepted/filtered/rejected. | WeatherSystem impact/update, Renderer và weather_tests. |
| [rejected](#f070) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số yêu cầu từ chối do invalid/full ở Splash, chỉ full ở Ripple. | WeatherSystem impact/update, Renderer và weather_tests. |
| [filtered](#f071) | Không có. | `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả số splash request bị bỏ bởi sampling 45%. | WeatherSystem impact/update, Renderer và weather_tests. |


<a id="f065"></a>
### SplashSystem

**Khai báo chính xác tại định nghĩa:**

```cpp
SplashSystem()
```

**Vị trí mã nguồn:** [include/SplashSystem.hpp:19](../include/SplashSystem.hpp#L19).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Constructor gọi reset để seed/pool/state của module nhất quán. Các default member initializer chạy trước thân constructor; chưa tạo tài nguyên GPU trong module mô phỏng này.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `reset`.


<a id="f066"></a>
### activeCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t activeCount() const
```

**Vị trí mã nguồn:** [include/SplashSystem.hpp:23](../include/SplashSystem.hpp#L23).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số slot splash có lifetime>0, không phải capacity 512. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f067"></a>
### particles

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<SplashParticle, MAX_SPLASHES>& particles() const
```

**Vị trí mã nguồn:** [include/SplashSystem.hpp:24](../include/SplashSystem.hpp#L24).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<SplashParticle, MAX_SPLASHES>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả pool cố định của Splash hoặc Ripple; caller lọc lifetime>0 để lấy slot sống. Trả const reference để tránh copy, chỉ còn hợp lệ khi owner còn sống.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f068"></a>
### revision

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t revision() const
```

**Vị trí mã nguồn:** [include/SplashSystem.hpp:25](../include/SplashSystem.hpp#L25).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả version dữ liệu để Renderer biết khi nào cần upload lại, không phải số frame. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f069"></a>
### requests

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t requests() const
```

**Vị trí mã nguồn:** [include/SplashSystem.hpp:26](../include/SplashSystem.hpp#L26).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả tổng yêu cầu emit splash từ reset, gồm accepted/filtered/rejected. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f070"></a>
### rejected

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t rejected() const
```

**Vị trí mã nguồn:** [include/SplashSystem.hpp:27](../include/SplashSystem.hpp#L27).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số yêu cầu từ chối do invalid/full ở Splash, chỉ full ở Ripple. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f071"></a>
### filtered

**Khai báo chính xác tại định nghĩa:**

```cpp
std::uint64_t filtered() const
```

**Vị trí mã nguồn:** [include/SplashSystem.hpp:28](../include/SplashSystem.hpp#L28).

**Tham số:** Không có.

**Giá trị trả về:** `std::uint64_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả số splash request bị bỏ bởi sampling 45%. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 31 | `float random(float low, float high)` | [SplashSystem::random](#f230) — [source src/SplashSystem.cpp:6](../src/SplashSystem.cpp#L6) |
| 20 | `void reset()` | [SplashSystem::reset](#f231) — [source src/SplashSystem.cpp:10](../src/SplashSystem.cpp#L10) |
| 21 | `void emit(glm::vec3 groundHit, bool sampled = false)` | [SplashSystem::emit](#f232) — [source src/SplashSystem.cpp:18](../src/SplashSystem.cpp#L18) |
| 22 | `void update(float deltaTime)` | [SplashSystem::update](#f233) — [source src/SplashSystem.cpp:38](../src/SplashSystem.cpp#L38) |

<a id="file-19"></a>
## include/Surface.hpp

**Module/chức năng:** Polygon nước dùng chung.

**Headers/phụ thuộc trực tiếp:** `array`, `glm/glm.hpp`.

**Kiểu/dữ liệu quản lý:** `Puddle`; xem bảng trạng thái phía trên.

Không có thân hàm trong file này. Các prototype được thực thi ở file .cpp cùng module; riêng Material chỉ có enum/aggregate.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 9 | `const std::array<Puddle, 3>& puddles()` | [puddles](#f234) — [source src/Surface.cpp:7](../src/Surface.cpp#L7) |
| 10 | `float outlineRadius(float angle)` | [outlineRadius](#f235) — [source src/Surface.cpp:15](../src/Surface.cpp#L15) |
| 11 | `const std::array<glm::vec2, OUTLINE_POINTS>& outline()` | [outline](#f236) — [source src/Surface.cpp:19](../src/Surface.cpp#L19) |
| 12 | `const std::array<glm::vec2, OUTLINE_POINTS>& polygon(std::size_t index)` | [polygon](#f238) — [source src/Surface.cpp:32](../src/Surface.cpp#L32) |
| 14 | `float waterMargin(glm::vec3 hit)` | [waterMargin](#f240) — [source src/Surface.cpp:52](../src/Surface.cpp#L52) |

<a id="file-20"></a>
## include/TimeOfDaySystem.hpp

**Module/chức năng:** Đồng hồ giờ mô phỏng.

**Headers/phụ thuộc trực tiếp:** `string`.

**Kiểu/dữ liệu quản lý:** `TimeOfDaySystem`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [shiftHours](#f072) | `double hours`: độ dịch theo giờ mô phỏng. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Gọi setHour(hour_+hours), nên giờ âm/qua 24 wrap bằng fmod. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [toggleDayNight](#f073) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu hour hiện trong[6, 18) thì setHour(0), ngược lại setHour(12). | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [setAutoAdvance](#f074) | `bool enabled`: giá trị cờ autoAdvance mới. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Ghi enabled vào autoAdvance_; không tiến giờ ngay. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [hour](#f075) | Không có. | `double`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả giờ mô phỏng double trong[0, 24). | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [dayDuration](#f076) | Không có. | `double`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả giây thực cho 24 giờ mô phỏng, default 600 và range 60..3600. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [autoAdvance](#f077) | Không có. | `bool`; xem ý nghĩa và điều kiện biên trong quy trình. | Đọc và trả cờ đồng hồ tự tiến, default false. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |


<a id="f072"></a>
### shiftHours

**Khai báo chính xác tại định nghĩa:**

```cpp
void shiftHours(double hours)
```

**Vị trí mã nguồn:** [include/TimeOfDaySystem.hpp:11](../include/TimeOfDaySystem.hpp#L11).

**Tham số:** `double hours`: độ dịch theo giờ mô phỏng.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Gọi setHour(hour_+hours), nên giờ âm/qua 24 wrap bằng fmod. units=giờ; bỏ qua bool return, đầu vào không finite khiến giờ giữ nguyên.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** `setHour`.


<a id="f073"></a>
### toggleDayNight

**Khai báo chính xác tại định nghĩa:**

```cpp
void toggleDayNight()
```

**Vị trí mã nguồn:** [include/TimeOfDaySystem.hpp:12](../include/TimeOfDaySystem.hpp#L12).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu hour hiện trong[6, 18) thì setHour(0), ngược lại setHour(12). Ví dụ 18:00→12:00→00:00. Không toggle autoAdvance hoặc reset particle.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** `setHour`.


<a id="f074"></a>
### setAutoAdvance

**Khai báo chính xác tại định nghĩa:**

```cpp
void setAutoAdvance(bool enabled)
```

**Vị trí mã nguồn:** [include/TimeOfDaySystem.hpp:13](../include/TimeOfDaySystem.hpp#L13).

**Tham số:** `bool enabled`: giá trị cờ autoAdvance mới.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Ghi enabled vào autoAdvance_; không tiến giờ ngay. update sau đó mới tiêu thụ dt nếu auto on và không pause.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f075"></a>
### hour

**Khai báo chính xác tại định nghĩa:**

```cpp
double hour() const
```

**Vị trí mã nguồn:** [include/TimeOfDaySystem.hpp:15](../include/TimeOfDaySystem.hpp#L15).

**Tham số:** Không có.

**Giá trị trả về:** `double`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả giờ mô phỏng double trong[0, 24). Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f076"></a>
### dayDuration

**Khai báo chính xác tại định nghĩa:**

```cpp
double dayDuration() const
```

**Vị trí mã nguồn:** [include/TimeOfDaySystem.hpp:16](../include/TimeOfDaySystem.hpp#L16).

**Tham số:** Không có.

**Giá trị trả về:** `double`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả giây thực cho 24 giờ mô phỏng, default 600 và range 60..3600. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f077"></a>
### autoAdvance

**Khai báo chính xác tại định nghĩa:**

```cpp
bool autoAdvance() const
```

**Vị trí mã nguồn:** [include/TimeOfDaySystem.hpp:17](../include/TimeOfDaySystem.hpp#L17).

**Tham số:** Không có.

**Giá trị trả về:** `bool`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc và trả cờ đồng hồ tự tiến, default false. Trả bản sao giá trị; không cập nhật state hoặc gọi OpenGL.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 8 | `void reset()` | [TimeOfDaySystem::reset](#f242) — [source src/TimeOfDaySystem.cpp:5](../src/TimeOfDaySystem.cpp#L5) |
| 9 | `bool setHour(double hour)` | [TimeOfDaySystem::setHour](#f243) — [source src/TimeOfDaySystem.cpp:6](../src/TimeOfDaySystem.cpp#L6) |
| 10 | `bool setDayDuration(double seconds)` | [TimeOfDaySystem::setDayDuration](#f244) — [source src/TimeOfDaySystem.cpp:12](../src/TimeOfDaySystem.cpp#L12) |
| 14 | `void update(double seconds,bool paused=false)` | [TimeOfDaySystem::update](#f245) — [source src/TimeOfDaySystem.cpp:16](../src/TimeOfDaySystem.cpp#L16) |
| 18 | `std::string label() const` | [TimeOfDaySystem::label](#f246) — [source src/TimeOfDaySystem.cpp:21](../src/TimeOfDaySystem.cpp#L21) |

<a id="file-21"></a>
## include/WeatherSystem.hpp

**Module/chức năng:** Điều phối fixed timestep.

**Headers/phụ thuộc trực tiếp:** `CloudSystem.hpp`, `RainSystem.hpp`, `LightningSystem.hpp`, `LightingSystem.hpp`, `SplashSystem.hpp`, `RippleSystem.hpp`.

**Kiểu/dữ liệu quản lý:** `WeatherSystem`; xem bảng trạng thái phía trên.

Không có thân hàm trong file này. Các prototype được thực thi ở file .cpp cùng module; riêng Material chỉ có enum/aggregate.



### Prototype trong header và nơi định nghĩa

Các khai báo sau không có thân trong header; chỉ mục đếm một lần tại implementation. Tham số mặc định/static/explicit thuộc khai báo này; phần chi tiết ở anchor hàm đích.

| Dòng header | Khai báo | Giải thích implementation |
|---:|---|---|
| 12 | `void reset()` | [WeatherSystem::reset](#f247) — [source src/WeatherSystem.cpp:7](../src/WeatherSystem.cpp#L7) |
| 17 | `void refreshLighting()` | [WeatherSystem::refreshLighting](#f248) — [source src/WeatherSystem.cpp:25](../src/WeatherSystem.cpp#L25) |
| 13 | `void update(float deltaTime)` | [WeatherSystem::update](#f249) — [source src/WeatherSystem.cpp:30](../src/WeatherSystem.cpp#L30) |
| 14 | `void triggerLightning()` | [WeatherSystem::triggerLightning](#f250) — [source src/WeatherSystem.cpp:55](../src/WeatherSystem.cpp#L55) |
| 15 | `void toggleLightning()` | [WeatherSystem::toggleLightning](#f251) — [source src/WeatherSystem.cpp:61](../src/WeatherSystem.cpp#L61) |
| 16 | `bool valid() const` | [WeatherSystem::valid](#f252) — [source src/WeatherSystem.cpp:67](../src/WeatherSystem.cpp#L67) |

<a id="file-22"></a>
## shaders/effects.frag

**Module/chức năng:** GLSL GPU.

**Headers/phụ thuộc trực tiếp:** Không có #include; nhận uniforms/attributes từ Renderer..

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [main](#f078) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tính softness=1−smoothstep(0.35, 1, abs(vEdge)), nearFade trong 0.5..1.5; alpha=vColor.a×uIntensity×mix(1, softness×nearFade, uSoftEdge). | Pipeline GPU cho main; object.frag::main cho helper fragment. |


<a id="f078"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```glsl
void main()
```

**Vị trí mã nguồn:** [shaders/effects.frag:13](../shaders/effects.frag#L13).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tính softness=1−smoothstep(0.35, 1, abs(vEdge)), nearFade trong 0.5..1.5; alpha=vColor.a×uIntensity×mix(1, softness×nearFade, uSoftEdge). Tính exponential fog có scale và khoảng gần 6; RGB=mix(fogColor, vColor.rgb×weatherLight, fog), output alpha riêng. Blending do C++ thiết lập, không tự nằm trong shader.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `smoothstep`, `abs`, `distance`, `mix`, `exp`, `max`, `vec4`.


<a id="file-23"></a>
## shaders/effects.vert

**Module/chức năng:** GLSL GPU.

**Headers/phụ thuộc trực tiếp:** Không có #include; nhận uniforms/attributes từ Renderer..

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [main](#f079) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Copy attributes sang varyings; d tới camera tính từ head. | Pipeline GPU cho main; object.frag::main cho helper fragment. |


<a id="f079"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```glsl
void main()
```

**Vị trí mã nguồn:** [shaders/effects.vert:12](../shaders/effects.vert#L12).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Copy attributes sang varyings; d tới camera tính từ head. Rain Pass: tail position=head−normalize(velocity)×aEdge×mix(1.12, 0.62, smoothstep(4, 30, d)); đổi màu từ velocity-packed sang RGB mưa và alpha opacity×0.78×near smoothstep 0.25..1.4, edge 0. Mọi pass có alpha distance Fade 1→0.28 trong 4..32 khi uDistanceFade bật. vWorldPosition là vị trí sau dựng tail; gl_Position=VP×position.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `distance`, `mix`, `smoothstep`, `max`, `length`, `vec4`.


<a id="file-24"></a>
## shaders/object.frag

**Module/chức năng:** GLSL GPU.

**Headers/phụ thuộc trực tiếp:** Không có #include; nhận uniforms/attributes từ Renderer..

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [moistureField](#f080) | `vec2 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | p là XZ world; trả clamp(0.5+0.28 sin(0.34px+0.8 sin(0.21py))+0.18 cos(0.47py−0.12px), 0, 1). | Pipeline GPU cho main; object.frag::main cho helper fragment. |
| [soilColor](#f081) | `float field`: mức moisture[0, 1]; `float wet`: độ ướt cục bộ[0, 1]. | `vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Mix soil colors theo moisture field; dựng mask đường đi cong từ world.xz và smoothstep bờ/đầu/cuối. | Pipeline GPU cho main; object.frag::main cho helper fragment. |
| [waterInterior](#f082) | `vec2 world`: điểm world-space cần probe hoặc XZ shader. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Đổi world XZ ngược rotation/scale của ba puddle, chọn điểm local có radius ² nhỏ nhất. | Pipeline GPU cho main; object.frag::main cho helper fragment. |
| [main](#f083) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Phân loại ground/water từ uSurface. | Pipeline GPU cho main; object.frag::main cho helper fragment. |


<a id="f080"></a>
### moistureField

**Khai báo chính xác tại định nghĩa:**

```glsl
float moistureField(vec2 p)
```

**Vị trí mã nguồn:** [shaders/object.frag:34](../shaders/object.frag#L34).

**Tham số:** `vec2 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** p là XZ world; trả clamp(0.5+0.28 sin(0.34px+0.8 sin(0.21py))+0.18 cos(0.47py−0.12px), 0, 1). Tạo patch ẩm rộng, cố định world-space, không noise pixel hay texture sample.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `clamp`, `sin`, `cos`.


<a id="f081"></a>
### soilColor

**Khai báo chính xác tại định nghĩa:**

```glsl
vec3 soilColor(float field, float wet)
```

**Vị trí mã nguồn:** [shaders/object.frag:39](../shaders/object.frag#L39).

**Tham số:** `float field`: mức moisture[0, 1]; `float wet`: độ ướt cục bộ[0, 1].

**Giá trị trả về:** `vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Mix soil colors theo moisture field; dựng mask đường đi cong từ world.xz và smoothstep bờ/đầu/cuối. Contact tối gần hộp chân nhà và bảy tree/rock anchors, chọn max để tránh cộng quá tối. Nhân darkening theo localWet/path và contact; trả RGB đất, không thay y/collision.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `mix`, `vec3`, `smoothstep`, `sin`, `abs`, `vec2`, `max`, `distance`.


<a id="f082"></a>
### waterInterior

**Khai báo chính xác tại định nghĩa:**

```glsl
float waterInterior(vec2 world)
```

**Vị trí mã nguồn:** [shaders/object.frag:50](../shaders/object.frag#L50).

**Tham số:** `vec2 world`: điểm world-space cần probe hoặc XZ shader.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đổi world XZ ngược rotation/scale của ba puddle, chọn điểm local có radius ² nhỏ nhất. Tính angle/radius theo cùng sin/cos outline, r=length(local)/radius; trả 1−smoothstep(0.84, 0.985, r). Đây là mask màu bờ liên tục; mesh/hit-test vẫn polygon 32 điểm, không alpha/discard.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `vec2`, `dot`, `atan`, `sin`, `cos`, `sqrt`, `smoothstep`.


<a id="f083"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```glsl
void main()
```

**Vị trí mã nguồn:** [shaders/object.frag:65](../shaders/object.frag#L65).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Phân loại ground/water từ uSurface. Tính moisture/localWet/interior và perturb normal nhẹ; Lambert nguồn định hướng, flash attenuation/Lambert và shade theo cloud height. Chọn base soil/water/material, view/half vectors, shininess và Blinn–Phong cap 0.45. Các material kind đổi detail plaster/tile/wood/stone/foliage/brick/rock/grass, grid có fwidth anti-alias. Nhân ambient/diffuse/flash + specular; Glass cộng emission đêm, vật không cloud cộng warm point light. Water mix fake sky reflection bằng Schlick F0=0.02, thêm flash glint; cloud thêm rim flash. Exponential fog cuối, fragColor alpha=uCoverage; không discard, gl_FragCoord, texture hoặc shadow sampling.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `step`, `normalize`, `moistureField`, `mix`, `smoothstep`, `waterInterior`, `vec2`, `cos`, `sin`, `vec3`, `max`, `dot`, `distance`, `soilColor`, `pow`, `length`, `clamp`, `min`, `abs`, `floor`, `mod`, `fract`, `fwidth`, `reflect`, `exp`, `vec4`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="file-25"></a>
## shaders/object.vert

**Module/chức năng:** GLSL GPU.

**Headers/phụ thuộc trực tiếp:** Không có #include; nhận uniforms/attributes từ Renderer..

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [main](#f084) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nhận position/normal local. | Pipeline GPU cho main; object.frag::main cho helper fragment. |


<a id="f084"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```glsl
void main()
```

**Vị trí mã nguồn:** [shaders/object.vert:18](../shaders/object.vert#L18).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhận position/normal local. Nếu treeHeight>0, h=clamp(y/H), phase theo model translation, amplitude=wind×0.05 sin(0.9 time+phase); position.xz+=amplitude h², normal.y−=dot(normal.xz, amplitude×2h/H) theo inverse-transpose Jacobian. Xuất normal qua normal matrix, local gốc, world đã bend; gl_Position=P·V·world. Không thay đổi CPU mesh; fragment normalize normal sau nội suy.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `clamp`, `sin`, `dot`, `vec4`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="file-26"></a>
## shaders/sky.frag

**Module/chức năng:** GLSL GPU.

**Headers/phụ thuộc trực tiếp:** Không có #include; nhận uniforms/attributes từ Renderer..

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [main](#f085) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nhận vUV từ rasterization; output vec4(mix(uHorizon, uTop, smoothstep(0, 1, vUV.y)), 1). | Pipeline GPU cho main; object.frag::main cho helper fragment. |


<a id="f085"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```glsl
void main()
```

**Vị trí mã nguồn:** [shaders/sky.frag:6](../shaders/sky.frag#L6).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhận vUV từ rasterization; output vec4(mix(uHorizon, uTop, smoothstep(0, 1, vUV.y)), 1). Không dùng view/camera direction nên gradient cố định theo màn hình khi camera ngửa/cúi.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `vec4`, `mix`, `smoothstep`.


<a id="file-27"></a>
## shaders/sky.vert

**Module/chức năng:** GLSL GPU.

**Headers/phụ thuộc trực tiếp:** Không có #include; nhận uniforms/attributes từ Renderer..

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [main](#f086) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đổi position.xy clip sang vUV=(xy+1)/2, đặt gl_Position=(xy, 0, 1). | Pipeline GPU cho main; object.frag::main cho helper fragment. |


<a id="f086"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```glsl
void main()
```

**Vị trí mã nguồn:** [shaders/sky.vert:4](../shaders/sky.vert#L4).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đổi position.xy clip sang vUV=(xy+1)/2, đặt gl_Position=(xy, 0, 1). Fullscreen triangle dùng các vertex ngoài viewport và clipping để phủ màn hình; không perspective projection.

**Quan hệ — nơi gọi:** Pipeline GPU cho main; object.frag::main cho helper fragment.

**Gọi tiếp/API nổi bật:** `vec4`.


<a id="file-28"></a>
## src/Application.cpp

**Module/chức năng:** Cửa sổ, vòng đời, input và GUI smoke.

**Headers/phụ thuộc trực tiếp:** `Application.hpp`, `GL/freeglut.h`, `algorithm`, `cctype`, `cmath`, `cstdio`, `fstream`, `filesystem`, `iostream`, `sstream`, `stdexcept`, `utility`, `vector`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Application.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [checkOpenGL](#f087) | `const char* where`: nhãn vị trí kiểm tra OpenGL. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Thu hết hàng đợi glGetError, ghép mã hex vào thông báo có nhãn where; nếu có ít nhất một lỗi thì ném runtime_error. | initialize/display/cleanup và smoke readback. |
| [require](#f088) | `bool condition`: điều kiện bắt buộc đúng; `const char* message`: thông báo lỗi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. | Các bước kiểm tra context, smoke, thống kê trong Application. |
| [readColor](#f089) | `int width`: chiều rộng pixel, riêng beam là world-unit theo signature; `int height`: chiều cao pixel. | `std::vector<unsigned char>`; xem ý nghĩa và điều kiện biên trong quy trình. | Cấp vector width×height×3 byte; chọn GL_BACK, PACK_ALIGNMENT=1, đọc RGB unsigned byte bằng glReadPixels và trả vector. | smokeStep và inspect helpers. |
| [changedPixels](#f090) | `const std::vector<unsigned char>& a`: điểm/vector/scalar đầu theo signature; `const std::vector<unsigned char>& b`: điểm/vector/scalar thứ hai theo signature. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Yêu cầu hai vector cùng kích thước; duyệt từng bộ ba RGB, đếm một pixel nếu bất kỳ kênh nào lệch tuyệt đối lớn hơn 5. | smokeStep. |
| [Application::Application](#f091) | `RunOptions options`: cấu hình CLI được move vào Application. | Constructor/destructor: không có kiểu trả về. | Move RunOptions vào options_; các thành viên Camera và WeatherSystem được dựng theo thứ tự khai báo trước khi vào thân constructor. | main. |
| [Application::~Application](#f092) | Không có. | Constructor/destructor: không có kiểu trả về. | Nếu còn window_, chuyển context tới cửa sổ đó, bỏ close callback để tránh gọi vào con trỏ hết đời; hủy Benchmark, Renderer, Scene rồi yêu cầu hủy window. | RAII khi rời main hoặc exception. |
| [Application::run](#f093) | `char* executableName`: argv[0] cho FreeGLUT. | `int`; xem ý nghĩa và điều kiện biên trong quy trình. | Bảo đảm chỉ một Application; gán active_=this rồi initialize. | main. |
| [Application::initialize](#f094) | `char* executableName`: argv[0] cho FreeGLUT. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tạo argv tối thiểu chỉ chứa executableName vì CLI đã parse; yêu cầu context 3.3 Core, double-buffer RGBA và depth, cửa sổ 800×600. | run. |
| [Application::update](#f095) | `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Từ keys_ dựng direction=(D−A, E−Q, W−S); camera.move trước, gán observerPosition từ camera mới, weather.update sau, rồi scene.update(dt, paused) nếu scene tồn tại. | tick và smokeStep. |
| [Application::tick](#f096) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu đang thoát thì bỏ qua. | timerCallback. |
| [Application::display](#f097) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bỏ qua khi exiting hoặc chưa có scene/renderer. | displayCallback hoặc tick benchmark. |
| [Application::reshape](#f098) | `int width`: chiều rộng pixel, riêng beam là world-unit theo signature; `int height`: chiều cao pixel. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Clamp width/height tối thiểu 1, lưu width_/height_ và gọi glViewport. | initialize và reshapeCallback. |
| [Application::key](#f099) | `unsigned char value`: mã phím hoặc điều kiện bool theo signature; `bool pressed`: true=down, false=up. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Chuyển ký tự sang lowercase, ghi keys_ cho cả press/release; release trở về ngay. | keyDownCallback/keyUpCallback và smokeStep. |
| [Application::printTimeHelp](#f100) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | In label HH:MM, trạng thái auto và dayDuration, sau đó hướng dẫn N/[ / ]/T/H. | initialize và key(H). |
| [Application::updateStatistics](#f101) | `Clock::time_point now`: mốc steady_clock. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Chỉ xử lý khi cách statisticsTick_ ít nhất một giây. | tick. |
| [Application::mouseButton](#f102) | `int button`: mã nút FreeGLUT; `int state`: mã trạng thái FreeGLUT; `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Chỉ nút trái điều khiển dragging_; GLUT_DOWN bật, các trạng thái khác tắt, đồng thời lưu tọa độ pixel chuột. | mouseButtonCallback và smokeStep. |
| [Application::mouseMotion](#f103) | `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu đang dragging, truyền chênh pixel từ lần trước tới Camera::rotate rồi lưu vị trí chuột hiện tại. | mouseMotionCallback và smokeStep. |
| [Application::releaseInput](#f104) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Xóa toàn bộ 256 key state và tắt dragging. | key/reset/preset, entryCallback, visibilityCallback, requestExit. |
| [Application::close](#f105) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Close callback còn context hiện hành: reset benchmark/renderer/scene, kiểm tra GL cleanup trong try/catch; lỗi in ra và exitCode_=1. | closeCallback. |
| [Application::requestExit](#f106) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Guard chống thoát lặp; đánh dấu exiting, xóa input, hủy benchmark/renderer/scene khi context còn sống, kiểm tra GL rồi glutLeaveMainLoop. | key(Esc), tick hết benchmark và smokeStep. |
| [Application::fail](#f107) | `const char* message`: thông báo lỗi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | In lỗi, đặt exitCode_=1; nếu còn cửa sổ thì đánh dấu thoát, giải phóng unique_ptr và rời main loop. | callback wrappers khi bắt std::exception. |
| [Application::inspectFramebuffer](#f108) | `const std::string& capturePath`: đường dẫn PPM, rỗng thì không ghi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Yêu cầu ngân sách <10000 triangles và <80 draw calls; đọc depth và RGB GL_BACK. | smokeStep và lambda captureTime. |
| [Application::smokeStep](#f109) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Frame 1: tạo thư mục capture nếu yêu cầu; inspect framebuffer và đếm particle submit. | display khi smokeTest. |
| [Application::smokeStep::renderAndRead@406](#f110) | Không có tham số; lambda capture như khai báo. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Capture this bằng tham chiếu; gọi renderer.draw với state hiện tại, checkOpenGL rồi readColor. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [Application::smokeStep::sample@545](#f111) | `const std::vector<unsigned char>& pixels`: vector RGB byte theo bottom-up; `glm::vec3 world`: điểm world-space cần probe hoặc XZ shader. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Chiếu điểm world qua P·V, require w>0, chia perspective và đổi sang pixel. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [Application::smokeStep::captureTime@607](#f112) | `const char* name`: tên uniform hoặc tên capture/test theo hàm. | void suy diễn; thay đổi dữ liệu qua capture/reference. | Nếu đã cấu hình thư mục ảnh, ghép đường dẫn bằng filesystem::path rồi inspectFramebuffer; nếu rỗng thì không ghi. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [Application::displayCallback](#f113) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | FreeGLUT gọi khi cần redraw; route active_->display trong try/catch std::exception, chuyển lỗi sang active_->fail. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::timerCallback](#f114) | `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nhận tham số timer int không dùng; route active_->tick với cùng hàng rào exception sang fail. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::idleCallback](#f115) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Idle callback không tham số; gọi timerCallback(0) để dùng lại tick và hàng rào lỗi trong chế độ benchmark-unlocked. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::reshapeCallback](#f116) | `int w`: chiều rộng callback pixel; `int h`: chiều cao callback pixel. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Route kích thước w/h từ FreeGLUT tới reshape. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::keyDownCallback](#f117) | `unsigned char k`: mã phím callback; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nhận k và hai tọa độ chuột không dùng; gọi key(k, true) trong try/catch, fail nếu exception. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::keyUpCallback](#f118) | `unsigned char k`: mã phím callback; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nhận k và tọa độ không dùng; gọi key(k, false), chỉ xóa key state rồi return trong key. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::mouseButtonCallback](#f119) | `int b`: điểm/vector/scalar thứ hai theo signature; `int s`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới; `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Chuyển nguyên button/state/x/y của FreeGLUT tới mouseButton. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::mouseMotionCallback](#f120) | `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Chuyển x/y tới mouseMotion; chỉ xoay camera nếu dragging. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::entryCallback](#f121) | `int state`: mã trạng thái FreeGLUT. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Khi state==GLUT_LEFT thì releaseInput; các trạng thái khác không làm gì. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::visibilityCallback](#f122) | `int state`: mã trạng thái FreeGLUT. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | visible_=(state==GLUT_VISIBLE); khi bị ẩn/occluded thì releaseInput. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |
| [Application::closeCallback](#f123) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Chuyển close event tới active_->close khi FreeGLUT còn context để tài nguyên GPU được xóa đúng lúc.. | FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback. |


<a id="f087"></a>
### checkOpenGL

**Khai báo chính xác tại định nghĩa:**

```cpp
void checkOpenGL(const char* where)
```

**Vị trí mã nguồn:** [src/Application.cpp:17](../src/Application.cpp#L17).

**Tham số:** `const char* where`: nhãn vị trí kiểm tra OpenGL.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Thu hết hàng đợi glGetError, ghép mã hex vào thông báo có nhãn where; nếu có ít nhất một lỗi thì ném runtime_error. Không chỉ đọc một lỗi; lời gọi này cũng làm rỗng trạng thái lỗi GL.

**Quan hệ — nơi gọi:** initialize/display/cleanup và smoke readback.

**Gọi tiếp/API nổi bật:** `glGetError`, `std::runtime_error`, `std::string`, `str`.


<a id="f088"></a>
### require

**Khai báo chính xác tại định nghĩa:**

```cpp
void require(bool condition, const char* message)
```

**Vị trí mã nguồn:** [src/Application.cpp:27](../src/Application.cpp#L27).

**Tham số:** `bool condition`: điều kiện bắt buộc đúng; `const char* message`: thông báo lỗi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. Dùng exception để test main hoặc callback có thể chuyển lỗi thành mã thoát 1.

**Quan hệ — nơi gọi:** Các bước kiểm tra context, smoke, thống kê trong Application.

**Gọi tiếp/API nổi bật:** `std::runtime_error`.


<a id="f089"></a>
### readColor

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<unsigned char> readColor(int width, int height)
```

**Vị trí mã nguồn:** [src/Application.cpp:31](../src/Application.cpp#L31).

**Tham số:** `int width`: chiều rộng pixel, riêng beam là world-unit theo signature; `int height`: chiều cao pixel.

**Giá trị trả về:** `std::vector<unsigned char>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Cấp vector width×height×3 byte; chọn GL_BACK, PACK_ALIGNMENT=1, đọc RGB unsigned byte bằng glReadPixels và trả vector. Thứ tự hàng vẫn từ dưới lên theo OpenGL; việc lật hàng chỉ thực hiện khi ghi PPM.

**Quan hệ — nơi gọi:** smokeStep và inspect helpers.

**Gọi tiếp/API nổi bật:** `pixels`, `glReadBuffer`, `glPixelStorei`, `glReadPixels`, `data`.


<a id="f090"></a>
### changedPixels

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t changedPixels(const std::vector<unsigned char>& a,
                          const std::vector<unsigned char>& b)
```

**Vị trí mã nguồn:** [src/Application.cpp:39](../src/Application.cpp#L39).

**Tham số:** `const std::vector<unsigned char>& a`: điểm/vector/scalar đầu theo signature; `const std::vector<unsigned char>& b`: điểm/vector/scalar thứ hai theo signature.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Yêu cầu hai vector cùng kích thước; duyệt từng bộ ba RGB, đếm một pixel nếu bất kỳ kênh nào lệch tuyệt đối lớn hơn 5. Trả số pixel khác biệt, không phải số kênh. Giả định kích thước là bội của 3, được bảo đảm bởi readColor.

**Quan hệ — nơi gọi:** smokeStep.

**Gọi tiếp/API nổi bật:** `require`, `size`, `std::abs`, `int`.


<a id="f091"></a>
### Application::Application

**Khai báo chính xác tại định nghĩa:**

```cpp
Application::Application(RunOptions options)
```

**Vị trí mã nguồn:** [src/Application.cpp:54](../src/Application.cpp#L54).

**Tham số:** `RunOptions options`: cấu hình CLI được move vào Application.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Move RunOptions vào options_; các thành viên Camera và WeatherSystem được dựng theo thứ tự khai báo trước khi vào thân constructor. Chưa tạo cửa sổ hoặc tài nguyên GL tại đây.

**Quan hệ — nơi gọi:** main.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f092"></a>
### Application::~Application

**Khai báo chính xác tại định nghĩa:**

```cpp
Application::~Application()
```

**Vị trí mã nguồn:** [src/Application.cpp:56](../src/Application.cpp#L56).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu còn window_, chuyển context tới cửa sổ đó, bỏ close callback để tránh gọi vào con trỏ hết đời; hủy Benchmark, Renderer, Scene rồi yêu cầu hủy window. Cuối cùng xóa active_. Các unique_ptr rỗng có thể reset an toàn nhiều lần.

**Quan hệ — nơi gọi:** RAII khi rời main hoặc exception.

**Gọi tiếp/API nổi bật:** `glutSetWindow`, `glutCloseFunc`, `reset`, `glutDestroyWindow`.


<a id="f093"></a>
### Application::run

**Khai báo chính xác tại định nghĩa:**

```cpp
int Application::run(char* executableName)
```

**Vị trí mã nguồn:** [src/Application.cpp:71](../src/Application.cpp#L71).

**Tham số:** `char* executableName`: argv[0] cho FreeGLUT.

**Giá trị trả về:** `int`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bảo đảm chỉ một Application; gán active_=this rồi initialize. Đặt mốc lastTick_/started_/statisticsTick_ sau khởi tạo, đăng ký idle nếu unlocked hoặc timer 16 ms, vào glutMainLoop. Sau khi loop trả về, smoke bị ngắt sớm thành lỗi; smoke hoàn tất và exitCode_=0 in kết quả đạt, rồi trả mã thoát.

**Quan hệ — nơi gọi:** main.

**Gọi tiếp/API nổi bật:** `require`, `initialize`, `Clock::now`, `glutIdleFunc`, `glutTimerFunc`, `glutMainLoop`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f094"></a>
### Application::initialize

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::initialize(char* executableName)
```

**Vị trí mã nguồn:** [src/Application.cpp:91](../src/Application.cpp#L91).

**Tham số:** `char* executableName`: argv[0] cho FreeGLUT.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tạo argv tối thiểu chỉ chứa executableName vì CLI đã parse; yêu cầu context 3.3 Core, double-buffer RGBA và depth, cửa sổ 800×600. Đăng ký close sớm; glewExperimental=true và glewInit; chỉ ở đây drain lỗi do probe GLEW. Đọc version/profile/depth, yêu cầu Core≥3.3 và depth≥16. Dựng Renderer rồi Scene, áp fog/time từ CLI, refreshLighting và tùy chọn Benchmark; reshape, kiểm tra GL, in thống kê và help. Tắt key repeat rồi đăng ký display/reshape/key down-up/mouse/entry/visibility callbacks. Không tạo fallback renderer.

**Quan hệ — nơi gọi:** run.

**Gọi tiếp/API nổi bật:** `glutInit`, `glutInitContextVersion`, `glutInitContextProfile`, `glutInitDisplayMode`, `glutInitWindowSize`, `glutSetOption`, `glutCreateWindow`, `require`, `glutCloseFunc`, `glewInit`, `std::runtime_error`, `std::string`, `glewGetErrorString`, `glGetError`, `glGetIntegerv`, `glGetFramebufferAttachmentParameteriv`, `checkOpenGL`, `glGetString`, `setHour`, `setDayDuration`, `setAutoAdvance`, `refreshLighting`, `reshape`, `objects`, `size`, `triangleCount`, `printTimeHelp`, `glutIgnoreKeyRepeat`, `glutDisplayFunc`, `glutReshapeFunc`, `glutKeyboardFunc`, `glutKeyboardUpFunc`, `glutMouseFunc`, `glutMotionFunc`, `glutEntryFunc`, `glutVisibilityFunc`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f095"></a>
### Application::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::update(float deltaTime)
```

**Vị trí mã nguồn:** [src/Application.cpp:159](../src/Application.cpp#L159).

**Tham số:** `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Từ keys_ dựng direction=(D−A, E−Q, W−S); camera.move trước, gán observerPosition từ camera mới, weather.update sau, rồi scene.update(dt, paused) nếu scene tồn tại. Camera vẫn tiến khi thời tiết pause; animationTime của cây không dùng accumulator của Weather.

**Quan hệ — nơi gọi:** tick và smokeStep.

**Gọi tiếp/API nổi bật:** `move`, `position`, `update`.


<a id="f096"></a>
### Application::tick

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::tick()
```

**Vị trí mã nguồn:** [src/Application.cpp:170](../src/Application.cpp#L170).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu đang thoát thì bỏ qua. Tính steady-clock dt, clamp [0, 0.1], lưu lastTick_ và updateStatistics trước update. Nếu benchmark đạt thời hạn thực, in FPS/max primitives/pool/time, gọi report và requestExit. Benchmark có cap lên lịch timer tiếp theo trước render; nếu visible hoặc benchmark thì update rồi benchmark gọi display trực tiếp, chế độ thường post redisplay. Timer thường được đăng ký lại ở cuối. GLUT có thể gọi display thêm khi expose, nên không giả định update và render luôn ghép 1:1.

**Quan hệ — nơi gọi:** timerCallback.

**Gọi tiếp/API nổi bật:** `Clock::now`, `std::clamp`, `count`, `updateStatistics`, `drops`, `size`, `activeCount`, `strikeCount`, `report`, `requests`, `filtered`, `rejected`, `hour`, `label`, `autoAdvance`, `dayDuration`, `requestExit`, `glutTimerFunc`, `update`, `display`, `glutPostRedisplay`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f097"></a>
### Application::display

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::display()
```

**Vị trí mã nguồn:** [src/Application.cpp:206](../src/Application.cpp#L206).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bỏ qua khi exiting hoặc chưa có scene/renderer. Frame smoke đầu reset animation và warmup weather bằng 5×0.1 giây. Begin query nếu benchmark; draw; cập nhật cực đại draw/triangles/lines, checkOpenGL, end query; tăng frames_, chạy smokeStep, swap nếu chưa thoát. GPU query và CPU submit kết thúc trước smokeStep và swap.

**Quan hệ — nơi gọi:** displayCallback hoặc tick benchmark.

**Gọi tiếp/API nổi bật:** `resetAnimation`, `update`, `beginFrame`, `draw`, `std::max`, `drawCalls`, `triangles`, `lines`, `checkOpenGL`, `endFrame`, `smokeStep`, `glutSwapBuffers`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f098"></a>
### Application::reshape

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::reshape(int width, int height)
```

**Vị trí mã nguồn:** [src/Application.cpp:225](../src/Application.cpp#L225).

**Tham số:** `int width`: chiều rộng pixel, riêng beam là world-unit theo signature; `int height`: chiều cao pixel.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Clamp width/height tối thiểu 1, lưu width_/height_ và gọi glViewport. Ngăn phép chia aspect cho 0; projection được tính lại ở draw.

**Quan hệ — nơi gọi:** initialize và reshapeCallback.

**Gọi tiếp/API nổi bật:** `std::max`, `glViewport`.


<a id="f099"></a>
### Application::key

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::key(unsigned char value, bool pressed)
```

**Vị trí mã nguồn:** [src/Application.cpp:231](../src/Application.cpp#L231).

**Tham số:** `unsigned char value`: mã phím hoặc điều kiện bool theo signature; `bool pressed`: true=down, false=up.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chuyển ký tự sang lowercase, ghi keys_ cho cả press/release; release trở về ngay. Esc thoát; R reset camera/weather/animation/input. 3/1 đổi clouds/rain; 2 toggleLightning; L cập nhật observer rồi trigger; Space pause; F fog; N noon/midnight; [ và ] dịch giờ; T đổi auto; H in help; 4..6 đổi preset và releaseInput. Cuối handler refreshLighting để đổi môi trường ngay cả khi pause. R dùng defaults, không khôi phục giá trị CLI ban đầu.

**Quan hệ — nơi gọi:** keyDownCallback/keyUpCallback và smokeStep.

**Gọi tiếp/API nổi bật:** `std::tolower`, `requestExit`, `reset`, `resetAnimation`, `releaseInput`, `toggleLightning`, `position`, `triggerLightning`, `toggleDayNight`, `shiftHours`, `setAutoAdvance`, `autoAdvance`, `printTimeHelp`, `preset`, `refreshLighting`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f100"></a>
### Application::printTimeHelp

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::printTimeHelp() const
```

**Vị trí mã nguồn:** [src/Application.cpp:264](../src/Application.cpp#L264).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** In label HH:MM, trạng thái auto và dayDuration, sau đó hướng dẫn N/[ / ]/T/H. Không thay đổi state; ostream console có thể ảnh hưởng wall-time nếu output chậm.

**Quan hệ — nơi gọi:** initialize và key(H).

**Gọi tiếp/API nổi bật:** `label`, `autoAdvance`, `dayDuration`.


<a id="f101"></a>
### Application::updateStatistics

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::updateStatistics(Clock::time_point now)
```

**Vị trí mã nguồn:** [src/Application.cpp:270](../src/Application.cpp#L270).

**Tham số:** `Clock::time_point now`: mốc steady_clock.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chỉ xử lý khi cách statisticsTick_ ít nhất một giây. FPS=(frames_−statisticsFrames_)/elapsed. Nếu benchmark thì require(weather.valid()), lấy mẫu RAM/pool. Dựng title 128 byte bằng snprintf với giờ, AUTO, rain/splash/ripple, FLASH/ON/OFF và pause; cập nhật title, mốc thời gian và frame thống kê. Đây là thống kê khoảng, khác FPS trung bình cả benchmark.

**Quan hệ — nơi gọi:** tick.

**Gọi tiếp/API nổi bật:** `count`, `require`, `valid`, `sample`, `std::snprintf`, `data`, `size`, `label`, `c_str`, `autoAdvance`, `std::size_t`, `activeCount`, `active`, `glutSetWindowTitle`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f102"></a>
### Application::mouseButton

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::mouseButton(int button, int state, int x, int y)
```

**Vị trí mã nguồn:** [src/Application.cpp:291](../src/Application.cpp#L291).

**Tham số:** `int button`: mã nút FreeGLUT; `int state`: mã trạng thái FreeGLUT; `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chỉ nút trái điều khiển dragging_; GLUT_DOWN bật, các trạng thái khác tắt, đồng thời lưu tọa độ pixel chuột. Nút khác không có tác dụng.

**Quan hệ — nơi gọi:** mouseButtonCallback và smokeStep.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f103"></a>
### Application::mouseMotion

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::mouseMotion(int x, int y)
```

**Vị trí mã nguồn:** [src/Application.cpp:299](../src/Application.cpp#L299).

**Tham số:** `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu đang dragging, truyền chênh pixel từ lần trước tới Camera::rotate rồi lưu vị trí chuột hiện tại. Không xoay khi không giữ nút trái.

**Quan hệ — nơi gọi:** mouseMotionCallback và smokeStep.

**Gọi tiếp/API nổi bật:** `rotate`.


<a id="f104"></a>
### Application::releaseInput

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::releaseInput()
```

**Vị trí mã nguồn:** [src/Application.cpp:307](../src/Application.cpp#L307).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Xóa toàn bộ 256 key state và tắt dragging. Dùng khi rời/ẩn cửa sổ, reset, preset hoặc thoát để tránh tiếp tục đi vì mất sự kiện key-up.

**Quan hệ — nơi gọi:** key/reset/preset, entryCallback, visibilityCallback, requestExit.

**Gọi tiếp/API nổi bật:** `fill`.


<a id="f105"></a>
### Application::close

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::close()
```

**Vị trí mã nguồn:** [src/Application.cpp:312](../src/Application.cpp#L312).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Close callback còn context hiện hành: reset benchmark/renderer/scene, kiểm tra GL cleanup trong try/catch; lỗi in ra và exitCode_=1. Đặt window_=0 và exiting_=true để destructor không hủy lại cửa sổ đã đóng.

**Quan hệ — nơi gọi:** closeCallback.

**Gọi tiếp/API nổi bật:** `reset`, `checkOpenGL`, `what`.


<a id="f106"></a>
### Application::requestExit

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::requestExit()
```

**Vị trí mã nguồn:** [src/Application.cpp:327](../src/Application.cpp#L327).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Guard chống thoát lặp; đánh dấu exiting, xóa input, hủy benchmark/renderer/scene khi context còn sống, kiểm tra GL rồi glutLeaveMainLoop. Nếu checkOpenGL ném lỗi từ callback có wrapper thì fail tiếp nhận.

**Quan hệ — nơi gọi:** key(Esc), tick hết benchmark và smokeStep.

**Gọi tiếp/API nổi bật:** `releaseInput`, `reset`, `checkOpenGL`, `glutLeaveMainLoop`.


<a id="f107"></a>
### Application::fail

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::fail(const char* message)
```

**Vị trí mã nguồn:** [src/Application.cpp:339](../src/Application.cpp#L339).

**Tham số:** `const char* message`: thông báo lỗi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** In lỗi, đặt exitCode_=1; nếu còn cửa sổ thì đánh dấu thoát, giải phóng unique_ptr và rời main loop. Không ném tiếp exception qua biên callback C của FreeGLUT.

**Quan hệ — nơi gọi:** callback wrappers khi bắt std::exception.

**Gọi tiếp/API nổi bật:** `reset`, `glutLeaveMainLoop`.


<a id="f108"></a>
### Application::inspectFramebuffer

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::inspectFramebuffer(const std::string& capturePath) const
```

**Vị trí mã nguồn:** [src/Application.cpp:352](../src/Application.cpp#L352).

**Tham số:** `const std::string& capturePath`: đường dẫn PPM, rỗng thì không ghi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Yêu cầu ngân sách <10000 triangles và <80 draw calls; đọc depth và RGB GL_BACK. Kiểm tra mỗi depth∈[0, 1], số pixel geometry nằm giữa 2% và 98%, depth/culling bật, depth writes bật và blend tắt, viewport đúng kích thước. Nếu capturePath không rỗng, mở P6 binary, ghi header rồi đảo thứ tự hàng để ảnh đứng đúng, kiểm tra stream sau close. In số pixel geometry; đây là kiểm tra cấu trúc ảnh, không đánh giá thẩm mỹ.

**Quan hệ — nơi gọi:** smokeStep và lambda captureTime.

**Gọi tiếp/API nổi bật:** `require`, `triangles`, `drawCalls`, `depth`, `color`, `glReadBuffer`, `glPixelStorei`, `glReadPixels`, `data`, `checkOpenGL`, `glIsEnabled`, `glGetBooleanv`, `glGetIntegerv`, `empty`, `file`, `std::runtime_error`, `write`, `close`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f109"></a>
### Application::smokeStep

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::smokeStep()
```

**Vị trí mã nguồn:** [src/Application.cpp:394](../src/Application.cpp#L394).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Frame 1: tạo thư mục capture nếu yêu cầu; inspect framebuffer và đếm particle submit. Lưu baseline RGB/depth/lighting/hour; cô lập cloud/rain bằng giữ lighting, tắt splash/ripple để so pixel; toggle fog. Giả lập W press/release, kéo chuột, R; warmup; tắt auto lightning rồi L để kiểm tra geometry, flash, cache upload và camera invalidation. So brightened pixels và giới hạn trắng; pause vẫn đi camera; chờ hết strike; chỉ tăng animationTime để kiểm tra sway. Làm đất khô/ướt, probe median 5×5, flash nước; kiểm tra N/[ / ]/T/Space/R/preset và cache mưa. Capture ngày/hoàng hôn/đêm/chớp/nước/nhà, ba preset và tùy chọn tree detail; yêu cầu resize 640×400. Từ frame≥12: kiểm tra callback resize và framebuffer; nếu có cloud-wrap capture, chạy cloud riêng 23 giây, kiểm tra fade có pixel không ghi depth, render lặp giống hệt. Đánh dấu smokeCompleted rồi requestExit. Các bước render phụ không tăng frames_; nhánh capture optional cũng có assertion bổ sung.

**Quan hệ — nơi gọi:** display khi smokeTest.

**Gọi tiếp/API nổi bật:** `empty`, `std::filesystem::create_directories`, `inspectFramebuffer`, `require`, `renderedRainCount`, `renderedSplashCount`, `activeCount`, `readColor`, `hour`, `baselineDepth`, `glReadPixels`, `data`, `draw`, `checkOpenGL`, `key`, `changedPixels`, `renderAndRead`, `drops`, `size`, `reset`, `rejected`, `position`, `update`, `glm::distance`, `forward`, `mouseButton`, `mouseMotion`, `flash`, `autoAdvance`, `setHour`, `refreshLighting`, `active`, `cancel`, `lightningUploadCount`, `flashIntensity`, `rotate`, `int`, `elapsed`, `animationTime`, `revision`, `triangleCount`, `resetAnimation`, `projection`, `view`, `glm::vec4`, `push_back`, `std::sort`, `begin`, `end`, `std::filesystem::path`, `string`, `sample`, `setFlashPosition`, `rainUploadCount`, `wetness`, `valid`, `captureTime`, `triggerLightning`, `preset`, `std::string`, `std::to_string`, `move`, `target`, `glm::normalize`, `glm::degrees`, `std::atan2`, `std::asin`, `yaw`, `pitch`, `glutReshapeWindow`, `CloudSystem::coverage`, `clusters`, `fadeDepth`, `clearDepth`, `std::abs`, `requestExit`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f110"></a>
### Application::smokeStep::renderAndRead@406

**Khai báo chính xác tại định nghĩa:**

```cpp
[&]
```

**Vị trí mã nguồn:** [src/Application.cpp:406](../src/Application.cpp#L406).

**Tham số:** Không có tham số; lambda capture như khai báo.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Capture this bằng tham chiếu; gọi renderer.draw với state hiện tại, checkOpenGL rồi readColor. Trả vector RGB để smoke có thể đo chênh pixel ngay mà không swap.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `draw`, `checkOpenGL`, `readColor`.


<a id="f111"></a>
### Application::smokeStep::sample@545

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](const std::vector<unsigned char>& pixels,glm::vec3 world)
```

**Vị trí mã nguồn:** [src/Application.cpp:545](../src/Application.cpp#L545).

**Tham số:** `const std::vector<unsigned char>& pixels`: vector RGB byte theo bottom-up; `glm::vec3 world`: điểm world-space cần probe hoặc XZ shader.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chiếu điểm world qua P·V, require w>0, chia perspective và đổi sang pixel. Lấy trung bình RGB nguyên tại 25 pixel vùng 5×5, kiểm tra bounds từng mẫu, sort rồi trả median. Giảm ảnh hưởng một rain line cắt probe; không phải luminance quang học tuyến tính.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `projection`, `view`, `glm::vec4`, `require`, `push_back`, `int`, `std::sort`, `begin`, `end`, `size`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f112"></a>
### Application::smokeStep::captureTime@607

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](const char* name)
```

**Vị trí mã nguồn:** [src/Application.cpp:607](../src/Application.cpp#L607).

**Tham số:** `const char* name`: tên uniform hoặc tên capture/test theo hàm.

**Giá trị trả về:** void suy diễn; thay đổi dữ liệu qua capture/reference.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu đã cấu hình thư mục ảnh, ghép đường dẫn bằng filesystem::path rồi inspectFramebuffer; nếu rỗng thì không ghi. name là tên file PPM của từng pha ánh sáng.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `empty`, `inspectFramebuffer`, `std::filesystem::path`, `string`.


<a id="f113"></a>
### Application::displayCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::displayCallback()
```

**Vị trí mã nguồn:** [src/Application.cpp:705](../src/Application.cpp#L705).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** FreeGLUT gọi khi cần redraw; route active_->display trong try/catch std::exception, chuyển lỗi sang active_->fail. active_ phải được thiết lập trong run.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `display`, `fail`, `what`.


<a id="f114"></a>
### Application::timerCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::timerCallback(int)
```

**Vị trí mã nguồn:** [src/Application.cpp:709](../src/Application.cpp#L709).

**Tham số:** `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhận tham số timer int không dùng; route active_->tick với cùng hàng rào exception sang fail. Được run/tick đăng ký bằng glutTimerFunc.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `tick`, `fail`, `what`.


<a id="f115"></a>
### Application::idleCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::idleCallback()
```

**Vị trí mã nguồn:** [src/Application.cpp:713](../src/Application.cpp#L713).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Idle callback không tham số; gọi timerCallback(0) để dùng lại tick và hàng rào lỗi trong chế độ benchmark-unlocked. Không tự tạo fixed timestep.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `timerCallback`.


<a id="f116"></a>
### Application::reshapeCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::reshapeCallback(int w, int h)
```

**Vị trí mã nguồn:** [src/Application.cpp:714](../src/Application.cpp#L714).

**Tham số:** `int w`: chiều rộng callback pixel; `int h`: chiều cao callback pixel.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Route kích thước w/h từ FreeGLUT tới reshape. Hàm đích clamp và glViewport; wrapper không có try/catch.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `reshape`.


<a id="f117"></a>
### Application::keyDownCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::keyDownCallback(unsigned char k, int, int)
```

**Vị trí mã nguồn:** [src/Application.cpp:715](../src/Application.cpp#L715).

**Tham số:** `unsigned char k`: mã phím callback; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhận k và hai tọa độ chuột không dùng; gọi key(k, true) trong try/catch, fail nếu exception. Khi register đã tắt autorepeat.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `key`, `fail`, `what`.


<a id="f118"></a>
### Application::keyUpCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::keyUpCallback(unsigned char k, int, int)
```

**Vị trí mã nguồn:** [src/Application.cpp:719](../src/Application.cpp#L719).

**Tham số:** `unsigned char k`: mã phím callback; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm; `int`: tham số không có tên; callback bỏ qua hoặc đối tượng copy bị cấm.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhận k và tọa độ không dùng; gọi key(k, false), chỉ xóa key state rồi return trong key. Không có try/catch riêng.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `key`.


<a id="f119"></a>
### Application::mouseButtonCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::mouseButtonCallback(int b, int s, int x, int y)
```

**Vị trí mã nguồn:** [src/Application.cpp:720](../src/Application.cpp#L720).

**Tham số:** `int b`: điểm/vector/scalar thứ hai theo signature; `int s`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới; `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chuyển nguyên button/state/x/y của FreeGLUT tới mouseButton. Callback chỉ đổi trạng thái kéo chuột.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `mouseButton`.


<a id="f120"></a>
### Application::mouseMotionCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::mouseMotionCallback(int x, int y)
```

**Vị trí mã nguồn:** [src/Application.cpp:721](../src/Application.cpp#L721).

**Tham số:** `int x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `int y`: tọa độ pixel chuột.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chuyển x/y tới mouseMotion; chỉ xoay camera nếu dragging. Không trực tiếp render hoặc update thời tiết.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `mouseMotion`.


<a id="f121"></a>
### Application::entryCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::entryCallback(int state)
```

**Vị trí mã nguồn:** [src/Application.cpp:722](../src/Application.cpp#L722).

**Tham số:** `int state`: mã trạng thái FreeGLUT.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khi state==GLUT_LEFT thì releaseInput; các trạng thái khác không làm gì. Ngăn kẹt phím khi con trỏ rời cửa sổ.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `releaseInput`.


<a id="f122"></a>
### Application::visibilityCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::visibilityCallback(int state)
```

**Vị trí mã nguồn:** [src/Application.cpp:723](../src/Application.cpp#L723).

**Tham số:** `int state`: mã trạng thái FreeGLUT.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** visible_=(state==GLUT_VISIBLE); khi bị ẩn/occluded thì releaseInput. tick thường bỏ update khi không visible; benchmark vẫn chạy.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `releaseInput`.


<a id="f123"></a>
### Application::closeCallback

**Khai báo chính xác tại định nghĩa:**

```cpp
void Application::closeCallback()
```

**Vị trí mã nguồn:** [src/Application.cpp:727](../src/Application.cpp#L727).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chuyển close event tới active_->close khi FreeGLUT còn context để tài nguyên GPU được xóa đúng lúc.

**Quan hệ — nơi gọi:** FreeGLUT gọi sau đăng ký trong initialize/run/tick; idleCallback còn gọi timerCallback.

**Gọi tiếp/API nổi bật:** `close`.


<a id="file-29"></a>
## src/Benchmark.cpp

**Module/chức năng:** Đo CPU/GPU/RAM và thống kê frame.

**Headers/phụ thuộc trực tiếp:** `Benchmark.hpp`, `GL/glxew.h`, `algorithm`, `cstdio`, `iostream`, `sys/resource.h`, `unistd.h`, `vector`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Benchmark.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [cpuSeconds](#f124) | Không có. | `double`; xem ý nghĩa và điều kiện biên trong quy trình. | Gọi getrusage(RUSAGE_SELF); cộng user+system time, đổi microsecond sang second. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [residentBytes](#f125) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Mở /proc/self/statm, đọc total và resident pages; lỗi đọc/mở thì resident giữ 0. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [highWaterBytes](#f126) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Mở /proc/self/status; đọc từng dòng bằng fgets, tìm VmHWM bằng sscanf; đóng FILE, đổi kB sang byte. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [Benchmark::Benchmark](#f127) | `bool unlocked`: yêu cầu bỏ timer và swap interval 0. | Constructor/destructor: không có kiểu trả về. | Sinh bốn GL query, chụp CPU/RSS ban đầu. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [Benchmark::~Benchmark](#f128) | Không có. | Constructor/destructor: không có kiểu trả về. | Nếu query còn active thì glEndQuery, sau đó glDeleteQueries cho cả bốn ID. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [Benchmark::beginFrame](#f129) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Lấy Clock::now; từ frame thứ hai ghi interval vào array vòng 65536 slot và tăng intervalCount_. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [Benchmark::endFrame](#f130) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu active_ hợp lệ thì end query, đánh dấu pending và xóa active. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [Benchmark::sample](#f131) | `const WeatherSystem& weather`: trạng thái mô phỏng/lighting CPU; `double seconds`: số giây (duration hoặc dt theo hàm); `double fps`: frame/giây của khoảng thống kê. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đọc RSS cuối, cập nhật peak splash/ripple theo mẫu khoảng một giây; tăng stateSamples và số mẫu splash đầy. | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [Benchmark::report](#f132) | `std::ostream& output`: ostream nhận report; `double seconds`: số giây (duration hoặc dt theo hàm); `int frames`: số display frame đã đếm. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Copy tối đa min(intervalCount, 65536) interval, sort rồi quantile chỉ số floor((N−1)p). | Application::initialize/display/tick/updateStatistics và destructor cleanup. |
| [Benchmark::report::quantile@118](#f133) | `double p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Nhận p là phân vị [0, 1] từ report, trả sorted[floor((N−1)p)], hoặc 0 nếu rỗng. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |


<a id="f124"></a>
### cpuSeconds

**Khai báo chính xác tại định nghĩa:**

```cpp
double cpuSeconds()
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:11](../src/Benchmark.cpp#L11).

**Tham số:** Không có.

**Giá trị trả về:** `double`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Gọi getrusage(RUSAGE_SELF); cộng user+system time, đổi microsecond sang second. Trả tổng CPU time của tiến trình, không phải steady-clock wall-time; mã không kiểm tra return code của getrusage.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `getrusage`.


<a id="f125"></a>
### residentBytes

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t residentBytes()
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:17](../src/Benchmark.cpp#L17).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Mở /proc/self/statm, đọc total và resident pages; lỗi đọc/mở thì resident giữ 0. Đóng FILE; nhân resident với sysconf(_SC_PAGESIZE) để trả RSS byte. Linux-specific, không đo VRAM.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `std::fopen`, `std::fscanf`, `std::fclose`, `sysconf`.


<a id="f126"></a>
### highWaterBytes

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t highWaterBytes()
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:23](../src/Benchmark.cpp#L23).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Mở /proc/self/status; đọc từng dòng bằng fgets, tìm VmHWM bằng sscanf; đóng FILE, đổi kB sang byte. Không có dữ liệu thì trả 0, không phân biệt 0 thật và lỗi đo.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `std::fopen`, `std::fgets`, `std::sscanf`, `std::fclose`.


<a id="f127"></a>
### Benchmark::Benchmark

**Khai báo chính xác tại định nghĩa:**

```cpp
Benchmark::Benchmark(bool unlocked)
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:36](../src/Benchmark.cpp#L36).

**Tham số:** `bool unlocked`: yêu cầu bỏ timer và swap interval 0.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Sinh bốn GL query, chụp CPU/RSS ban đầu. Khi unlocked yêu cầu GLX swap interval 0 qua EXT hoặc MESA nếu hỗ trợ và in interval khi có thể; cảnh báo compositor vẫn có thể giới hạn. Không thay cấu hình driver hệ thống; Benchmark chỉ tồn tại khi benchmarkSeconds>0.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `glGenQueries`, `size`, `data`, `cpuSeconds`, `residentBytes`, `glxewInit`, `glXGetCurrentDisplay`, `glXGetCurrentDrawable`, `glXSwapIntervalEXT`, `glXQueryDrawable`, `glXSwapIntervalMESA`, `glXGetSwapIntervalMESA`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f128"></a>
### Benchmark::~Benchmark

**Khai báo chính xác tại định nghĩa:**

```cpp
Benchmark::~Benchmark()
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:59](../src/Benchmark.cpp#L59).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu query còn active thì glEndQuery, sau đó glDeleteQueries cho cả bốn ID. Cần GL context còn sống; Application hủy Benchmark trước cửa sổ.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `glEndQuery`, `glDeleteQueries`, `size`, `data`.


<a id="f129"></a>
### Benchmark::beginFrame

**Khai báo chính xác tại định nghĩa:**

```cpp
void Benchmark::beginFrame()
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:64](../src/Benchmark.cpp#L64).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Lấy Clock::now; từ frame thứ hai ghi interval vào array vòng 65536 slot và tăng intervalCount_. Lưu frameStart_; duyệt pending query, chỉ lấy RESULT khi AVAILABLE, cộng ns/1e6 vào gpuMs_ và đếm mẫu. Chọn query trống đầu tiên để bắt GL_TIME_ELAPSED; nếu cả bốn pending thì frame này không có GPU sample, không block chờ.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `Clock::now`, `size`, `count`, `glGetQueryObjectiv`, `glGetQueryObjectui64v`, `glBeginQuery`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f130"></a>
### Benchmark::endFrame

**Khai báo chính xác tại định nghĩa:**

```cpp
void Benchmark::endFrame()
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:89](../src/Benchmark.cpp#L89).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu active_ hợp lệ thì end query, đánh dấu pending và xóa active. Cộng Clock::now−frameStart_ vào submitMs_, tăng renderCount_. Khoảng wall này gồm CPU gọi draw/checkOpenGL và chi phí beginFrame sau mốc bắt đầu, không gồm simulation và swap.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `glEndQuery`, `Clock::now`, `count`.


<a id="f131"></a>
### Benchmark::sample

**Khai báo chính xác tại định nghĩa:**

```cpp
void Benchmark::sample(const WeatherSystem& weather, double seconds, double fps)
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:99](../src/Benchmark.cpp#L99).

**Tham số:** `const WeatherSystem& weather`: trạng thái mô phỏng/lighting CPU; `double seconds`: số giây (duration hoặc dt theo hàm); `double fps`: frame/giây của khoảng thống kê.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đọc RSS cuối, cập nhật peak splash/ripple theo mẫu khoảng một giây; tăng stateSamples và số mẫu splash đầy. Khi floor(seconds/60) đổi, in TRACE gồm FPS/RSS/pool/strikes/wetness/rejections. Chữ validation=ok dựa trên require(weather.valid()) của Application trước lời gọi này.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `residentBytes`, `std::max`, `activeCount`, `strikeCount`, `wetness`, `rejected`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f132"></a>
### Benchmark::report

**Khai báo chính xác tại định nghĩa:**

```cpp
void Benchmark::report(std::ostream& output, double seconds, int frames) const
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:114](../src/Benchmark.cpp#L114).

**Tham số:** `std::ostream& output`: ostream nhận report; `double seconds`: số giây (duration hoặc dt theo hàm); `int frames`: số display frame đã đếm.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Copy tối đa min(intervalCount, 65536) interval, sort rồi quantile chỉ số floor((N−1)p). In avg_frame_ms=seconds×1000/max(frames, 1), P95/P99, submit/GPU bình quân mẫu, CPU usage từ rusage/seconds, RSS/VmHWM, pool peaks và saturation. Không flush query pending trước report; average cả lượt khác cửa sổ quantile khi vượt 65536 mẫu.

**Quan hệ — nơi gọi:** Application::initialize/display/tick/updateStatistics và destructor cleanup.

**Gọi tiếp/API nổi bật:** `std::min`, `size`, `sorted`, `begin`, `std::sort`, `end`, `empty`, `std::max`, `quantile`, `cpuSeconds`, `highWaterBytes`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f133"></a>
### Benchmark::report::quantile@118

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](double p)
```

**Vị trí mã nguồn:** [src/Benchmark.cpp:118](../src/Benchmark.cpp#L118).

**Tham số:** `double p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhận p là phân vị [0, 1] từ report, trả sorted[floor((N−1)p)], hoặc 0 nếu rỗng. Không nội suy giữa hai mẫu; report chỉ gọi p=0.95 và 0.99.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `empty`, `size`.


<a id="file-30"></a>
## src/Camera.cpp

**Module/chức năng:** Camera tự do và phép chiếu.

**Headers/phụ thuộc trực tiếp:** `Camera.hpp`, `algorithm`, `cmath`, `glm/gtc/matrix_transform.hpp`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Camera.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Camera::Camera](#f134) | Không có. | Constructor/destructor: không có kiểu trả về. | Gọi reset ngay khi dựng Camera để position/yaw/pitch/forward nhất quán; speed/up/FOV giữ default từ header.. | Application input/update, Renderer::draw, smoke và core_tests. |
| [Camera::reset](#f135) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đặt position=(14, 10, 18), yaw=−128°, pitch=−24° rồi updateDirection. | Application input/update, Renderer::draw, smoke và core_tests. |
| [Camera::preset](#f136) | `unsigned index`: chỉ số phần tử/preset theo hàm. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | index=0 gọi reset rồi return; index>2 bỏ qua. | Application input/update, Renderer::draw, smoke và core_tests. |
| [Camera::updateDirection](#f137) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đổi yaw/pitch độ sang radian; forward=normalize((cos yaw cos pitch, sin pitch, sin yaw cos pitch)). | Application input/update, Renderer::draw, smoke và core_tests. |
| [Camera::rotate](#f138) | `float deltaX`: chênh chuột ngang pixel; `float deltaY`: chênh chuột dọc pixel. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tăng yaw theo deltaX×0.12 độ/pixel và remainder 360; giảm pitch theo deltaY×0.12 rồi clamp ±89°. | Application input/update, Renderer::draw, smoke và core_tests. |
| [Camera::move](#f139) | `const glm::vec3& localDirection`: hướng nhập local camera x phải, y lên, z tiến; `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bỏ dt không hữu hạn/≤0 hoặc direction local bằng 0. | Application input/update, Renderer::draw, smoke và core_tests. |
| [Camera::view](#f140) | Không có. | `glm::mat4`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả glm::lookAt(position, position+forward, up). | Application input/update, Renderer::draw, smoke và core_tests. |
| [Camera::projection](#f141) | `float aspect`: width/height không có đơn vị. | `glm::mat4`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả perspective với FOV dọc 55° đổi radian, aspect=max(aspect, 0.001), near=0.1, far=80. | Application input/update, Renderer::draw, smoke và core_tests. |


<a id="f134"></a>
### Camera::Camera

**Khai báo chính xác tại định nghĩa:**

```cpp
Camera::Camera()
```

**Vị trí mã nguồn:** [src/Camera.cpp:7](../src/Camera.cpp#L7).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Gọi reset ngay khi dựng Camera để position/yaw/pitch/forward nhất quán; speed/up/FOV giữ default từ header.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `reset`.


<a id="f135"></a>
### Camera::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void Camera::reset()
```

**Vị trí mã nguồn:** [src/Camera.cpp:9](../src/Camera.cpp#L9).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đặt position=(14, 10, 18), yaw=−128°, pitch=−24° rồi updateDirection. Không tác động thời tiết; preset(0) dùng lại reset.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `updateDirection`.


<a id="f136"></a>
### Camera::preset

**Khai báo chính xác tại định nghĩa:**

```cpp
void Camera::preset(unsigned index)
```

**Vị trí mã nguồn:** [src/Camera.cpp:16](../src/Camera.cpp#L16).

**Tham số:** `unsigned index`: chỉ số phần tử/preset theo hàm.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** index=0 gọi reset rồi return; index>2 bỏ qua. index=1 đặt (5, 3.2, 8) nhìn (−1, 1.4, −1); index=2 đặt (−0.4, 0.85, 7) nhìn (−2, 0.12, 4.5). Normalize target−eye; yaw=degrees(atan2(z, x)), pitch=clamp(degrees(asin(y)), −89, 89), rồi updateDirection.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `reset`, `glm::vec3`, `glm::normalize`, `glm::degrees`, `std::atan2`, `std::clamp`, `std::asin`, `updateDirection`.


<a id="f137"></a>
### Camera::updateDirection

**Khai báo chính xác tại định nghĩa:**

```cpp
void Camera::updateDirection()
```

**Vị trí mã nguồn:** [src/Camera.cpp:27](../src/Camera.cpp#L27).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đổi yaw/pitch độ sang radian; forward=normalize((cos yaw cos pitch, sin pitch, sin yaw cos pitch)). Ví dụ yaw=0, pitch=0 nhìn +X. Không dùng quaternion hoặc roll.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `glm::radians`, `glm::normalize`, `std::cos`, `std::sin`.


<a id="f138"></a>
### Camera::rotate

**Khai báo chính xác tại định nghĩa:**

```cpp
void Camera::rotate(float deltaX, float deltaY)
```

**Vị trí mã nguồn:** [src/Camera.cpp:35](../src/Camera.cpp#L35).

**Tham số:** `float deltaX`: chênh chuột ngang pixel; `float deltaY`: chênh chuột dọc pixel.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tăng yaw theo deltaX×0.12 độ/pixel và remainder 360; giảm pitch theo deltaY×0.12 rồi clamp ±89°. updateDirection duy trì forward đơn vị; giới hạn pitch tránh lookAt suy biến với up.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `std::remainder`, `std::clamp`, `updateDirection`.


<a id="f139"></a>
### Camera::move

**Khai báo chính xác tại định nghĩa:**

```cpp
void Camera::move(const glm::vec3& localDirection, float deltaTime)
```

**Vị trí mã nguồn:** [src/Camera.cpp:41](../src/Camera.cpp#L41).

**Tham số:** `const glm::vec3& localDirection`: hướng nhập local camera x phải, y lên, z tiến; `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bỏ dt không hữu hạn/≤0 hoặc direction local bằng 0. Tính right=normalize(cross(forward, up)), ghép hướng local, normalize để giữ tốc độ đường chéo bằng tốc độ thẳng. Cộng direction×5×dt rồi clamp position trong (−16, 0.35, −16)..(16, 14, 18). Di chuyển theo forward có cả thành phần Y; không có va chạm nhà/cây/đá. dt=0.1 cho quãng đường 0.5 nếu chưa chạm biên.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `glm::dot`, `glm::normalize`, `glm::cross`, `glm::clamp`, `glm::vec3`.


<a id="f140"></a>
### Camera::view

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::mat4 Camera::view() const
```

**Vị trí mã nguồn:** [src/Camera.cpp:55](../src/Camera.cpp#L55).

**Tham số:** Không có.

**Giá trị trả về:** `glm::mat4`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả glm::lookAt(position, position+forward, up). Biến world thành eye space: eye→origin, hướng nhìn→−Z; không tự thay đổi position.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `glm::lookAt`.


<a id="f141"></a>
### Camera::projection

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::mat4 Camera::projection(float aspect) const
```

**Vị trí mã nguồn:** [src/Camera.cpp:59](../src/Camera.cpp#L59).

**Tham số:** `float aspect`: width/height không có đơn vị.

**Giá trị trả về:** `glm::mat4`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả perspective với FOV dọc 55° đổi radian, aspect=max(aspect, 0.001), near=0.1, far=80. Đầu vào là tỉ số width/height, không phải số pixel. Không tự kiểm tra aspect NaN.

**Quan hệ — nơi gọi:** Application input/update, Renderer::draw, smoke và core_tests.

**Gọi tiếp/API nổi bật:** `glm::perspective`, `glm::radians`, `std::max`.


<a id="file-31"></a>
## src/CloudSystem.cpp

**Module/chức năng:** Dữ liệu và chuyển động bốn cụm mây.

**Headers/phụ thuộc trực tiếp:** `CloudSystem.hpp`, `algorithm`, `cmath`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/CloudSystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [wrap](#f142) | `float x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Tính x−20 floor((x+10)/20), ánh xạ về [−10, 10). | WeatherSystem, LightningSystem, Renderer và CPU tests. |
| [CloudSystem::reset](#f143) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Ghi bốn vị trí tâm, proportions và skew cố định; mỗi cluster có speed=0.20+0.025i và năm lobe offset/scale/shade. | WeatherSystem, LightningSystem, Renderer và CPU tests. |
| [CloudSystem::update](#f144) | `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `const glm::vec3& windVelocity`: vector gió chung, chỉ XZ tác động. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bỏ dt không hữu hạn/≤0; clamp tối đa 0.1. | WeatherSystem, LightningSystem, Renderer và CPU tests. |
| [CloudSystem::coverage](#f145) | `glm::vec3 center`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | edge=10−max(abs(center.x), abs(center.z)); t=clamp(edge/2.5, 0, 1), trả t²(3−2t). | WeatherSystem, LightningSystem, Renderer và CPU tests. |


<a id="f142"></a>
### wrap

**Khai báo chính xác tại định nghĩa:**

```cpp
float wrap(float x)
```

**Vị trí mã nguồn:** [src/CloudSystem.cpp:7](../src/CloudSystem.cpp#L7).

**Tham số:** `float x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tính x−20 floor((x+10)/20), ánh xạ về [−10, 10). Ví dụ 10.2→−9.8. Chỉ wrap tọa độ ngang, không fade tại đây.

**Quan hệ — nơi gọi:** WeatherSystem, LightningSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `std::floor`.


<a id="f143"></a>
### CloudSystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void CloudSystem::reset()
```

**Vị trí mã nguồn:** [src/CloudSystem.cpp:12](../src/CloudSystem.cpp#L12).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Ghi bốn vị trí tâm, proportions và skew cố định; mỗi cluster có speed=0.20+0.025i và năm lobe offset/scale/shade. Ba lobe đáy rộng, hai lobe đỉnh bất đối xứng. Renderer dùng offsets/scales để dựng skin một lần; trường shade hiện không được renderer/mesh builder đọc.

**Quan hệ — nơi gọi:** WeatherSystem, LightningSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `glm::vec3`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f144"></a>
### CloudSystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void CloudSystem::update(float deltaTime, const glm::vec3& windVelocity)
```

**Vị trí mã nguồn:** [src/CloudSystem.cpp:34](../src/CloudSystem.cpp#L34).

**Tham số:** `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `const glm::vec3& windVelocity`: vector gió chung, chỉ XZ tác động.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bỏ dt không hữu hạn/≤0; clamp tối đa 0.1. Cộng (wind.x, 0, wind.z)×cloud.speed×dt cho mỗi cluster, wrap x và z. Không đổi y hoặc hình dạng lobe.

**Quan hệ — nơi gọi:** WeatherSystem, LightningSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::min`, `glm::vec3`, `wrap`.


<a id="f145"></a>
### CloudSystem::coverage

**Khai báo chính xác tại định nghĩa:**

```cpp
float CloudSystem::coverage(glm::vec3 center)
```

**Vị trí mã nguồn:** [src/CloudSystem.cpp:44](../src/CloudSystem.cpp#L44).

**Tham số:** `glm::vec3 center`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** edge=10−max(abs(center.x), abs(center.z)); t=clamp(edge/2.5, 0, 1), trả t²(3−2t). Tâm cách biên≥2.5 thì coverage=1, ở biên=0. Chỉ alpha của cả skin; không dùng discard/dither.

**Quan hệ — nơi gọi:** WeatherSystem, LightningSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `std::max`, `std::abs`, `std::clamp`.


<a id="file-32"></a>
## src/DynamicBatch.cpp

**Module/chức năng:** VAO/VBO động cho các hiệu ứng.

**Headers/phụ thuộc trực tiếp:** `DynamicBatch.hpp`, `limits`, `stdexcept`, `type_traits`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/DynamicBatch.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [DynamicBatch::DynamicBatch](#f146) | `std::size_t capacity`: số vertex tối đa; `GLenum primitive`: GL_LINES hoặc GL_TRIANGLES. | Constructor/destructor: không có kiểu trả về. | Kiểm tra capacity>0 và vừa GLsizei, ghi primitive; cấp một VAO/VBO và glBufferData(capacity×sizeof(EffectVertex), nullptr, GL_DYNAMIC_DRAW). | Renderer constructor/draw; destructor khi Renderer bị hủy. |
| [DynamicBatch::~DynamicBatch](#f147) | Không có. | Constructor/destructor: không có kiểu trả về. | Xóa VBO rồi VAO bằng glDeleteBuffers/glDeleteVertexArrays. | Renderer constructor/draw; destructor khi Renderer bị hủy. |
| [DynamicBatch::upload](#f148) | `const EffectVertex* vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `std::size_t count`: số vertex upload. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu count>capacity thì length_error. | Renderer constructor/draw; destructor khi Renderer bị hủy. |
| [DynamicBatch::draw](#f149) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | count_=0 bỏ qua; ngược lại bind VAO và glDrawArrays(primitive_, 0, count_). | Renderer constructor/draw; destructor khi Renderer bị hủy. |


<a id="f146"></a>
### DynamicBatch::DynamicBatch

**Khai báo chính xác tại định nghĩa:**

```cpp
DynamicBatch::DynamicBatch(std::size_t capacity, GLenum primitive)
```

**Vị trí mã nguồn:** [src/DynamicBatch.cpp:7](../src/DynamicBatch.cpp#L7).

**Tham số:** `std::size_t capacity`: số vertex tối đa; `GLenum primitive`: GL_LINES hoặc GL_TRIANGLES.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra capacity>0 và vừa GLsizei, ghi primitive; cấp một VAO/VBO và glBufferData(capacity×sizeof(EffectVertex), nullptr, GL_DYNAMIC_DRAW). Khai báo attr 0 position vec3, 1 color vec4, 2 edge float bằng offsetof/stride; unbind. Không có EBO hoặc buffer allocation mỗi upload.

**Quan hệ — nơi gọi:** Renderer constructor/draw; destructor khi Renderer bị hủy.

**Gọi tiếp/API nổi bật:** `max`, `std::invalid_argument`, `glGenVertexArrays`, `glGenBuffers`, `glBindVertexArray`, `glBindBuffer`, `glBufferData`, `glEnableVertexAttribArray`, `glVertexAttribPointer`, `offsetof`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f147"></a>
### DynamicBatch::~DynamicBatch

**Khai báo chính xác tại định nghĩa:**

```cpp
DynamicBatch::~DynamicBatch()
```

**Vị trí mã nguồn:** [src/DynamicBatch.cpp:31](../src/DynamicBatch.cpp#L31).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Xóa VBO rồi VAO bằng glDeleteBuffers/glDeleteVertexArrays. Cần context hiện hành; copy đã delete để tránh hai object cùng xóa ID.

**Quan hệ — nơi gọi:** Renderer constructor/draw; destructor khi Renderer bị hủy.

**Gọi tiếp/API nổi bật:** `glDeleteBuffers`, `glDeleteVertexArrays`.


<a id="f148"></a>
### DynamicBatch::upload

**Khai báo chính xác tại định nghĩa:**

```cpp
void DynamicBatch::upload(const EffectVertex* vertices, std::size_t count)
```

**Vị trí mã nguồn:** [src/DynamicBatch.cpp:36](../src/DynamicBatch.cpp#L36).

**Tham số:** `const EffectVertex* vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `std::size_t count`: số vertex upload.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu count>capacity thì length_error. Ghi count_; count=0 thì return, không upload và không tăng uploads_. Ngược lại bind VBO, glBufferSubData vùng [0, count), tăng counter rồi unbind. Không kiểm tra con trỏ null cho count>0 vì caller cung cấp staging array hợp lệ.

**Quan hệ — nơi gọi:** Renderer constructor/draw; destructor khi Renderer bị hủy.

**Gọi tiếp/API nổi bật:** `std::length_error`, `glBindBuffer`, `glBufferSubData`.


<a id="f149"></a>
### DynamicBatch::draw

**Khai báo chính xác tại định nghĩa:**

```cpp
void DynamicBatch::draw() const
```

**Vị trí mã nguồn:** [src/DynamicBatch.cpp:46](../src/DynamicBatch.cpp#L46).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** count_=0 bỏ qua; ngược lại bind VAO và glDrawArrays(primitive_, 0, count_). draw không tăng counter thống kê renderer; Renderer làm việc đó.

**Quan hệ — nơi gọi:** Renderer constructor/draw; destructor khi Renderer bị hủy.

**Gọi tiếp/API nổi bật:** `glBindVertexArray`, `glDrawArrays`.


<a id="file-33"></a>
## src/EnvironmentGeometry.cpp

**Module/chức năng:** Sinh mesh nước/đá/mây/cỏ hiện tại.

**Headers/phụ thuộc trực tiếp:** `EnvironmentGeometry.hpp`, `Surface.hpp`, `ModelGeometry.hpp`, `algorithm`, `array`, `cmath`, `random`, `stdexcept`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/EnvironmentGeometry.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [triangle](#f150) | `std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 outward`: hướng tham chiếu để sửa winding. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Dựa outward để kiểm tra dot(cross(b−a, c−a), outward); nếu âm đổi b/c, tính normal đơn vị và thêm ba Vertex. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [grassAllowed](#f151) | `glm::vec2 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | `bool`; xem ý nghĩa và điều kiện biên trong quy trình. | Loại p ngoài ±9.6; footprint nhà/bậc; hành lang đường đi cong; khoảng cách gốc bốn cây <0.7; khoảng cách đá theo scale; năm probe tâm/offset 0.2 đụng nước. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [grassTufts](#f152) | Không có. | `const std::array<GrassTuft,64>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả const reference tới array 64 GrassTuft static khởi tạo bởi lambda một lần. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [grassTufts::tufts@34](#f153) | Không có tham số; lambda capture như khai báo. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Seed mt19937=824, lấy candidate x/z trong ±9.5, tối đa 4096 lần; grassAllowed sai thì continue. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [grass](#f154) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Reserve 64×18 vertex; mỗi tuft tạo ba lá với góc lệch 2π/3, nửa bề rộng 0.045 và chiều cao giảm 12% theo blade. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [puddles](#f155) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Duyệt ba surface puddle, lấy world polygon dùng chung hit-test; mỗi cạnh tạo fan triangle với tâm p.center và normal hướng +Y. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [rocks](#f156) | Không có. | `const std::array<RockInstance,3>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả const reference ba RockInstance static có position, scale, color, yaw, seed riêng: 137/251/389. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [rock](#f157) | `std::uint32_t seed`: seed uint32 cho RNG. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Seed RNG; tám góc lệch ngẫu nhiên và bán kính 0.82..1.12. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [rock::random@91](#f158) | `float low`: cận dưới random; `float high`: cận trên random. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Capture RNG bằng tham chiếu, tạo uniform_real_distribution<float>(low, high) và lấy một số. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [rock::add@108](#f159) | `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba. | void suy diễn; thay đổi dữ liệu qua capture/reference. | Chuyển ba điểm tới helper triangle, dùng centroid (a+b+c)/3 làm vector outward ước lượng từ gốc đá. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [cloud](#f160) | `const CloudCluster& cluster`: năm lobe cấu hình skin mây. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Kiểm tra từng lobe scale dương và độ dài scale/offset hữu hạn. | Scene/Renderer khởi tạo, placement helpers và environment_tests. |
| [cloud::field@129](#f161) | `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể; `glm::vec3* normal=nullptr`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Với từng lobe, scale.y nhân 1.16; delta=p−offset; cộng exp(−2 dot(delta/scale, delta/scale)) vào density và value×delta/(scale ²) vào gradient hướng ngoài. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [cloud::add@161](#f162) | `Vertex a`: điểm/vector/scalar đầu theo signature; `Vertex b`: điểm/vector/scalar thứ hai theo signature; `Vertex c`: đỉnh thứ ba. | void suy diễn; thay đổi dữ liệu qua capture/reference. | Tính cross cạnh tam giác; nếu ngược tổng normal đỉnh thì đổi b/c và dấu face normal. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |


<a id="f150"></a>
### triangle

**Khai báo chính xác tại định nghĩa:**

```cpp
void triangle(std::vector<Vertex>& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 outward)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:11](../src/EnvironmentGeometry.cpp#L11).

**Tham số:** `std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 outward`: hướng tham chiếu để sửa winding.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Dựa outward để kiểm tra dot(cross(b−a, c−a), outward); nếu âm đổi b/c, tính normal đơn vị và thêm ba Vertex. Bảo đảm winding hướng ra cho đá, cỏ, nước. Tam giác suy biến vẫn có thể normalize zero nếu caller sai.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** `glm::dot`, `glm::cross`, `std::swap`, `glm::normalize`, `push_back`.


<a id="f151"></a>
### grassAllowed

**Khai báo chính xác tại định nghĩa:**

```cpp
bool grassAllowed(glm::vec2 p)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:19](../src/EnvironmentGeometry.cpp#L19).

**Tham số:** `glm::vec2 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** `bool`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Loại p ngoài ±9.6; footprint nhà/bậc; hành lang đường đi cong; khoảng cách gốc bốn cây <0.7; khoảng cách đá theo scale; năm probe tâm/offset 0.2 đụng nước. Còn lại trả true. Đây là quy tắc placement CPU, không phải collision động hay kiểm tra toàn bộ diện tích từng lá.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** `std::abs`, `std::sin`, `models::treeLayout`, `glm::distance`, `glm::vec2`, `rocks`, `std::max`, `surface::waterMargin`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f152"></a>
### grassTufts

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<GrassTuft,64>& grassTufts()
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:33](../src/EnvironmentGeometry.cpp#L33).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<GrassTuft,64>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả const reference tới array 64 GrassTuft static khởi tạo bởi lambda một lần. Tránh RNG/placement mỗi frame. Nếu lambda không tìm đủ vị trí sẽ ném runtime_error.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** `rng`, `coordinate`, `unit`, `size`, `p`, `grassAllowed`, `std::runtime_error`.


<a id="f153"></a>
### grassTufts::tufts@34

**Khai báo chính xác tại định nghĩa:**

```cpp
[]
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:34](../src/EnvironmentGeometry.cpp#L34).

**Tham số:** Không có tham số; lambda capture như khai báo.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Seed mt19937=824, lấy candidate x/z trong ±9.5, tối đa 4096 lần; grassAllowed sai thì continue. Chấp nhận đặt y=0.005, height=0.10+U×0.12, angle=U×2π. Require đủ 64 trước trả array. Các tham số này tạo mẫu deterministic cho cùng implementation thư viện chuẩn.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `rng`, `coordinate`, `unit`, `size`, `p`, `grassAllowed`, `std::runtime_error`.


<a id="f154"></a>
### grass

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> grass()
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:50](../src/EnvironmentGeometry.cpp#L50).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reserve 64×18 vertex; mỗi tuft tạo ba lá với góc lệch 2π/3, nửa bề rộng 0.045 và chiều cao giảm 12% theo blade. Tính normal, thêm tam giác xuôi và đảo để hai mặt hiển thị trong khi global culling bật. Tổng 384 triangles, không animate grass.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** `reserve`, `grassTufts`, `size`, `side`, `std::cos`, `std::sin`, `glm::vec3`, `glm::normalize`, `glm::cross`, `triangle`.


<a id="f155"></a>
### puddles

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> puddles()
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:65](../src/EnvironmentGeometry.cpp#L65).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Duyệt ba surface puddle, lấy world polygon dùng chung hit-test; mỗi cạnh tạo fan triangle với tâm p.center và normal hướng +Y. Reserve 3×32×3 vertex, gộp 96 triangles thành một mesh. Mặt ở y=0.012, render opaque.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** `reserve`, `surface::puddles`, `size`, `surface::polygon`, `triangle`.


<a id="f156"></a>
### rocks

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<RockInstance,3>& rocks()
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:79](../src/EnvironmentGeometry.cpp#L79).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<RockInstance,3>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả const reference ba RockInstance static có position, scale, color, yaw, seed riêng: 137/251/389. Scene dựng ba mesh, áp T·R·S. grassAllowed và contact shading dùng cùng layout.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f157"></a>
### rock

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> rock(std::uint32_t seed)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:88](../src/EnvironmentGeometry.cpp#L88).

**Tham số:** `std::uint32_t seed`: seed uint32 cho RNG.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Seed RNG; tám góc lệch ngẫu nhiên và bán kính 0.82..1.12. Tạo ba vòng ở y=−0.55/−0.04/0.54, width=0.65/1/0.67 cùng lean và jitter. Nối hai khoảng vòng bằng quad tách đôi, thêm fan đáy/đỉnh. add/triangle sửa winding hướng centroid; 48 triangles kín, flat shading. Không phải geometry::rock octahedron.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** `rng`, `random`, `lean`, `std::cos`, `std::sin`, `reserve`, `triangle`, `add`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f158"></a>
### rock::random@91

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](float low,float high)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:91](../src/EnvironmentGeometry.cpp#L91).

**Tham số:** `float low`: cận dưới random; `float high`: cận trên random.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Capture RNG bằng tham chiếu, tạo uniform_real_distribution<float>(low, high) và lấy một số. Dùng seed local trong builder; thứ tự gọi RNG tác động silhouette; không đồng nghĩa bit-exact trên mọi standard library.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f159"></a>
### rock::add@108

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](glm::vec3 a,glm::vec3 b,glm::vec3 c)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:108](../src/EnvironmentGeometry.cpp#L108).

**Tham số:** `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba.

**Giá trị trả về:** void suy diễn; thay đổi dữ liệu qua capture/reference.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chuyển ba điểm tới helper triangle, dùng centroid (a+b+c)/3 làm vector outward ước lượng từ gốc đá. Không tự push vertex; triangle quyết định winding và normal.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `triangle`.


<a id="f160"></a>
### cloud

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> cloud(const CloudCluster& cluster)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:121](../src/EnvironmentGeometry.cpp#L121).

**Tham số:** `const CloudCluster& cluster`: năm lobe cấu hình skin mây.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra từng lobe scale dương và độ dài scale/offset hữu hạn. Dùng field Gaussian năm ellipsoid; 28 slices×14 stacks, mỗi hướng ray chạy 24 vòng bisection radius∈[0, 6] tới density 0.18. Lấy negative-gradient normal tại điểm; kết nối vòng, bỏ tam giác suy biến ở cực; add sửa winding và trộn normal gradient 85% + face 15%. Trả 728 triangles/cluster. Chỉ gọi lúc Renderer khởi tạo; đây là trích skin theo tia giả định star-shaped, không Marching Cubes hoặc raymarch GPU.

**Quan hệ — nơi gọi:** Scene/Renderer khởi tạo, placement helpers và environment_tests.

**Gọi tiếp/API nổi bật:** `glm::any`, `glm::lessThanEqual`, `glm::vec3`, `std::isfinite`, `glm::length`, `std::invalid_argument`, `gradient`, `std::exp`, `glm::dot`, `glm::normalize`, `ray`, `std::sin`, `std::cos`, `field`, `reserve`, `glm::cross`, `std::swap`, `push_back`, `add`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f161"></a>
### cloud::field@129

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](glm::vec3 p,glm::vec3* normal=nullptr)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:129](../src/EnvironmentGeometry.cpp#L129).

**Tham số:** `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể; `glm::vec3* normal=nullptr`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Với từng lobe, scale.y nhân 1.16; delta=p−offset; cộng exp(−2 dot(delta/scale, delta/scale)) vào density và value×delta/(scale ²) vào gradient hướng ngoài. Nếu normal không null thì normalize gradient để ghi kết quả; luôn trả density. Bỏ hệ số chung 4 của negative gradient vì normalize sẽ triệt tiêu.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `gradient`, `glm::vec3`, `std::exp`, `glm::dot`, `glm::normalize`.


<a id="f162"></a>
### cloud::add@161

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](Vertex a,Vertex b,Vertex c)
```

**Vị trí mã nguồn:** [src/EnvironmentGeometry.cpp:161](../src/EnvironmentGeometry.cpp#L161).

**Tham số:** `Vertex a`: điểm/vector/scalar đầu theo signature; `Vertex b`: điểm/vector/scalar thứ hai theo signature; `Vertex c`: đỉnh thứ ba.

**Giá trị trả về:** void suy diễn; thay đổi dữ liệu qua capture/reference.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tính cross cạnh tam giác; nếu ngược tổng normal đỉnh thì đổi b/c và dấu face normal. Chuẩn hóa face, mỗi Vertex giữ position và normalize(0.85 gradient Normal+0.15 face Normal) rồi push. Mượt chỗ nối lobe nhưng vẫn có facet nhẹ.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `glm::cross`, `glm::dot`, `std::swap`, `glm::normalize`, `push_back`.


<a id="file-34"></a>
## src/Geometry.cpp

**Module/chức năng:** Primitive triangle lists cơ sở.

**Headers/phụ thuộc trực tiếp:** `Geometry.hpp`, `Surface.hpp`, `cmath`, `stdexcept`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Geometry.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [triangle](#f163) | `std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tính normal=normalize(cross(b−a, c−a)), gán cùng normal cho ba đỉnh rồi push. | Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test). |
| [quad](#f164) | `std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 d`: đỉnh thứ tư. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tách quad thành triangle(a, b, c) và triangle(a, c, d). | Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test). |
| [screenTriangle](#f165) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả ba Vertex ở (−1, −1), (3, −1), (−1, 3), normal +Z. | Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test). |
| [puddle](#f166) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Lấy outline 32 điểm local, với mỗi cạnh thêm fan tâm (0, 0, 0), b rồi a để normal +Y. | core_tests.cpp::geometryTests; Scene dùng environment::puddles. |
| [ground](#f167) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Reserve sáu vertex, gọi quad với bốn góc X/Z±10 ở y=0 theo CCW nhìn từ trên. | Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test). |
| [box](#f168) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Reserve 36 vertex; sáu mặt unit cube tâm gốc, nửa cạnh 0.5, mỗi mặt một quad. | Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test). |
| [roof](#f169) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Định nghĩa sáu điểm mái ở z=±0.5, chân x=±0.5/y=0, đỉnh x=0/y=1; thêm hai tam giác đầu hồi và ba quad sườn/đáy. | core_tests.cpp::geometryTests; Scene hiện dùng models::house/roofShell. |
| [cone](#f170) | `unsigned segments`: số sector cone. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Reject segments ngoài 3..64. | core_tests.cpp::geometryTests; không dùng cone để dựng tán cây hiện tại. |
| [rock](#f171) | Không có. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Tạo bốn điểm rim không đều, mỗi cạnh nối đỉnh lệch (0.15, 1, −0.1) và đáy lệch (−0.1, −0.55, 0). | core_tests.cpp::geometryTests; Scene dùng environment::rock(seed). |
| [sphere](#f172) | `unsigned slices`: số lát longitude; `unsigned stacks`: số khoảng latitude. | `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình. | Reject slices ngoài 3..32 hoặc stacks ngoài 2..16. | core_tests.cpp::geometryTests; Renderer hiện dùng environment::cloud. |
| [sphere::point@103](#f173) | `unsigned row`: chỉ số latitude; `unsigned column`: chỉ số longitude. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Capture slices/stacks bằng giá trị; latitude=π row/stacks, longitude=2π column/slices, trả (sin lat cos lon, cos lat, sin lat sin lon). | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |


<a id="f163"></a>
### triangle

**Khai báo chính xác tại định nghĩa:**

```cpp
void triangle(std::vector<Vertex>& vertices, glm::vec3 a, glm::vec3 b, glm::vec3 c)
```

**Vị trí mã nguồn:** [src/Geometry.cpp:8](../src/Geometry.cpp#L8).

**Tham số:** `std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tính normal=normalize(cross(b−a, c−a)), gán cùng normal cho ba đỉnh rồi push. Không kiểm tra/đảo winding; các caller primitive phải cung cấp CCW đúng từ ngoài.

**Quan hệ — nơi gọi:** Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test).

**Gọi tiếp/API nổi bật:** `glm::normalize`, `glm::cross`, `push_back`.


<a id="f164"></a>
### quad

**Khai báo chính xác tại định nghĩa:**

```cpp
void quad(std::vector<Vertex>& vertices, glm::vec3 a, glm::vec3 b,
          glm::vec3 c, glm::vec3 d)
```

**Vị trí mã nguồn:** [src/Geometry.cpp:15](../src/Geometry.cpp#L15).

**Tham số:** `std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 d`: đỉnh thứ tư.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tách quad thành triangle(a, b, c) và triangle(a, c, d). Hai tam giác dùng lại vị trí nhưng lưu đỉnh lặp; không lập index/EBO.

**Quan hệ — nơi gọi:** Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test).

**Gọi tiếp/API nổi bật:** `triangle`.


<a id="f165"></a>
### screenTriangle

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> screenTriangle()
```

**Vị trí mã nguồn:** [src/Geometry.cpp:23](../src/Geometry.cpp#L23).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả ba Vertex ở (−1, −1), (3, −1), (−1, 3), normal +Z. Tam giác vượt viewport phủ kín màn hình sau clipping; sky vertex shader dùng trực tiếp tọa độ clip, không P/V/M.

**Quan hệ — nơi gọi:** Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test).

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f166"></a>
### puddle

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> puddle()
```

**Vị trí mã nguồn:** [src/Geometry.cpp:26](../src/Geometry.cpp#L26).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Lấy outline 32 điểm local, với mỗi cạnh thêm fan tâm (0, 0, 0), b rồi a để normal +Y. Trả 32 triangles; reserve(48) chỉ là gợi ý capacity thấp hơn số vertex cần, vector vẫn tự tăng. Đây là primitive dùng trong core test, Scene hiện dùng environment::puddles.

**Quan hệ — nơi gọi:** core_tests.cpp::geometryTests; Scene dùng environment::puddles.

**Gọi tiếp/API nổi bật:** `reserve`, `surface::outline`, `size`, `triangle`.


<a id="f167"></a>
### ground

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> ground()
```

**Vị trí mã nguồn:** [src/Geometry.cpp:36](../src/Geometry.cpp#L36).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reserve sáu vertex, gọi quad với bốn góc X/Z±10 ở y=0 theo CCW nhìn từ trên. Trả đúng hai tam giác, không height map hoặc displacement mesh.

**Quan hệ — nơi gọi:** Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test).

**Gọi tiếp/API nổi bật:** `reserve`, `quad`.


<a id="f168"></a>
### box

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> box()
```

**Vị trí mã nguồn:** [src/Geometry.cpp:43](../src/Geometry.cpp#L43).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reserve 36 vertex; sáu mặt unit cube tâm gốc, nửa cạnh 0.5, mỗi mặt một quad. Flat normals từng mặt; unit mesh được cache static trong helper models::box để bake chi tiết nhà.

**Quan hệ — nơi gọi:** Scene/Renderer hoặc helper models::box; core_tests (một số primitive chỉ dùng test).

**Gọi tiếp/API nổi bật:** `reserve`, `quad`.


<a id="f169"></a>
### roof

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> roof()
```

**Vị trí mã nguồn:** [src/Geometry.cpp:56](../src/Geometry.cpp#L56).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Định nghĩa sáu điểm mái ở z=±0.5, chân x=±0.5/y=0, đỉnh x=0/y=1; thêm hai tam giác đầu hồi và ba quad sườn/đáy. Trả tám triangles kín; đây là primitive legacy/test, nhà hiện dùng roofShell/gable.

**Quan hệ — nơi gọi:** core_tests.cpp::geometryTests; Scene hiện dùng models::house/roofShell.

**Gọi tiếp/API nổi bật:** `reserve`, `triangle`, `quad`.


<a id="f170"></a>
### cone

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> cone(unsigned segments)
```

**Vị trí mã nguồn:** [src/Geometry.cpp:69](../src/Geometry.cpp#L69).

**Tham số:** `unsigned segments`: số sector cone.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reject segments ngoài 3..64. Mỗi sector lấy hai điểm vòng tròn cos/sin bán kính 1; thêm một triangle cạnh tới apex y=1 và một triangle đáy tới gốc. Tổng 2×segments triangles; O(segments), normals flat.

**Quan hệ — nơi gọi:** core_tests.cpp::geometryTests; không dùng cone để dựng tán cây hiện tại.

**Gọi tiếp/API nổi bật:** `std::invalid_argument`, `reserve`, `std::cos`, `std::sin`, `triangle`.


<a id="f171"></a>
### rock

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> rock()
```

**Vị trí mã nguồn:** [src/Geometry.cpp:87](../src/Geometry.cpp#L87).

**Tham số:** Không có.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tạo bốn điểm rim không đều, mỗi cạnh nối đỉnh lệch (0.15, 1, −0.1) và đáy lệch (−0.1, −0.55, 0). Trả tám triangles octahedron bất đối xứng; Scene không dùng phiên bản này cho ba đá hiện tại.

**Quan hệ — nơi gọi:** core_tests.cpp::geometryTests; Scene dùng environment::rock(seed).

**Gọi tiếp/API nổi bật:** `reserve`, `triangle`.


<a id="f172"></a>
### sphere

**Khai báo chính xác tại định nghĩa:**

```cpp
std::vector<Vertex> sphere(unsigned slices, unsigned stacks)
```

**Vị trí mã nguồn:** [src/Geometry.cpp:98](../src/Geometry.cpp#L98).

**Tham số:** `unsigned slices`: số lát longitude; `unsigned stacks`: số khoảng latitude.

**Giá trị trả về:** `std::vector<Vertex>`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reject slices ngoài 3..32 hoặc stacks ngoài 2..16. point sinh tọa độ spherical; duyệt từng ô latitude/longitude, thêm hai tam giác trừ một ở mỗi cực để tránh zero-area. Trả 2×slices×(stacks−1) triangles, default 100; normals flat từ triangle. Mây hiện tại không render primitive này.

**Quan hệ — nơi gọi:** core_tests.cpp::geometryTests; Renderer hiện dùng environment::cloud.

**Gọi tiếp/API nổi bật:** `std::invalid_argument`, `glm::vec3`, `std::sin`, `std::cos`, `reserve`, `point`, `triangle`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f173"></a>
### sphere::point@103

**Khai báo chính xác tại định nghĩa:**

```cpp
[=](unsigned row, unsigned column)
```

**Vị trí mã nguồn:** [src/Geometry.cpp:103](../src/Geometry.cpp#L103).

**Tham số:** `unsigned row`: chỉ số latitude; `unsigned column`: chỉ số longitude.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Capture slices/stacks bằng giá trị; latitude=π row/stacks, longitude=2π column/slices, trả (sin lat cos lon, cos lat, sin lat sin lon). row ở cực tạo một vị trí chung; column+1 tại seam khép vòng.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `glm::vec3`, `std::sin`, `std::cos`.


<a id="file-35"></a>
## src/LightingSystem.cpp

**Module/chức năng:** Ánh sáng màu, flash, fog và wetness.

**Headers/phụ thuộc trực tiếp:** `LightingSystem.hpp`, `algorithm`, `cmath`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/LightingSystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [LightingSystem::reset](#f174) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Reset flash=0, wetness=0.65, flashPosition=(0, 8, 0), fogDensity=0.018 rồi setEnvironment(DEFAULT_HOUR=18, storm=0.9). | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [LightingSystem::setEnvironment](#f175) | `float hour`: giờ mô phỏng; 24 giờ/một ngày; `float storminess`: độ giông[0, 1]. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bỏ hour/storminess không finite; wrap hour, clamp storm. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [LightingSystem::setEnvironment::smooth@18](#f176) | `float a`: điểm/vector/scalar đầu theo signature; `float b`: điểm/vector/scalar thứ hai theo signature; `float x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Tính t=clamp((x−a)/(b−a), 0, 1), trả t²(3−2t). | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [LightingSystem::particleColor](#f177) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Mix màu particle đêm (0.48, 0.64, 0.83) tới ngày (0.88, 0.94, 1) theo daylight, cộng RGB 0.20×flash. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [LightingSystem::update](#f178) | `float flashIntensity`: cường độ flash vô thứ nguyên[0, 1]; `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `bool raining`: cờ quyết định tăng hay giảm độ ướt. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | flashIntensity finite thì clamp [0, 1], sai thì flash=0. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [LightingSystem::skyColor](#f179) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả mix(top_, horizon_, 0.45)+(0.14, 0.16, 0.20)×flash. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [LightingSystem::attenuationAt](#f180) | `glm::vec3 point`: điểm world cần đo attenuation. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | d=distance(point, flashPosition), trả 1/(1+0.025d+0.018 d²). | weather_tests.cpp::lightningTests; object.frag lặp attenuation theo uniforms, không gọi CPU helper này. |
| [LightingSystem::skyTop](#f181) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả top_ cộng tint flash (0.14, 0.16, 0.20)×flash. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [LightingSystem::fogColor](#f182) | Không có. | `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả horizon_ cộng tint flash (0.14, 0.16, 0.20)×flash. | WeatherSystem, Renderer, smoke và weather/lighting_time_tests. |
| [LightingSystem::fogFactor](#f183) | `float distance`: khoảng cách world-unit tới camera. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | distance không finite trả 0; ngược lại exp(−max(fogDensity, 0)×max(distance−6, 0)). | weather_tests.cpp::surfaceTests; Renderer không đọc fogFactor CPU để shade fragment. |


<a id="f174"></a>
### LightingSystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightingSystem::reset()
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:6](../src/LightingSystem.cpp#L6).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reset flash=0, wetness=0.65, flashPosition=(0, 8, 0), fogDensity=0.018 rồi setEnvironment(DEFAULT_HOUR=18, storm=0.9). Không gọi reset của Rain/TimeOfDay; Weather phối hợp các reset đó.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `setEnvironment`.


<a id="f175"></a>
### LightingSystem::setEnvironment

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightingSystem::setEnvironment(float hour, float storminess)
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:14](../src/LightingSystem.cpp#L14).

**Tham số:** `float hour`: giờ mô phỏng; 24 giờ/một ngày; `float storminess`: độ giông[0, 1].

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bỏ hour/storminess không finite; wrap hour, clamp storm. Tính dawn=smooth(5, 7, h), dusk=smooth(17, 19, h), daylight=dawn(1−dusk), twilight=4[dawn(1−dawn)+dusk(1−dusk)]. Windows giảm theo daylight; mix ambient ngày/đêm, cộng twilight, giảm theo storm. Góc (h−6)π/12 cho hướng sun, elevation≥0; blend với hướng moon và đảo dấu thành hướng tia tới. Mix directional/sky top/horizon với storm và twilight. Không sửa wetness/flash; mô hình thẩm mỹ, không thiên văn hay radiometry.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::fmod`, `std::clamp`, `smooth`, `glm::mix`, `glm::vec3`, `std::max`, `std::sin`, `glm::normalize`, `std::cos`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f176"></a>
### LightingSystem::setEnvironment::smooth@18

**Khai báo chính xác tại định nghĩa:**

```cpp
[](float a,float b,float x)
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:18](../src/LightingSystem.cpp#L18).

**Tham số:** `float a`: điểm/vector/scalar đầu theo signature; `float b`: điểm/vector/scalar thứ hai theo signature; `float x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tính t=clamp((x−a)/(b−a), 0, 1), trả t²(3−2t). a/b là mốc giờ hoặc daylight; mọi call trong setEnvironment dùng b>a nên không chia 0. Đây là CPU smoothstep tương ứng GLSL.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `std::clamp`.


<a id="f177"></a>
### LightingSystem::particleColor

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 LightingSystem::particleColor() const
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:42](../src/LightingSystem.cpp#L42).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Mix màu particle đêm (0.48, 0.64, 0.83) tới ngày (0.88, 0.94, 1) theo daylight, cộng RGB 0.20×flash. Renderer dùng tint này cho rain/splash/ripple, rồi đổi về trắng cho bolt.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `glm::mix`, `glm::vec3`.


<a id="f178"></a>
### LightingSystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightingSystem::update(float flashIntensity, float deltaTime, bool raining)
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:46](../src/LightingSystem.cpp#L46).

**Tham số:** `float flashIntensity`: cường độ flash vô thứ nguyên[0, 1]; `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `bool raining`: cờ quyết định tăng hay giảm độ ướt.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** flashIntensity finite thì clamp [0, 1], sai thì flash=0. Nếu dt finite dương, wetness+=rate×min(dt, 0.1), rate=0.06 lúc mưa và −0.012 khi khô; clamp [0, 1]. dt default 0 cho phép cập nhật flash tức thời mà không thay wetness.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::clamp`, `std::min`.


<a id="f179"></a>
### LightingSystem::skyColor

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 LightingSystem::skyColor() const
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:54](../src/LightingSystem.cpp#L54).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả mix(top_, horizon_, 0.45)+(0.14, 0.16, 0.20)×flash. Dùng làm glClearColor; fullscreen sky pass sẽ phủ color bằng gradient, nên clear color không thay cho sky gradient.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `glm::mix`, `glm::vec3`.


<a id="f180"></a>
### LightingSystem::attenuationAt

**Khai báo chính xác tại định nghĩa:**

```cpp
float LightingSystem::attenuationAt(glm::vec3 point) const
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:58](../src/LightingSystem.cpp#L58).

**Tham số:** `glm::vec3 point`: điểm world cần đo attenuation.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** d=distance(point, flashPosition), trả 1/(1+0.025d+0.018 d²). Đây là CPU helper kiểm thử; lighting shader lặp công thức từ uniforms, không gọi CPU mỗi fragment.

**Quan hệ — nơi gọi:** weather_tests.cpp::lightningTests; object.frag lặp attenuation theo uniforms, không gọi CPU helper này.

**Gọi tiếp/API nổi bật:** `glm::distance`.


<a id="f181"></a>
### LightingSystem::skyTop

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 LightingSystem::skyTop() const
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:63](../src/LightingSystem.cpp#L63).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả top_ cộng tint flash (0.14, 0.16, 0.20)×flash. Được renderer đưa vào sky và fake water reflection.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `glm::vec3`.


<a id="f182"></a>
### LightingSystem::fogColor

**Khai báo chính xác tại định nghĩa:**

```cpp
glm::vec3 LightingSystem::fogColor() const
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:67](../src/LightingSystem.cpp#L67).

**Tham số:** Không có.

**Giá trị trả về:** `glm::vec3`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả horizon_ cộng tint flash (0.14, 0.16, 0.20)×flash. Cùng màu dùng cho chân trời và fog; cloud fragment còn làm tối màu fog riêng.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather/lighting_time_tests.

**Gọi tiếp/API nổi bật:** `glm::vec3`.


<a id="f183"></a>
### LightingSystem::fogFactor

**Khai báo chính xác tại định nghĩa:**

```cpp
float LightingSystem::fogFactor(float distance) const
```

**Vị trí mã nguồn:** [src/LightingSystem.cpp:71](../src/LightingSystem.cpp#L71).

**Tham số:** `float distance`: khoảng cách world-unit tới camera.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** distance không finite trả 0; ngược lại exp(−max(fogDensity, 0)×max(distance−6, 0)). Trong 6 đơn vị gần transmission=1; density=0 thì luôn 1. CPU test helper; GLSL có density scale theo loại hiệu ứng.

**Quan hệ — nơi gọi:** weather_tests.cpp::surfaceTests; Renderer không đọc fogFactor CPU để shade fragment.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::exp`, `std::max`.


<a id="file-36"></a>
## src/LightningSystem.cpp

**Module/chức năng:** Polyline sét đệ quy, nhánh và xung.

**Headers/phụ thuộc trực tiếp:** `LightningSystem.hpp`, `algorithm`, `cmath`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/LightningSystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [LightningSystem::random](#f184) | `float low`: cận dưới random; `float high`: cận trên random. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả một mẫu phân bố đều float giữa low/high dùng mt19937 thành viên. | WeatherSystem, Renderer, smoke và weather_tests. |
| [LightningSystem::reset](#f185) | `std::uint32_t seed`: seed uint32 cho RNG. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Seed RNG theo seed (default 6174 ở header), xóa segmentCount/strikeCount/active/elapsed/duration, chọn nextStrike U(2, 6), tăng revision. | WeatherSystem, Renderer, smoke và weather_tests. |
| [LightningSystem::cancel](#f186) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đặt active=false, segmentCount=0 và lấy lại khoảng chờ ngẫu nhiên 2..6. | WeatherSystem, Renderer, smoke và weather_tests. |
| [LightningSystem::subdivide](#f187) | `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `unsigned depth`: độ sâu chia đệ quy (beam depth riêng là bề dày world-unit); `float displacement`: biên độ offset world-unit; `float width`: chiều rộng pixel, riêng beam là world-unit theo signature; `bool branch`: cờ đoạn nhánh. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu depth=0, append segment khi còn MAX_SEGMENTS. | WeatherSystem, Renderer, smoke và weather_tests. |
| [LightningSystem::trigger](#f188) | `const CloudSystem& clouds`: dữ liệuCloudSystem CPU; `glm::vec3 observer`: vị trí camera world-space. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Chọn cloud coverage>0.65 và khoảng cách XZ tới observer>4.5; nếu không có chọn xa nhất. | WeatherSystem, Renderer, smoke và weather_tests. |
| [LightningSystem::update](#f189) | `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `const CloudSystem& clouds`: dữ liệuCloudSystem CPU; `bool randomEnabled`: cho phép phát strike tự động khi inactive; `glm::vec3 observer`: vị trí camera world-space. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bỏ dt không finite/≤0, clamp 0.1. | WeatherSystem, Renderer, smoke và weather_tests. |
| [LightningSystem::flashIntensity](#f190) | Không có. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Inactive trả 0; duyệt pulseCount, tìm local=elapsed−start trong [0, duration). | WeatherSystem, Renderer, smoke và weather_tests. |


<a id="f184"></a>
### LightningSystem::random

**Khai báo chính xác tại định nghĩa:**

```cpp
float LightningSystem::random(float low, float high)
```

**Vị trí mã nguồn:** [src/LightningSystem.cpp:6](../src/LightningSystem.cpp#L6).

**Tham số:** `float low`: cận dưới random; `float high`: cận trên random.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả một mẫu phân bố đều float giữa low/high dùng mt19937 thành viên. Không tự kiểm tra thứ tự low/high; caller nội bộ cung cấp khoảng đúng. RNG được reset seed để tái hiện kiểm thử.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f185"></a>
### LightningSystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightningSystem::reset(std::uint32_t seed)
```

**Vị trí mã nguồn:** [src/LightningSystem.cpp:10](../src/LightningSystem.cpp#L10).

**Tham số:** `std::uint32_t seed`: seed uint32 cho RNG.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Seed RNG theo seed (default 6174 ở header), xóa segmentCount/strikeCount/active/elapsed/duration, chọn nextStrike U(2, 6), tăng revision. pulseCount/pulses không reset tại đây nhưng active=false và segmentCount=0 khiến flash/geometry không dùng dữ liệu cũ.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `seed`, `random`.


<a id="f186"></a>
### LightningSystem::cancel

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightningSystem::cancel()
```

**Vị trí mã nguồn:** [src/LightningSystem.cpp:20](../src/LightningSystem.cpp#L20).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đặt active=false, segmentCount=0 và lấy lại khoảng chờ ngẫu nhiên 2..6. Không tăng revision; renderer không draw khi inactive, lần trigger sau sẽ tăng revision. Không xóa strikeCount.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `random`.


<a id="f187"></a>
### LightningSystem::subdivide

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightningSystem::subdivide(glm::vec3 a, glm::vec3 b, unsigned depth,
                                float displacement, float width, bool branch)
```

**Vị trí mã nguồn:** [src/LightningSystem.cpp:26](../src/LightningSystem.cpp#L26).

**Tham số:** `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `unsigned depth`: độ sâu chia đệ quy (beam depth riêng là bề dày world-unit); `float displacement`: biên độ offset world-unit; `float width`: chiều rộng pixel, riêng beam là world-unit theo signature; `bool branch`: cờ đoạn nhánh.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu depth=0, append segment khi còn MAX_SEGMENTS. Ngược lại normalize b−a, tạo cơ sở side/other vuông góc với đoạn, offset random hai chiều. Cap độ dài offset=0.12|b−a|; lấy midpoint chính, tính amount chung theo ba biên hộp để scale toàn offset mà giữ phương vuông góc. Đệ quy hai nửa với depth−1 và displacement×0.40. O(2^depth); điểm đầu/cuối mặc định hợp lệ và không trùng nhau.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `glm::normalize`, `std::abs`, `glm::vec3`, `glm::cross`, `random`, `glm::distance`, `glm::length`, `low`, `high`, `std::min`, `std::clamp`, `subdivide`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f188"></a>
### LightningSystem::trigger

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightningSystem::trigger(const CloudSystem& clouds, glm::vec3 observer)
```

**Vị trí mã nguồn:** [src/LightningSystem.cpp:55](../src/LightningSystem.cpp#L55).

**Tham số:** `const CloudSystem& clouds`: dữ liệuCloudSystem CPU; `glm::vec3 observer`: vị trí camera world-space.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chọn cloud coverage>0.65 và khoảng cách XZ tới observer>4.5; nếu không có chọn xa nhất. Jitter start và clamp hộp (±9.5, y 6..9.5), end ở y 0.03 với x/z gần start rồi clamp ±8. Gọi subdivide depth 5→32 đoạn chính width 0.095, taper tới 65%. Sáu nhánh lấy root tại segments[5+4i].end, alternating side, subdivide depth 3→8 đoạn/nhánh width 0.032 taper tới 40%. Chọn duration 0.28..0.42, 1..3 pulses với duration pulse 55..70% interval; peak đầu 1, sau 0.65..0.92. Reset elapsed, nextStrike 2..6, active=true, tăng count/revision. Tổng thực 80/128 segments; tránh camera là heuristic, không collision chính xác.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `clusters`, `glm::length`, `glm::vec2`, `CloudSystem::coverage`, `glm::clamp`, `glm::vec3`, `random`, `std::clamp`, `subdivide`, `std::max`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f189"></a>
### LightningSystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void LightningSystem::update(float deltaTime, const CloudSystem& clouds, bool randomEnabled,
                             glm::vec3 observer)
```

**Vị trí mã nguồn:** [src/LightningSystem.cpp:102](../src/LightningSystem.cpp#L102).

**Tham số:** `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `const CloudSystem& clouds`: dữ liệuCloudSystem CPU; `bool randomEnabled`: cho phép phát strike tự động khi inactive; `glm::vec3 observer`: vị trí camera world-space.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bỏ dt không finite/≤0, clamp 0.1. Nếu active tăng elapsed, đến duration thì tắt; nhánh random countdown chỉ chạy ở lần update khi đã inactive. Khi inactive và randomEnabled, giảm nextStrike, ≤0 gọi trigger(clouds, observer). Thời gian chờ 2..6 bắt đầu tiêu thụ sau khi strike hết đời.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::min`, `trigger`.


<a id="f190"></a>
### LightningSystem::flashIntensity

**Khai báo chính xác tại định nghĩa:**

```cpp
float LightningSystem::flashIntensity() const
```

**Vị trí mã nguồn:** [src/LightningSystem.cpp:115](../src/LightningSystem.cpp#L115).

**Tham số:** Không có.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Inactive trả 0; duyệt pulseCount, tìm local=elapsed−start trong [0, duration). Trả peak exp(−32 local) clamp((duration−local)/0.012, 0, 1). Ngoài các cửa sổ pulse trả 0 dù active=true; tạo khoảng tối thật giữa xung.

**Quan hệ — nơi gọi:** WeatherSystem, Renderer, smoke và weather_tests.

**Gọi tiếp/API nổi bật:** `std::clamp`, `std::exp`.


<a id="file-37"></a>
## src/Mesh.cpp

**Module/chức năng:** Tài nguyên GPU của mesh tĩnh.

**Headers/phụ thuộc trực tiếp:** `Mesh.hpp`, `cstddef`, `limits`, `stdexcept`, `type_traits`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Mesh.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Mesh::Mesh](#f191) | `const std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature. | Constructor/destructor: không có kiểu trả về. | Compile-time bảo đảm Vertex standard-layout; reject vector rỗng, số vertex không chia hết 3, hoặc vượt GLsizei. | Scene/Renderer constructors/draw và RAII cleanup. |
| [Mesh::~Mesh](#f192) | Không có. | Constructor/destructor: không có kiểu trả về. | glDeleteBuffers(vbo_) rồi glDeleteVertexArrays(vao_). | Scene/Renderer constructors/draw và RAII cleanup. |
| [Mesh::draw](#f193) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bind VAO rồi glDrawArrays(GL_TRIANGLES, 0, count_). | Scene/Renderer constructors/draw và RAII cleanup. |


<a id="f191"></a>
### Mesh::Mesh

**Khai báo chính xác tại định nghĩa:**

```cpp
Mesh::Mesh(const std::vector<Vertex>& vertices)
```

**Vị trí mã nguồn:** [src/Mesh.cpp:8](../src/Mesh.cpp#L8).

**Tham số:** `const std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Compile-time bảo đảm Vertex standard-layout; reject vector rỗng, số vertex không chia hết 3, hoặc vượt GLsizei. Sinh VAO/VBO, upload GL_STATIC_DRAW, khai báo position attr0 và normal attr1 bằng offsetof, unbind. Giữ count_ để glDrawArrays, không giữ CPU vector và không tạo EBO.

**Quan hệ — nơi gọi:** Scene/Renderer constructors/draw và RAII cleanup.

**Gọi tiếp/API nổi bật:** `empty`, `size`, `max`, `std::invalid_argument`, `glGenVertexArrays`, `glGenBuffers`, `glBindVertexArray`, `glBindBuffer`, `glBufferData`, `data`, `glEnableVertexAttribArray`, `glVertexAttribPointer`, `offsetof`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f192"></a>
### Mesh::~Mesh

**Khai báo chính xác tại định nghĩa:**

```cpp
Mesh::~Mesh()
```

**Vị trí mã nguồn:** [src/Mesh.cpp:31](../src/Mesh.cpp#L31).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** glDeleteBuffers(vbo_) rồi glDeleteVertexArrays(vao_). Application hủy owner trước context; copy/assignment bị delete, destructor không tự kiểm tra context.

**Quan hệ — nơi gọi:** Scene/Renderer constructors/draw và RAII cleanup.

**Gọi tiếp/API nổi bật:** `glDeleteBuffers`, `glDeleteVertexArrays`.


<a id="f193"></a>
### Mesh::draw

**Khai báo chính xác tại định nghĩa:**

```cpp
void Mesh::draw() const
```

**Vị trí mã nguồn:** [src/Mesh.cpp:36](../src/Mesh.cpp#L36).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bind VAO rồi glDrawArrays(GL_TRIANGLES, 0, count_). Shader và uniforms phải do Renderer thiết lập trước; không đổi mesh/material hoặc đếm visible triangles.

**Quan hệ — nơi gọi:** Scene/Renderer constructors/draw và RAII cleanup.

**Gọi tiếp/API nổi bật:** `glBindVertexArray`, `glDrawArrays`.


<a id="file-38"></a>
## src/ModelGeometry.cpp

**Module/chức năng:** Mô hình nhà/cây procedural.

**Headers/phụ thuộc trực tiếp:** `ModelGeometry.hpp`, `glm/gtc/matrix_transform.hpp`, `algorithm`, `cmath`, `random`, `stdexcept`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/ModelGeometry.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [tri](#f194) | `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 outward`: hướng tham chiếu để sửa winding. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | So dot(cross(b−a, c−a), outward), đổi b/c nếu âm; tính normal đơn vị và append ba Vertex flat. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [quad](#f195) | `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 d`: đỉnh thứ tư; `glm::vec3 outward`: hướng tham chiếu để sửa winding. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Gọi tri hai lần với cùng outward để tách quad theo đường a→c. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [box](#f196) | `MeshData& v`: vector mesh output cần append; `glm::vec3 position`: tọa độ world hoặc local theo caller; `glm::vec3 scale`: vector tỉ lệ transform; `const glm::mat4& parent = glm::mat4(1)`: ma trận cha CPU. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Cache unit geometry::box static; model=parent·T(position)·S(scale), normal=transpose(inverse(mat3(model))). | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [beam](#f197) | `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `float width`: chiều rộng pixel, riêng beam là world-unit theo signature; `float depth`: độ sâu chia đệ quy (beam depth riêng là bề dày world-unit). | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Từ a→b tạo axis, side=cross(axis, +Z) chuẩn hóa, back=cross(side, axis). | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [tube](#f198) | `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `float ra`: bán kính đầu a; `float rb`: bán kính đầu b; `unsigned sides`: số sector ống. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tạo cơ sở quanh axis, chọn reference +Y hoặc +X nếu gần song song Y. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [roofShell](#f199) | `MeshData& v`: vector mesh output cần append. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Xác định mái ngoài y 2.56→4.20 và mặt trong thấp hơn 0.12, z±2.15. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [roofShell::back@50](#f200) | `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Nhận bản sao điểm p, đặt z=−2.15 và trả lại. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [gable](#f201) | `MeshData& v`: vector mesh output cần append. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tính độ cao edge bằng nội suy slope mặt dưới mái; tạo profile ngũ giác x±2, y 2.6..4.08. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [gable::point@65](#f202) | `std::size_t i`: chỉ số vòng/đỉnh/cloud; `float z`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Capture profile; lấy điểm 2D tại i rồi thêm thành phần z thành vec3. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [house](#f203) | Không có. | `House`; xem ý nghĩa và điều kiện biên trong quy trình. | Tạo House chín vector theo HousePart, reserve mỗi part. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [house::part@82](#f204) | `HousePart p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Chuyển HousePart sang size_t, trả reference h[index] để helper thêm vào đúng nhóm vật liệu. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [house::window@109](#f205) | `glm::vec3 center`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới; `float yaw`: góc quanh Y theo độ. | void suy diễn; thay đổi dữ liệu qua capture/reference. | Tạo parent=T(center)·Ry(yaw). | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [treeLayout](#f206) | Không có. | `const std::array<TreeInstance, 4>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả const reference bốn TreeInstance static với vị trí/shade/height/trunkRadius/canopyWidth/branchCount/seed riêng. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [tree](#f207) | `const TreeParameters& p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | `Tree`; xem ý nghĩa và điều kiện biên trong quy trình. | Reject thông số không finite hoặc ngoài budget height 2..6, trunkRadius(0, 0.35], canopyWidth 0.6..2, branchCount 4..12. | Scene constructor, Renderer contact setup, environment placement và model_tests. |
| [tree::random@149](#f208) | `float low`: cận dưới random; `float high`: cận trên random. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Capture RNG bằng tham chiếu, tạo uniform_real_distribution<float>(low, high) và lấy một số. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |


<a id="f194"></a>
### tri

**Khai báo chính xác tại định nghĩa:**

```cpp
void tri(MeshData& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 outward)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:10](../src/ModelGeometry.cpp#L10).

**Tham số:** `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 outward`: hướng tham chiếu để sửa winding.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** So dot(cross(b−a, c−a), outward), đổi b/c nếu âm; tính normal đơn vị và append ba Vertex flat. outward giúp các nhà/cây procedural không bị cull sai; helper nội bộ namespace ẩn.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `glm::dot`, `glm::cross`, `std::swap`, `glm::normalize`, `push_back`.


<a id="f195"></a>
### quad

**Khai báo chính xác tại định nghĩa:**

```cpp
void quad(MeshData& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 outward)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:15](../src/ModelGeometry.cpp#L15).

**Tham số:** `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `glm::vec3 c`: đỉnh thứ ba; `glm::vec3 d`: đỉnh thứ tư; `glm::vec3 outward`: hướng tham chiếu để sửa winding.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Gọi tri hai lần với cùng outward để tách quad theo đường a→c. Mỗi tam giác có normal riêng, hữu ích với mặt low-poly không hoàn toàn đồng phẳng.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `tri`.


<a id="f196"></a>
### box

**Khai báo chính xác tại định nghĩa:**

```cpp
void box(MeshData& v, glm::vec3 position, glm::vec3 scale, const glm::mat4& parent = glm::mat4(1))
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:18](../src/ModelGeometry.cpp#L18).

**Tham số:** `MeshData& v`: vector mesh output cần append; `glm::vec3 position`: tọa độ world hoặc local theo caller; `glm::vec3 scale`: vector tỉ lệ transform; `const glm::mat4& parent = glm::mat4(1)`: ma trận cha CPU.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Cache unit geometry::box static; model=parent·T(position)·S(scale), normal=transpose(inverse(mat3(model))). Bake mỗi vị trí và normal vào vector MeshData của material part; normalize sau transform. Đây là CPU mesh assembly, không phải một GPU draw cho từng hộp.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `geometry::box`, `glm::scale`, `glm::translate`, `glm::transpose`, `glm::inverse`, `glm::mat3`, `push_back`, `glm::vec3`, `glm::vec4`, `glm::normalize`.


<a id="f197"></a>
### beam

**Khai báo chính xác tại định nghĩa:**

```cpp
void beam(MeshData& v, glm::vec3 a, glm::vec3 b, float width, float depth)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:24](../src/ModelGeometry.cpp#L24).

**Tham số:** `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `float width`: chiều rộng pixel, riêng beam là world-unit theo signature; `float depth`: độ sâu chia đệ quy (beam depth riêng là bề dày world-unit).

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Từ a→b tạo axis, side=cross(axis, +Z) chuẩn hóa, back=cross(side, axis). Dựng basis cột side/axis/back và midpoint; gọi box với scale(width,|b−a|, depth). Chỉ dùng cho thanh mái không song song +Z; nếu mở rộng cho đoạn song song reference sẽ cần fallback.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `glm::normalize`, `glm::cross`, `glm::vec3`, `basis`, `glm::vec4`, `box`, `glm::distance`.


<a id="f198"></a>
### tube

**Khai báo chính xác tại định nghĩa:**

```cpp
void tube(MeshData& v, glm::vec3 a, glm::vec3 b, float ra, float rb, unsigned sides)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:31](../src/ModelGeometry.cpp#L31).

**Tham số:** `MeshData& v`: vector mesh output cần append; `glm::vec3 a`: điểm/vector/scalar đầu theo signature; `glm::vec3 b`: điểm/vector/scalar thứ hai theo signature; `float ra`: bán kính đầu a; `float rb`: bán kính đầu b; `unsigned sides`: số sector ống.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tạo cơ sở quanh axis, chọn reference +Y hoặc +X nếu gần song song Y. Mỗi sector dùng cos/sin tạo p/q; nối hai vòng bán kính ra/rb bằng quad, thêm cap tại a và b bằng tri. Tạo cylinder hoặc truncated cone tùy ra/rb; normals flat, tổng 4×sides triangles. Caller hiện bảo đảm a≠b, sides≥3 và bán kính dương.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `glm::normalize`, `std::abs`, `glm::vec3`, `glm::cross`, `std::cos`, `std::sin`, `quad`, `tri`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f199"></a>
### roofShell

**Khai báo chính xác tại định nghĩa:**

```cpp
void roofShell(MeshData& v)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:46](../src/ModelGeometry.cpp#L46).

**Tham số:** `MeshData& v`: vector mesh output cần append.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Xác định mái ngoài y 2.56→4.20 và mặt trong thấp hơn 0.12, z±2.15. back đối xứng Z. Tạo mười quad cho hai slope ngoài, hai trong, mép eave và đầu hồi để roof skin kín; House còn thêm ridge box riêng.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `quad`, `back`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f200"></a>
### roofShell::back@50

**Khai báo chính xác tại định nghĩa:**

```cpp
[](glm::vec3 p)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:50](../src/ModelGeometry.cpp#L50).

**Tham số:** `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhận bản sao điểm p, đặt z=−2.15 và trả lại. Chỉ mirror mặt trước sang mặt sau trong roofShell, không sửa điểm gốc.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f201"></a>
### gable

**Khai báo chính xác tại định nghĩa:**

```cpp
void gable(MeshData& v)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:62](../src/ModelGeometry.cpp#L62).

**Tham số:** `MeshData& v`: vector mesh output cần append.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tính độ cao edge bằng nội suy slope mặt dưới mái; tạo profile ngũ giác x±2, y 2.6..4.08. Fan triangulation hai mặt z±1.8, rồi nối cạnh bằng quad với outward=(dy, −dx, 0). Gable chạm underside roof; test dùng undersideAt để kiểm tra khe/hở.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `glm::vec3`, `size`, `tri`, `point`, `quad`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f202"></a>
### gable::point@65

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](std::size_t i, float z)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:65](../src/ModelGeometry.cpp#L65).

**Tham số:** `std::size_t i`: chỉ số vòng/đỉnh/cloud; `float z`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Capture profile; lấy điểm 2D tại i rồi thêm thành phần z thành vec3. Helper cho gable; i phải trong [0, 4].

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `glm::vec3`.


<a id="f203"></a>
### house

**Khai báo chính xác tại định nghĩa:**

```cpp
House house()
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:79](../src/ModelGeometry.cpp#L79).

**Tham số:** Không có.

**Giá trị trả về:** `House`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tạo House chín vector theo HousePart, reserve mỗi part. Ghép hộp walls và gable; nền/bậc stone; roofShell/ridge; trim góc nhà/eave/thanh chéo/khung cửa; cửa panel gỗ, tay nắm metal; ba cửa sổ gồm khung, kính lùi và bệ; chimney bằng bốn vách, viền stone và cavity tối. Tất cả bake trong local coordinates; trả chín material batches, test hiện đếm 734 triangles. Glass là hình học opaque có emission, không phải kính xuyên sáng.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `reserve`, `part`, `box`, `gable`, `roofShell`, `beam`, `tube`, `glm::rotate`, `glm::translate`, `glm::mat4`, `glm::radians`, `glm::vec3`, `window`, `quad`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f204"></a>
### house::part@82

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](HousePart p) -> MeshData&
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:82](../src/ModelGeometry.cpp#L82).

**Tham số:** `HousePart p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Chuyển HousePart sang size_t, trả reference h[index] để helper thêm vào đúng nhóm vật liệu. Enum Count là sentinel không được dùng làm index hợp lệ.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f205"></a>
### house::window@109

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](glm::vec3 center, float yaw)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:109](../src/ModelGeometry.cpp#L109).

**Tham số:** `glm::vec3 center`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới; `float yaw`: góc quanh Y theo độ.

**Giá trị trả về:** void suy diễn; thay đổi dữ liệu qua capture/reference.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tạo parent=T(center)·Ry(yaw). Ghép cặp thanh dọc/ngang, hai crossbars vào Trim; thêm glass box lùi và stone sill. Không khoét lỗ walls; khung/kính đặt nhô ngoài mặt tường để nhìn thấy.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `glm::rotate`, `glm::translate`, `glm::mat4`, `glm::radians`, `glm::vec3`, `box`, `part`.


<a id="f206"></a>
### treeLayout

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<TreeInstance, 4>& treeLayout()
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:130](../src/ModelGeometry.cpp#L130).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<TreeInstance, 4>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả const reference bốn TreeInstance static với vị trí/shade/height/trunkRadius/canopyWidth/branchCount/seed riêng. Số nhánh lần lượt 8, 9, 7, 10; seed 11, 29, 47, 83. Placement, grass exclusions và contact uniform cùng đọc layout này.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f207"></a>
### tree

**Khai báo chính xác tại định nghĩa:**

```cpp
Tree tree(const TreeParameters& p)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:140](../src/ModelGeometry.cpp#L140).

**Tham số:** `const TreeParameters& p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** `Tree`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reject thông số không finite hoặc ngoài budget height 2..6, trunkRadius(0, 0.35], canopyWidth 0.6..2, branchCount 4..12. Dùng seed riêng, tạo trunk tube 8 cạnh taper; các branch tube 6 cạnh với góc i×2.3999632 và jitter, chiều dài giảm theo height. Tạo tán 12 vòng×10 sector với profile height/width cố định, sectorScale random và sin irregularity; nối quad từng tầng, cap đáy và tip. Trả bark/foliage riêng; foliage 240 triangles, bark 32+24×branchCount. Không sinh cây bằng L-system và không dựng nhiều cone rời.

**Quan hệ — nơi gọi:** Scene constructor, Renderer contact setup, environment placement và model_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::invalid_argument`, `reserve`, `rng`, `random`, `tube`, `root`, `tip`, `std::cos`, `std::sin`, `center`, `std::clamp`, `outward`, `quad`, `bottom`, `tri`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f208"></a>
### tree::random@149

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](float low, float high)
```

**Vị trí mã nguồn:** [src/ModelGeometry.cpp:149](../src/ModelGeometry.cpp#L149).

**Tham số:** `float low`: cận dưới random; `float high`: cận trên random.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Capture RNG bằng tham chiếu, tạo uniform_real_distribution<float>(low, high) và lấy một số. Dùng seed local trong builder; thứ tự gọi RNG tác động silhouette; không đồng nghĩa bit-exact trên mọi standard library.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="file-39"></a>
## src/RainSystem.cpp

**Module/chức năng:** Pool 1000 giọt và ground impacts.

**Headers/phụ thuộc trực tiếp:** `RainSystem.hpp`, `algorithm`, `cmath`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/RainSystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [RainSystem::random](#f209) | `float low`: cận dưới random; `float high`: cận trên random. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả một mẫu phân bố đều float giữa low/high dùng mt19937 thành viên. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [RainSystem::respawn](#f210) | `Raindrop& drop`: reference slot Raindrop sẽ được ghi; `const glm::vec3& wind`: vector vận tốc gió world-unit/s (chỉ XZ dùng); `bool initial`: spawn lần đầu trải khắp volume, false chỉ spawn trên cao. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đặt x/z U(−9.95, 9.95), y ban đầu 0.05..8.4 hoặc tái sinh 7.6..8.4; velocity=(wind.x, U(−9, −6), wind.z), streakScale 0.85..1.15, opacity 0.55..0.82. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [RainSystem::reset](#f211) | `const glm::vec3& wind`: vector vận tốc gió world-unit/s (chỉ XZ dùng). | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Seed 1337; respawn toàn bộ 1000 slot với initial=true, xóa impactCount/recycledCount rồi tăng revision. | WeatherSystem::reset/update, Renderer và weather_tests. |
| [RainSystem::update](#f212) | `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `const glm::vec3& wind`: vector vận tốc gió world-unit/s (chỉ XZ dùng). | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Xóa impactCount trước cả kiểm tra dt; bỏ dt sai, clamp 0.1. | WeatherSystem::reset/update, Renderer và weather_tests. |


<a id="f209"></a>
### RainSystem::random

**Khai báo chính xác tại định nghĩa:**

```cpp
float RainSystem::random(float low, float high)
```

**Vị trí mã nguồn:** [src/RainSystem.cpp:6](../src/RainSystem.cpp#L6).

**Tham số:** `float low`: cận dưới random; `float high`: cận trên random.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả một mẫu phân bố đều float giữa low/high dùng mt19937 thành viên. Không tự kiểm tra thứ tự low/high; caller nội bộ cung cấp khoảng đúng. RNG được reset seed để tái hiện kiểm thử.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f210"></a>
### RainSystem::respawn

**Khai báo chính xác tại định nghĩa:**

```cpp
void RainSystem::respawn(Raindrop& drop, const glm::vec3& wind, bool initial)
```

**Vị trí mã nguồn:** [src/RainSystem.cpp:10](../src/RainSystem.cpp#L10).

**Tham số:** `Raindrop& drop`: reference slot Raindrop sẽ được ghi; `const glm::vec3& wind`: vector vận tốc gió world-unit/s (chỉ XZ dùng); `bool initial`: spawn lần đầu trải khắp volume, false chỉ spawn trên cao.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đặt x/z U(−9.95, 9.95), y ban đầu 0.05..8.4 hoặc tái sinh 7.6..8.4; velocity=(wind.x, U(−9, −6), wind.z), streakScale 0.85..1.15, opacity 0.55..0.82. length=clamp(speed×0.024×streakScale, 0.12, 0.42), lifetime=0. Chỉ đổi slot đã có, không cấp phát.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `random`, `std::clamp`, `glm::length`.


<a id="f211"></a>
### RainSystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void RainSystem::reset(const glm::vec3& wind)
```

**Vị trí mã nguồn:** [src/RainSystem.cpp:20](../src/RainSystem.cpp#L20).

**Tham số:** `const glm::vec3& wind`: vector vận tốc gió world-unit/s (chỉ XZ dùng).

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Seed 1337; respawn toàn bộ 1000 slot với initial=true, xóa impactCount/recycledCount rồi tăng revision. wind default lấy ở header; Weather reset truyền gió chung.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `seed`, `respawn`.


<a id="f212"></a>
### RainSystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void RainSystem::update(float deltaTime, const glm::vec3& wind)
```

**Vị trí mã nguồn:** [src/RainSystem.cpp:28](../src/RainSystem.cpp#L28).

**Tham số:** `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `const glm::vec3& wind`: vector vận tốc gió world-unit/s (chỉ XZ dùng).

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Xóa impactCount trước cả kiểm tra dt; bỏ dt sai, clamp 0.1. Mỗi drop lưu previous; velocity.y−=9.81 dt rồi max(−17), x/z tiến về wind bằng drag=min(2 dt, 1); position+=velocity mới×dt. Cập nhật length và tuổi. Nếu y≤0, t=prev.y/(prev.y−new.y), mix hit và đặt y 0; chỉ lưu impact trong ground±10, respawn và tăng recycled. Nếu chưa chạm đất nhưng x/z ra biên thì respawn không impact. Cuối tăng revision. Không xét mái/cây/đá hoặc thời hạn sống cực đại.

**Quan hệ — nơi gọi:** WeatherSystem::reset/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::min`, `glm::vec3`, `std::max`, `std::clamp`, `glm::length`, `glm::mix`, `std::abs`, `respawn`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="file-40"></a>
## src/Renderer.cpp

**Module/chức năng:** Render passes, uniforms và dynamic staging.

**Headers/phụ thuộc trực tiếp:** `Renderer.hpp`, `Surface.hpp`, `EnvironmentGeometry.hpp`, `ModelGeometry.hpp`, `algorithm`, `cmath`, `glm/gtc/type_ptr.hpp`, `glm/gtc/matrix_transform.hpp`, `string`.

**Kiểu/dữ liệu quản lý:** `FadingCloud`; xem bảng trạng thái phía trên.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Renderer::Renderer](#f213) | Không có. | Constructor/destructor: không có kiểu trả về. | Dựng ba shader programs, sky mesh và bốn DynamicBatch với capacity cố định; cache uniform locations. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [Renderer::Renderer::contactPoint@76](#f214) | `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể; `float radius`: bán kính world-unit của contact. | void suy diễn; thay đổi dữ liệu qua capture/reference. | Upload vec3(p.x, p.z, radius) vào uContacts[contact], tăng contact. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [Renderer::draw](#f215) | `const Scene& scene`: danh sách vật thể/animation CPU; `const Camera& camera`: trạng thái camera để tính ma trận/billboard; `const WeatherSystem& weather`: trạng thái mô phỏng/lighting CPU; `int width`: chiều rộng pixel, riêng beam là world-unit theo signature; `int height`: chiều cao pixel. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Reset counters; clear color/depth, vẽ sky không depth test/write rồi phục hồi. | Application initialize/display/smoke; getters dùng thống kê/caching tests. |
| [Renderer::draw::drawCloud@158](#f216) | `std::size_t i`: chỉ số vòng/đỉnh/cloud. | void suy diễn; thay đổi dữ liệu qua capture/reference. | Từ index cloud lấy position, chuyển height range local sang world bằng cộng y, upload coverage; model chỉ translation và normal identity. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [Renderer::draw::lambda@180](#f217) | `const auto& a`: điểm/vector/scalar đầu theo signature; `const auto& b`: điểm/vector/scalar thứ hai theo signature. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Comparator generic nhận const auto& a/b của struct local FadingCloud, trả a.depth>b.depth. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |


<a id="f213"></a>
### Renderer::Renderer

**Khai báo chính xác tại định nghĩa:**

```cpp
Renderer::Renderer()
```

**Vị trí mã nguồn:** [src/Renderer.cpp:12](../src/Renderer.cpp#L12).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Dựng ba shader programs, sky mesh và bốn DynamicBatch với capacity cố định; cache uniform locations. Dựng bốn skin mây bằng layout mặc định, lưu height min/max; ring 12 điểm. Bật depth LESS/cull BACK/CCW, upload flash attenuation, groundColor, ba puddle transforms và bảy contact anchors một lần. Cuối glUseProgram(0). Không có constructor tạo scene hoặc simulation update.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** `environment::cloud`, `clusters`, `glm::vec2`, `std::min`, `std::max`, `glEnable`, `size`, `std::cos`, `std::sin`, `glDepthFunc`, `glCullFace`, `glFrontFace`, `glClearColor`, `use`, `glUniform2f`, `uniform`, `glUniform3fv`, `glm::value_ptr`, `surface::puddles`, `glm::radians`, `glUniform4f`, `std::to_string`, `c_str`, `glUniform3f`, `models::treeLayout`, `contactPoint`, `environment::rocks`, `glUseProgram`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f214"></a>
### Renderer::Renderer::contactPoint@76

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](glm::vec3 p,float radius)
```

**Vị trí mã nguồn:** [src/Renderer.cpp:76](../src/Renderer.cpp#L76).

**Tham số:** `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể; `float radius`: bán kính world-unit của contact.

**Giá trị trả về:** void suy diễn; thay đổi dữ liệu qua capture/reference.

**Mục đích, quy trình và lưu ý kỹ thuật:** Upload vec3(p.x, p.z, radius) vào uContacts[contact], tăng contact. Constructor gọi cho bốn cây và ba đá, khớp array shader 7 phần tử. Bán kính tree 0.48, đá 0.9×max(scale.x, scale.z).

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `glUniform3f`, `uniform`, `std::to_string`, `c_str`.


<a id="f215"></a>
### Renderer::draw

**Khai báo chính xác tại định nghĩa:**

```cpp
void Renderer::draw(const Scene& scene, const Camera& camera, const WeatherSystem& weather,
                    int width, int height)
```

**Vị trí mã nguồn:** [src/Renderer.cpp:84](../src/Renderer.cpp#L84).

**Tham số:** `const Scene& scene`: danh sách vật thể/animation CPU; `const Camera& camera`: trạng thái camera để tính ma trận/billboard; `const WeatherSystem& weather`: trạng thái mô phỏng/lighting CPU; `int width`: chiều rộng pixel, riêng beam là world-unit theo signature; `int height`: chiều cao pixel.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reset counters; clear color/depth, vẽ sky không depth test/write rồi phục hồi. Upload common uniforms lighting/fog/camera/time/wind và matrices. Duyệt scene objects theo thứ tự lưu, upload material/model/normal, draw. Mây coverage=1 vẽ opaque trước, coverage∈(0, 1) sort theo projected camera depth rồi blend, không depth write. Rain dựng hai vertex/head mỗi drop nếu revision đổi, shader dựng tail; vẽ một GL_LINES batch. Splash gom slot sống, alpha theo remaining/initial, tail theo velocity; ripple dựng vòng 12 cạnh cho slot sống và dùng màu riêng theo waterMargin; mỗi loại upload khi revision đổi. Sét chỉ draw lúc active và intensity>0; rebuild billboard core/halo khi revision hoặc camera đổi, mỗi segment 12 vertex; intensity uniform nên pulse không bắt upload. Mọi effect alpha blend SRC_ALPHA/ONE_MINUS_SRC_ALPHA, giữ depth test và tắt depth write; sét tắt cull tạm thời. Khôi phục depth/cull/blend, unbind VAO/program cuối frame. Xem tài liệu 06 và 05 để theo dõi từng pass/uniform.

**Quan hệ — nơi gọi:** Application initialize/display/smoke; getters dùng thống kê/caching tests.

**Gọi tiếp/API nổi bật:** `skyColor`, `glClearColor`, `glClear`, `skyTop`, `fogColor`, `use`, `glUniform3fv`, `glm::value_ptr`, `glDisable`, `glDepthMask`, `draw`, `glEnable`, `glUniform1f`, `particleColor`, `position`, `ambientColor`, `directionalColor`, `lightDirection`, `windowStrength`, `flash`, `flashPosition`, `wetness`, `animationTime`, `glUniform2f`, `view`, `projection`, `glUniformMatrix4fv`, `objects`, `glUniformMatrix3fv`, `glUniform1i`, `glUniform4fv`, `triangleCount`, `glUniform4f`, `clusters`, `glm::vec2`, `CloudSystem::coverage`, `glm::translate`, `glm::mat4`, `normal`, `glUniform3f`, `drawCloud`, `glm::dot`, `forward`, `std::sort`, `begin`, `glBlendFunc`, `revision`, `drops`, `color`, `upload`, `data`, `size`, `glLineWidth`, `vertexCount`, `activeCount`, `particles`, `glm::normalize`, `std::max`, `surface::waterMargin`, `glm::vec4`, `std::min`, `glm::vec3`, `active`, `flashIntensity`, `glm::any`, `glm::notEqual`, `segmentCount`, `segments`, `glm::cross`, `glBindVertexArray`, `glUseProgram`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f216"></a>
### Renderer::draw::drawCloud@158

**Khai báo chính xác tại định nghĩa:**

```cpp
[&](std::size_t i)
```

**Vị trí mã nguồn:** [src/Renderer.cpp:158](../src/Renderer.cpp#L158).

**Tham số:** `std::size_t i`: chỉ số vòng/đỉnh/cloud.

**Giá trị trả về:** void suy diễn; thay đổi dữ liệu qua capture/reference.

**Mục đích, quy trình và lưu ý kỹ thuật:** Từ index cloud lấy position, chuyển height range local sang world bằng cộng y, upload coverage; model chỉ translation và normal identity. Upload cloud Color cố định, draw cached Mesh, tăng triangles/drawCalls. Lambda dùng chung cho opaque và fade pass; caller thiết lập blend/depth trước.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `clusters`, `glm::vec2`, `glUniform2f`, `glUniform1f`, `CloudSystem::coverage`, `glm::translate`, `glm::mat4`, `normal`, `glUniformMatrix4fv`, `glm::value_ptr`, `glUniformMatrix3fv`, `glUniform3f`, `draw`, `triangleCount`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f217"></a>
### Renderer::draw::lambda@180

**Khai báo chính xác tại định nghĩa:**

```cpp
[](const auto& a,const auto& b)
```

**Vị trí mã nguồn:** [src/Renderer.cpp:180](../src/Renderer.cpp#L180).

**Tham số:** `const auto& a`: điểm/vector/scalar đầu theo signature; `const auto& b`: điểm/vector/scalar thứ hai theo signature.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Comparator generic nhận const auto& a/b của struct local FadingCloud, trả a.depth>b.depth. Sort giảm camera-space depth để draw từ xa đến gần; không sort Euclidean distance hoặc từng triangle. Đây là generic lambda, không có template function viết tách riêng.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="file-41"></a>
## src/RippleSystem.cpp

**Module/chức năng:** Pool 48 vòng gợn.

**Headers/phụ thuộc trực tiếp:** `RippleSystem.hpp`, `algorithm`, `cmath`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/RippleSystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [RippleSystem::reset](#f218) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Fill pool Ripple bằng default aggregate, xóa active/cursor/rejected, seed 76, tăng revision. | WeatherSystem impact/update, Renderer và weather/environment_tests. |
| [RippleSystem::emit](#f219) | `glm::vec3 hit`: điểm impact world-space; `bool sampled`: bật bộ lọc xác suất emission; `float waterMargin`: khoảng cách tới mép nước; 0 là đất. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Reject y không kiểm tra, chỉ x/z finite và trong ±9.5; sampling nếu bật dùng 35% khi waterMargin>0 hoặc 5.5% ở đất. | WeatherSystem impact/update, Renderer và weather/environment_tests. |
| [RippleSystem::update](#f220) | `float dt`: bước thời gian giây. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bỏ dt không finite/≤0 hoặc active=0, clamp 0.1; duyệt slot lifetime>0. | WeatherSystem impact/update, Renderer và weather/environment_tests. |


<a id="f218"></a>
### RippleSystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void RippleSystem::reset()
```

**Vị trí mã nguồn:** [src/RippleSystem.cpp:5](../src/RippleSystem.cpp#L5).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Fill pool Ripple bằng default aggregate, xóa active/cursor/rejected, seed 76, tăng revision. Slot lifetime=0 là rỗng; radius/maxRadius còn default nhưng không được draw.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** `fill`, `seed`.


<a id="f219"></a>
### RippleSystem::emit

**Khai báo chính xác tại định nghĩa:**

```cpp
void RippleSystem::emit(glm::vec3 hit, bool sampled, float waterMargin)
```

**Vị trí mã nguồn:** [src/RippleSystem.cpp:13](../src/RippleSystem.cpp#L13).

**Tham số:** `glm::vec3 hit`: điểm impact world-space; `bool sampled`: bật bộ lọc xác suất emission; `float waterMargin`: khoảng cách tới mép nước; 0 là đất.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reject y không kiểm tra, chỉ x/z finite và trong ±9.5; sampling nếu bật dùng 35% khi waterMargin>0 hoặc 5.5% ở đất. maxRadius=min(0.45, 0.9 margin) trên nước, 0.45 trên đất; <0.08 thì bỏ. Pool đầy 48 mới tăng rejected_. Tìm slot chết từ cursor, đặt center y=max(0.028, hit.y+0.01), radius 0.04, maxRadius, elapsed 0, lifetime 0.55, alpha 0.50; tăng active/revision. Early return do sampling/vị trí/bờ không được đếm rejected.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::abs`, `std::min`, `std::max`.


<a id="f220"></a>
### RippleSystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void RippleSystem::update(float dt)
```

**Vị trí mã nguồn:** [src/RippleSystem.cpp:27](../src/RippleSystem.cpp#L27).

**Tham số:** `float dt`: bước thời gian giây.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bỏ dt không finite/≤0 hoặc active=0, clamp 0.1; duyệt slot lifetime>0. elapsed+=dt; t=clamp(elapsed/lifetime, 0, 1); radius=mix(0.04, maxRadius, t); alpha=0.5(1−t)². Khi t≥1 đặt lifetime 0, giảm active; tăng revision sau lượt. lifetime ở Ripple là tổng đời, khác Splash dùng remaining lifetime.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather/environment_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::min`, `std::clamp`, `glm::mix`.


<a id="file-42"></a>
## src/Scene.cpp

**Module/chức năng:** Sở hữu mesh và danh sách vật thể.

**Headers/phụ thuộc trực tiếp:** `Scene.hpp`, `Surface.hpp`, `ModelGeometry.hpp`, `EnvironmentGeometry.hpp`, `algorithm`, `cmath`, `glm/gtc/matrix_transform.hpp`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Scene.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [Scene::Scene](#f221) | Không có. | Constructor/destructor: không có kiểu trả về. | Dựng ground và batched puddles Mesh ngay trong initializer list. | Application initialize/update/display/smoke và Renderer. |
| [Scene::modelPart](#f222) | `const std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 position`: tọa độ world hoặc local theo caller; `glm::vec3 color`: RGB vật thể; `Material material`: loại và hệ số ánh sáng; `float treeHeight`: chiều cao cây world-unit, 0 tắt sway. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tạo Mesh từ vertices trong modelMeshes_ bằng unique_ptr; add với scale 1 và vị trí; ghi material/treeHeight lên object vừa thêm. | Application initialize/update/display/smoke và Renderer. |
| [Scene::add](#f223) | `const Mesh& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `glm::vec3 position`: tọa độ world hoặc local theo caller; `glm::vec3 scale`: vector tỉ lệ transform; `glm::vec3 color`: RGB vật thể; `float yaw`: góc quanh Y theo độ. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tạo M=T(position)·Ry(yaw)·S(scale) bằng GLM, normal=transpose(inverse(mat3(M))). | Application initialize/update/display/smoke và Renderer. |
| [Scene::triangleCount](#f224) | Không có. | `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình. | Duyệt objects_ cộng mesh->triangleCount, trả số triangles tĩnh submit nếu vẽ toàn scene. | Application initialize/update/display/smoke và Renderer. |
| [Scene::update](#f225) | `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `bool paused`: true thì không tiến giờ/animation. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu không paused và dt finite dương, animationTime_+=min(dt, 0.1). | Application initialize/update/display/smoke và Renderer. |


<a id="f221"></a>
### Scene::Scene

**Khai báo chính xác tại định nghĩa:**

```cpp
Scene::Scene()
```

**Vị trí mã nguồn:** [src/Scene.cpp:10](../src/Scene.cpp#L10).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Dựng ground và batched puddles Mesh ngay trong initializer list. Reserve objects 32/modelMeshes 21; add ground rồi puddle với cờ riêng. models::house tạo 9 material/color groups tại (−1, 0, −1). Mỗi bốn cây thêm bark và foliage có treeHeight. Ba đá mesh riêng theo seed, add scale/yaw rồi material Rock; cuối thêm mesh cỏ một batch. Tổng 23 SceneObject/3264 triangles; Mesh unique_ptr sở hữu tài nguyên, SceneObject giữ con trỏ không sở hữu.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** `reserve`, `add`, `back`, `models::house`, `size`, `modelPart`, `models::treeLayout`, `models::tree`, `glm::vec3`, `environment::rocks`, `push_back`, `environment::rock`, `environment::grass`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f222"></a>
### Scene::modelPart

**Khai báo chính xác tại định nghĩa:**

```cpp
void Scene::modelPart(const std::vector<Vertex>& vertices, glm::vec3 position,
                      glm::vec3 color, Material material, float treeHeight)
```

**Vị trí mã nguồn:** [src/Scene.cpp:53](../src/Scene.cpp#L53).

**Tham số:** `const std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 position`: tọa độ world hoặc local theo caller; `glm::vec3 color`: RGB vật thể; `Material material`: loại và hệ số ánh sáng; `float treeHeight`: chiều cao cây world-unit, 0 tắt sway.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tạo Mesh từ vertices trong modelMeshes_ bằng unique_ptr; add với scale 1 và vị trí; ghi material/treeHeight lên object vừa thêm. Dùng cho từng nhóm nhà/cây/cỏ; vertex CPU có thể giải phóng sau upload.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** `push_back`, `add`, `back`.


<a id="f223"></a>
### Scene::add

**Khai báo chính xác tại định nghĩa:**

```cpp
void Scene::add(const Mesh& mesh, glm::vec3 position, glm::vec3 scale,
                glm::vec3 color, float yaw)
```

**Vị trí mã nguồn:** [src/Scene.cpp:61](../src/Scene.cpp#L61).

**Tham số:** `const Mesh& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `glm::vec3 position`: tọa độ world hoặc local theo caller; `glm::vec3 scale`: vector tỉ lệ transform; `glm::vec3 color`: RGB vật thể; `float yaw`: góc quanh Y theo độ.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tạo M=T(position)·Ry(yaw)·S(scale) bằng GLM, normal=transpose(inverse(mat3(M))). Push SceneObject trỏ tới mesh với color; các cờ/material/treeHeight còn defaults. Mesh phải sống lâu hơn object; scale zero sẽ làm inverse suy biến, caller hiện dùng scale dương.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** `glm::scale`, `glm::rotate`, `glm::translate`, `glm::mat4`, `glm::radians`, `glm::vec3`, `push_back`, `glm::transpose`, `glm::inverse`, `glm::mat3`.


<a id="f224"></a>
### Scene::triangleCount

**Khai báo chính xác tại định nghĩa:**

```cpp
std::size_t Scene::triangleCount() const
```

**Vị trí mã nguồn:** [src/Scene.cpp:69](../src/Scene.cpp#L69).

**Tham số:** Không có.

**Giá trị trả về:** `std::size_t`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Duyệt objects_ cộng mesh->triangleCount, trả số triangles tĩnh submit nếu vẽ toàn scene. Không gồm sky, cloud hoặc effect; không đếm sau culling.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** `triangleCount`.


<a id="f225"></a>
### Scene::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void Scene::update(float deltaTime, bool paused)
```

**Vị trí mã nguồn:** [src/Scene.cpp:75](../src/Scene.cpp#L75).

**Tham số:** `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1; `bool paused`: true thì không tiến giờ/animation.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu không paused và dt finite dương, animationTime_+=min(dt, 0.1). Shader dùng thời gian này để sway cây. Không sửa vertex CPU và không tăng revision của rain.

**Quan hệ — nơi gọi:** Application initialize/update/display/smoke và Renderer.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::min`.


<a id="file-43"></a>
## src/Shader.cpp

**Module/chức năng:** Đọc/compile/link GLSL.

**Headers/phụ thuộc trực tiếp:** `Shader.hpp`, `fstream`, `sstream`, `stdexcept`, `vector`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Shader.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [compile](#f226) | `GLenum type`: GL_VERTEX_SHADER hoặc GL_FRAGMENT_SHADER; `const std::string& path`: đường dẫn file shader. | `GLuint`; xem ý nghĩa và điều kiện biên trong quy trình. | Mở/đọc toàn shader text; lỗi file/read thì runtime_error. | Renderer constructor/draw; RAII cleanup. |
| [Shader::Shader](#f227) | `const std::string& vertexPath`: đường dẫn vertex shader; `const std::string& fragmentPath`: đường dẫn fragment shader. | Constructor/destructor: không có kiểu trả về. | Compile vertex trước; trong try compile fragment, tạo program, attach và link, kiểm tra LINK_STATUS và lấy info log khi lỗi. | Renderer constructor/draw; RAII cleanup. |
| [Shader::~Shader](#f228) | Không có. | Constructor/destructor: không có kiểu trả về. | glDeleteProgram(program_). | Renderer constructor/draw; RAII cleanup. |
| [Shader::uniform](#f229) | `const char* name`: tên uniform hoặc tên capture/test theo hàm. | `GLint`; xem ý nghĩa và điều kiện biên trong quy trình. | Gọi glGetUniformLocation(program_, name); âm thì throw Missing active shader uniform. | Renderer constructor/draw; RAII cleanup. |


<a id="f226"></a>
### compile

**Khai báo chính xác tại định nghĩa:**

```cpp
GLuint compile(GLenum type, const std::string& path)
```

**Vị trí mã nguồn:** [src/Shader.cpp:9](../src/Shader.cpp#L9).

**Tham số:** `GLenum type`: GL_VERTEX_SHADER hoặc GL_FRAGMENT_SHADER; `const std::string& path`: đường dẫn file shader.

**Giá trị trả về:** `GLuint`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Mở/đọc toàn shader text; lỗi file/read thì runtime_error. glCreateShader, shader Source/compile, đọc COMPILE_STATUS; nếu fail lấy info log, xóa shader và ném lỗi có path. Thành công trả shader GLuint chưa gắn program, caller chịu trách nhiệm cleanup.

**Quan hệ — nơi gọi:** Renderer constructor/draw; RAII cleanup.

**Gọi tiếp/API nổi bật:** `file`, `std::runtime_error`, `rdbuf`, `bad`, `str`, `c_str`, `glCreateShader`, `glShaderSource`, `glCompileShader`, `glGetShaderiv`, `log`, `glGetShaderInfoLog`, `size`, `data`, `glDeleteShader`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f227"></a>
### Shader::Shader

**Khai báo chính xác tại định nghĩa:**

```cpp
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
```

**Vị trí mã nguồn:** [src/Shader.cpp:41](../src/Shader.cpp#L41).

**Tham số:** `const std::string& vertexPath`: đường dẫn vertex shader; `const std::string& fragmentPath`: đường dẫn fragment shader.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** Compile vertex trước; trong try compile fragment, tạo program, attach và link, kiểm tra LINK_STATUS và lấy info log khi lỗi. Catch xóa program và shader đã tạo rồi rethrow. Thành công detach/xóa hai shader object vì program giữ executable đã link. program_ sống tới destructor.

**Quan hệ — nơi gọi:** Renderer constructor/draw; RAII cleanup.

**Gọi tiếp/API nổi bật:** `compile`, `glCreateProgram`, `std::runtime_error`, `glAttachShader`, `glLinkProgram`, `glGetProgramiv`, `log`, `glGetProgramInfoLog`, `size`, `data`, `std::string`, `glDeleteProgram`, `glDeleteShader`, `glDetachShader`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f228"></a>
### Shader::~Shader

**Khai báo chính xác tại định nghĩa:**

```cpp
Shader::~Shader()
```

**Vị trí mã nguồn:** [src/Shader.cpp:75](../src/Shader.cpp#L75).

**Tham số:** Không có.

**Giá trị trả về:** Constructor/destructor: không có kiểu trả về.

**Mục đích, quy trình và lưu ý kỹ thuật:** glDeleteProgram(program_). Không gọi glUseProgram(0) ở đây; renderer kết thúc draw đã unbind, Application bảo đảm context còn sống.

**Quan hệ — nơi gọi:** Renderer constructor/draw; RAII cleanup.

**Gọi tiếp/API nổi bật:** `glDeleteProgram`.


<a id="f229"></a>
### Shader::uniform

**Khai báo chính xác tại định nghĩa:**

```cpp
GLint Shader::uniform(const char* name) const
```

**Vị trí mã nguồn:** [src/Shader.cpp:77](../src/Shader.cpp#L77).

**Tham số:** `const char* name`: tên uniform hoặc tên capture/test theo hàm.

**Giá trị trả về:** `GLint`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Gọi glGetUniformLocation(program_, name); âm thì throw Missing active shader uniform. Trả GLint để Renderer cache. Uniform có thể bị optimizer loại nếu không dùng; API này cố tình coi trường hợp đó là lỗi initialization.

**Quan hệ — nơi gọi:** Renderer constructor/draw; RAII cleanup.

**Gọi tiếp/API nổi bật:** `glGetUniformLocation`, `std::runtime_error`, `std::string`.


<a id="file-44"></a>
## src/SplashSystem.cpp

**Module/chức năng:** Pool 512 hạt bắn.

**Headers/phụ thuộc trực tiếp:** `SplashSystem.hpp`, `algorithm`, `cmath`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/SplashSystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [SplashSystem::random](#f230) | `float low`: cận dưới random; `float high`: cận trên random. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả một mẫu phân bố đều float giữa low/high dùng mt19937 thành viên. | WeatherSystem impact/update, Renderer và weather_tests. |
| [SplashSystem::reset](#f231) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Seed 2026, fill pool bằng default SplashParticle, xóa cursor/activeCount và requests/rejected/filtered, tăng revision. | WeatherSystem impact/update, Renderer và weather_tests. |
| [SplashSystem::emit](#f232) | `glm::vec3 hit`: điểm impact world-space; `bool sampled`: bật bộ lọc xác suất emission. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Tăng requests đầu tiên. | WeatherSystem impact/update, Renderer và weather_tests. |
| [SplashSystem::update](#f233) | `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Bỏ dt sai hoặc không có active; clamp 0.1; duyệt slot sống, vy−=9.81 dt, position+=v mới×dt, lifetime−=dt. | WeatherSystem impact/update, Renderer và weather_tests. |


<a id="f230"></a>
### SplashSystem::random

**Khai báo chính xác tại định nghĩa:**

```cpp
float SplashSystem::random(float low, float high)
```

**Vị trí mã nguồn:** [src/SplashSystem.cpp:6](../src/SplashSystem.cpp#L6).

**Tham số:** `float low`: cận dưới random; `float high`: cận trên random.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả một mẫu phân bố đều float giữa low/high dùng mt19937 thành viên. Không tự kiểm tra thứ tự low/high; caller nội bộ cung cấp khoảng đúng. RNG được reset seed để tái hiện kiểm thử.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f231"></a>
### SplashSystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void SplashSystem::reset()
```

**Vị trí mã nguồn:** [src/SplashSystem.cpp:10](../src/SplashSystem.cpp#L10).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Seed 2026, fill pool bằng default SplashParticle, xóa cursor/activeCount và requests/rejected/filtered, tăng revision. Không đổi capacity 512 hoặc cấp array mới.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `seed`, `fill`.


<a id="f232"></a>
### SplashSystem::emit

**Khai báo chính xác tại định nghĩa:**

```cpp
void SplashSystem::emit(glm::vec3 hit, bool sampled)
```

**Vị trí mã nguồn:** [src/SplashSystem.cpp:18](../src/SplashSystem.cpp#L18).

**Tham số:** `glm::vec3 hit`: điểm impact world-space; `bool sampled`: bật bộ lọc xác suất emission.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tăng requests đầu tiên. x/z không finite hoặc ngoài ground±10 tăng rejected; sampled và U>0.45 tăng filtered; còn <2 slot trống tăng rejected. Nếu được nhận, hai lần scan vòng từ cursor tới slot lifetime≤0; đặt y 0.02, velocity radial speed 0.5..1.4 và y 1.2..2.4, lifetime/initialLifetime 0.16..0.28, length 0.05..0.10; tăng active và cursor. Cuối tăng revision. Không ghi đè particle sống; accepted request phát đúng hai hạt.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::abs`, `random`, `std::cos`, `std::sin`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f233"></a>
### SplashSystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void SplashSystem::update(float deltaTime)
```

**Vị trí mã nguồn:** [src/SplashSystem.cpp:38](../src/SplashSystem.cpp#L38).

**Tham số:** `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Bỏ dt sai hoặc không có active; clamp 0.1; duyệt slot sống, vy−=9.81 dt, position+=v mới×dt, lifetime−=dt. Nếu lifetime≤0 hoặc y≤0.01 đặt lifetime 0 và giảm activeCount. Cuối tăng revision. Không có va chạm ngang/nhà và không nảy lại.

**Quan hệ — nơi gọi:** WeatherSystem impact/update, Renderer và weather_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::min`.


<a id="file-45"></a>
## src/Surface.cpp

**Module/chức năng:** Polygon nước dùng chung.

**Headers/phụ thuộc trực tiếp:** `Surface.hpp`, `algorithm`, `cmath`, `limits`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/Surface.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [puddles](#f234) | Không có. | `const std::array<Puddle, 3>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả const reference ba Puddle static: center(−2, 0.012, 4.5)/scale(1.8, 1)/yaw 15°; (3.6, 0.012, 1.9)/(1.2, 0.75)/−22°; (−5, 0.012, −4)/(0.95, 0.65)/35°. | EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests. |
| [outlineRadius](#f235) | `float a`: điểm/vector/scalar đầu theo signature. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | a tính radian; trả 1+0.10 sin(3 a)+0.055 cos(5 a)+0.035 sin(7 a+0.6). | EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests. |
| [outline](#f236) | Không có. | `const std::array<glm::vec2, OUTLINE_POINTS>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả const reference outline static khởi tạo một lần bằng lambda points. | EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests. |
| [outline::points@20](#f237) | Không có tham số; lambda capture như khai báo. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Duyệt 32 góc a=2πi/32, lấy outlineRadius(a), đặt vec2(cos a, sin a)×r và trả array. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [polygon](#f238) | `std::size_t index`: chỉ số phần tử/preset theo hàm. | `const std::array<glm::vec2, OUTLINE_POINTS>&`; xem ý nghĩa và điều kiện biên trong quy trình. | Static cache chứa ba polygon world-space do lambda polygons dựng. | EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests. |
| [polygon::polygons@35](#f239) | Không có tham số; lambda capture như khai báo. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Với mỗi puddle, lấy yaw radian; scale từng outline point theo XZ, quay (x, z) thành (cos a·x+sin a·z, −sin a·x+cos a·z), cộng center XZ. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |
| [waterMargin](#f240) | `glm::vec3 hit`: điểm impact world-space. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Static bounds AABB tính một lần; chỉ xét hit.xz, bỏ qua y. | EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests. |
| [waterMargin::bounds@53](#f241) | Không có tham số; lambda capture như khai báo. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Duyệt world polygon mỗi vũng, khởi low=maxfloat/high=−maxfloat; lấy min/max từng point, lưu vec4(minX, minZ, maxX, maxZ). | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |


<a id="f234"></a>
### puddles

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<Puddle, 3>& puddles()
```

**Vị trí mã nguồn:** [src/Surface.cpp:7](../src/Surface.cpp#L7).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<Puddle, 3>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả const reference ba Puddle static: center(−2, 0.012, 4.5)/scale(1.8, 1)/yaw 15°; (3.6, 0.012, 1.9)/(1.2, 0.75)/−22°; (−5, 0.012, −4)/(0.95, 0.65)/35°. Dữ liệu dùng chung CPU geometry, hit-test và shader uniforms.

**Quan hệ — nơi gọi:** EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f235"></a>
### outlineRadius

**Khai báo chính xác tại định nghĩa:**

```cpp
float outlineRadius(float a)
```

**Vị trí mã nguồn:** [src/Surface.cpp:15](../src/Surface.cpp#L15).

**Tham số:** `float a`: điểm/vector/scalar đầu theo signature.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** a tính radian; trả 1+0.10 sin(3 a)+0.055 cos(5 a)+0.035 sin(7 a+0.6). Tạo bán kính biến thiên trơn quanh gốc; không noise/RNG. Shader waterInterior lặp công thức để blend bờ, nhưng polygon thực chỉ 32 mẫu.

**Quan hệ — nơi gọi:** EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `std::sin`, `std::cos`.


<a id="f236"></a>
### outline

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<glm::vec2, OUTLINE_POINTS>& outline()
```

**Vị trí mã nguồn:** [src/Surface.cpp:19](../src/Surface.cpp#L19).

**Tham số:** Không có.

**Giá trị trả về:** `const std::array<glm::vec2, OUTLINE_POINTS>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả const reference outline static khởi tạo một lần bằng lambda points. 32 điểm local XZ trước scale/rotation/translation; caller không được sửa array.

**Quan hệ — nơi gọi:** EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `size`, `outlineRadius`, `glm::vec2`, `std::cos`, `std::sin`.


<a id="f237"></a>
### outline::points@20

**Khai báo chính xác tại định nghĩa:**

```cpp
[]
```

**Vị trí mã nguồn:** [src/Surface.cpp:20](../src/Surface.cpp#L20).

**Tham số:** Không có tham số; lambda capture như khai báo.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Duyệt 32 góc a=2πi/32, lấy outlineRadius(a), đặt vec2(cos a, sin a)×r và trả array. Không sinh lại outline cho từng collision.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `size`, `outlineRadius`, `glm::vec2`, `std::cos`, `std::sin`.


<a id="f238"></a>
### polygon

**Khai báo chính xác tại định nghĩa:**

```cpp
const std::array<glm::vec2, OUTLINE_POINTS>& polygon(std::size_t index)
```

**Vị trí mã nguồn:** [src/Surface.cpp:32](../src/Surface.cpp#L32).

**Tham số:** `std::size_t index`: chỉ số phần tử/preset theo hàm.

**Giá trị trả về:** `const std::array<glm::vec2, OUTLINE_POINTS>&`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Static cache chứa ba polygon world-space do lambda polygons dựng. Trả polygons.at(index), ngoài index 0..2 ném out_of_range. Geometry nước và waterMargin đọc cùng array nên hình học/containment khớp polygon.

**Quan hệ — nơi gọi:** EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `puddles`, `size`, `glm::radians`, `outline`, `glm::vec2`, `std::cos`, `std::sin`, `at`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f239"></a>
### polygon::polygons@35

**Khai báo chính xác tại định nghĩa:**

```cpp
[]
```

**Vị trí mã nguồn:** [src/Surface.cpp:35](../src/Surface.cpp#L35).

**Tham số:** Không có tham số; lambda capture như khai báo.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Với mỗi puddle, lấy yaw radian; scale từng outline point theo XZ, quay (x, z) thành (cos a·x+sin a·z, −sin a·x+cos a·z), cộng center XZ. Đây là Ry dương theo GLM; trả ba array 32 điểm, được chạy một lần.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `puddles`, `size`, `glm::radians`, `outline`, `glm::vec2`, `std::cos`, `std::sin`.


<a id="f240"></a>
### waterMargin

**Khai báo chính xác tại định nghĩa:**

```cpp
float waterMargin(glm::vec3 hit)
```

**Vị trí mã nguồn:** [src/Surface.cpp:52](../src/Surface.cpp#L52).

**Tham số:** `glm::vec3 hit`: điểm impact world-space.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Static bounds AABB tính một lần; chỉ xét hit.xz, bỏ qua y. Với mỗi puddle, reject nhanh ngoài AABB; ray crossing theo từng cạnh toggle inside. Đồng thời chiếu point lên segment với t clamp[0, 1], giữ khoảng cách nhỏ nhất. Nếu inside trả margin, ngoài tất cả trả 0. Tối đa 3×32 cạnh, không phải kiểm tra ellipsoid hay physics nước.

**Quan hệ — nơi gọi:** EnvironmentGeometry/Geometry, WeatherSystem, Renderer và CPU tests.

**Gọi tiếp/API nổi bật:** `size`, `low`, `max`, `high`, `polygon`, `glm::min`, `glm::max`, `point`, `puddles`, `surface::polygon`, `std::clamp`, `glm::dot`, `std::min`, `glm::distance`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f241"></a>
### waterMargin::bounds@53

**Khai báo chính xác tại định nghĩa:**

```cpp
[]
```

**Vị trí mã nguồn:** [src/Surface.cpp:53](../src/Surface.cpp#L53).

**Tham số:** Không có tham số; lambda capture như khai báo.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Duyệt world polygon mỗi vũng, khởi low=maxfloat/high=−maxfloat; lấy min/max từng point, lưu vec4(minX, minZ, maxX, maxZ). Cache này chỉ hợp lệ vì layout Surface bất biến.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `size`, `low`, `max`, `high`, `polygon`, `glm::min`, `glm::max`.


<a id="file-46"></a>
## src/TimeOfDaySystem.cpp

**Module/chức năng:** Đồng hồ giờ mô phỏng.

**Headers/phụ thuộc trực tiếp:** `TimeOfDaySystem.hpp`, `cmath`, `cstdio`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/TimeOfDaySystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [TimeOfDaySystem::reset](#f242) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Khôi phục hour 18, dayDuration 600 và autoAdvance=false. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [TimeOfDaySystem::setHour](#f243) | `double hour`: giờ mô phỏng; 24 giờ/một ngày. | `bool`; xem ý nghĩa và điều kiện biên trong quy trình. | Reject nonfinite bằng false và giữ giờ trước; finite dùng fmod(hour, 24), âm cộng 24; nếu làm tròn lên 24 thì đặt 0. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [TimeOfDaySystem::setDayDuration](#f244) | `double seconds`: số giây (duration hoặc dt theo hàm). | `bool`; xem ý nghĩa và điều kiện biên trong quy trình. | Reject nonfinite hoặc seconds<60 hay>3600 bằng false; còn lại ghi dayDuration và true. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [TimeOfDaySystem::update](#f245) | `double seconds`: số giây (duration hoặc dt theo hàm); `bool paused`: true thì không tiến giờ/animation. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Nếu auto off, paused, dt sai hoặc≤0 thì return. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |
| [TimeOfDaySystem::label](#f246) | Không có. | `std::string`; xem ý nghĩa và điều kiện biên trong quy trình. | minute=unsigned(hour×60)%1440, snprintf vào char[6] theo %02u:%02u, trả std::string. | Application CLI/input, WeatherSystem fixed-step và lighting_time_tests. |


<a id="f242"></a>
### TimeOfDaySystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void TimeOfDaySystem::reset()
```

**Vị trí mã nguồn:** [src/TimeOfDaySystem.cpp:5](../src/TimeOfDaySystem.cpp#L5).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khôi phục hour 18, dayDuration 600 và autoAdvance=false. Weather reset gọi khi R; không đồng bộ đồng hồ hệ điều hành.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** Không gọi thêm hàm; đọc/ghi trực tiếp state..


<a id="f243"></a>
### TimeOfDaySystem::setHour

**Khai báo chính xác tại định nghĩa:**

```cpp
bool TimeOfDaySystem::setHour(double hour)
```

**Vị trí mã nguồn:** [src/TimeOfDaySystem.cpp:6](../src/TimeOfDaySystem.cpp#L6).

**Tham số:** `double hour`: giờ mô phỏng; 24 giờ/một ngày.

**Giá trị trả về:** `bool`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reject nonfinite bằng false và giữ giờ trước; finite dùng fmod(hour, 24), âm cộng 24; nếu làm tròn lên 24 thì đặt 0. Trả true. Ví dụ−1→23; API chấp nhận mọi giờ hữu hạn, CLI giới hạn [0, 24].

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `std::fmod`.


<a id="f244"></a>
### TimeOfDaySystem::setDayDuration

**Khai báo chính xác tại định nghĩa:**

```cpp
bool TimeOfDaySystem::setDayDuration(double seconds)
```

**Vị trí mã nguồn:** [src/TimeOfDaySystem.cpp:12](../src/TimeOfDaySystem.cpp#L12).

**Tham số:** `double seconds`: số giây (duration hoặc dt theo hàm).

**Giá trị trả về:** `bool`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Reject nonfinite hoặc seconds<60 hay>3600 bằng false; còn lại ghi dayDuration và true. Đơn vị giây thực cho 24 giờ mô phỏng. Không thay hour hoặc autoAdvance.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`.


<a id="f245"></a>
### TimeOfDaySystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void TimeOfDaySystem::update(double seconds,bool paused)
```

**Vị trí mã nguồn:** [src/TimeOfDaySystem.cpp:16](../src/TimeOfDaySystem.cpp#L16).

**Tham số:** `double seconds`: số giây (duration hoặc dt theo hàm); `bool paused`: true thì không tiến giờ/animation.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nếu auto off, paused, dt sai hoặc≤0 thì return. setHour(hour+fmod(seconds, dayDuration)×24/dayDuration). Modulo trước tránh nhân overflow và bỏ nguyên số ngày; mặc định 10 giây→0.4 giờ. Bản thân API hỗ trợ dt lớn, Weather caller vẫn clamp stall 0.1.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `setHour`, `std::fmod`.


<a id="f246"></a>
### TimeOfDaySystem::label

**Khai báo chính xác tại định nghĩa:**

```cpp
std::string TimeOfDaySystem::label() const
```

**Vị trí mã nguồn:** [src/TimeOfDaySystem.cpp:21](../src/TimeOfDaySystem.cpp#L21).

**Tham số:** Không có.

**Giá trị trả về:** `std::string`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** minute=unsigned(hour×60)%1440, snprintf vào char[6] theo %02u:%02u, trả std::string. Cắt xuống phút, không làm tròn; 6.5→06:30. Invariant hour∈[0, 24) từ setHour bảo đảm cast an toàn.

**Quan hệ — nơi gọi:** Application CLI/input, WeatherSystem fixed-step và lighting_time_tests.

**Gọi tiếp/API nổi bật:** `std::snprintf`.


<a id="file-47"></a>
## src/WeatherSystem.cpp

**Module/chức năng:** Điều phối fixed timestep.

**Headers/phụ thuộc trực tiếp:** `WeatherSystem.hpp`, `Surface.hpp`, `algorithm`, `cmath`.

**Kiểu/dữ liệu quản lý:** Implementation dùng state khai báo tại `include/WeatherSystem.hpp` và local variables theo từng hàm.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [WeatherSystem::reset](#f247) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Khôi phục wind, reset clouds/rain/lightning/lighting/time/splash/ripple, bật mọi weather/fog, bỏ pause, accumulator 0 và refreshLighting. | Application initialize/update/key/smoke/statistics, CPU tests. |
| [WeatherSystem::refreshLighting](#f248) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Storminess được ánh xạ từ booleans: clouds+rain 0.9, chỉ cloud 0.55, chỉ rain 0.45, không cả hai 0. | Application initialize/update/key/smoke/statistics, CPU tests. |
| [WeatherSystem::update](#f249) | `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Pause/dt sai/≤0 chỉ refreshLighting rồi return. | Application initialize/update/key/smoke/statistics, CPU tests. |
| [WeatherSystem::triggerLightning](#f250) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Trigger strike ngay từ clouds và observerPosition; cập nhật vị trí nguồn và flashIntensity vào Lighting với default dt 0. | Application initialize/update/key/smoke/statistics, CPU tests. |
| [WeatherSystem::toggleLightning](#f251) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Đảo lightningEnabled; nếu vừa tắt thì lightning.cancel; cập nhật lighting.flash từ hệ sét, không tăng wetness vì dt mặc định 0. | Application initialize/update/key/smoke/statistics, CPU tests. |
| [WeatherSystem::valid](#f252) | Không có. | `bool`; xem ý nghĩa và điều kiện biên trong quy trình. | Kiểm tra hour finite/range, wetness/flash∈[0, 1]; rain position/velocity/length finite và bounds; đếm lại splash/ripple slot lifetime>0 để so active counters; cloud position finite và bounds; lightning endpoint finite. | Application initialize/update/key/smoke/statistics, CPU tests. |
| [WeatherSystem::valid::finite@69](#f253) | `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình. | Trả conjunction isfinite của ba thành phần vec3. | Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi. |


<a id="f247"></a>
### WeatherSystem::reset

**Khai báo chính xác tại định nghĩa:**

```cpp
void WeatherSystem::reset()
```

**Vị trí mã nguồn:** [src/WeatherSystem.cpp:7](../src/WeatherSystem.cpp#L7).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Khôi phục wind, reset clouds/rain/lightning/lighting/time/splash/ripple, bật mọi weather/fog, bỏ pause, accumulator 0 và refreshLighting. observerPosition không reset ở hàm này; Application::update/handler L gán lại camera khi cần.

**Quan hệ — nơi gọi:** Application initialize/update/key/smoke/statistics, CPU tests.

**Gọi tiếp/API nổi bật:** `reset`, `refreshLighting`.


<a id="f248"></a>
### WeatherSystem::refreshLighting

**Khai báo chính xác tại định nghĩa:**

```cpp
void WeatherSystem::refreshLighting()
```

**Vị trí mã nguồn:** [src/WeatherSystem.cpp:25](../src/WeatherSystem.cpp#L25).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Storminess được ánh xạ từ booleans: clouds+rain 0.9, chỉ cloud 0.55, chỉ rain 0.45, không cả hai 0. Gọi lighting.setEnvironment(float(timeOfDay.hour), storm). Không trộn flash vào state nền hoặc đổi particle.

**Quan hệ — nơi gọi:** Application initialize/update/key/smoke/statistics, CPU tests.

**Gọi tiếp/API nổi bật:** `setEnvironment`, `hour`.


<a id="f249"></a>
### WeatherSystem::update

**Khai báo chính xác tại định nghĩa:**

```cpp
void WeatherSystem::update(float deltaTime)
```

**Vị trí mã nguồn:** [src/WeatherSystem.cpp:30](../src/WeatherSystem.cpp#L30).

**Tham số:** `float deltaTime`: bước thời gian giây; các module mô phỏng tự clamp 0.1.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Pause/dt sai/≤0 chỉ refreshLighting rồi return. Cộng min(dt, 0.1) vào accumulator; while accumulator+1e−7≥1/120: update clock; cloud nếu bật; rain nếu bật; lightning luôn với random flag; set flash position; update splash rồi ripple cũ; sau đó mỗi rain impact emit splash sampled và ripple sampled với surface margin; cuối update flash/wetness và trừ step bằng max(0, ..). Sau vòng refreshLighting. Particle mới phát chưa tích phân cho tới substep kế. Không có render interpolation; clamp stall làm thời gian mô phỏng chậm hơn wall-time.

**Quan hệ — nơi gọi:** Application initialize/update/key/smoke/statistics, CPU tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `refreshLighting`, `std::min`, `update`, `setFlashPosition`, `lightPosition`, `impactCount`, `emit`, `impacts`, `surface::waterMargin`, `flashIntensity`, `std::max`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f250"></a>
### WeatherSystem::triggerLightning

**Khai báo chính xác tại định nghĩa:**

```cpp
void WeatherSystem::triggerLightning()
```

**Vị trí mã nguồn:** [src/WeatherSystem.cpp:55](../src/WeatherSystem.cpp#L55).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trigger strike ngay từ clouds và observerPosition; cập nhật vị trí nguồn và flashIntensity vào Lighting với default dt 0. Không xét paused hoặc lightningEnabled: L luôn hoạt động, ngay cả lúc pause.

**Quan hệ — nơi gọi:** Application initialize/update/key/smoke/statistics, CPU tests.

**Gọi tiếp/API nổi bật:** `trigger`, `setFlashPosition`, `lightPosition`, `update`, `flashIntensity`.


<a id="f251"></a>
### WeatherSystem::toggleLightning

**Khai báo chính xác tại định nghĩa:**

```cpp
void WeatherSystem::toggleLightning()
```

**Vị trí mã nguồn:** [src/WeatherSystem.cpp:61](../src/WeatherSystem.cpp#L61).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Đảo lightningEnabled; nếu vừa tắt thì lightning.cancel; cập nhật lighting.flash từ hệ sét, không tăng wetness vì dt mặc định 0. Nếu bật lại chỉ chờ countdown hiện tại, không tạo strike tức thì.

**Quan hệ — nơi gọi:** Application initialize/update/key/smoke/statistics, CPU tests.

**Gọi tiếp/API nổi bật:** `cancel`, `update`, `flashIntensity`.


<a id="f252"></a>
### WeatherSystem::valid

**Khai báo chính xác tại định nghĩa:**

```cpp
bool WeatherSystem::valid() const
```

**Vị trí mã nguồn:** [src/WeatherSystem.cpp:67](../src/WeatherSystem.cpp#L67).

**Tham số:** Không có.

**Giá trị trả về:** `bool`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra hour finite/range, wetness/flash∈[0, 1]; rain position/velocity/length finite và bounds; đếm lại splash/ripple slot lifetime>0 để so active counters; cloud position finite và bounds; lightning endpoint finite. Trả false ở lỗi đầu, true nếu qua hết. Không kiểm tra mọi thuộc tính: wind, dayDuration, widths, pulse arrays, material/GL không thuộc validator này.

**Quan hệ — nơi gọi:** Application initialize/update/key/smoke/statistics, CPU tests.

**Gọi tiếp/API nổi bật:** `std::isfinite`, `hour`, `wetness`, `flash`, `drops`, `finite`, `std::abs`, `particles`, `activeCount`, `clusters`, `segmentCount`, `segments`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f253"></a>
### WeatherSystem::valid::finite@69

**Khai báo chính xác tại định nghĩa:**

```cpp
[](glm::vec3 p)
```

**Vị trí mã nguồn:** [src/WeatherSystem.cpp:69](../src/WeatherSystem.cpp#L69).

**Tham số:** `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** Kiểu suy diễn từ return; giá trị cụ thể và ý nghĩa được nêu trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả conjunction isfinite của ba thành phần vec3. Dùng cho position/velocity/center/endpoints trong valid; không kiểm tra length hoặc bounds.

**Quan hệ — nơi gọi:** Hàm bao ngoài hoặc static initializer cùng file; riêng comparator được std::sort gọi.

**Gọi tiếp/API nổi bật:** `std::isfinite`.


<a id="file-48"></a>
## src/main.cpp

**Module/chức năng:** Parse CLI và entry point.

**Headers/phụ thuộc trực tiếp:** `Application.hpp`, `cmath`, `exception`, `iostream`, `stdexcept`, `string`, `utility`.

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [main](#f254) | `int argc`: số argv; `char** argv`: mảng chuỗi CLI. | `int`; xem ý nghĩa và điều kiện biên trong quy trình. | Duyệt argv từ 1; nhận booleans, time/fog/benchmark numeric với stod/stof, kiểm tra đã tiêu thụ toàn chuỗi, finite và range. | Runtime của executable chính. |


<a id="f254"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```cpp
int main(int argc, char** argv)
```

**Vị trí mã nguồn:** [src/main.cpp:10](../src/main.cpp#L10).

**Tham số:** `int argc`: số argv; `char** argv`: mảng chuỗi CLI.

**Giá trị trả về:** `int`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Duyệt argv từ 1; nhận booleans, time/fog/benchmark numeric với stod/stof, kiểm tra đã tiêu thụ toàn chuỗi, finite và range. Lưu paths capture. --help in hướng dẫn và return 0; option lạ/thiếu value throw. Sau parse bắt buộc capture đi cùng smoke, smoke loại trừ benchmark; move options vào Application và run(argv[0]). Catch std::exception in lỗi và return 1. Không tạo GL trước khi validation CLI xong, ngoại trừ --help kết thúc ngay khi gặp nên không xét phần argv phía sau.

**Quan hệ — nơi gọi:** Runtime của executable chính.

**Gọi tiếp/API nổi bật:** `std::stod`, `size`, `std::isfinite`, `std::invalid_argument`, `std::stof`, `empty`, `application`, `std::move`, `run`, `what`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="file-49"></a>
## tests/core_tests.cpp

**Module/chức năng:** Kiểm thử CPU.

**Headers/phụ thuộc trực tiếp:** `Camera.hpp`, `Geometry.hpp`, `algorithm`, `array`, `cmath`, `iostream`, `map`, `limits`, `stdexcept`, `string`, `utility`.

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [require](#f255) | `bool condition`: điều kiện bắt buộc đúng; `const std::string& message`: thông báo lỗi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. | main và các helper/nhóm test trong cùng translation unit. |
| [near](#f256) | `float a`: điểm/vector/scalar đầu theo signature; `float b`: điểm/vector/scalar thứ hai theo signature; `float epsilon = 0.0001f`: sai số tuyệt đối mặc định 1e−4. | `bool`; xem ý nghĩa và điều kiện biên trong quy trình. | Trả abs(a−b)<epsilon, mặc định 1e−4. | main và các helper/nhóm test trong cùng translation unit. |
| [point](#f257) | `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | `Point`; xem ý nghĩa và điều kiện biên trong quy trình. | Nhân từng thành phần position với 100000 rồi lround sang long; trả array 3 làm key topo. | main và các helper/nhóm test trong cùng translation unit. |
| [mesh](#f258) | `const std::string& name`: tên uniform hoặc tên capture/test theo hàm; `const std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 interior`: điểm trong mesh để kiểm outward normals; `bool closed = true`: bật kiểm mỗi edge hai chiều đối nhau. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Require triangle list không rỗng; duyệt mỗi triangle kiểm nondegenerate, unit normal và normal khớp winding. | main và các helper/nhóm test trong cùng translation unit. |
| [geometryTests](#f259) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm ground dimensions/Y 0 và winding, local puddle; box/roof/cone 3, 8, 16, 64/rock/sphere bằng mesh. | main và các helper/nhóm test trong cùng translation unit. |
| [cameraTests](#f260) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm view eye→origin/target→−Z; move 1 giây với 100×0.01, speed 5 và diagonal; rotate clamp±89 và normal forward; reset, floor/ceiling/xz bounds, dt 0/âm/NaN. | main và các helper/nhóm test trong cùng translation unit. |
| [main](#f261) | Không có. | `int`; xem ý nghĩa và điều kiện biên trong quy trình. | Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. | CTest hoặc chạy executable test trực tiếp. |


<a id="f255"></a>
### require

**Khai báo chính xác tại định nghĩa:**

```cpp
void require(bool condition, const std::string& message)
```

**Vị trí mã nguồn:** [tests/core_tests.cpp:15](../tests/core_tests.cpp#L15).

**Tham số:** `bool condition`: điều kiện bắt buộc đúng; `const std::string& message`: thông báo lỗi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. Dùng exception để test main hoặc callback có thể chuyển lỗi thành mã thoát 1.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::runtime_error`.


<a id="f256"></a>
### near

**Khai báo chính xác tại định nghĩa:**

```cpp
bool near(float a, float b, float epsilon = 0.0001f)
```

**Vị trí mã nguồn:** [tests/core_tests.cpp:19](../tests/core_tests.cpp#L19).

**Tham số:** `float a`: điểm/vector/scalar đầu theo signature; `float b`: điểm/vector/scalar thứ hai theo signature; `float epsilon = 0.0001f`: sai số tuyệt đối mặc định 1e−4.

**Giá trị trả về:** `bool`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trả abs(a−b)<epsilon, mặc định 1e−4. So sánh sai số tuyệt đối dùng cho float camera/normals; không so sai số tương đối.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::abs`.


<a id="f257"></a>
### point

**Khai báo chính xác tại định nghĩa:**

```cpp
Point point(glm::vec3 p)
```

**Vị trí mã nguồn:** [tests/core_tests.cpp:22](../tests/core_tests.cpp#L22).

**Tham số:** `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** `Point`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhân từng thành phần position với 100000 rồi lround sang long; trả array 3 làm key topo. Welding vị trí dù vertices lặp do flat normals; sai khác nhỏ hơn xấp xỉ 1e−5 có thể bị gộp. Không phải index buffer của renderer.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::lround`.


<a id="f258"></a>
### mesh

**Khai báo chính xác tại định nghĩa:**

```cpp
void mesh(const std::string& name, const std::vector<Vertex>& vertices,
          glm::vec3 interior, bool closed = true)
```

**Vị trí mã nguồn:** [tests/core_tests.cpp:26](../tests/core_tests.cpp#L26).

**Tham số:** `const std::string& name`: tên uniform hoặc tên capture/test theo hàm; `const std::vector<Vertex>& vertices`: triangle/effect vertex data; vector hoặc con trỏ như signature; `glm::vec3 interior`: điểm trong mesh để kiểm outward normals; `bool closed = true`: bật kiểm mỗi edge hai chiều đối nhau.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Require triangle list không rỗng; duyệt mỗi triangle kiểm nondegenerate, unit normal và normal khớp winding. Lập map cạnh đã quantize với số lần dùng và tổng hướng. Closed yêu cầu normal hướng ra so interior và mỗi cạnh đúng hai chiều đối nhau; open yêu cầu +Y. In PASS gồm triangle count; không kiểm tra tự giao mặt.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `empty`, `size`, `glm::cross`, `glm::length`, `glm::normalize`, `near`, `glm::dot`, `point`, `std::swap`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f259"></a>
### geometryTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void geometryTests()
```

**Vị trí mã nguồn:** [tests/core_tests.cpp:66](../tests/core_tests.cpp#L66).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm ground dimensions/Y 0 và winding, local puddle; box/roof/cone 3, 8, 16, 64/rock/sphere bằng mesh. Thử cone 2 phải throw invalid_argument. Có test primitive legacy dù Scene không dùng một số primitive đó.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `geometry::ground`, `mesh`, `geometry::puddle`, `require`, `std::min`, `std::max`, `geometry::box`, `geometry::roof`, `std::to_string`, `geometry::cone`, `geometry::rock`, `geometry::sphere`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f260"></a>
### cameraTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void cameraTests()
```

**Vị trí mã nguồn:** [tests/core_tests.cpp:90](../tests/core_tests.cpp#L90).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm view eye→origin/target→−Z; move 1 giây với 100×0.01, speed 5 và diagonal; rotate clamp±89 và normal forward; reset, floor/ceiling/xz bounds, dt 0/âm/NaN. Kiểm perspective và aspect giữ vertical FOV; ba preset ngoài footprint nhà/cây. Test dùng camera.move dt lớn trực tiếp để ép bounds, không đi qua clamp tick.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `position`, `require`, `view`, `glm::vec4`, `forward`, `glm::length`, `glm::vec3`, `near`, `move`, `glm::distance`, `rotate`, `pitch`, `std::abs`, `yaw`, `reset`, `projection`, `preset`, `glm::vec2`, `quiet_NaN`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f261"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```cpp
int main()
```

**Vị trí mã nguồn:** [tests/core_tests.cpp:151](../tests/core_tests.cpp#L151).

**Tham số:** Không có.

**Giá trị trả về:** `int`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. Catch std::exception in FAIL và return 1. Không tạo cửa sổ/GL context; CMake liên kết scene_core và đăng ký executable này trong CTest.

**Quan hệ — nơi gọi:** CTest hoặc chạy executable test trực tiếp.

**Gọi tiếp/API nổi bật:** `geometryTests`, `cameraTests`, `what`.


<a id="file-50"></a>
## tests/environment_tests.cpp

**Module/chức năng:** Kiểm thử CPU.

**Headers/phụ thuộc trực tiếp:** `EnvironmentGeometry.hpp`, `Surface.hpp`, `RippleSystem.hpp`, `algorithm`, `array`, `cmath`, `iostream`, `map`, `stdexcept`.

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [require](#f262) | `bool value`: mã phím hoặc điều kiện bool theo signature; `const char* message`: thông báo lỗi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. | main và các helper/nhóm test trong cùng translation unit. |
| [point](#f263) | `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | `Point`; xem ý nghĩa và điều kiện biên trong quy trình. | Nhân từng thành phần position với 100000 rồi lround sang long; trả array 3 làm key topo. | main và các helper/nhóm test trong cùng translation unit. |
| [valid](#f264) | `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `bool closed`: bật kiểm mỗi edge hai chiều đối nhau; `bool smooth=false`: cho phép normal mượt khác face normal. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Duyệt environment triangles kiểm list/nondegenerate/position finite/unit normals. | main và các helper/nhóm test trong cùng translation unit. |
| [waterTests](#f265) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm ba puddle chung một mesh 96 triangles, normal+Y và height nhỏ dương. | main và các helper/nhóm test trong cùng translation unit. |
| [rockTests](#f266) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Mỗi ba đá: valid closed, đúng 48 triangles, cùng seed tái hiện vị trí; world minY sát 0 theo scale/translation, seed khác đổi silhouette. | main và các helper/nhóm test trong cùng translation unit. |
| [cloudTests](#f267) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Mỗi bốn cloud skinvalid kín/smooth, đúng 728 triangles. | main và các helper/nhóm test trong cùng translation unit. |
| [grassTests](#f268) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm grass mesh 384 triangles, tuft placement grassAllowed, height 0.10..0.22, vertex trong ground và y 0.005..0.23; loại path/puddle/house probes. | main và các helper/nhóm test trong cùng translation unit. |
| [main](#f269) | Không có. | `int`; xem ý nghĩa và điều kiện biên trong quy trình. | Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. | CTest hoặc chạy executable test trực tiếp. |


<a id="f262"></a>
### require

**Khai báo chính xác tại định nghĩa:**

```cpp
void require(bool value, const char* message)
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:12](../tests/environment_tests.cpp#L12).

**Tham số:** `bool value`: mã phím hoặc điều kiện bool theo signature; `const char* message`: thông báo lỗi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. Dùng exception để test main hoặc callback có thể chuyển lỗi thành mã thoát 1.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::runtime_error`.


<a id="f263"></a>
### point

**Khai báo chính xác tại định nghĩa:**

```cpp
Point point(glm::vec3 p)
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:14](../tests/environment_tests.cpp#L14).

**Tham số:** `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** `Point`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhân từng thành phần position với 100000 rồi lround sang long; trả array 3 làm key topo. Welding vị trí dù vertices lặp do flat normals; sai khác nhỏ hơn xấp xỉ 1e−5 có thể bị gộp. Không phải index buffer của renderer.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::lround`.


<a id="f264"></a>
### valid

**Khai báo chính xác tại định nghĩa:**

```cpp
void valid(const std::vector<Vertex>& mesh, bool closed, bool smooth=false)
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:15](../tests/environment_tests.cpp#L15).

**Tham số:** `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `bool closed`: bật kiểm mỗi edge hai chiều đối nhau; `bool smooth=false`: cho phép normal mượt khác face normal.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Duyệt environment triangles kiểm list/nondegenerate/position finite/unit normals. Dot face-normal phải>0.999 hoặc>0.45 khi smooth. Quantize cạnh; nếu closed yêu cầu mỗi edge dùng hai lần và tổng hướng 0. Không require flat normal trong cloud smooth mesh.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `empty`, `size`, `glm::cross`, `glm::length`, `std::isfinite`, `std::abs`, `glm::dot`, `glm::normalize`, `point`, `std::swap`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f265"></a>
### waterTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void waterTests()
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:35](../tests/environment_tests.cpp#L35).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm ba puddle chung một mesh 96 triangles, normal+Y và height nhỏ dương. Các center/interior samples phải nằm trong shared polygon; emit ripple với margin, nhiều thời điểm và 24 điểm quanh vòng phải còn trong nước và phía trên mặt. Dry(9, 9) margin 0. Không kiểm collision với nhà.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `environment::puddles`, `valid`, `require`, `size`, `surface::puddles`, `surface::polygon`, `surface::waterMargin`, `glm::mix`, `glm::vec2`, `hit`, `emit`, `particles`, `glm::vec3`, `std::cos`, `std::sin`, `update`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f266"></a>
### rockTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void rockTests()
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:69](../tests/environment_tests.cpp#L69).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Mỗi ba đá: valid closed, đúng 48 triangles, cùng seed tái hiện vị trí; world minY sát 0 theo scale/translation, seed khác đổi silhouette. Không dùng raytrace ảnh hoặc kiểm mọi intersection với ground.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `environment::rocks`, `size`, `environment::rock`, `valid`, `std::min`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f267"></a>
### cloudTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void cloudTests()
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:85](../tests/environment_tests.cpp#L85).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Mỗi bốn cloud skinvalid kín/smooth, đúng 728 triangles. Tại mỗi vertex tính lại density Gaussian≈0.18 và negative-gradient cùng hướng normal; min/max height hợp lý, profiles khác. Kiểm mesh CPU, chưa chứng minh bisection đúng cho mọi input lobe tùy ý.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `previous`, `clusters`, `environment::cloud`, `valid`, `require`, `size`, `std::min`, `std::max`, `outward`, `glm::vec3`, `std::exp`, `glm::dot`, `std::abs`, `glm::normalize`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f268"></a>
### grassTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void grassTests()
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:111](../tests/environment_tests.cpp#L111).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm grass mesh 384 triangles, tuft placement grassAllowed, height 0.10..0.22, vertex trong ground và y 0.005..0.23; loại path/puddle/house probes. Không kiểm động sway vì grass hiện tĩnh.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `environment::grass`, `valid`, `require`, `size`, `environment::grassTufts`, `environment::grassAllowed`, `std::abs`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f269"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```cpp
int main()
```

**Vị trí mã nguồn:** [tests/environment_tests.cpp:126](../tests/environment_tests.cpp#L126).

**Tham số:** Không có.

**Giá trị trả về:** `int`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. Catch std::exception in FAIL và return 1. Không tạo cửa sổ/GL context; CMake liên kết scene_core và đăng ký executable này trong CTest.

**Quan hệ — nơi gọi:** CTest hoặc chạy executable test trực tiếp.

**Gọi tiếp/API nổi bật:** `waterTests`, `rockTests`, `cloudTests`, `grassTests`, `what`.


<a id="file-51"></a>
## tests/lighting_time_tests.cpp

**Module/chức năng:** Kiểm thử CPU.

**Headers/phụ thuộc trực tiếp:** `LightingSystem.hpp`, `WeatherSystem.hpp`, `limits`, `cmath`, `iostream`, `stdexcept`.

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [require](#f270) | `bool value`: mã phím hoặc điều kiện bool theo signature; `const char* message`: thông báo lỗi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. | main và các helper/nhóm test trong cùng translation unit. |
| [lightingTests](#f271) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Giữ wet/flash khi setEnvironment qua nhiều mốc giờ, hướng sáng đơn vị và màu finite; adjacent hour ambient liên tục. | main và các helper/nhóm test trong cùng translation unit. |
| [clockTests](#f272) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm default/wrap 24/âm/tiny negative/label; reject giờ Inf và duration sai. | main và các helper/nhóm test trong cùng translation unit. |
| [main](#f273) | Không có. | `int`; xem ý nghĩa và điều kiện biên trong quy trình. | Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. | CTest hoặc chạy executable test trực tiếp. |


<a id="f270"></a>
### require

**Khai báo chính xác tại định nghĩa:**

```cpp
void require(bool value,const char* message)
```

**Vị trí mã nguồn:** [tests/lighting_time_tests.cpp:9](../tests/lighting_time_tests.cpp#L9).

**Tham số:** `bool value`: mã phím hoặc điều kiện bool theo signature; `const char* message`: thông báo lỗi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. Dùng exception để test main hoặc callback có thể chuyển lỗi thành mã thoát 1.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::runtime_error`.


<a id="f271"></a>
### lightingTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void lightingTests()
```

**Vị trí mã nguồn:** [tests/lighting_time_tests.cpp:10](../tests/lighting_time_tests.cpp#L10).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Giữ wet/flash khi setEnvironment qua nhiều mốc giờ, hướng sáng đơn vị và màu finite; adjacent hour ambient liên tục. Day sáng hơn night, windows đêm bật/ngày tắt, storm làm tối sky; noon direction từ trên; flash ngày/đêm sáng rồi trở về nền.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `update`, `wetness`, `setEnvironment`, `require`, `flash`, `std::abs`, `glm::length`, `lightDirection`, `ambientColor`, `skyTop`, `std::isfinite`, `glm::all`, `glm::greaterThan`, `glm::vec3`, `glm::lessThan`, `glm::distance`, `directionalColor`, `windowStrength`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f272"></a>
### clockTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void clockTests()
```

**Vị trí mã nguồn:** [tests/lighting_time_tests.cpp:39](../tests/lighting_time_tests.cpp#L39).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm default/wrap 24/âm/tiny negative/label; reject giờ Inf và duration sai. So một bước/1000 bước auto clock, pause/resume và midnight continuity. Qua Weather kiểm đổi giờ giữ rain/pools/wetness, pause auto, reset về 18/OFF và fixed-step clock cùng kết quả cho frame rate khác.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `hour`, `autoAdvance`, `dayDuration`, `setHour`, `label`, `shiftHours`, `denorm_min`, `infinity`, `setDayDuration`, `quiet_NaN`, `setAutoAdvance`, `update`, `std::abs`, `toggleDayNight`, `setEnvironment`, `glm::distance`, `skyTop`, `lightDirection`, `drops`, `activeCount`, `wetness`, `refreshLighting`, `valid`, `reset`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f273"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```cpp
int main()
```

**Vị trí mã nguồn:** [tests/lighting_time_tests.cpp:79](../tests/lighting_time_tests.cpp#L79).

**Tham số:** Không có.

**Giá trị trả về:** `int`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. Catch std::exception in FAIL và return 1. Không tạo cửa sổ/GL context; CMake liên kết scene_core và đăng ký executable này trong CTest.

**Quan hệ — nơi gọi:** CTest hoặc chạy executable test trực tiếp.

**Gọi tiếp/API nổi bật:** `lightingTests`, `clockTests`, `what`.


<a id="file-52"></a>
## tests/model_tests.cpp

**Module/chức năng:** Kiểm thử CPU.

**Headers/phụ thuộc trực tiếp:** `ModelGeometry.hpp`, `algorithm`, `array`, `cmath`, `iostream`, `limits`, `map`, `stdexcept`, `string`.

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [require](#f274) | `bool ok`: điều kiện assertion; `const char* text`: chuỗi báo lỗi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. | main và các helper/nhóm test trong cùng translation unit. |
| [vertices](#f275) | `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Require triangle list không rỗng, không degenerate, mọi position finite; normals đơn vị và dot face>0.999. | main và các helper/nhóm test trong cùng translation unit. |
| [key](#f276) | `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể. | `Point`; xem ý nghĩa và điều kiện biên trong quy trình. | Nhân từng thành phần position với 100000 rồi lround sang long; trả array 3 làm key topo. | main và các helper/nhóm test trong cùng translation unit. |
| [closed](#f277) | `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Dựng map cạnh từ key quantize; mỗi cạnh phải xuất hiện hai lần có hướng trái nhau. | main và các helper/nhóm test trong cùng translation unit. |
| [undersideAt](#f278) | `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `float x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `float z`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới. | `float`; xem ý nghĩa và điều kiện biên trong quy trình. | Tìm tam giác mặt dưới normal.y<−0.1; chiếu xuống XZ, bỏ determinant gần 0, giải barycentric u/v. | main và các helper/nhóm test trong cùng translation unit. |
| [houseTests](#f279) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm mọi house part vertices/bounds, roof closed, dùng undersideAt để so đỉnh gable với mặt dưới mái, glass nằm lùi sau frame, tổng<1500. | main và các helper/nhóm test trong cùng translation unit. |
| [treeTests](#f280) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Layout phải đúng 4 cây; từng tree valid bark/foliage, foliage closed, cùng seed tái hiện vertices; canopy nằm trong bounds/height, mỗi cây<700 triangles. | main và các helper/nhóm test trong cùng translation unit. |
| [main](#f281) | Không có. | `int`; xem ý nghĩa và điều kiện biên trong quy trình. | Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. | CTest hoặc chạy executable test trực tiếp. |


<a id="f274"></a>
### require

**Khai báo chính xác tại định nghĩa:**

```cpp
void require(bool ok, const char* text)
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:12](../tests/model_tests.cpp#L12).

**Tham số:** `bool ok`: điều kiện assertion; `const char* text`: chuỗi báo lỗi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. Dùng exception để test main hoặc callback có thể chuyển lỗi thành mã thoát 1.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::runtime_error`.


<a id="f275"></a>
### vertices

**Khai báo chính xác tại định nghĩa:**

```cpp
void vertices(const std::vector<Vertex>& mesh)
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:13](../tests/model_tests.cpp#L13).

**Tham số:** `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Require triangle list không rỗng, không degenerate, mọi position finite; normals đơn vị và dot face>0.999. Dùng cho cả nhà/bark/foliage flat; không kiểm topo kín tại đây.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `empty`, `size`, `glm::cross`, `glm::length`, `std::isfinite`, `std::abs`, `glm::dot`, `glm::normalize`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f276"></a>
### key

**Khai báo chính xác tại định nghĩa:**

```cpp
Point key(glm::vec3 p)
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:28](../tests/model_tests.cpp#L28).

**Tham số:** `glm::vec3 p`: tham số cấu hình/điểm hoặc phân vị; xem quy trình cụ thể.

**Giá trị trả về:** `Point`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Nhân từng thành phần position với 100000 rồi lround sang long; trả array 3 làm key topo. Welding vị trí dù vertices lặp do flat normals; sai khác nhỏ hơn xấp xỉ 1e−5 có thể bị gộp. Không phải index buffer của renderer.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::lround`.


<a id="f277"></a>
### closed

**Khai báo chính xác tại định nghĩa:**

```cpp
void closed(const std::vector<Vertex>& mesh)
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:29](../tests/model_tests.cpp#L29).

**Tham số:** `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Dựng map cạnh từ key quantize; mỗi cạnh phải xuất hiện hai lần có hướng trái nhau. Dùng roof mesh và foliage để bắt khe/edge hở; các hộp nhà ghép chồng không bị buộc thành một manifold tổng thể.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `size`, `key`, `std::swap`, `require`.


<a id="f278"></a>
### undersideAt

**Khai báo chính xác tại định nghĩa:**

```cpp
float undersideAt(const std::vector<Vertex>& mesh, float x, float z)
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:40](../tests/model_tests.cpp#L40).

**Tham số:** `const std::vector<Vertex>& mesh`: mesh triangle CPU hoặc Mesh GPU theo signature; `float x`: tọa độ pixel chuột hoặc scalar smoothstep/wrap tùy signature; `float z`: giá trị theo kiểu trong khai báo; vai trò cụ thể được mô tả trong quy trình bên dưới.

**Giá trị trả về:** `float`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Tìm tam giác mặt dưới normal.y<−0.1; chiếu xuống XZ, bỏ determinant gần 0, giải barycentric u/v. Nếu point nằm trong triangle (epsilon 1e−4), nội suy y và lấy minimum. Không có triangle chứa trả infinity; dùng để so gable chạm underside roof.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `infinity`, `p`, `size`, `ab`, `ac`, `glm::vec2`, `std::abs`, `std::min`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f279"></a>
### houseTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void houseTests()
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:55](../tests/model_tests.cpp#L55).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm mọi house part vertices/bounds, roof closed, dùng undersideAt để so đỉnh gable với mặt dưới mái, glass nằm lùi sau frame, tổng<1500. In count 734 và 9 batches. Không đánh giá nội thất hoặc độ thật vật liệu.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `models::house`, `vertices`, `size`, `require`, `std::abs`, `closed`, `undersideAt`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f280"></a>
### treeTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void treeTests()
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:76](../tests/model_tests.cpp#L76).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Layout phải đúng 4 cây; từng tree valid bark/foliage, foliage closed, cùng seed tái hiện vertices; canopy nằm trong bounds/height, mỗi cây<700 triangles. Đổi seed phải đổi silhouette, branchCount 10000 phải throw. Tổng thực tế 1904 triangles.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `models::treeLayout`, `require`, `size`, `models::tree`, `vertices`, `closed`, `std::abs`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f281"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```cpp
int main()
```

**Vị trí mã nguồn:** [tests/model_tests.cpp:106](../tests/model_tests.cpp#L106).

**Tham số:** Không có.

**Giá trị trả về:** `int`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. Catch std::exception in FAIL và return 1. Không tạo cửa sổ/GL context; CMake liên kết scene_core và đăng ký executable này trong CTest.

**Quan hệ — nơi gọi:** CTest hoặc chạy executable test trực tiếp.

**Gọi tiếp/API nổi bật:** `houseTests`, `treeTests`, `what`.


<a id="file-53"></a>
## tests/weather_tests.cpp

**Module/chức năng:** Kiểm thử CPU.

**Headers/phụ thuộc trực tiếp:** `WeatherSystem.hpp`, `Surface.hpp`, `algorithm`, `cmath`, `iostream`, `limits`, `stdexcept`.

**Kiểu/dữ liệu quản lý:** Dữ liệu local của helper/test hoặc inputs/outputs GLSL như phần dưới.

### Danh mục hàm trong file

| Hàm | Tham số | Trả về | Chức năng | Được gọi ở đâu |
|---|---|---|---|---|
| [require](#f282) | `bool condition`: điều kiện bắt buộc đúng; `const char* message`: thông báo lỗi. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. | main và các helper/nhóm test trong cùng translation unit. |
| [cloudTests](#f283) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm cloud move ngang theo shared wind, toggle freeze/reset, wrap sau 10000 steps và coverage seam. | main và các helper/nhóm test trong cùng translation unit. |
| [rainTests](#f284) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm pool 1000 và address không đổi; phân bố 16 bins, length/opacity variation; gravity/wind/length sau một step. | main và các helper/nhóm test trong cùng translation unit. |
| [lightningTests](#f285) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm countdown 2..6, trigger flash 1, 80 segments, trunk/branch connectivity và zigzag/taper. | main và các helper/nhóm test trong cùng translation unit. |
| [splashTests](#f286) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Emit một impact→2 hạt đi lên; stress 2000 requests lấp 512 slot, không đổi address; update để tất cả chết, reject ngoài ground/reset. | main và các helper/nhóm test trong cùng translation unit. |
| [integrationTests](#f287) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm mặc định auto lightning bật/idle; rain impacts phát splash; manual flash; pause giữ rain/cloud/splash/flash. | main và các helper/nhóm test trong cùng translation unit. |
| [rippleTests](#f288) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm ripple radius tăng/alpha giảm; emit 200 để full 48 và rejected>0; update drain, address fixed. | main và các helper/nhóm test trong cùng translation unit. |
| [surfaceTests](#f289) | Không có. | void; kết quả thể hiện qua state/output hoặc lời gọi GL. | Kiểm wetness tăng/giảm và clamp 0..1 qua 1000 bước; mỗi puddle center margin>0.3 và ripple không vượt margin/ở trên nước. | main và các helper/nhóm test trong cùng translation unit. |
| [main](#f290) | Không có. | `int`; xem ý nghĩa và điều kiện biên trong quy trình. | Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. | CTest hoặc chạy executable test trực tiếp. |


<a id="f282"></a>
### require

**Khai báo chính xác tại định nghĩa:**

```cpp
void require(bool condition, const char* message)
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:11](../tests/weather_tests.cpp#L11).

**Tham số:** `bool condition`: điều kiện bắt buộc đúng; `const char* message`: thông báo lỗi.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm tra điều kiện assertion do project định nghĩa; điều kiện sai ném runtime_error kèm thông báo, đúng thì trở về. Dùng exception để test main hoặc callback có thể chuyển lỗi thành mã thoát 1.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `std::runtime_error`.


<a id="f283"></a>
### cloudTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void cloudTests()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:15](../tests/weather_tests.cpp#L15).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm cloud move ngang theo shared wind, toggle freeze/reset, wrap sau 10000 steps và coverage seam. Kiểm năm lobe có graph overlap liên thông/profile khác; motion 0.1 so 10×0.01. Overlap là kiểm tham số ellipsoid, chưa thay thế test skin density riêng.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `clusters`, `update`, `require`, `glm::distance`, `reset`, `std::abs`, `CloudSystem::coverage`, `glm::length`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f284"></a>
### rainTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void rainTests()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:65](../tests/weather_tests.cpp#L65).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm pool 1000 và address không đổi; phân bố 16 bins, length/opacity variation; gravity/wind/length sau một step. Chạy 2400 fixed steps, impacts trên plane và recycling>1000; bounds/terminal/streak. Kiểm no wind/reversed wind; qua Weather so 1000 drop ở 10×0.1 và 100×0.01; tắt rain đóng băng pool.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `drops`, `size`, `data`, `std::min`, `std::max`, `update`, `impactCount`, `impacts`, `std::abs`, `recycledCount`, `reset`, `glm::distance`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f285"></a>
### lightningTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void lightningTests()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:132](../tests/weather_tests.cpp#L132).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm countdown 2..6, trigger flash 1, 80 segments, trunk/branch connectivity và zigzag/taper. Update decay/expiry/new shape/cancel/random; attenuation/sky flash. Với 100 seed kiểm finite/bounds/nondegenerate, góc không đảo mạnh, branch ngắn hơn trunk, pulse gap và reuse revision. Đặt observer tại bốn cloud centers để kiểm khoảng cách tới bolt>0.75 cho các trường hợp đó. Đây là regression hữu hạn, không chứng minh mọi seed hoặc observer.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `nextStrike`, `trigger`, `active`, `flashIntensity`, `segmentCount`, `segments`, `glm::normalize`, `glm::distance`, `glm::length`, `glm::cross`, `update`, `cancel`, `strikeCount`, `setFlashPosition`, `attenuationAt`, `skyColor`, `reset`, `std::isfinite`, `std::abs`, `glm::dot`, `pulseCount`, `pulses`, `elapsed`, `std::min`, `revision`, `clusters`, `std::clamp`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f286"></a>
### splashTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void splashTests()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:256](../tests/weather_tests.cpp#L256).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Emit một impact→2 hạt đi lên; stress 2000 requests lấp 512 slot, không đổi address; update để tất cả chết, reject ngoài ground/reset. Chạy Weather 1200 fixed steps đối chiếu từng slot sống với activeCount và lifetime; peak dưới cap, có filtered. Tắt rain và kiểm pool drain; chuỗi assertion không chứng minh không có leak GL.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `particles`, `data`, `emit`, `require`, `activeCount`, `update`, `reset`, `std::isfinite`, `std::max`, `filtered`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f287"></a>
### integrationTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void integrationTests()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:298](../tests/weather_tests.cpp#L298).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm mặc định auto lightning bật/idle; rain impacts phát splash; manual flash; pause giữ rain/cloud/splash/flash. Resume và tắt rain cho splash chết, pool rain giữ; manual hoạt động khi auto off; reset defaults. dt 10 clamp giống 0.1, dt âm/NaN/Inf không tiến. So fast 600×0.01 và slow 60×0.1 về strikes/active và mọi vị trí rain/splash; wetness tăng làm groundMultiplier<1.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `require`, `active`, `nextStrike`, `update`, `activeCount`, `triggerLightning`, `drops`, `clusters`, `particles`, `flash`, `glm::distance`, `toggleLightning`, `reset`, `strikeCount`, `quiet_NaN`, `infinity`, `wetness`, `groundMultiplier`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f288"></a>
### rippleTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void rippleTests()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:362](../tests/weather_tests.cpp#L362).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm ripple radius tăng/alpha giảm; emit 200 để full 48 và rejected>0; update drain, address fixed. Weather một giây có ripple, reset xóa ripple. Không có dòng PASS riêng dù được main gọi.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `particles`, `data`, `emit`, `update`, `require`, `activeCount`, `rejected`, `reset`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f289"></a>
### surfaceTests

**Khai báo chính xác tại định nghĩa:**

```cpp
void surfaceTests()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:382](../tests/weather_tests.cpp#L382).

**Tham số:** Không có.

**Giá trị trả về:** void; kết quả thể hiện qua state/output hoặc lời gọi GL.

**Mục đích, quy trình và lưu ý kỹ thuật:** Kiểm wetness tăng/giảm và clamp 0..1 qua 1000 bước; mỗi puddle center margin>0.3 và ripple không vượt margin/ở trên nước. Dry margin 0; fog gần 1/xa giảm/density 0 tắt. Đây là CPU helper, ảnh shader được kiểm qua smoke.

**Quan hệ — nơi gọi:** main và các helper/nhóm test trong cùng translation unit.

**Gọi tiếp/API nổi bật:** `wetness`, `update`, `require`, `surface::puddles`, `surface::waterMargin`, `emit`, `particles`, `fogFactor`.

**Thuật toán/công thức và ví dụ liên quan:** đối chiếu [thuật toán](03_GRAPHICS_ALGORITHMS.md) và [bảng công thức](04_MATHEMATICAL_FORMULAS.md); các biểu thức/điều kiện cụ thể của hàm đã nêu trong quy trình trên.


<a id="f290"></a>
### main

**Khai báo chính xác tại định nghĩa:**

```cpp
int main()
```

**Vị trí mã nguồn:** [tests/weather_tests.cpp:407](../tests/weather_tests.cpp#L407).

**Tham số:** Không có.

**Giá trị trả về:** `int`; xem ý nghĩa và điều kiện biên trong quy trình.

**Mục đích, quy trình và lưu ý kỹ thuật:** Trong try gọi lần lượt các nhóm test của file; không lỗi return 0. Catch std::exception in FAIL và return 1. Không tạo cửa sổ/GL context; CMake liên kết scene_core và đăng ký executable này trong CTest.

**Quan hệ — nơi gọi:** CTest hoặc chạy executable test trực tiếp.

**Gọi tiếp/API nổi bật:** `cloudTests`, `rainTests`, `lightningTests`, `splashTests`, `integrationTests`, `rippleTests`, `surfaceTests`, `what`.
