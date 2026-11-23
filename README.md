# 3D Rain and Thunderstorm Simulation

Project Đồ họa máy tính trên Ubuntu: **C++17, OpenGL 3.3 Core, GLSL 330,
FreeGLUT, GLEW, GLM và CMake**. Đã triển khai giai đoạn 1–5 và cải tiến hạng mục
1–9 trên cùng kiến trúc. Báo cáo mới hạng mục 5–9 nằm ở cuối tài liệu;
phần 1–4 được giữ làm lịch sử. **Chưa triển khai âm thanh (hạng mục 10).**

Cảnh gốc giữ nguyên: mặt đất 20 × 20 tại Y=0, một nhà, 4 cây, 3 đá và camera tự
do. Có thêm trời tối, 4 cụm mây chuyển động, đúng **1.000 hạt mưa**, sét phân
nhánh, ánh chớp, splash giới hạn **512 hạt**, ripple **48 slot**, 3 vũng nước,
specular nhẹ, sky gradient và distance fog. Không thêm dependency, âm thanh,
ImGui, texture lớn, HDR/bloom, SSR/planar reflection hoặc hiệu ứng thể tích.

## Cài đặt trên Ubuntu

Chỉ cài nếu máy thiếu các gói tương ứng:

```bash
sudo apt update
sudo apt install build-essential cmake libgl1-mesa-dev freeglut3-dev libglew-dev libglm-dev
```

Tùy chọn để xem khả năng driver:

```bash
sudo apt install mesa-utils
glxinfo -B
```

Cần phiên đồ họa có DISPLAY hoạt động và driver hỗ trợ OpenGL 3.3 Core trở lên.
Chương trình kiểm tra version, Core Profile và depth buffer khi khởi động.

## Build và chạy

Từ thư mục project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/rain_thunderstorm
```

Cửa sổ ban đầu 800 × 600. CMake chép shader vào `build/shaders` và đưa đường dẫn
tuyệt đối vào executable, vì vậy có thể chạy từ thư mục khác:

```bash
cd /tmp
/home/tuanluc/Projects/3D-Rain-Thunderstorm/build/rain_thunderstorm
```

Nếu di chuyển thư mục build, cấu hình/build lại để cập nhật đường dẫn shader.
Sửa GLSL trong `shaders/` rồi build lại để CMake chép bản mới. Build không tải dữ
liệu từ Internet. Có thể mở thư mục này bằng VS Code và dùng terminal tích hợp
để chạy các lệnh trên; không cần cấu hình linker dành cho Windows.

## Điều khiển

| Phím/thao tác | Chức năng |
|---|---|
| W / S | Tiến / lùi theo hướng nhìn |
| A / D | Sang trái / phải |
| Q / E | Hạ / nâng theo trục Y |
| Giữ chuột trái và kéo | Xoay yaw/pitch; có thể kéo nhiều lần khi chạm mép cửa sổ |
| 1 | Bật/tắt mưa; giữ nguyên pool 1.000 hạt, bật lại sẽ tiếp tục |
| 2 | Bật/tắt sét tự động; khi tắt sẽ hủy đợt chớp hiện tại |
| 3 | Bật/tắt hiển thị và chuyển động mây |
| L | Kích hoạt ngay một đợt sét, kể cả khi sét tự động đang tắt |
| Space | Dừng/tiếp tục thời gian mô phỏng; camera vẫn điều khiển được |
| 4 / 5 / 6 | Camera toàn cảnh / gần nhà / thấp gần mặt đất; chỉ đổi camera |
| F | Bật/tắt distance fog |
| R | Reset camera mặc định, toàn bộ weather/ripple/wetness, fog và các công tắc |
| Esc / nút đóng cửa sổ | Giải phóng tài nguyên OpenGL và thoát |

Khi tạm dừng, các lệnh bật/tắt và L vẫn áp dụng tức thì; sét kích hoạt lúc này
đóng băng ở trạng thái sáng cho đến khi tiếp tục. Khi tắt mưa, splash đã có vẫn
hết vòng đời bình thường. R khôi phục mọi hiệu ứng về bật và bỏ tạm dừng.
Khi ẩn cửa sổ, chế độ bình thường bỏ update/render; benchmark vẫn tiếp tục; khi con trỏ rời cửa sổ, xóa trạng thái
phím để tránh kẹt di chuyển.

Tiêu đề cập nhật mỗi giây, ví dụ `Storm | 60 FPS | R:1000 S:300 P:40 | L:ON C:ON F:ON`.
P là số ripple active, F là trạng thái fog.
R là số hạt mưa active khi bật, S là số slot splash còn sống, không phải capacity.
L:ON = tự động bật/đang chờ; L:OFF = tự động tắt; L:FLASH = đợt đang active, kể cả
sét thủ công khi tự động tắt. Trong khoảng tối giữa các xung, đợt vẫn active.
Thêm `PAUSE` khi tạm dừng. Pool splash cố định 512 slot; `S:512` là trạng thái
đầy hợp lệ trong stress test. Khi chạy mưa bình thường, chính sách phát hạt mới
giúp tránh bão hòa liên tục; rejected/filtered được ghi trong benchmark.

## Kiến trúc và các file

Logic mô phỏng không phụ thuộc OpenGL, nằm trong thư viện `scene_core` để có thể
kiểm thử CPU. Renderer giữ tài nguyên GPU. Không viết lại hệ render giai đoạn 1.

| File | Thay đổi / nhiệm vụ |
|---|---|
| `include/CloudSystem.hpp`, `src/CloudSystem.cpp` | Mới: 4 cụm mây, mỗi cụm 5 ellipsoid, dịch chuyển và wrap |
| `include/RainSystem.hpp`, `src/RainSystem.cpp` | Mới: pool 1.000 giọt, gravity/wind, tái sinh, danh sách impact |
| `include/LightningSystem.hpp`, `src/LightningSystem.cpp` | Mới: midpoint displacement, nhánh, lịch sét, vòng đời và cường độ chớp |
| `include/SplashSystem.hpp`, `src/SplashSystem.cpp` | Mới: pool 512 hạt bắn từ impact, vòng đời và tái sử dụng |
| `include/LightingSystem.hpp`, `src/LightingSystem.cpp` | Ánh chớp, wetness, sky/fog color và fog factor |
| `include/RippleSystem.hpp`, `src/RippleSystem.cpp` | Pool 48 vòng gợn lan rộng/fade, thống kê từ chối |
| `include/Surface.hpp`, `src/Surface.cpp` | 3 polygon vũng nước dùng chung cho mesh và va chạm ripple |
| `include/Benchmark.hpp`, `src/Benchmark.cpp` | Đo frame time, GPU query, RAM/CPU; chỉ tạo trong benchmark |
| `shaders/sky.vert`, `shaders/sky.frag` | Fullscreen triangle gradient, không ghi depth |
| `include/WeatherSystem.hpp`, `src/WeatherSystem.cpp` | Mới: gió chung, fixed timestep, thứ tự update, pause/reset/toggles |
| `include/DynamicBatch.hpp`, `src/DynamicBatch.cpp` | Mới: VAO/VBO động có dung lượng cố định, batch draw cho hiệu ứng |
| `shaders/effects.vert`, `shaders/effects.frag` | Mới: GLSL 330 cho màu/alpha của mưa, splash, sét |
| `tests/weather_tests.cpp` | Mới: cloud/rain/lightning/splash/integration tests thuần CPU |
| `include/Application.hpp`, `src/Application.cpp` | Mở rộng: cập nhật weather, input, FPS/title, benchmark, GUI/pixel smoke test |
| `include/Renderer.hpp`, `src/Renderer.cpp` | Mở rộng: mây, batch hiệu ứng, ánh chớp, màu đất ướt và khôi phục GL state |
| `include/Geometry.hpp`, `src/Geometry.cpp` | Mở rộng: sphere low-poly với winding và normal phẳng |
| `include/Scene.hpp`, `src/Scene.cpp` | Đánh dấu mặt đất để áp dụng màu ướt; giữ nguyên vị trí/mesh vật thể gốc |
| `shaders/object.frag` | Thêm flash light và coverage cho mây khi wrap |
| `src/main.cpp` | Tham số capture, capture-lightning, benchmark và kiểm tra đầu vào |
| `CMakeLists.txt` | Build module mới, chép shader và đăng ký weather test |
| `tests/core_tests.cpp` | Bổ sung kiểm tra topology/normal/winding sphere |
| `README.md` | Tài liệu, thuật toán, kết quả kiểm thử |

`Mesh`, `Shader` và quy ước tọa độ giai đoạn 1 được giữ nguyên. Camera bổ sung
preset nhưng giữ cách di chuyển/xoay/reset cũ; object shader có world position,
wet material/specular và fog. `.gitignore` tiếp tục bỏ qua `build*/` và ảnh PPM.

Luồng mỗi timer tick:

1. Trạng thái input → camera với deltaTime thực tế.
2. Weather nhận deltaTime hợp lệ, giới hạn 0,1 giây, cộng vào accumulator.
3. Mỗi bước 1/120 giây: mây → mưa → sét → splash/ripple → ánh sáng/wetness.
4. Render sky → vật thể đặc/vũng nước → mây → mưa → splash → ripple → sét.

Khi pause, accumulator và toàn bộ weather không tiến thời gian. DeltaTime âm,
NaN hoặc infinity bị bỏ qua; sau stall không cố chạy bù nhiều giây gây giật.

## Thuật toán mô phỏng

### Mây và gió

Dùng một sphere 10 lát × 6 tầng, 100 tam giác, tái sử dụng qua model matrix.
Mỗi cụm gồm 5 ellipsoid chồng lên nhau thành một khối liên thông: 3 khối đáy
rộng/dẹt, 2 khối đỉnh bất đối xứng. Tỷ lệ X/Y/Z và độ lệch đỉnh khác nhau giữa
các cụm. Shader tô sắc độ theo chiều cao thế giới, đáy tối hơn phần trên.

```text
position += windVelocity * cloudSpeed * deltaTime
windVelocity = (0.65, 0, 0.20)
cloudSpeed = 0.20 .. 0.275
```

Tâm mây wrap theo X/Z trong [-10,10). Trong dải 2,5 đơn vị gần biên, coverage
smoothstep giảm về 0 ở cả hai phía của điểm wrap. Mây opacity=1 vẽ như vật thể
đặc; các lobe đang fade được sort từ xa tới gần và alpha blend, depth test/culling
vẫn bật, depth write chỉ tắt trong pass fade. Không dùng discard ngẫu nhiên.
Cách cũ dùng screen-door discard đã được xác minh là nguyên nhân mây lấm tấm.
Mặt đất và phạm vi cảnh gốc giữ nguyên 20 × 20.

### Rain Particle System

Pool `std::array<Raindrop, 1000>` được khởi tạo một lần. Mỗi giọt có position,
velocity, length và lifetime (tuổi hiện tại). Ban đầu giọt trải ngẫu nhiên từ
Y=0,05 đến 8,4, phủ X/Z gần [-10,10], nên thấy mưa ngay khi chạy.

```text
velocity += (0, -9.81, 0) * dt
velocity.y = max(velocity.y, -17)
velocity.xz += (windVelocity.xz - velocity.xz) * min(2 * dt, 1)
position += velocity * dt
lifetime += dt
```

Đây là semi-implicit Euler: cập nhật vận tốc rồi cập nhật vị trí bằng vận tốc mới.
Weather chạy bước cố định 1/120 giây nên thay đổi FPS không thay đổi quỹ đạo.
Khi cắt Y=0, nội suy vị trí impact giữa điểm trước/sau. Khi chạm đất hoặc ra ngoài
X/Z, tái sinh chính slot đó ở Y=7,6..8,4 với vận tốc Y=-9..-6.

Chiều dài vệt là `clamp(speed * 0.024 * streakScale, 0.12, 0.42)`, với
`streakScale` ngẫu nhiên 0,85..1,15 cố định trong một vòng đời. Vệt hướng ngược
vận tốc. Alpha riêng 0,55..0,82 và nhân fade khoảng cách từ 1 tới 0,42 trong
khoảng 8..34 đơn vị, tính trong vertex shader để cập nhật ngay khi camera di chuyển.
Perspective làm vệt xa ngắn hơn trên màn hình. Toàn bộ 1.000 vệt
được upload vào một VBO đã cấp dung lượng và vẽ bằng **một GL_LINES draw call**,
độ rộng 1 pixel. Không tạo/xóa hạt hoặc OpenGL buffer mỗi frame.

### Sét và Midpoint Displacement

Chọn nguồn trong một cụm mây và điểm đích gần mặt đất. Với mỗi đoạn AB:

```text
M = (A + B) * 0.5
M' = M + u * random(-d, d) + v * random(-d, d)
```

`u`, `v` tạo cơ sở vuông góc với hướng AB. Chia đệ quy A→M' và M'→B, giảm độ lệch
còn 0,40 mỗi cấp. Độ lệch vuông góc bị giới hạn 12% độ dài đoạn cha; khi gần
biên, scale cả vector offset để giữ tính vuông góc. Geometry nằm trong X/Z ±9,8,
Y từ 0,03 tới 10. Đường chính sâu 5 → 32 đoạn; 6 nhánh sâu 3 → 48 đoạn. Gốc mỗi
nhánh là đúng một đỉnh trên đường chính. Tổng 80 đoạn, ngân sách cứng 128 đoạn.
Hình dạng thay đổi theo RNG mỗi lần; R reset seed để dễ tái hiện kiểm thử.

Renderer tạo quad hướng về camera cho từng đoạn, hai lớp core trắng và halo xanh
alpha thấp, mép mềm, trong một batch GL_TRIANGLES. Đường chính rộng 0,095 giảm
dần tới 0,06175; nhánh từ 0,032 tới 0,0128. Không dùng line width lớn hoặc bloom.
Chỉ upload geometry khi có đợt mới hoặc camera thay đổi; cường độ từng xung dùng
uniform. Nguồn ưu tiên mây đang hiện rõ và cách camera trên XZ hơn 4,5 đơn vị;
geometry gần camera còn fade trong khoảng 0,5..1,5 đơn vị để tránh che toàn màn hình.

Sét tự động nghỉ ngẫu nhiên 2–6 giây giữa các đợt, mỗi đợt dài 0,28–0,42 giây,
có 1–3 xung. Mỗi xung chỉ chiếm 55–70% interval, tạo khoảng ngắt tối thực sự.
Xung đầu peak=1, các xung sau peak=0,65..0,92. Trong một xung:

```text
flash = peak * exp(-32 * timeSincePulse)
flash *= clamp((pulseDuration - timeSincePulse) / 0.012, 0, 1)
attenuation = 1 / (1 + 0.025*d + 0.018*d*d)
light = flash * attenuation * (0.45 + 0.65 * max(dot(normal, toLight), 0))
```

`toLight` được chuẩn hóa từ fragment đến nguồn ở đầu tia sét. LightingSystem
truyền chung vị trí và cường độ cho shader của nhà, cây, đá, đất và mây. Vì vậy
mây gần sáng hơn mây xa. Màu trời sáng lên cùng nhịp. Trong gap hoặc hết đợt,
flash bằng 0 và ánh sáng trở về nền; không thay đổi vĩnh viễn scene.

### Splash và mặt đất ướt

Mỗi impact có xác suất 45% phát 2 hạt vào pool 512 slot. Chỉ tái dùng slot đã
hết đời; thiếu 2 slot trống thì từ chối yêu cầu, không ghi đè hạt đang sống.
Hạt hướng lên/radial, chịu gravity, sống 0,16–0,28 giây, độ dài 0,05–0,10.
Splash dùng một batch GL_LINES, alpha giảm theo lifetime. Benchmark phân biệt
requests, filtered do sampling và rejected do thiếu slot. Tắt mưa ngừng phát mới.

Ripple dùng một pool riêng 48 slot và một batch GL_LINES gồm vòng 12 cạnh.
Xác suất phát là 5,5% trên đất ướt, 35% trong vũng nước. Lifetime 0,55 giây;
`radius=mix(0.04,maxRadius,t)`, `alpha=0.5*(1-t)^2`. Max radius 0,45, hoặc
90% khoảng cách gần nhất đến mép polygon vũng nước để vòng không vượt bờ.
Mặt vòng Y=0,028, cao hơn mặt vũng Y=0,012, tránh z-fighting.

Độ ướt bắt đầu 0,65, tăng 0,06/giây khi mưa, giảm 0,012/giây khi tắt mưa và
clamp [0,1]. Fragment shader nội suy màu khô/ướt, tương đương nhân
`(1 - 0.24 * wetness)`. Blinn–Phong với shininess 24..64 chỉ áp dụng đất/nước;
strength đất tăng theo wetness, có thành phần flash specular bị cap 0,45.
Có 3 vũng polygon 16 tam giác, cùng một mesh được scale/rotate. Không phản chiếu
hình ảnh thực hoặc thay đổi mesh mặt đất.

### Rendering và giới hạn tài nguyên

- Scene gốc: 25 draw call, 306 tam giác. Vũng nước thêm 3 call/48 tam giác;
  mây 20 call/2.000 tam giác; sky 1 call/1 tam giác.
- Mưa/splash/ripple/sét: tối đa thêm 4 call. Sét 320 tam giác. Tổng tối đa
  **53 draw call, 2.675 tam giác**, cộng GL_LINES được đếm riêng.
- Mưa 1.000 slot; splash 512; ripple 48; sét tối đa 128 segment; mây 4 cụm.
- Dữ liệu particle và staging vertex dùng array cố định; GPU buffer cấp một lần.
  Buffer mưa/splash dùng revision; buffer sét dùng revision geometry + camera.
  Không upload lại geometry sét khi chỉ cường độ xung thay đổi.
- Sky tắt depth test/write riêng rồi khôi phục; các pass cảnh/hiệu ứng giữ depth
  test. Hiệu ứng dùng alpha blending, depth write tắt
  tạm thời; sau pass khôi phục depth write, blend và culling.
- Không sort từng hạt trong batch; blending trong suốt chỉ mang tính gần đúng.
- VAO/VBO/shader dùng RAII và được hủy trước khi context đóng, kể cả khi lỗi.
- Timer 16 ms tránh idle loop quay liên tục; đây là giới hạn chủ động khoảng
  60 FPS, không phải benchmark FPS tối đa của GPU.

## Kiểm thử và kết quả

Số đo phần này của hạng mục 1–4 được giữ để đối chiếu lịch sử. Kết quả mới 5–9
ở báo cáo cuối tài liệu.

Đã thực hiện đúng thứ tự **2 → build/test → 3 → build/test → 4 → build/test →
5 → build/test**, với CTest và cửa sổ OpenGL thật ở từng giai đoạn.

### CPU

```bash
ctest --test-dir build --output-on-failure
```

Hai CTest hiện tại đều đạt:

- `geometry_and_camera`: topology kín, winding CCW, normal đơn vị, tam giác không
  suy biến, mặt đất đúng 20 × 20, sphere, view/projection, tốc độ, pitch và bounds.
- `weather_simulation`: cloud movement/wrap/wind/toggle; đúng 1.000 hạt và pool cố
  định; gravity/terminal velocity/recycling/impact; midpoint zigzag và kết nối
  trunk/branch; lịch/lifecycle/flash; splash cap/lifetime; pause/reset; dt bất thường;
  so sánh toàn bộ rain/splash khi render ở 10 FPS và 100 FPS cùng thời gian mô phỏng.
  Bổ sung kiểm tra phân bố mưa 16 vùng, không gió/đảo gió, chiều dài theo vận tốc,
  lobe mây liên thông, active splash bằng số slot sống và thoát khỏi trạng thái
  đầy khi tắt mưa; 100 seed sét kiểm tra bounds/góc/nhánh/pulse gap và taper.

Kiểm tra bộ nhớ/undefined behavior của CPU cũng đạt:

```bash
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-sanitize --target core_tests weather_tests -j$(nproc)
ctest --test-dir build-sanitize --output-on-failure
```

AddressSanitizer/UndefinedBehaviorSanitizer/LeakSanitizer không báo lỗi trong hai
CPU test. LeakSanitizer không hoạt động dưới ptrace của sandbox nên lượt xác minh
này chạy ngoài sandbox. Kết quả này không phải phép đo rò rỉ bên trong driver GPU.

### GUI smoke test và ảnh framebuffer

```bash
./build/rain_thunderstorm --smoke-test \
  --capture /tmp/storm.ppm \
  --capture-lightning /tmp/storm-flash.ppm \
  --capture-cloud-wrap /tmp/cloud-wrap.ppm
```

Smoke test warmup đúng 0,5 giây mô phỏng rồi chụp cảnh, dùng handler L kích hoạt
sét có kiểm soát để chụp flash. Nó chạy ít nhất 12 frame qua main loop, kiểm tra
resize 800 × 600 → 640 × 400, camera, toggle 1/2/3, L khi sét tự động tắt,
pause với camera vẫn di chuyển, reset, hết vòng đời flash và cleanup.

Ngoài GL error, smoke test so sánh framebuffer cùng một trạng thái mô phỏng khi
ẩn từng hiệu ứng để xác nhận có pixel thực sự được render. Kết quả lượt cuối:

| Kiểm tra | Pixel thay đổi |
|---|---:|
| Mây | 18.112 |
| Mưa | 5.494 |
| Splash | 340 |
| Hình học sét, giữ nguyên flash light | 1.022 |
| Geometry mây sáng hơn khi chớp | 18.095 |
| Geometry cảnh phía dưới sáng hơn khi chớp | 118.631 |

Build Debug và GUI smoke đều đạt, mã thoát 0, không ghi nhận GL error lúc khởi
tạo/render/cleanup. Đã xem ảnh thường và ảnh flash, thấy cảnh gốc cùng các hiệu
ứng. Đã chạy từ `/tmp` để kiểm tra đường dẫn shader.

Môi trường đo: GCC/G++ 15.2, CMake 4.2.3, FreeGLUT 3.4, GLEW 2.2, GLM 1.0.1;
Mesa/zink trên NVIDIA RTX 3050 Laptop, depth 24 bit. Driver cung cấp OpenGL 4.6
Core. Cũng đã chạy smoke thành công với OpenGL 3.3 Core / GLSL 3.30 bằng override
chỉ cho tiến trình kiểm thử:

```bash
MESA_GL_VERSION_OVERRIDE=3.3 MESA_GLSL_VERSION_OVERRIDE=330 \
  ./build/rain_thunderstorm --smoke-test
```

Khi không có DISPLAY hoặc sandbox chặn display, build/CPU test vẫn chạy được;
GUI test cần quyền truy cập phiên đồ họa. Không thay renderer bằng ảnh tĩnh.

### FPS thực tế

```bash
./build/rain_thunderstorm --benchmark 15
```

Hai lượt đo trong lần cải tiến này, cùng Debug, 800 × 600, timer 16 ms, 15 giây:

| | Trước | Sau |
|---|---:|---:|
| Frame | 905 | 899 |
| Thời gian (giây) | 15,0148 | 15,0103 |
| FPS trung bình | 60,2738 | 59,8920 |
| Rain | 1.000 | 1.000 |
| Splash active cuối / capacity | 512/512 | 512/512 |
| Đợt sét tự động | 3 | 3 |
| Cloud | 4 | 4 |
| Draw call tối đa | 48 theo pipeline | 48 đo bằng bộ đếm renderer |

Chênh lệch FPS khoảng -0,63%, không ghi nhận suy giảm đáng kể trong hai lượt
ngắn này. Timer đang giới hạn FPS; không kết luận về FPS tối đa hoặc GPU yếu hơn.
`--benchmark` nhận thời gian (0,600] giây và tự thoát; không dùng cùng smoke test.

## Giới hạn và checklist kiểm tra trực tiếp

- Va chạm mưa/splash chỉ với mặt đất Y=0; chưa chặn giọt tại mái nhà, cây hoặc đá.
- Camera tự do giữ nguyên, có thể đi xuyên vật thể; chưa có collision camera.
- Mây dùng mesh ghép low-poly; alpha fade sort theo tâm lobe nên chưa giải quyết
  chính xác mọi giao cắt trong suốt. Không còn dither ngẫu nhiên.
- Ánh chớp dùng một nguồn điểm với fill/Lambert và suy giảm khoảng cách; chưa có
  shadow mapping hoặc ánh sáng thể tích.
- Kết quả kiểm thử dài hạn mới nằm trong báo cáo 5–9. Chưa có phép đo GPU memory
  riêng trên nhiều cấu hình. Smoke gọi handler nội bộ, không thay thế kiểm tra
  cảm giác bàn phím/chuột thật.

Checklist trên máy của bạn:

- [ ] Góc nhìn mặc định thấy đất, nhà, 4 cây, 3 đá và các cụm mây phía trên.
- [ ] Mưa rơi xuống, nghiêng nhẹ theo gió; nhìn gần đất thấy splash.
- [ ] L tạo sét phân nhánh và ánh sáng chớp; sau đó cảnh tối lại.
- [ ] 1/2/3 bật/tắt đúng; sau khi tắt 2, L vẫn tạo sét.
- [ ] Space đóng băng mây/mưa/sét/splash, camera vẫn điều khiển được; Space tiếp tục.
- [ ] 4/5/6 chọn góc; F bật/tắt fog; nhìn gần thấy ripple và vũng nước.
- [ ] R reset toàn cảnh, WASD/QE/chuột vẫn như giai đoạn 1.
- [ ] Resize không méo hình; tiêu đề có FPS và số hạt; chạy vài phút không crash.
- [ ] Esc và nút đóng cửa sổ đều thoát bình thường.

## Lịch sử: báo cáo cải tiến hạng mục 1–4

*Đây là báo cáo của lần trước; trạng thái hiện tại xem hạng mục 5–9 phía dưới.*


Đã thực hiện theo thứ tự 1 → build/CTest/GUI → 2 → build/CTest/GUI → 3 →
build/CTest/GUI → 4 → build/CTest/GUI. Không đổi camera, mô hình nhà/cây/đá,
mặt đất, dung lượng pool hoặc dependency. Không triển khai hạng mục 5–10.

### Hạng mục 1 — Điều tra và sửa lỗi hiển thị

**Vấn đề và nguyên nhân xác minh:** Đã tái hiện cụm mây bên phải dạng chấm sau
23 giây chuyển động, khi coverage=0,507495. `object.frag` cũ dùng hash của
`gl_FragCoord` để discard pixel ngẫu nhiên theo coverage. Đây là nguyên nhân
trực tiếp của lỗi được chụp. Sphere dùng triangle list, không có EBO/indices;
CPU test xác minh mesh kín, winding/normal đúng. VAO/VBO dùng lại giữa các cụm;
model và normal matrix hợp lệ. Depth/culling và near/far không bị tắt/thay đổi.
Không có GL error trong các lượt tái hiện và kiểm tra sau sửa.

**Giải pháp:** Bỏ stochastic discard, trả alpha=coverage. Lobe opacity=1 vẽ ở
pass opaque; lobe fade được sort theo depth từ xa tới gần, blend SrcAlpha /
OneMinusSrcAlpha, depth test và back-face culling bật, depth write tắt riêng
trong pass fade rồi khôi phục. Cùng trạng thái 23 giây nay là một khối mây fade
liền, không còn các điểm nhiễu.

**Splash:** Số thứ nhất của 512/512 cũ là active count thực, số thứ hai là sức
chứa `std::array` cố định. 512 slot được giữ sẵn; slot còn lifetime>0 mới active
và được submit thành 2 vertex/slot. Số pixel nhìn thấy còn phụ thuộc depth test.
Kiểm thử mới đếm lại mọi slot sống ở từng bước trong 10 giây, khớp bộ đếm;
pool có thể đầy hợp lệ do mỗi impact phát 3 hạt. Sau khi tắt mưa, tất cả hạt
hết vòng đời trong lượt kiểm tra 0,5 giây. Không có lifetime vô hạn, không tăng
capacity, không đổi thuật toán splash hoặc thêm ripple.

**Sét và tiêu đề:** Mặc định bật/idle; bộ đếm thời gian 2–6 giây hoạt động.
`OFF/idle` là trạng thái hợp lệ khi tắt bằng 2, không phải lỗi mặc định.
L vẫn tạo sét thủ công lúc tự động tắt. Tiêu đề đã rút gọn, dùng active count và
L:ON/OFF/FLASH để phân biệt chờ/tắt/đang có đợt chớp.

**File sửa:** `include/Application.hpp`, `src/Application.cpp`, `src/main.cpp`,
`include/Renderer.hpp`, `src/Renderer.cpp`, `include/DynamicBatch.hpp`,
`shaders/object.frag`, `tests/weather_tests.cpp`.

**Xác minh:** Build Debug đạt; CTest 2/2 đạt; GUI smoke đạt, exit 0. Kiểm tra
submitted count rain/splash bằng active count và GL state được khôi phục.
Ảnh cùng camera, thời gian và opacity:
[trước sửa](build/reports/cloud-before.png), [sau sửa](build/reports/cloud-fixed.png).

**Giới hạn:** Sort theo tâm lobe là gần đúng đối với các ellipsoid giao nhau;
không có order-independent transparency. Không thể suy ra lịch sử thao tác phím
của ảnh người dùng chỉ từ chữ OFF/idle.

### Hạng mục 2 — Chất lượng mưa và chiều sâu

**Vấn đề xác minh:** Tích phân bán ẩn, gravity, terminal velocity, wind và fixed
step hiện có đã đúng. Chiều dài cũ chỉ ngẫu nhiên khi spawn và không đổi theo
vận tốc; mọi giọt cùng alpha, không có fade theo khoảng cách camera.

**Giải pháp/công thức:** Giữ tích phân và pool. Mỗi giọt có streakScale và alpha
ngẫu nhiên nhẹ; `length=clamp(speed*0.024*streakScale,0.12,0.42)`. Vertex shader
nhân alpha với `mix(1,0.42,smoothstep(8,34,distanceToCamera))`. Hướng vệt vẫn
ngược vector vận tốc. GL_LINES rộng 1, một draw call, VBO/shader dùng lại.
Không tạo vùng spawn phụ thuộc camera hoặc tăng hạt.

**File sửa:** `include/RainSystem.hpp`, `src/RainSystem.cpp`,
`include/Renderer.hpp`, `src/Renderer.cpp`, `shaders/effects.vert`,
`tests/weather_tests.cpp`.

**Xác minh:** Build Debug đạt; CTest 2/2 đạt; GUI smoke đạt, exit 0. Test kiểm tra
đúng 1.000 slot, phân bố cả 16 vùng mặt đất, độ dài tăng khi tăng tốc và có giới
hạn; rơi thẳng lúc không gió, đổi hướng khi đảo gió; recycling và độc lập FPS
vẫn đạt. Ảnh giai đoạn 2 đã được xem; ảnh tổng hợp cuối:
[cảnh mưa](build/reports/storm.png).

**Giới hạn:** Vệt vẫn rộng 1 pixel, không có giọt nước mesh/DOF. Alpha blending
trong batch không sort từng giọt. Các giọt tái sinh ở vùng phía trên như trước;
va chạm chỉ với mặt đất, không thêm va chạm mái nhà trong lần này.

### Hạng mục 3 — Hình dạng và chiếu sáng mây

**Vấn đề xác minh:** Các cụm trước đây dùng gần cùng một cấu hình 5 lobe và chỉ
thay đổi nhẹ scale. Màu không dựa trên chiều cao; ánh chớp trước đây đồng đều
không phụ thuộc khoảng cách nguồn sét.

**Giải pháp/công thức:** Vẫn 4 cụm × 5 lobe, dùng chung sphere 100 tam giác.
Thêm tỷ lệ X/Y/Z và skew riêng: 3 lobe đáy rộng chồng nhau, 2 lobe đỉnh bất đối
xứng. Màu nhân 0,72..1,08 theo smoothstep chiều cao, kết hợp Lambert/normal gốc.
CloudSystem tiếp tục dùng gió chung và deltaTime; fade/wrap dùng bản sửa hạng
mục 1. LightingSystem truyền vị trí nguồn chung; attenuation là
`1/(1+0.025*d+0.018*d*d)`, nên cụm gần nhận ánh chớp mạnh hơn.

**File sửa:** `src/CloudSystem.cpp`, `include/LightningSystem.hpp`,
`include/LightingSystem.hpp`, `src/LightingSystem.cpp`, `src/WeatherSystem.cpp`,
`include/Renderer.hpp`, `src/Renderer.cpp`, `shaders/object.vert`,
`shaders/object.frag`, `tests/weather_tests.cpp`.

**Xác minh:** Build Debug đạt; CTest 2/2 đạt; GUI smoke đạt, exit 0. Test kiểm tra
các lobe giao nhau thành một khối liên thông, profile khác nhau, chuyển động và
wrap; attenuation gần lớn hơn xa. Đã xem ảnh thường/flash và chụp lại wrap;
không thấy mây dạng điểm. Ảnh cuối:
[mây khi wrap](build/reports/cloud-wrap-final.png),
[mây khi chớp](build/reports/storm-flash.png).

**Giới hạn:** Mây là mesh low-poly ghép; vẫn thấy facet/lobe. Không mô phỏng tán
xạ hoặc mật độ thể tích. Geometry không được tạo lại mỗi frame.

### Hạng mục 4 — Sét, xung và ánh sáng môi trường

**Vấn đề xác minh:** Midpoint Displacement và kết nối trunk/branch cũ đúng, có
ngân sách 80/128 segment. Hai lớp quad đã có nhưng alpha đổi gây upload lại VBO
mỗi frame; các xung có peak giống nhau và không có khoảng tối rõ. Ánh chớp lên
vật thể đặc còn đồng đều toàn cảnh.

**Giải pháp/công thức:** Giữ đệ quy sâu 5/3 và sáu nhánh. Offset vuông góc bị cap
12% độ dài đoạn cha, giảm 0,40 mỗi cấp và scale tại biên để không tạo kink do
clamp từng tọa độ. Giới hạn vị trí X/Z ±9,8, Y 0,03..10. Taper đường chính và
nhánh, core trắng/halo xanh alpha thấp với mép mềm trong cùng một batch. Mỗi
đợt có 1–3 pulse, duration pulse 55–70% interval; peak đầu=1 và các peak sau
0,65..0,92, decay exp(-32*t), có gap thật. Nguồn tránh vùng camera; shader fade
hình học quá gần camera. Revision/camera cache quyết định upload, intensity là
uniform. Ánh điểm fill + Lambert và attenuation khoảng cách dùng chung cho mây,
nhà, cây, đá và đất; không thay đổi màu/vị trí/model các vật thể đó.

**File sửa:** `include/LightningSystem.hpp`, `src/LightningSystem.cpp`,
`include/WeatherSystem.hpp`, `src/WeatherSystem.cpp`, `src/Application.cpp`,
`include/Renderer.hpp`, `src/Renderer.cpp`, `include/DynamicBatch.hpp`,
`src/DynamicBatch.cpp`, `shaders/effects.vert`, `shaders/effects.frag`,
`shaders/object.frag`, `tests/weather_tests.cpp`.

**Xác minh:** Build Debug đạt; CTest 2/2 đạt; GUI smoke đạt, exit 0. Kiểm tra 100
seed: bounds/finite/nondegenerate, không có góc đảo chiều quá lớn, nhánh ngắn hơn
trunk, taper và kết nối; pulse có peak khác nhau, gap=0 rồi sáng lại; revision
không đổi giữa các xung. Test vị trí camera gần bốn tâm mây không chọn strike
đâm vào observer. GUI dùng L có kiểm soát, xác minh có geometry và ánh sáng,
không washout toàn cảnh, kết thúc/tắt trả flash=0. Bộ đếm upload xác nhận không
upload khi chỉ đổi intensity, nhưng refresh khi camera thay đổi.
[Ảnh sét/flash cuối](build/reports/storm-flash.png).

**Giới hạn:** Nguồn điểm + fill là xấp xỉ ánh sáng của cả tia sét, không có shadow
hoặc bloom. Cache billboard phải upload khi camera thay đổi. 100 seed là bộ
regression có kiểm soát, không phải chứng minh mọi quỹ đạo ngẫu nhiên có thể có.

### Tổng hợp và phạm vi dừng

Số đo trước/sau ở phần FPS phía trên: **60,27 → 59,89 FPS**, cùng điều kiện;
không khẳng định cải thiện hiệu năng từ số đo bị timer cap. Pipeline vẫn tối đa
48 draw call, 2.306 tam giác opaque/mây + 320 tam giác sét và các line particle.
Rain=1.000; splash active cuối=512/capacity=512; cloud=4; lightning mặc định
ON/idle, benchmark sau có 3 đợt ngẫu nhiên. Cả hai lượt benchmark kết thúc mã 0.

Chi phí thêm: sort tối đa 20 lobe khi fade, distance/point light/soft-edge trong
shader và một float edge mỗi EffectVertex (28 → 32 byte). Không thêm draw call,
VAO/VBO mới trong vòng lặp hoặc shader mới mỗi pulse. Cache giúp tránh upload
geometry sét khi chỉ brightness thay đổi; đây không phải nâng cấp tối ưu chuyên
sâu ở hạng mục 9.

CTest sanitizer CPU (Address/Undefined/LeakSanitizer) 2/2 đạt ngoài sandbox.
GUI smoke mới cũng đạt khi ép tiến trình về OpenGL 3.3 Core/GLSL 3.30; không có
GL error khi khởi tạo, render, đổi state và cleanup. Các ảnh trong `build/reports`
là artifact kiểm thử local, bị Git ignore cùng build; có thể tạo lại bằng các
lệnh capture phía trên.

Không nâng cấp splash/ripple, mặt đất/vũng nước, sky gradient/fog, scene/camera
preset, tối ưu chuyên sâu hoặc âm thanh. Mặt đất ướt cũ được giữ nguyên. Cần
kiểm tra thêm cảm giác điều khiển và chạy lâu trên máy khác; chưa đo GPU memory
riêng hoặc FPS tối đa không bị cap. Dừng sau hạng mục 4.

## Báo cáo hiện tại: hạng mục 5–9

Đã triển khai theo thứ tự **5 → build/CTest/GUI → 6 → build/CTest/GUI → 7 →
build/CTest/GUI → 8 → build/CTest/GUI → 9**. Các cải tiến 1–4 được giữ nguyên.
Mỗi gate 5–8 đạt Debug build, CTest 2/2 và GUI smoke exit 0, không có GL error.
Bản cuối cũng được kiểm thử với OpenGL 3.3 Core / GLSL 3.30.

### 5. Splash và Ripple

Cơ chế cũ đã được đọc trước khi sửa: mỗi impact Y=0 phát 3 hạt radial/upward,
lifetime 0,18–0,32; ring pool 512 ghi đè slot đang sống khi đầy. Số 512/512 là
trạng thái active/capacity hợp lệ, không phải lỗi lifetime.

Cơ chế mới vẫn dùng đúng SplashSystem đó, capacity **512**:

- Sampling 45% impact, phát 2 giọt mỗi impact được chấp nhận.
- Vận tốc radial 0,5–1,4, hướng lên 1,2–2,4; semi-implicit Euler với gravity -9,81.
- Lifetime 0,16–0,28 giây; chiều dài 0,05–0,10, alpha theo lifetime.
- Tìm slot trống từ cursor; đầy thì reject, không ghi đè hạt đang sống.
- Benchmark thống kê requests, filtered (sampling) và rejected (thiếu slot).

RippleSystem là một hệ thống thống nhất, pool **48**, không tạo thêm hệ splash.
Mỗi ripple sống 0,55 giây, radius tăng từ 0,04 tới tối đa 0,45 và alpha giảm bình
phương theo normalized age. Ring 12 cạnh ở Y=0,028; một VBO/batch cho toàn pool.
Phát có sampling, không phát cho mọi giọt. Ripple hết đời nhả slot, pool đầy sẽ
reject; không cấp phát mới mỗi frame.

Lượt smoke ở 0,5 giây có **304 splash/512**, rejected=0, **243 pixel ripple** và
247 pixel splash thay đổi khi tắt riêng hiệu ứng. Test CPU xác minh lifetime,
active count, cap, recycling, radius tăng/alpha giảm và reset. Stress test có
thể làm đầy pool nhưng pool vẫn thoát hết hạt khi ngừng phát.

File mới: `include/RippleSystem.hpp`, `src/RippleSystem.cpp`.
File sửa: `SplashSystem.hpp/.cpp`, `WeatherSystem.hpp/.cpp`, `Renderer.hpp/.cpp`,
`Application.cpp`, `CMakeLists.txt`, `tests/weather_tests.cpp`.
Ảnh: [splash/ripple](build/reports/h5-splash-ripple.png).

### 6. Wet Ground và Puddles

Tái sử dụng wetness của LightingSystem, không tạo trạng thái ướt thứ hai:

```text
raining: wetness = clamp(wetness + 0.06 * dt, 0, 1)
drying:  wetness = clamp(wetness - 0.012 * dt, 0, 1)
groundColor = mix(dryColor, dryColor * 0.76, wetness)
specular = pow(max(dot(normal, normalize(lightDir + viewDir)), 0), shininess)
```

Màu ướt và Blinn–Phong nằm trong object fragment shader. Shininess 24..64;
strength đất tăng theo wetness, vũng nước có strength riêng, specular tổng cap
0,45. Dùng cả ánh định hướng và flash point light hiện có. Nhà/cây không nhận
cùng độ bóng với nước. Không có SSR, planar reflection hoặc ảnh phản chiếu thực.

Thêm **3 vũng nước**, dùng chung một polygon 16 tam giác, biên biến thiên theo
sin(3a)/cos(5a), scale/rotate khác nhau, Y=0,012. Namespace Surface chứa cùng
outline/transform cho geometry và hit test, tránh vùng va chạm lệch mesh. Với
impact trong polygon, RippleSystem dùng xác suất 35%, max radius giới hạn bằng
90% khoảng cách tới cạnh gần nhất. Ring nằm cao hơn mặt nước, không vượt bờ.

Test CPU đạt: wetness tăng/giảm/clamp, polygon winding +Y, tâm nằm trong vũng,
ripple nằm trên và trong mặt nước. GUI smoke đạt, ảnh thường/flash đã được xem.
File mới: `include/Surface.hpp`, `src/Surface.cpp`.
File sửa: `Geometry.hpp/.cpp`, `Scene.hpp/.cpp`, `RippleSystem.hpp/.cpp`,
`WeatherSystem.cpp`, `Renderer.hpp/.cpp`, `shaders/object.frag`, CMake và hai test.
Ảnh: [vũng nước](build/reports/h6-puddles.png),
[góc thấp khi chớp](build/reports/ground-flash.png).

### 7. Sky Gradient, Fog và Lighting

Một fullscreen triangle với shader sky pha màu trên trời xanh xám đậm và màu
chân trời sáng hơn nhẹ. Đây là gradient đơn giản theo chiều dọc màn hình, không
phải mô hình khí quyển. Pass sky tắt depth test/write riêng, rồi khôi phục trước
khi vẽ cảnh; không tạo skybox hay nhiều khối hình.

```text
fogFactor = exp(-density * max(worldDistanceToCamera - 6, 0))
finalColor = mix(fogColor, objectColor, fogFactor)
```

Density mặc định **0,018**, F bật/tắt. Cấu hình lúc khởi chạy:

```bash
./build/rain_thunderstorm --fog-density 0.012
./build/rain_thunderstorm --no-fog
```

Cloud dùng density scale 0,65 và fog color tối hơn; rain 0,35 để tránh nhân fade
quá mạnh; sét 0,2 để giữ nổi bật. Splash/ripple dùng scale 1. Mọi khoảng cách là
world space. Sky và fog color cùng nhận flash của LightingSystem, rồi trở về nền.
R khôi phục fog bật và density mặc định, kể cả khi khởi chạy với tùy chọn khác.

CPU test kiểm tra fog gần rõ, xa mờ và density=0. GUI smoke bật/tắt F và so sánh
pixel, đồng thời giữ kiểm thử mưa/mây/sét/splash/ripple, depth/blending/cleanup.
File mới: `shaders/sky.vert`, `shaders/sky.frag`.
File sửa: Geometry, LightingSystem, WeatherSystem, Renderer, Application,
`src/main.cpp`, CMake, object/effects fragment shader và weather tests.
Ảnh: [sky/fog](build/reports/h7-sky-fog.png),
[toàn cảnh cuối](build/reports/h9-overview.png).

### 8. Scene và Camera Presets

Kiểm tra geometry/normal/winding/culling không phát hiện lỗi bắt buộc sửa ở nhà,
cây hoặc đá. Không đổi vị trí/kích thước cơ bản, không thêm polygon cho chi tiết:
chỉ tăng nhẹ tương phản màu cửa sổ và thay sắc độ tán giữa bốn cây. Các cây đã có
chiều cao khác nhau; ba đá đã có scale/yaw/màu khác nhau nên giữ nguyên.

| Phím | Vị trí | Góc nhìn |
|---|---|---|
| 4 | (14,10,18) | Toàn cảnh, đúng reset mặc định cũ |
| 5 | (5,3.2,8) | Nhìn nhà từ phía trước/phải |
| 6 | (-0.4,0.85,7) | Nhìn vũng nước/splash/ripple ở góc thấp |

Preset đổi camera tức thì, không reset thời tiết. WASD, Q/E, kéo chuột, pitch
limit và R giữ cách hoạt động cũ. Camera từ chối deltaTime NaN để không làm hỏng
view matrix. Test CPU kiểm tra preset ngoài nhà/cây, trên đất, pitch hợp lệ và
projection/aspect ratio cũ; GUI render cả ba preset và kiểm tra hồi quy input.

File sửa: `Camera.hpp/.cpp`, `Scene.hpp/.cpp`, `Application.hpp/.cpp`,
`src/main.cpp`, `tests/core_tests.cpp`.
Ảnh: [toàn cảnh](build/reports/camera-4.png),
[gần nhà](build/reports/camera-5.png), [góc thấp](build/reports/camera-6.png).

### 9. Performance, Validation và kiểm thử tổng thể

Không chuyển sang engine, instancing hoặc compute shader. Audit cho thấy mesh,
shader, VAO/VBO và pool đã tái sử dụng. Mỗi frame dùng staging array cố định;
geometry sét chỉ upload khi đổi revision/camera. Ripple dùng circle template tạo
một lần. Chỉ sort tối đa 20 lobe đang fade. Chưa có bằng chứng cần đổi kiến trúc
render hoặc tối ưu sâu; ưu tiên giữ các thuật toán hiện tại dễ giải thích.

Module Benchmark chỉ được tạo khi đo:

- Array vòng tối đa 65.536 frame interval; quantile tính trên các mẫu còn lưu.
- Mean/P95/P99 gồm timer, cập nhật, submit và chờ presentation.
- CPU submit wall time đo riêng trước swap, không phải CPU usage.
- Bốn GL_TIME_ELAPSED query dùng lại, đọc bất đồng bộ khi sẵn sàng; không glFinish
  mỗi frame. GPU command time không phải phần trăm GPU utilization.
- CPU usage từ getrusage, 100% tương đương một core; RAM RSS/VmHWM từ `/proc`.
- Mỗi giây kiểm tra finite/bounds/count của rain, splash, ripple, mây, sét,
  wetness và flash; trace mỗi phút, GL error được kiểm tra mỗi frame.
- Bộ đếm renderer đo số call, triangle và GL_LINES submit thực, trước culling.

Chế độ mặc định vẫn timer 16 ms và bỏ update khi cửa sổ bị ẩn. Benchmark tiếp tục
update/render khi occluded để không đo một trạng thái pause; timer được lên lịch
trước render trong benchmark để tránh cộng thêm render time vào khoảng chờ 16 ms.
Presentation vẫn có thể bị compositor/driver trì hoãn.

```bash
# Đo ngắn, timer cap, camera 4, 800x600; không thao tác trong lúc đo
./build/rain_thunderstorm --benchmark 15
# Tắt fog để đối chiếu với bản cũ không có fog
./build/rain_thunderstorm --benchmark 15 --no-fog
# Bỏ timer, yêu cầu swap interval 0 riêng cho tiến trình
./build/rain_thunderstorm --benchmark-unlocked 15
# Kiểm thử dài 10 phút, toàn bộ hiệu ứng bật
./build/rain_thunderstorm --benchmark 600 > build/reports/soak-600.log 2>&1
```

`--benchmark-unlocked` không thay driver/cấu hình hệ thống. GLX báo swap interval
0 trong lượt đo, nhưng compositor hoặc presentation vẫn có thể giới hạn. Vì vậy
không dùng số khoảng 60 hoặc 144 FPS để suy ra GPU tối đa, CPU-bound hay GPU-bound.
Một lượt đo ban đầu bị pause/che cửa sổ và một lượt có lịch timer cộng render đã
bị loại khỏi so sánh; không dùng chúng để khẳng định hiệu năng ứng dụng giảm.

Build Debug, CTest 2/2, GUI smoke trên OpenGL 4.6 và 3.3 Core đều đạt. CPU
ASan/UBSan/LSan 2/2 đạt ngoài sandbox (LSan không chạy được dưới ptrace). Không
coi sanitizer CPU là xác minh GPU memory. Không có nvidia-smi trong môi trường;
không có số đo GPU usage/memory riêng đủ tin cậy để báo cáo.

File mới: `include/Benchmark.hpp`, `src/Benchmark.cpp`.
File sửa: Application, Renderer (thống kê primitive), WeatherSystem (`valid()`),
main/CMake. README và các test đồng bộ với các module mới.

Ảnh và lệnh kiểm tra mới:

```bash
./build/rain_thunderstorm --smoke-test \
  --capture /tmp/storm.ppm --capture-lightning /tmp/flash.ppm \
  --capture-cloud-wrap /tmp/wrap.ppm --capture-views build/reports/views
```

Smoke chụp sét bằng handler L, không phụ thuộc thời điểm ngẫu nhiên. Pixel test
bản cuối: mây 18.048, mưa 5.496, splash 247, ripple 243, geometry sét 1.016;
18.131 pixel mây và 124.148 pixel cảnh phía dưới sáng hơn khi chớp. Cả ba preset,
F, pause/reset, resize, state restore, shader compile/link và cleanup đều đạt.
[Ảnh flash](build/reports/h9-flash.png).

#### Kết quả kiểm thử dài 10 phút

[Log thô 600 giây](build/reports/soak-600.log). Tiến trình kết thúc **mã 0** sau
**600,188 giây**, 10.331 frame, 43 đợt sét tự động. Không crash/GL error; kiểm tra
finite/bounds/count mỗi giây không phát hiện dữ liệu không hợp lệ. Mây di chuyển
và wrap, mưa/splash/ripple tiếp tục hoạt động, wetness tăng tới 1.

| Chỉ số cả lượt dài | Đo được |
|---|---:|
| Average FPS | 17,213 |
| Mean frame time | 58,0958 ms |
| P95 / P99 frame interval | 17,4336 / 1.002,50 ms |
| CPU submit wall time/frame | 0,74811 ms |
| GPU command time/frame | 0,311818 ms, 10.330 mẫu |
| CPU usage (100%=1 core) | 4,85664% |
| RSS mẫu bắt đầu / gần cuối | 87,7578 / 94,2383 MiB |
| Peak RSS (VmHWM) | 94,3164 MiB |
| Draw call / triangle submit tối đa | 53 / 2.675 |
| GL_LINES segment tối đa | 1.935 |
| Rain | 1.000 |
| Splash cuối / capacity | 235 / 512 |
| Ripple cuối / capacity | 40 / 48 |
| Peak splash/ripple lấy mẫu mỗi giây | 339 / 48 |
| Mẫu splash đầy / tổng mẫu | 0 / 594 |
| Splash requests / filtered / rejected | 275.326 / 151.509 / 0 |
| Ripple yêu cầu bị từ chối do đầy | 912 |

**Giới hạn quan trọng:** Sau khoảng 2–3 phút, trace có nhiều đoạn gần 1 FPS khi
chạy nền, dù command GPU trung bình chỉ khoảng 0,31 ms. Chưa xác minh đầy đủ
nguyên nhân trễ presentation/dispatch trong phiên hiển thị này. Vì vậy không
khẳng định FPS dài hạn ổn định, không dùng average 17,213 để so sánh trực tiếp
với baseline cửa sổ hoạt động, và không quy kết CPU-bound/GPU-bound. DeltaTime
bị clamp khi stall, nên 600 giây thời gian thực không đồng nghĩa mô phỏng đã
tiến đủ 600 giây. Kết quả này xác minh chạy GUI lâu không crash và các invariant
được kiểm tra, không chứng minh throughput dài hạn không bị môi trường giới hạn.

RSS mẫu từ phút 3 đến cuối quanh **94,19–94,24 MiB**; không thấy tăng lớn trong
khoảng quan sát đó. Đây không phải chứng minh tuyệt đối không rò rỉ driver GPU.
Cần đo lại dài hạn khi cửa sổ được giữ hiển thị trên máy Ubuntu của bạn.

Benchmark OpenGL 3.3 riêng 5 giây cũng đạt mã 0 với GL_TIME_ELAPSED/cleanup:
718 frame / 5,00483 giây = 143,461 FPS, driver báo swap interval=0. Đây chỉ là
lượt kiểm tra tương thích của phần đo GPU, không thay thế phép đo dài hạn.

#### Đo ngắn trong cùng điều kiện

Hai lượt cuối: Debug, 800 × 600, camera 4, rain=1.000, cloud=4, fog ON/density
0,018, sét tự động ON với seed mặc định, 15 giây, không thao tác input.

| Chỉ số | Timer 16 ms | Timer bỏ, swap interval yêu cầu 0 |
|---|---:|---:|
| Frame / giây | 931 / 15,0123 | 2.155 / 15,0014 |
| Average FPS | **62,0157** | **143,653** |
| Mean frame time | 16,1250 ms | 6,96121 ms |
| P95 | 17,0118 ms | 7,82905 ms |
| P99 | 17,3708 ms | 8,29330 ms |
| Mẫu frame interval | 930 | 2.154 |
| CPU submit wall/frame | 0,665359 ms | 0,505272 ms |
| GPU command/frame | 0,311914 ms | 0,631823 ms |
| CPU usage, một core=100% | 15,1686% | 20,5406% |
| RSS mẫu gần cuối / peak | 92,8203 / 92,8320 MiB | 93,3008 / 93,3242 MiB |
| Draw call / triangle tối đa | 53 / 2.675 | 53 / 2.675 |
| GL_LINES tối đa | 1.935 | 1.935 |
| Splash cuối / capacity | 315 / 512 | 315 / 512 |
| Ripple cuối / capacity | 41 / 48 | 41 / 48 |
| Cloud / đợt sét | 4 / 3 | 4 / 3 |
| Splash rejected / mẫu full | 0 / 0 | 0 / 0 |

Log: [timer cap](build/reports/benchmark-fog-capped.log),
[timer bỏ](build/reports/benchmark-unlocked.log). GPU samples có thể ít hơn
frame count vì query được đọc bất đồng bộ, không chờ kết quả ở mỗi frame.
Số GPU command ms là khoảng GPU timer của batch lệnh, không phải GPU utilization
hoặc thời gian presentation. Không suy ngược FPS tối đa từ số này.

Baseline trước 5–9, cùng Debug/800 × 600/camera mặc định, chưa có fog: **913 frame /
15,0138 giây = 60,8107 FPS**, draw call tối đa 48. Lượt sau với fog OFF:
**930 frame / 15,0072 giây = 61,9701 FPS**, mean 16,1368 ms, P95 17,0458 ms,
P99 17,4394 ms, draw call 53. [Log sau, fog OFF](build/reports/benchmark-capped.log).
Các số trước không có GPU query/P95/P99. Benchmark mới có instrumentation và
cách lên lịch khi occluded khác bản cũ; vì vậy baseline không hoàn toàn tương
đương để kết luận tăng/giảm hiệu năng GPU. Các số bị timer cap chỉ là đối chiếu
mô tả; không khẳng định tối ưu làm ứng dụng nhanh hơn.

#### Phạm vi dừng và kiểm tra trên máy của bạn

Đã hoàn thành phần triển khai 5–9. Giữ nguyên 1.000 rain, 4 cloud, splash cap 512,
nhà/cây/đá và mặt đất 20 × 20. Ripple cap 48, thêm đúng 3 vũng nước. Pipeline từ
48 lên tối đa 53 draw call: +3 vũng nước, +1 sky, +1 batch ripple. Tam giác thêm
48 cho vũng nước và 1 cho sky; mesh nhà/cây/đá không tăng polygon.

Không thêm âm thanh, SDL mixer, Dear ImGui hoặc hạng mục 10. Không thêm thư viện,
không đổi driver/cấu hình Ubuntu. Git metadata không khả dụng trong môi trường
phiên này; đã đối chiếu source với snapshot trước khi sửa. RainSystem,
CloudSystem, LightningSystem, Mesh và Shader giữ nguyên trong lần 5–9.

Giới hạn còn lại: collision chỉ Y=0; camera vẫn tự do xuyên vật thể khi tự di
chuyển; alpha sorting theo tâm là gần đúng; specular không phản chiếu hình ảnh;
gradient theo màn hình là mô hình đơn giản; chưa đo GPU memory riêng; FPS dài
hạn trong phiên hiển thị nền chưa được xác minh ổn định. Các số RAM/CPU/GPU time
được đo trên RTX 3050 Laptop qua Mesa/zink, không suy ra cho GPU yếu hơn.

Checklist cuối:

- [ ] 4: thấy cảnh, mưa, mây; 5: quan sát nhà; 6: quan sát splash/ripple/vũng nước.
- [ ] 1 tắt mưa: hạt bắn/vòng gợn hết đời, wetness giảm chậm; bật lại tiếp tục.
- [ ] F thay đổi fog nhưng vật gần vẫn rõ; thử density 0 và 0,012 qua CLI.
- [ ] L làm cảnh/nước sáng lên rồi trở lại nền; 2 tắt sét tự động vẫn cho L.
- [ ] Space đóng băng cả ripple/wetness; camera vẫn đi được; R về trạng thái mặc định.
- [ ] Chạy benchmark 600 khi giữ cửa sổ hiển thị; đối chiếu log dài với môi trường này.
- [ ] Esc và nút đóng cửa sổ thoát bình thường.

**Dừng sau hạng mục 9. Hạng mục 10 — âm thanh — chưa triển khai.**
