# 07 — Kiểm thử, build và hiệu năng

[Tổng quan](01_PROJECT_OVERVIEW.md) · [Hạn chế](08_LIMITATIONS_AND_IMPROVEMENTS.md)

## Mục lục

- [Kết quả thực hiện trong lần review](#s01)
- [Quy trình build và chạy hiện có](#s02)
- [Test thực sự kiểm gì](#s03)
- [GUI smoke và capture](#s04)
- [Benchmark và ý nghĩa chỉ số](#s05)
- [Số liệu có nguồn của bản hiện tại](#s06)
- [Log dài cũ không đại diện bản mới](#s07)
- [Đề xuất quy trình đo, chưa thực hiện](#s08)

<a id="s01"></a>

## Kết quả thực hiện trong lần review

Ngày 23/11/2026 đã chạy **trực tiếp các executable CPU có sẵn** trong `build/`, không chạy lại build hoặc CTest runner:

| Executable | CTest name đăng ký | Kết quả thực chạy |
|---|---|---|
| ./build/core_tests | geometry_and_camera | exit 0, các kiểm tra geometry/camera đạt |
| ./build/weather_tests | weather_simulation | exit 0, toàn bộ 7 nhóm được gọi, kể cả ripple/surface không in PASS riêng |
| ./build/model_tests | procedural_models | exit 0, house 734 triangles/9 batches, 4 trees 1904 triangles |
| ./build/environment_tests | environment_geometry | exit 0, water 96 triangles, rock 48/mesh, cloud 728/mesh, grass 384 triangles |
| ./build/lighting_time_tests | lighting_and_time | exit 0, lighting và clock đạt |

Đây là bằng chứng hành vi của **binary hiện có**, không phải xác nhận đã biên dịch mới đúng snapshot trong phiên review. Đã đọc đối chiếu toàn bộ source test và CMake; không đồng nhất việc chạy binary với kiểm chứng reproducible build. Các lệnh trực tiếp không tạo object/binary/log/capture. Chỉ có 10 Markdown trong docs là artifact mới của project. Không chạy mới GUI/sanitizer/benchmark; các số bên dưới lấy từ log đã tồn tại.

<a id="s02"></a>

## Quy trình build và chạy hiện có

[CMakeLists.txt:1](../CMakeLists.txt#L1) định nghĩa thư viện scene_core và executable rain_thunderstorm. Cần compiler C++17, CMake≥3.16, OpenGL/FreeGLUT/GLEW/GLM đã cài; không cần download assets hoặc network trong build.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
./build/rain_thunderstorm
ctest --test-dir build --output-on-failure
```

Các lệnh trên là **hướng dẫn tái hiện**, không được thực thi trong lần review này. Configure/build sẽ sinh/chỉnh CMakeCache/Makefile/CMakeFiles, object files, libscene_core.a, executable/test và `build/shaders/{object,effects,sky}.{vert,frag}`. CTest sẽ sinh/cập nhật `build/Testing/Temporary/LastTest.log`, `CTestCostData.txt` và có thể LastTestsFailed.log khi lỗi. Shader COPYONLY nằm trong configure; build thường chạy configure lại khi shader input đổi. Không chỉnh `.cpp/.hpp/.vert/.frag` để chạy test.

Không có target sanitizer đặc biệt; sanitizer được bật qua flags cấu hình riêng như README. Gợi ý tái hiện tất cả test hiện có thay vì chỉ hai target của phần README lịch sử:

```bash
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-sanitize -j$(nproc)
ctest --test-dir build-sanitize --output-on-failure
```

ASan phát hiện một số lỗi access/lifetime CPU; UBSan phát hiện một số UB CPU. LSan có thể chạy theo ASan trên môi trường hỗ trợ; không có cấu hình LSan riêng trong CMake. README ghi LSan không chạy dưới ptrace ở sandbox của lần kiểm thử lịch sử. Không coi CPU sanitizer là kiểm tra VRAM/driver OpenGL. Log [daynight-after/sanitize.log](../build/reports/daynight-after/sanitize.log) ghi 5/5 đạt; đây là log cũ, không phải chạy mới.

<a id="s03"></a>

## Test thực sự kiểm gì

Có 5 executable/CTest và **17 hàm nhóm test** (2+7+2+4+2). Số assertion không được dùng làm số test; các helper `require/mesh/valid/closed` được giải thích riêng trong 02/10.

| File/hàm (dòng bắt đầu) | Kiểm tra chính | Giới hạn |
|---|---|---|
| [core_tests.cpp:66](../tests/core_tests.cpp#L66) geometryTests | Ground 20×20/Y 0; triangle list; normal/winding; closed edges; cone bounds; primitives kể cả sphere | Không tạo context/ảnh; primitives legacy vẫn được kiểm |
| [core_tests.cpp:90](../tests/core_tests.cpp#L90) cameraTests | lookAt, projection, aspect, FPS independence, diagonal, pitch, bounds, preset/reset/invalid dt | Không input hardware hoặc collision scene |
| [weather_tests.cpp:15](../tests/weather_tests.cpp#L15) cloudTests | Move/wind/toggle/reset/wrap/coverage, lobe overlap graph, deltaTime | Không kiểm visibility pixel hoặc mọi lobe input tùy ý |
| [weather_tests.cpp:65](../tests/weather_tests.cpp#L65) rainTests |1000 slot, 16 region distribution, gravity/wind/recycling/impacts/terminal, length/alpha variation, fixed storage/fixed step | Không roof/tree collision hoặc ảnh streak |
| [weather_tests.cpp:132](../tests/weather_tests.cpp#L132) lightningTests |32+48 segments, connectivity/taper, 100 seed, bounds/kink, pulse gap/revision, auto/manual/observer/lighting helpers |100 seed không chứng minh mọi RNG/camera |
| [weather_tests.cpp:256](../tests/weather_tests.cpp#L256) splashTests | Emit 2, full 512, life/upward/expire, active recount, filter/drain, stable pool | Không GL upload/render leak |
| [weather_tests.cpp:298](../tests/weather_tests.cpp#L298) integrationTests | Pause/reset/toggles, clamp/NaN/Inf, compare all rain/splash across step grouping, wetness | Không thread scheduling hoặc driver |
| [weather_tests.cpp:362](../tests/weather_tests.cpp#L362) rippleTests | Radius↑, alpha↓, cap 48/rejected, expiry/storage, real impacts/reset | Không shading ring trên framebuffer |
| [weather_tests.cpp:382](../tests/weather_tests.cpp#L382) surfaceTests | Wetness clamp, watermargin/ring height/radius, fog monotonic/density 0 | CPU helper fog không được GPU gọi trực tiếp |
| [model_tests.cpp:55](../tests/model_tests.cpp#L55) houseTests | Nondegenerate, bounds, closed roof, gable-underroof alignment, recessed glass, budget | Không CSG/no-interpenetration tổng nhà hoặc interior |
| [model_tests.cpp:76](../tests/model_tests.cpp#L76) treeTests |4 cây, closed canopy, unit normal, seed reproducibility, bounds/budget, reject large params | Không kiểm sway shader |
| [environment_tests.cpp:35](../tests/environment_tests.cpp#L35) waterTests | Mesh 96 triangles, height/+Y, shared polygon, ring containment 24 directions qua nhiều tuổi | Không fluid/reflection physics |
| [environment_tests.cpp:69](../tests/environment_tests.cpp#L69) rockTests |3 seed mesh 48 triangles kín, determinism, grounded minY | Không ảnh lighting |
| [environment_tests.cpp:85](../tests/environment_tests.cpp#L85) cloudTests |728 triangles/cluster, closed skin, iso.18, gradient normal, height/silhouette | Không general star-shaped proof |
| [environment_tests.cpp:111](../tests/environment_tests.cpp#L111) grassTests |64 tuft/384 triangles, height/bounds/exclusions | Không coverage toàn footprint mỗi blade |
| [lighting_time_tests.cpp:10](../tests/lighting_time_tests.cpp#L10) lightingTests | Day/night brightness, direction unit, transition/ambient continuity, window/storm, flash/wetness preservation | Không physical radiance/gamma calibration |
| [lighting_time_tests.cpp:39](../tests/lighting_time_tests.cpp#L39) clockTests | Wrap/tiny negative/invalid duration, label, auto FPS independence, pause/reset, midnight, weather preservation | Không timezone/system clock vì không được dùng |

Log lịch sử [daynight-after/ctest.log](../build/reports/daynight-after/ctest.log) ghi 5/5 test đạt, tổng 0.89s. Phiên review chỉ báo 5/5 binary trực tiếp exit 0, không dùng log đó giả làm kết quả CTest mới.

<a id="s04"></a>

## GUI smoke và capture

```bash
./build/rain_thunderstorm --smoke-test \
  --capture /tmp/storm-review.ppm \
  --capture-lightning /tmp/storm-review-flash.ppm \
  --capture-cloud-wrap /tmp/storm-review-wrap.ppm \
  --capture-views /tmp/storm-review-views
```

Lệnh này cần DISPLAY/driver/context và sẽ tạo các file PPM/thư mục chỉ định. Chưa chạy trong review. Biến override sau là quy trình hiện có để kiểm tương thích driver Mesa, không thay yêu cầu source:

```bash
MESA_GL_VERSION_OVERRIDE=3.3 MESA_GLSL_VERSION_OVERRIDE=330 \
  ./build/rain_thunderstorm --smoke-test
```

[Application::smokeStep:394](../src/Application.cpp#L394) warmup weather 0.5 giây, kiểm framebuffer/GL state, isolate clouds/rain bằng baselineLighting, splash/ripple pixels, input handlers/reset/pause/manual bolt/cache/cameras, wet/dry/time/daynight/flash, resize rồi cleanup. Optional cloud-wrap chạy 23 giây riêng, kiểm transparent pixels không depth-write và deterministic rerender. Nhánh capture-views còn render/capture góc cây và nhiều pha. Các phép thử gọi handler nội bộ, không phát sự kiện bàn phím/chuột thật từ OS.

Log có sẵn [daynight-after/smoke-gl33.log](../build/reports/daynight-after/smoke-gl33.log) ghi OpenGL 3.3 Core/GLSL 3.30/depth 24 và smoke passed. Số pixel từ log: cloud 20247, rain 4074, splash 241, ripple 225, bolt 961; brightened cloud/scene 20404/124961; tree sway 110. Probe median RGB soil khô/ướt 45/38, nước trước/chớp 39/77. Đây là byte pixel đo được, không phải luminance vật lý hoặc ảnh đã được xem trực tiếp trong phiên review.

<a id="s05"></a>

## Benchmark và ý nghĩa chỉ số

Nguồn [Benchmark.cpp:64](../src/Benchmark.cpp#L64), [report:114](../src/Benchmark.cpp#L114), [Application.cpp:170](../src/Application.cpp#L170).

```bash
./build/rain_thunderstorm --benchmark 15
./build/rain_thunderstorm --benchmark-unlocked 15
./build/rain_thunderstorm --benchmark 15 --no-fog
./build/rain_thunderstorm --benchmark 600
```

Không có tùy chọn export CSV/JSON/VRAM; report in stdout. Nếu chuyển hướng stdout ra log, chính lệnh shell sẽ tạo log. Thời gian finite trong(0, 600], không cùng smoke. Giữ nguyên resolution, camera, hour/auto, flags, seed, build type, driver và mức hiển thị window khi so sánh; ghi cả cache warmup và instrumentation.

| Chỉ số | Công thức/cơ chế chính xác | Cách hiểu và giới hạn |
|---|---|---|
| Average FPS | totalFrames/seconds từ started_ tới tick báo cáo | Wall-time gồm timer/simulation/submit/swap; context creation không tính |
| avg_frame_ms |1000 seconds/max(frames, 1)|Nghịch đảo FPS trung bình; không phải average array interval trong code |
| Frame interval | beginFrame hiện tại−beginFrame trước, ms | Có thời gian xen giữa render như swap/timer/update; không đo riêng rasterization |
| P95/P99 | Sort N stored samples; index floor((N−1)p) | Không nội suy; ring tối đa 65536 interval, đo dài chỉ retained window |
| CPU submit wall | Clock::now−frame Start, bình quân render Count | Có query polling/draw/checkGL sau timestamp; không gồm update/timer/swap; khác CPU usage |
| GPU command ms | GL_TIME_ELAPSED ns/1e6, average completed samples |4 query reused, poll AVAILABLE; không glFinish, không chờ query pending ở report; không GPU utilization |
| CPU percent one core |100Δ(user+system CPU seconds)/wall seconds |100%=1 core; khác toàn bộ máy, không chia số core |
| Draw call | Tăng khi Renderer gọi Mesh/Batch draw | Counter trước clipping/depth, không draw-call driver internal |
| Triangle count | Sum static/cloud/bolt vertexCount/3 và sky 1 | Không tính lines và không phải triangles thấy được |
| Lines | rain/splash vertexCount/2+ripple edge count | One line streak không phải 2 triangles |
| RSS | Resident pages `/proc/self/statm`×pageSize | RAM tiến trình, không VRAM; includes driver mappings nếu resident |
| Peak RSS | VmHWM `/proc/self/status` | Process high-water mark, không chỉ particle pool |
| Pool peak/full | Lấy sample khoảng 1 giây | Có thể bỏ qua peak/saturation ngắn giữa hai mẫu |
| Timer cap | glutTimerFunc 16 ms | Khoảng danh nghĩa 62.5Hz trước overhead, không bằng VSync 60 Hz một cách bắt buộc |
| VSync/unlocked | Yêu cầu swap interval 0 qua GLX, đổi timer sang idle | Compositor/presentation có thể còn cap, unlocked không đồng nghĩa GPU saturation |
| GPU memory | Không có phép đo | Không suy VRAM từ RSS hoặc số query |

Ví dụ 931 frames/15.0022s≈62.0576 FPS và 1000/FPS≈16.1141 ms. P99 cao có thể xảy ra dù mean GPU thấp vì latency ở timer/swap/dispatch, không tự quy kết bottleneck.

<a id="s06"></a>

## Số liệu có nguồn của bản hiện tại

Hai log cùng source giai đoạn daynight-after nhưng **khác chế độ scheduling**; dùng để mô tả hai mode, không coi là một phép so tối ưu code. README cuối ghi Debug/GCC 15.2, 800×600, camera 4, hour 18/auto OFF, fog ON.018, rain 1000/cloud 4/lightning auto ON, seed mặc định, 15 giây. Log tự ghi Mesa 26.0.8, zink/Vulkan, NVIDIA RTX 3050 Laptop NVK, OpenGL 4.6 Core/depth 24. Không tái xác minh driver/phần cứng hiện tại.

| Metric | [Timer 16 ms: capped-final.log](../build/reports/daynight-after/capped-final.log) | [Unlocked: unlocked.log](../build/reports/daynight-after/unlocked.log) |
|---|---:|---:|
| Frames / seconds |931/15.0022|2154/15.001|
| FPS |62.0576|143.590|
| avg frame ms |16.1141|6.96425|
| P95 / P99 ms |16.8539/17.0082|7.45584/7.99327|
| CPU submit ms/frame |0.499433|0.291673|
| GPU command ms/frame |0.512323|0.392293|
| GPU samples |930|2152|
| RSS start / last / peak MiB |89.5273/94.7578/94.7656|89.6445/95.2500/95.2773|
| Max calls / triangles / lines |32/6497/1935|32/6497/1935|
| Splash rejected / ripple rejected |0/65|0/65|
| Strikes |3|3|

Không suy throughput tối đa từ 143.590 FPS hoặc 0.392293 GPU ms; không gọi GPU-bound/CPU-bound chỉ từ hai số. Không lấy bảng phần đầu README 53 calls/2675 triangles hoặc cloud 20 draw làm hiện tại.

Cận trên pipeline hiện tại từ source và test counts: Scene 3264 + sky 1 + clouds 4×728 + lightning 80×4=**6497 triangles**, calls 23+1+4+4 effect=**32** khi mọi effect có mặt. Dùng hết capacity 128 segments sẽ là 6689 triangles, nhưng trigger hiện chỉ tạo 80. Mưa/splash/ripple tối đa 1000+512+48×12=2088 lines về capacity; **1935** là cực đại đo được trong log, không cận trên capacity. Số cặp triangle/calls tối đa có thể đo ở các frame khác nhau.

<a id="s07"></a>

## Log dài cũ không đại diện bản mới

[build/reports/soak-600.log](../build/reports/soak-600.log) ghi 10331 frames/600.188s=17.213 FPS, P99=1002.5 ms và 53 calls/2675 triangles. Điều đó thuộc pipeline cũ, khác bản daynight có 32 calls/6497 triangles. TRACE cuối có nhiều đoạn≈1 FPS; nguyên nhân presentation/dispatch chưa được xác minh đầy đủ. Không dùng log này chứng minh bản hiện tại chạy 600 giây ổn định về throughput hoặc so với cửa sổ foreground 15 giây. dt clamp làm thời gian mô phỏng tiến ít hơn wall-time khi stall; 43 strikes trong log dài không thể trực tiếp so tần suất wall-time lý thuyết.

<a id="s08"></a>

## Đề xuất quy trình đo, chưa thực hiện

Chạy clean/reproducible build trên môi trường được ghi rõ; kiểm CPU trước, GUI smoke sau; benchmark fixed hour/auto OFF và auto ON tách riêng, window giữ visible; warm cache và lặp đủ lượt. So cùng mode/cấu hình, đưa variance và tail latency thay vì chỉ FPS. Nếu có bottleneck, profile simulation/submit/swap/query riêng; đo VRAM bằng công cụ driver phù hợp được người dùng cài sẵn. Đây là đề xuất kiểm chứng, không phải tính năng hoặc kết quả đã có trong project.
