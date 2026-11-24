# Sửa ripple và va chạm mưa với đất đá nước

Cập nhật 24/11/2026. Mưa hiện tìm lần va chạm đầu tiên với nhà, ba tảng đá,
mặt đất hoặc mặt nước. Đất, đá và nhà chỉ tạo splash; chỉ nước tạo ripple.
Hệ thống giữ 1.000 giọt mưa, dùng chung pool 512 splash và pool 48 ripple.

## Nguyên nhân gợn sóng trên đất

Trong mã cũ, `WeatherSystem::update()` gửi mọi impact đất vào
`ripples.emit(hit, true, surface::waterMargin(hit))`. Khi margin bằng 0, hàm
`RippleSystem::emit()` vẫn dùng bán kính tối đa 0.45 và xác suất phát 5.5%.
Renderer có thêm nhánh màu dành cho ripple trên đất nên các vòng này vẫn được
vẽ. Kiểm tra polygon đã tồn tại, nhưng kết quả nằm ngoài nước không bị từ chối.

Đối với nước, mã cũ đã giới hạn bán kính theo khoảng cách tới bờ. Bản sửa giữ
nguyên nguyên tắc đó, bỏ hoàn toàn nhánh ripple đất và cho phép vòng nhỏ sát bờ.
Các test cũ phát ripple tại `(0,0,0)` hoặc `(1,0,1)` đã được đổi sang vị trí
nước thật; test mới bắt buộc các điểm đất này không tạo vòng.

## Các file thay đổi trong lần sửa này

| File | Nhiệm vụ |
|---|---|
| `include/Surface.hpp`, `src/Surface.cpp` | `SurfaceType` và truy vấn `waterAt()` trả polygon nước, độ cao và khoảng cách tới bờ |
| `include/HouseCollider.hpp`, `src/HouseCollider.cpp` | Tách thuật toán BVH và segment–triangle thành `TriangleCollider` dùng chung; giữ mesh nhà và hành vi va chạm mái |
| `include/SceneCollision.hpp`, `src/SceneCollision.cpp` | Mới: `RockCollider`, truy vấn `earliestHit()` và cắt vệt theo mọi bề mặt |
| `include/EnvironmentGeometry.hpp`, `src/EnvironmentGeometry.cpp`, `src/Scene.cpp` | Cache mesh đá và dùng chung `rockGeometry()`/`rockTransform()` giữa renderer và collider |
| `include/RainSystem.hpp`, `src/RainSystem.cpp` | Một danh sách impact chứa điểm, pháp tuyến, vận tốc, loại bề mặt và tham số t; respawn đúng một lần |
| `include/SplashSystem.hpp`, `src/SplashSystem.cpp` | Dùng các profile đất/đá/mái/nước trong cùng pool; hạt thứ cấp va chạm lại với toàn cảnh |
| `include/RippleSystem.hpp`, `src/RippleSystem.cpp` | Từ chối ngoài polygon hoặc sai độ cao nước, giới hạn vòng theo bờ và giữ alpha fade |
| `src/WeatherSystem.cpp` | Phân phối splash theo loại bề mặt; chỉ gọi ripple khi impact là Water |
| `include/Renderer.hpp`, `src/Renderer.cpp` | Bỏ màu ripple đất; kiểm lại phạm vi và độ cao nước, đếm vòng thực sự được submit |
| `include/Application.hpp`, `src/Application.cpp` | Thêm fixture ảnh đất/nước/ba đá trong ngày, đêm và sấm chớp vào smoke test |
| `tests/collision_tests.cpp` | Kiểm tra đường bờ, collider đá, gió mạnh, một impact duy nhất, profile splash và chạy dài |
| `tests/weather_tests.cpp`, `tests/environment_tests.cpp` | Cập nhật test theo impact có loại bề mặt và mặt nước thật |
| `CMakeLists.txt` | Thêm `SceneCollision.cpp` vào thư viện hiện có; giữ các dependency và cấu hình Ubuntu |
| `README.md`, `docs/11_RAIN_COLLISION.md`, `docs/12_SURFACE_COLLISIONS.md` | Cập nhật mô tả hiện tại, phân biệt báo cáo mái ban đầu với bản sửa mới |

## Phân biệt nước với đất và giới hạn vòng gợn

`surface::waterAt()` dùng đúng 32 đỉnh world space của mỗi polygon do
`surface::polygon()` tạo. Đây cũng là dữ liệu để `environment::puddles()` tạo
mesh hiển thị. AABB chỉ loại trừ sơ bộ; phép ray crossing point in polygon mới
quyết định vị trí thuộc nước. Khoảng cách tới bờ là giá trị nhỏ nhất tới tất cả
đoạn cạnh của polygon:

```text
s = clamp(dot(P-A,B-A) / dot(B-A,B-A), 0, 1)
d = min(distance(P, A+s*(B-A)))
```

Mặt nước hiện có Y=0.012. `earliestHit()` giao quỹ đạo với độ cao thật này và
kiểm tra polygon tại điểm giao; mặt đất ở Y=0. Nếu hai mặt cùng độ cao, Water
được ưu tiên khi cùng thời điểm chạm. Va chạm vào đá hoặc nhà ở t nhỏ hơn sẽ
loại bỏ sự kiện đất/nước phía sau.

`RippleSystem::emit()` tự kiểm tra lại polygon và độ cao nước để một lời gọi
trực tiếp sai cũng không tạo vòng trên đất hoặc trên đá ở phía trên nước.
Vòng được đặt ở `water.height + 0.002` để tránh Z-fighting:

```text
maxRadius     = min(0.45, 0.9*d)
initialRadius = min(0.04, 0.25*maxRadius)
```

Chỉ phát khi maxRadius ít nhất 0.002. Theo thời gian, bán kính nội suy từ
initialRadius đến maxRadius và alpha giảm về 0 trong 0.55 giây. Toàn bộ đĩa có
bán kính nhỏ hơn khoảng cách gần nhất tới bờ nằm trong polygon, kể cả polygon
lõm. Mỗi đoạn GL_LINES nối hai điểm của vòng cũng nằm trong đĩa đó. Renderer
kiểm tra lại polygon, độ cao và bán kính trước khi upload. Vì vậy không cần
stencil, đổi shader hoặc đổi hình dạng nước để giới hạn vòng.

## Va chạm đá và vòng đời giọt mưa

`environment::rockGeometry()` cache nguyên mesh low poly đã có của cả ba đá,
mỗi mesh 48 tam giác. `rockTransform()` trả đúng ma trận `T*R_yaw*S` mà Scene
dùng để vẽ. `RockCollider` biến đổi mọi đỉnh sang world space một lần, tính
`N=normalize(cross(B-A,C-A))` từ các đỉnh đã biến đổi, rồi xây cây BVH AABB.
Cách tính này bao gồm cả scale không đều và giữ pháp tuyến hướng ra ngoài.

`TriangleCollider::intersect()` tái sử dụng Möller–Trumbore và các kiểm tra
EPSILON của mái. Với `P(t)=previous+t*(next-previous)`, hàm chỉ nhận t trong
[0,1] và barycentric nằm trong tam giác. `earliestHit()` so sánh kết quả nhà,
đá, đất và nước rồi giữ t nhỏ nhất. `RainSystem::update()` phát một impact,
respawn và `continue`; không tiếp tục phát splash hoặc ripple dưới đá.

`clipStreak()` cũng gọi truy vấn toàn cảnh theo cả hai phía bề mặt để cắt đuôi
mưa và splash trước đá, nhà, đất hoặc nước. Vị trí trước khi respawn được đặt
bằng vị trí mới trên trời, tránh vệt nối từ điểm va chạm lên vùng sinh mưa.
Renderer dùng nguyên điểm đuôi đã cắt, bỏ phép nâng Y cố định trước đây để
không làm sai điểm cắt trên các mặt đá nghiêng hoặc gần mặt nước.

## Profile splash và vật lý

`SplashSystem::emitSurface()` nhận vị trí, pháp tuyến, vận tốc tới và SurfaceType.
Các profile tập trung trong `include/SplashSystem.hpp`:

| Thông số | Đất và nước | Đá | Mái và các mặt nhà |
|---|---:|---:|---:|
| Số hạt mỗi nhóm được nhận | 2–4 | 3–5 | 3–6 |
| Lifetime giây | 0.10–0.20 | 0.12–0.30 | 0.12–0.30 |
| Chiều dài GL_LINES | 0.010–0.025 | 0.015–0.030 | 0.015–0.035 |
| Tốc độ theo pháp tuyến | 0.30–0.60 | 0.40–0.90 | 0.45–1.05 |
| Tốc độ tiếp tuyến | 0.15–0.45 | 0.20–0.75 | 0.25–0.90 |
| Hệ số giữ vận tốc phản xạ | 0.02 | 0.05 | 0.06 |

Vận tốc ban đầu kết hợp `R=V-2*dot(V,N)*N`, thành phần theo N và hướng ngẫu
nhiên trong hai trục tiếp tuyến. Hàm bảo đảm `dot(Vsplash,N)>0`; hạt sinh tại
`hit+0.002*N`. Chuyển động dùng `g=(0,-9.81,0)`, bước không quá 1/120 giây:

```text
Pnext = P + V*dt + 0.5*g*dt*dt
Vnext = V + g*dt
alpha = 0.85*clamp(lifetime/initialLifetime,0,1)
```

Hạt chạm lại bất kỳ bề mặt nào hoặc hết lifetime sẽ kết thúc, không phát thêm
splash/ripple. Pool cố định 512, không cấp phát từng hạt trong vòng lặp. Chính
sách lấy mẫu có sẵn được giữ cho đất/nước: 45% yêu cầu splash và 35% ripple
nước; mái/đá không bị lọc xác suất. Nếu pool không đủ cho cả nhóm, yêu cầu bị
từ chối và được đếm. Mật độ mưa vẫn là 1.000 hạt.

## Build và kết quả kiểm thử

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 4
ctest --test-dir build --output-on-failure
./build/rain_thunderstorm --smoke-test \
  --capture-views build/reports/surface-collision-views
./build/rain_thunderstorm --benchmark 30
```

Build Debug thành công. 6/6 CTest đạt, gồm toàn bộ kiểm thử cũ đã cập nhật và
kiểm thử va chạm mở rộng. ASan/UBSan/LeakSanitizer cũng đạt 6/6 CTest ngoài
sandbox, không báo lỗi bộ nhớ CPU; LeakSanitizer cần chạy ngoài cơ chế ptrace.

| Trường hợp | Kiểm tra và kết quả |
|---|---|
| Đất ngoài nước | Từ chối mọi ripple, tạo splash nhỏ theo profile đất; giọt được respawn |
| Giữa vũng | Water được chọn trước đất, điểm chạm đúng Y=0.012; ripple và splash hiển thị |
| Sát mép trong | Kiểm tra đủ 96 cạnh bờ, lấy mẫu toàn vòng và trung điểm 12 đoạn trong suốt lifetime; không ra khỏi polygon |
| Sát mép ngoài | Các điểm cách bờ 0.01 được phân loại Ground và không tạo ripple |
| Đỉnh và mặt nghiêng đá | Đối chiếu 144 tam giác đã biến đổi, điểm chạm và normal khớp mesh; có splash hướng ra ngoài |
| Gió mạnh và tốc độ cao | Đoạn xiên qua cả ba đá; đoạn Y=100 đến -100 không bỏ qua đá; bộ kiểm tra mái Y=1000 đến -1000 vẫn qua |
| Một sự kiện trên đá | Inject giọt vào từng đá qua WeatherSystem: đúng một Rock impact, một yêu cầu splash, không Ground/Water/ripple phía dưới |
| Splash rơi lại đá | Được kết thúc trước khi xuyên bề mặt; không tăng số yêu cầu phát hạt thứ cấp |
| Mái và ống khói | Các kiểm tra hai mái, đỉnh, mép, phần nhô, ống khói và splash cũ đều qua |
| Mưa dày và bộ nhớ | 120 giây mô phỏng, 1.000 giọt, gió đổi chiều và sét; địa chỉ pool ổn định, kiểm tra giới hạn và thoát bão hòa |
| Camera và chiếu sáng | Smoke chụp hai mái, đất, nước, cả ba đá và các preset; ngày, đêm, chớp đều nhìn thấy hiệu ứng |

Chạy 120 giây ghi nhận số impact đất/đá/nhà/nước lần lượt
**121.240 / 2.105 / 13.242 / 5.192**, splash đỉnh **474/512**. Mỗi bước xác nhận
không còn giọt sống hoặc rain streak cắt qua collider, và mỗi ripple sống có
đĩa nằm hoàn toàn trong nước. Toàn bộ splash tắt sau khi dừng phát mưa.

Smoke trên OpenGL 4.6 Core, Mesa 26.0.8, zink/NVK, RTX 3050 Laptop, 800×600 đã
qua, không có lỗi GL. Fixture đất đóng góp 79 pixel, mỗi đá lần lượt 37/20/22
pixel và có **0 ripple**; fixture nước có chín vòng. Các fixture so ảnh với
hiệu ứng bật/tắt trong ngày, đêm và sấm chớp. Vật thể opaque vẫn vẽ trước
particle, giữ depth test, tắt depth writing trong pass trong suốt rồi khôi phục;
không thêm stencil hoặc thay đổi hệ thống ánh sáng.

Ảnh nằm trong `build/reports/surface-collision-views/`: `ground-*.ppm`,
`water-*.ppm`, `rock-0-*.ppm`, `rock-1-*.ppm`, `rock-2-*.ppm`, `roof-*.ppm`.

## Hiệu năng và giới hạn kiểm chứng

Lượt đo lại riêng 30 giây đạt **61.94 FPS**, P95 **17.59 ms**, P99 **18.11 ms**,
render submit CPU **0.620 ms**, GPU command **0.478 ms**, CPU một core **17.71%**.
RSS từ **89.56 lên 94.91 MiB**, peak **94.93 MiB**. Giữ đủ 1.000 giọt, splash
đỉnh **434/512**, ripple đỉnh **15/48**, không có yêu cầu bị từ chối hoặc mẫu
pool bão hòa. Lượt 12 giây đạt **61.97 FPS**, P99 **17.97 ms**.

Một lượt 30 giây trước đó chỉ đạt **25.73 FPS**, P99 khoảng **1001 ms**, trong
khi thời gian submit vẫn khoảng 0.613 ms. Nguyên nhân các khoảng chờ dài chưa
được xác minh; lượt đo lại riêng không tái hiện. Các số bị giới hạn bởi timer
16 ms, không chứng minh FPS tối đa hay ổn định nhiều giờ trên mọi máy.

Chưa kiểm thử thao tác chuột/phím thật từ mọi góc tùy ý, chạy nhiều giờ, GPU
khác hoặc rò rỉ VRAM/driver. Cây vẫn chưa có collider. Không đổi hình dạng nhà,
đá, cây, đất, vũng nước, camera, điều khiển, shader hay thư viện phụ thuộc.
