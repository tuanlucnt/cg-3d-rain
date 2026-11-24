# Va chạm mưa và nước bắn trên mái nhà

Hệ thống đã được mở rộng cho đất, đá và nước trong
[bản sửa ripple và va chạm bề mặt](12_SURFACE_COLLISIONS.md).
Các mô tả API và số đo dưới đây thuộc lần triển khai mái ban đầu.

Cập nhật ngày 24/11/2026. Giọt mưa dừng ở lần chạm nhà đầu tiên và được tái sinh
trên trời, đồng thời tạo 3–6 hạt nước bắn theo pháp tuyến. Hai mặt mái nghiêng,
phần nhô ra, sống mái, viền gỗ, tường, bậc cửa và ống khói dùng chính hình học
hiển thị hiện có. Pool vẫn gồm 1.000 giọt mưa và tối đa 512 hạt splash.

## Các file thay đổi

| File | Chức năng |
|---|---|
| `include/HouseCollider.hpp`, `src/HouseCollider.cpp` | Mới: tam giác, pháp tuyến, phương trình mặt phẳng, AABB, cây BVH và giao đoạn với tam giác |
| `include/ModelGeometry.hpp`, `src/ModelGeometry.cpp` | `HOUSE_POSITION` và `houseGeometry()` dùng chung cho hiển thị và va chạm |
| `src/Scene.cpp` | Lấy hình học và vị trí nhà từ dữ liệu chung |
| `include/RainSystem.hpp`, `src/RainSystem.cpp` | Vị trí trước bước cập nhật, va chạm đầu tiên, danh sách impact nhà và tái sinh |
| `include/SplashSystem.hpp`, `src/SplashSystem.cpp` | Mở rộng pool có sẵn để phát splash tại bề mặt, chịu trọng lực, fade và kết thúc khi chạm lại nhà |
| `src/WeatherSystem.cpp` | Gửi impact nhà tới splash; impact mặt đất vẫn đi vào splash và ripple cũ |
| `src/Renderer.cpp` | Cắt đuôi vệt mưa/splash trước khi upload, dùng alpha theo lifetime |
| `include/Application.hpp`, `src/Application.cpp` | Thêm kiểm thử ảnh splash trên hai mái, ban ngày, ban đêm và sấm chớp vào smoke test |
| `tests/collision_tests.cpp` | Mới: kiểm thử hình học, vòng đời, tái va chạm, giới hạn pool và chạy dài |
| `CMakeLists.txt` | Build collider trong `scene_core`, đăng ký test `rain_house_collision` |
| `README.md`, `docs/11_RAIN_COLLISION.md` | Ghi nhận tính năng và hướng dẫn kiểm chứng |

## Hình học và các hàm chính

`models::houseGeometry()` lưu mesh nhà bất biến, dùng chung với `Scene`. Tọa độ
local được cộng `HOUSE_POSITION = (-1,0,-1)` đúng một lần khi xây collider.
Không sửa đỉnh, kích thước, vật liệu hay mesh nhà. Mái trên có các đỉnh local
`(-2.38,2.56,±2.15)`, `(0,4.20,±2.15)`, `(2.38,2.56,±2.15)`.
Mỗi mặt nghiêng gồm hai tam giác; các phần trang trí, sống mái cao 4.26 và miệng
ống khói cao 4.77 cũng được giữ để điểm chạm khớp bề mặt nhìn thấy.

`HouseCollider::HouseCollider()` tính trước cạnh, pháp tuyến, hằng số mặt phẳng
và AABB cho 734 tam giác nhà. `buildNode()` chia đôi danh sách chỉ số theo trục
có phạm vi tâm lớn nhất, tạo cây hộp bao BVH; mỗi lá tối đa tám tam giác.
Đây là mesh low poly, không kiểm tra từng viên ngói do shader vẽ.

`HouseCollider::intersect()` loại trừ bằng AABB của đoạn, sau đó duyệt cây bằng
stack cố định và gọi `segmentTriangle()` tại các lá phù hợp. Hàm trả điểm,
pháp tuyến, loại phần nhà và tham số `t` nhỏ nhất. Chế độ mặc định chỉ nhận
chuyển động đi vào mặt có pháp tuyến hướng ra ngoài, cho phép splash rời mái.
`clipStreak()` kiểm tra cả hai phía để cắt đoạn hiển thị trước bề mặt.

`RainSystem::update()` lưu `previousPosition`, tính `next`, so sánh va chạm nhà
với thời điểm chạm đất. Khi chạm nhà, phát đúng một impact rồi respawn và
`continue`; không để lại điểm cũ để render hoặc phát ripple dưới nhà. Respawn
đặt cả vị trí trước và vị trí hiện tại ở trên trời, tránh vệt nối xuyên cảnh.
Phân bố ban đầu cũng được kiểm tra để không sinh giọt dưới mái.

`SplashSystem::emitSurface()` dựng hai vector tiếp tuyến vuông góc với pháp
tuyến, kết hợp phản xạ đã giảm năng lượng và hướng ngẫu nhiên. Hạt dùng lại
slot có `lifetime <= 0`; nếu pool không đủ chỗ cho cả nhóm, yêu cầu bị từ chối
và được đếm trong `rejected()`. Splash mái không bị lọc xác suất như splash đất.

`SplashSystem::update()` dùng bước không quá 1/120 giây, cập nhật quỹ đạo dưới
trọng lực và kiểm tra đoạn chuyển động với nhà. Hạt rơi lại mái, chạm vật thể
khác của nhà, xuống đất hoặc hết lifetime sẽ kết thúc; không phát splash con.
`surfaceReturns()` đếm số hạt bị kết thúc bởi va chạm với nhà.

`RainSystem::updateStreak()` giới hạn chiều dài đuôi theo bề mặt. Rain shader
hiện có co giãn vệt theo khoảng cách camera, nên phép cắt tính theo hệ số tối
đa 1.12. Hệ số này phải đồng bộ với `shaders/effects.vert` nếu chỉnh shader.
Renderer cũng cắt đuôi splash để hạt vừa sinh không có đoạn nằm trong mái.

## Công thức toán

Với tam giác ABC, đặt `E1 = B-A`, `E2 = C-A`:

```text
N = normalize(cross(E1,E2))
D = -dot(N,A)
dot(N,P) + D = 0
```

Thuật toán Möller–Trumbore áp dụng cho đoạn `P(t)=S+tQ`, `Q=E-S`:

```text
H   = cross(Q,E2)
det = dot(E1,H)
T   = S-A
u   = dot(T,H)/det
K   = cross(T,E1)
v   = dot(Q,K)/det
t   = dot(E2,K)/det
```

Loại đoạn song song khi `abs(det) <= 1e-7 * length(E1)*length(E2)*length(Q)`.
Giao điểm hợp lệ khi `u >= -EPSILON`, `v >= -EPSILON`,
`u+v <= 1+EPSILON`, `-EPSILON <= t <= 1+EPSILON`; sau đó clamp `t` về [0,1].
Giữ `t` nhỏ nhất trong tất cả tam giác. Cách này xử lý cả chuyển động xiên và
đoạn đi qua toàn bộ mái giữa hai frame, không dùng một độ cao mái cố định.

Vận tốc splash ban đầu kết hợp pháp tuyến N và hai tiếp tuyến U, W:

```text
R       = V - 2*dot(V,N)*N
Vsplash = 0.06*R + a*N + b*(cos(theta)*U + sin(theta)*W)
Psplash = Phit + 0.002*N
```

Nếu thành phần hướng ra ngoài quá nhỏ, cộng thêm N để đạt tối thiểu 0.45.
Trên mái có `N.y > 0.5`, bảo đảm `Vsplash.y >= 0.15` để hạt bắn lên nhẹ.
Với `g=(0,-9.81,0)`, chuyển động và độ trong suốt là:

```text
Pnext = P + V*dt + 0.5*g*dt*dt
Vnext = V + g*dt
life  = max(0,life-dt)
alpha = 0.85*clamp(life/initialLifetime,0,1)
```

## Thông số điều chỉnh

Các hằng số splash mái tập trung trong `include/SplashSystem.hpp`:

| Thông số | Giá trị hiện tại |
|---|---|
| `MIN_ROOF_PARTICLES`, `MAX_ROOF_PARTICLES` | 3–6 hạt mỗi impact |
| `MIN_LIFETIME`, `MAX_LIFETIME` | 0.12–0.30 giây |
| `MIN_SIZE`, `MAX_SIZE` | Chiều dài GL_LINES 0.015–0.035 đơn vị |
| `MIN_NORMAL_SPEED`, `MAX_NORMAL_SPEED` | 0.45–1.05 đơn vị/giây, trước khi cộng phản xạ |
| `MIN_TANGENT_SPEED`, `MAX_TANGENT_SPEED` | 0.25–0.90 đơn vị/giây |
| `REFLECTION_RETENTION` | 0.06 |
| `INITIAL_ALPHA` | 0.85 |
| `GRAVITY` | 9.81 đơn vị/giây² |
| `MAX_STEP` | 1/120 giây |
| `SplashSystem::MAX_SPLASHES` | 512 slot dùng chung với splash đất |

`include/HouseCollider.hpp` chứa `EPSILON=1e-5`, `SURFACE_OFFSET=0.002` và
`MAX_RAIN_STREAK_SCALE=1.12`. Màu splash trong Renderer là `(0.65,0.83,0.95)`,
được nhân ánh sáng hạt hiện có trong shader để phản ứng với ngày/đêm và chớp.
Opaque được vẽ trước particle; giữ depth test, tắt depth writing trong pass
particle và bật blending `SRC_ALPHA, ONE_MINUS_SRC_ALPHA`, rồi khôi phục trạng thái.

## Build và kiểm thử

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 4
ctest --test-dir build --output-on-failure
./build/rain_thunderstorm --smoke-test \
  --capture build/reports/rain-collision.ppm \
  --capture-views build/reports/rain-collision-views
./build/rain_thunderstorm --benchmark 12
```

Đã build thành công bằng CMake hiện có; 6/6 CTest đạt. Bộ 5 kiểm thử cũ vẫn
kiểm tra weather, geometry/camera, models, environment và lighting/time.
Test mới chạy đủ 120 giây mô phỏng với 1.000 giọt, gió mạnh đổi chiều và sét tự
động: 13.237 va chạm vào nhà, splash tối đa 442/512. Mỗi bước kiểm tra không
còn giọt sống hoặc rain streak cắt qua hình học nhà, pool không đổi địa chỉ
và toàn bộ splash biến mất sau khi tắt phát mưa.

| Trường hợp | Kết quả và cách kiểm tra |
|---|---|
| Hai mặt nghiêng | Đạt giao điểm và pháp tuyến trên cả hai mái |
| Đỉnh mái | Đạt tại sống mái cao 4.26 và sát hai bên cap |
| Mép mái, phần nhô và đường nối tam giác | Đạt tại eave, biên trước/sau và đường chéo chung |
| Gió mạnh | Đạt đoạn xiên, va chạm mặt bên ống khói và gió đổi chiều trong chạy dài |
| Vận tốc lớn, deltaTime bất thường | Đạt đoạn từ Y=1000 đến -1000; dt=10 được clamp theo cơ chế hiện có |
| Ngoài phạm vi nhà | Đạt, vẫn phát ground splash/ripple theo hệ thống cũ |
| Ống khói | Đạt tại viền 4.77, lòng tối 4.60 và mặt bên |
| Splash rơi lại mái | Đạt: `surfaceReturns()>0`, hạt kết thúc và không phát hạt con |
| Nhiều góc camera | Đạt smoke tại các preset và hai vị trí riêng quan sát từng mái |
| Mưa dày và giới hạn số lượng | Đạt chạy 120 giây, stress 3.000 yêu cầu splash và kiểm tra pool thoát bão hòa |
| Ban đêm và sấm chớp | Đạt phép so pixel splash riêng trên mỗi mái trong ngày/đêm và chớp |
| Quản lý bộ nhớ | ASan/UBSan/LSan: 6/6 CTest đạt ngoài sandbox; LSan trong sandbox bị ptrace chặn |

Smoke OpenGL 4.6 Core, Mesa 26.0.8, zink/NVK, RTX 3050 Laptop, 800×600 đã đạt.
Fixture splash mái đóng góp 25 pixel phía trái và 27 pixel phía phải ở cả ngày,
đêm và chớp; fixture đặt impact có kiểm soát để tách khỏi splash đất. Smoke cũng
xác nhận mưa, mây, sét, ripple, sương mù, wetness, input handlers, pause/reset,
resize, depth/blending và cleanup; không ghi nhận lỗi OpenGL.
Ảnh PPM nằm trong `build/reports/rain-collision-views/roof-*.ppm`.

## Hiệu năng đo thực tế

Cùng cấu hình Debug, 800×600, timer 16 ms, RTX 3050 Laptop qua Mesa/zink:

| Phép đo | Trước thay đổi 12 giây | Bản cuối 12 giây | Bản cuối đo lại 30 giây |
|---|---:|---:|---:|
| FPS trung bình | 61.85 | 62.07 | 62.11 |
| P95 frame time | 16.87 ms | 17.15 ms | 17.16 ms |
| P99 frame time | 17.05 ms | 17.65 ms | 17.57 ms |
| CPU một core | 5.70% | 14.10% | 14.36% |
| Thời gian submit/render CPU | 0.381 ms | 0.592 ms | 0.596 ms |
| GPU command time | 0.484 ms | 0.518 ms | 0.569 ms |
| RSS cuối | 94.69 MiB | 94.96 MiB | 95.25 MiB |
| Số hạt mưa | 1000 | 1000 | 1000 |
| Splash đạt đỉnh | 307/512 | 412/512 | 399/512 |

Lượt đo lại 30 giây có 1.864 frame, bảy đợt sét, không có yêu cầu splash bị từ
chối và không có mẫu pool đầy trong 29 lần lấy mẫu. Kết quả phù hợp mục tiêu
gần 60 FPS trên máy này; timer đang giới hạn nên không suy ra FPS tối đa hoặc
khẳng định tăng tốc. Chi phí CPU tăng do kiểm tra hình học và số splash tăng.

Một lượt 30 giây trước đó ghi nhận 50.06 FPS trung bình, P99 17.71 ms, CPU 11.45%
và RSS cuối 95.10 MiB. Chưa xác định nguyên nhân các frame kéo dài trong lượt
đó; lượt đo lại riêng ở bảng trên không tái hiện mức FPS này. Các lượt ngắn
không thay thế phép đo nhiều giờ hoặc xác nhận trên các GPU khác.

Kiểm thử này chưa thay thế thao tác bàn phím/chuột thật từ mọi góc tùy ý,
chạy nhiều giờ, đo VRAM hoặc thử trên GPU khác. Sanitizer kiểm tra bộ nhớ CPU;
không kết luận toàn bộ driver/GPU không có leak. Collider mới áp dụng cho nhà;
cây và đá vẫn giữ hành vi cũ.
