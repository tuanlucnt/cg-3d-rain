# 04 — Công thức toán học trong implementation

[Tổng quan](01_PROJECT_OVERVIEW.md) · [Thuật toán](03_GRAPHICS_ALGORITHMS.md)

Ba mươi nhóm công thức dưới đây có nguồn cụ thể. Ký hiệu trình bày toán học có thể đổi tên biến cho dễ học nhưng hệ số/thứ tự bám code. Các ví dụ làm tròn để minh họa, không phải số benchmark. Số world-unit không được project gắn hệ đo SI bắt buộc. Công thức thống kê hiệu năng đặt riêng tại 07.

## Mục lục

- [F01 — Cộng/trừ vector và khoảng cách](#m01)

- [F02 — Dot và cross product](#m02)

- [F03 — Chuẩn hóa và tốc độ đường chéo](#m03)

- [F04 — Ma trận model và vector cột](#m04)

- [F05 — View matrix lookAt](#m05)

- [F06 — Perspective projection và chia w](#m06)

- [F07 — Hướng camera yaw/pitch](#m07)

- [F08 — Normal Matrix](#m08)

- [F09 — Sway và Jacobian](#m09)

- [F10 — mix, clamp, smoothstep](#m10)

- [F11 — Procedural material và grid](#m11)

- [F12 — Hình cầu và ống tròn](#m12)

- [F13 — Outline và transform nước](#m13)

- [F14 — Gaussian cloud field, gradient và bisection](#m14)

- [F15 — Advection, wrap và coverage mây](#m15)

- [F16 — Fixed timestep và dt clamp](#m16)

- [F17 — Euler bán ẩn, gió và terminal velocity](#m17)

- [F18 — Rain streak và fade theo camera](#m18)

- [F19 — Giao đoạn với plane Y 0](#m19)

- [F20 — Ray crossing và khoảng cách cạnh polygon](#m20)

- [F21 — Splash lifetime và alpha](#m21)

- [F22 — Ripple radius và alpha decay](#m22)

- [F23 — Midpoint displacement, constraints và taper](#m23)

- [F24 — Envelope flash và pulse gaps](#m24)

- [F25 — Ambient, Lambert, point attenuation và warm lamp](#m25)

- [F26 — Blinn–Phong specular](#m26)

- [F27 — Schlick Fresnel và reflection direction](#m27)

- [F28 — Exponential fog và sky gradient](#m28)

- [F29 — Wetness, moisture, path và contact](#m29)

- [F30 — Clock, daylight và màu trời](#m30)


<a id="m01"></a>

## F01 — Cộng/trừ vector và khoảng cách

$$
\mathbf d=\mathbf b-\mathbf a,\quad\mathbf p\leftarrow\mathbf p+\mathbf v\Delta t,\quad d=\|\mathbf b-\mathbf a\|_2
$$

**Ký hiệu và ý nghĩa:** a, b, p là vị trí; v vận tốc; Δt giây, d khoảng cách world-unit.

**Ví dụ:** a=(1, 2, 3), b=(4, 2, 7) ⇒ b−a=(3, 0, 4), d=5. v=(0, −10, 0), dt=.01 làm y giảm .1.

**Nguồn:** [Camera::move: src/Camera.cpp:41](../src/Camera.cpp#L41), [CloudSystem::update: src/CloudSystem.cpp:34](../src/CloudSystem.cpp#L34), [RainSystem::update: src/RainSystem.cpp:28](../src/RainSystem.cpp#L28).

**Điều kiện/giới hạn:** Không dùng distance trong clip-space. World-unit do scene quy ước, không bảo đảm tỷ lệ vật lý thực.


<a id="m02"></a>

## F02 — Dot và cross product

$$
\mathbf a\cdot\mathbf b=\sum_i a_ib_i,\quad \mathbf N_f=(\mathbf b-\mathbf a)\times(\mathbf c-\mathbf a)
$$

**Ký hiệu và ý nghĩa:** Dot đo góc khi vector đơn vị; cross cho pháp tuyến có hướng, độ dài bằng hai lần diện tích tam giác.

**Ví dụ:** a=(0, 0, 0), b=(0, 0, 1), c=(1, 0, 0) ⇒ cross=(0, 1, 0), mặt hướng +Y. Dot của +Y với +Y=1.

**Nguồn:** [triangle: src/Geometry.cpp:8](../src/Geometry.cpp#L8), [tri: src/ModelGeometry.cpp:10](../src/ModelGeometry.cpp#L10), [Renderer::draw::lambda@180: src/Renderer.cpp:180](../src/Renderer.cpp#L180), [main: shaders/object.frag:65](../shaders/object.frag#L65).

**Điều kiện/giới hạn:** Đổi thứ tự b/c đảo normal. Dot projected depth của mây có dấu, khác Euclidean distance.


<a id="m03"></a>

## F03 — Chuẩn hóa và tốc độ đường chéo

$$
\widehat{\mathbf v}=\frac{\mathbf v}{\|\mathbf v\|},\quad \Delta\mathbf p=\widehat{r d_x+u d_y+f d_z}\,s\Delta t
$$

**Ký hiệu và ý nghĩa:** r=right=normalize(f×up), u=world up, f=forward, s=5.

**Ví dụ:** Hướng local(1, 0, 1) không bị tăng tốc√2; dt=.1 cho displacement length=.5 nếu chưa clamp.

**Nguồn:** [Camera::move: src/Camera.cpp:41](../src/Camera.cpp#L41), [Camera::updateDirection: src/Camera.cpp:27](../src/Camera.cpp#L27), [LightningSystem::subdivide: src/LightningSystem.cpp:26](../src/LightningSystem.cpp#L26).

**Điều kiện/giới hạn:** Zero vector không normalize an toàn; Camera có ngưỡng 1e−6 sau ghép, shader dùng max(length, .001) một số chỗ.


<a id="m04"></a>

## F04 — Ma trận model và vector cột

$$
M=T(\mathbf p)R_y(\theta)S(\mathbf s),\quad \mathbf p_w=M\begin{bmatrix}\mathbf p_l\\1\end{bmatrix}
$$

**Ký hiệu và ý nghĩa:** p_l local, p_w world. GLM nhân vector cột; thao tác bên phải áp trước: scale→rotate→translate.

**Ví dụ:** p=(1, 0, 0), scale(2, 1, 1), yaw 90°, translate(1, 0, 0) ⇒ world(1, 0, −2), theo Ry của GLM.

**Nguồn:** [Scene::add: src/Scene.cpp:61](../src/Scene.cpp#L61), [box: src/ModelGeometry.cpp:18](../src/ModelGeometry.cpp#L18).

**Điều kiện/giới hạn:** CPU upload GL_FALSE không transpose. Scale 0 làm inverse normal suy biến; caller hiện không dùng scale 0.


<a id="m05"></a>

## F05 — View matrix lookAt

$$
r=\widehat{f\times up},\quad u=r\times f,\quad p_e=(r\cdot(p-eye),u\cdot(p-eye),-f\cdot(p-eye))
$$

**Ký hiệu và ý nghĩa:** f=forward đơn vị; eye vị trí camera. lookAt biến world→eye với nhìn về−Z.

**Ví dụ:** eye được ánh xạ thành(0, 0, 0); eye+forward thành(0, 0, −1), đúng assertion core_tests.

**Nguồn:** [Camera::view: src/Camera.cpp:55](../src/Camera.cpp#L55), [cameraTests: tests/core_tests.cpp:90](../tests/core_tests.cpp#L90).

**Điều kiện/giới hạn:** f không song song up; pitch clamp±89° tránh suy biến. Không có roll.


<a id="m06"></a>

## F06 — Perspective projection và chia w

$$
f_c=\cot(55^\circ/2),\quad P=\begin{bmatrix}f_c/a&0&0&0\\0&f_c&0&0\\0&0&-(F+n)/(F-n)&-2Fn/(F-n)\\0&0&-1&0\end{bmatrix},\quad p_{ndc}=p_{clip}.xyz/p_{clip}.w
$$

**Ký hiệu và ý nghĩa:** a=aspect width/height, n=.1, F=80; ma trận hiển thị theo hàng, GLM lưu column-major. clip=PVMp.

**Ví dụ:** f_c≈1.921; aspect 4/3 ⇒ P00≈1.441. Với eye z=−10 thì wclip=10, nên điểm xa nhỏ hơn sau chia.

**Nguồn:** [Camera::projection: src/Camera.cpp:59](../src/Camera.cpp#L59), [main: shaders/object.vert:18](../shaders/object.vert#L18), [Application::smokeStep::sample@545: src/Application.cpp:545](../src/Application.cpp#L545).

**Điều kiện/giới hạn:** Công thức GLM OpenGL RH mặc định của build, không zero-to-one define. Depth không tuyến tính theo khoảng cách; precision phụ thuộc near/far.


<a id="m07"></a>

## F07 — Hướng camera yaw/pitch

$$
f=\widehat{(\cos\psi\cos\phi,\sin\phi,\sin\psi\cos\phi)},\quad\psi\leftarrow\operatorname{remainder}(\psi+.12\Delta x,360),\quad\phi\leftarrow\operatorname{clamp}(\phi-.12\Delta y,-89,89)
$$

**Ký hiệu và ý nghĩa:** ψ yaw, φ pitch theo độ ở state, dùng radian trong sin/cos; delta chuột pixel.

**Ví dụ:** yaw 0, pitch 0⇒+X. Kéo ngang 100 pixel⇒yaw tăng 12°. Kéo dọc xuống 100 pixel⇒pitch giảm 12°.

**Nguồn:** [Camera::updateDirection: src/Camera.cpp:27](../src/Camera.cpp#L27), [Camera::rotate: src/Camera.cpp:35](../src/Camera.cpp#L35), [Camera::preset: src/Camera.cpp:16](../src/Camera.cpp#L16).

**Điều kiện/giới hạn:** preset suy góc bằng atan2(z, x) và asin(y); remainder có thể trả góc âm, đó là biểu diễn hợp lệ.


<a id="m08"></a>

## F08 — Normal Matrix

$$
N=\left(M_{3\times3}^{-1}\right)^T,\quad n_w=\widehat{N n_l}
$$

**Ký hiệu và ý nghĩa:** Normal phải vẫn vuông góc tangent sau scale không đều. CPU Scene::add tính N; object.vert truyền, fragment normalize.

**Ví dụ:** S=diag(2, 1, 1), normal local(1, 1, 0)/√2 ⇒ sau inverse transpose và normalize là(1, 2, 0)/√5; nhân trực tiếp S sẽ cho hướng sai.

**Nguồn:** [Scene::add: src/Scene.cpp:61](../src/Scene.cpp#L61), [box: src/ModelGeometry.cpp:18](../src/ModelGeometry.cpp#L18), [main: shaders/object.vert:18](../shaders/object.vert#L18), [main: shaders/object.frag:65](../shaders/object.frag#L65).

**Điều kiện/giới hạn:** Không áp translation lên normal. Với model translation-only cloud, N=identity; cây còn sửa normal bằng Jacobian bend trước N.


<a id="m09"></a>

## F09 — Sway và Jacobian

$$
h=\operatorname{clamp}(y/H,0,1),\quad A=w_{xz}\,.05\sin(.9t+phase),\quad p_{xz}^{\prime}=p_{xz}+Ah^2,\quad n_y^{\prime}=n_y-n_{xz}\cdot(2Ah/H)
$$

**Ký hiệu và ý nghĩa:** H height, t animationTime giây, w wind, phase=.37 model X+.29 modelZ.

**Ví dụ:** H=4, y=2, h=.5, w=(1, 0), sin=1⇒A=(.05, 0), dịch x=.0125; gốc y 0 dịch 0.

**Nguồn:** [main: shaders/object.vert:18](../shaders/object.vert#L18), [Scene::update: src/Scene.cpp:75](../src/Scene.cpp#L75).

**Điều kiện/giới hạn:** Normal correction là inverse-transpose Jacobian, chính xác cho shear bend này trong miền chưa clamp h; không phải mô phỏng đàn hồi cây.


<a id="m10"></a>

## F10 — mix, clamp, smoothstep

$$
\operatorname{mix}(a,b,t)=(1-t)a+tb,\quad u=\operatorname{clamp}((x-a)/(b-a),0,1),\quad S(a,b,x)=u^2(3-2u)
$$

**Ký hiệu và ý nghĩa:** mix dùng scalar/vector; smoothstep có đạo hàm 0 hai đầu; clamp chặn biên.

**Ví dụ:** S(5, 7, 6)=.5; mix((0, 0, 0), (1, 1, 1), .5)=(.5, .5, .5). Cloud edge 1.25 ⇒coverage=.5.

**Nguồn:** [LightingSystem::setEnvironment::smooth@18: src/LightingSystem.cpp:18](../src/LightingSystem.cpp#L18), [CloudSystem::coverage: src/CloudSystem.cpp:44](../src/CloudSystem.cpp#L44), [LightingSystem::setEnvironment: src/LightingSystem.cpp:14](../src/LightingSystem.cpp#L14), [main: shaders/sky.frag:6](../shaders/sky.frag#L6).

**Điều kiện/giới hạn:** Các mốc smoothstep b>a trong code. Đây là interpolation thẩm mỹ; không mặc định là hàm thời tiết vật lý.


<a id="m11"></a>

## F11 — Procedural material và grid

$$
grid_x\leftarrow grid_x+.5\operatorname{mod}(\lfloor grid_y\rfloor,2),\quad cell=\operatorname{fract}(grid),\quad aa=\max(\operatorname{fwidth}(grid),.015)
$$

**Ký hiệu và ý nghĩa:** Grid local theo loại vật liệu; fwidth≈|dFdx|+|dFdy| phục vụ anti-alias mép grout.

**Ví dụ:** Hàng y 1.2 có row 1 nên x được cộng.5; y 2.2 row 2 không lệch. aa tăng khi grid dày trên màn hình làm mép mềm hơn.

**Nguồn:** [main: shaders/object.frag:65](../shaders/object.frag#L65).

**Điều kiện/giới hạn:** Fwidth là fragment derivative trên GPU, không texture filtering hoặc MSAA tự viết. Plaster/wood/rock dùng sin/height khác, xem shader 05.


<a id="m12"></a>

## F12 — Hình cầu và ống tròn

$$
p_{sphere}=(\sin\phi\cos\theta,\cos\phi,\sin\phi\sin\theta),\quad p_{ring}=a+r(u\cos\theta+w\sin\theta)
$$

**Ký hiệu và ý nghĩa:** φ=πrow/stacks, θ=2πcol/slices; u, w là basis vuông góc axis ống.

**Ví dụ:** Sphere slices 10/stacks 6 ⇒2×10×5=100 triangles. Tube 8 sector⇒32 triangles kể cả 2 cap.

**Nguồn:** [sphere: src/Geometry.cpp:98](../src/Geometry.cpp#L98), [sphere::point@103: src/Geometry.cpp:103](../src/Geometry.cpp#L103), [tube: src/ModelGeometry.cpp:31](../src/ModelGeometry.cpp#L31).

**Điều kiện/giới hạn:** Flat normals, độ phân giải thấp; sphere primitive không phải mây render hiện tại. Cone 2n triangles nếu apex chung.



### Thông số cây, đá và cỏ cũng được tính bằng công thức CPU

Nguồn [models::tree:140](../src/ModelGeometry.cpp#L140), [environment::rock:88](../src/EnvironmentGeometry.cpp#L88), [grass:50](../src/EnvironmentGeometry.cpp#L50). Đây là công thức tạo hình có kiểm soát, không mô hình sinh học/địa chất.

Với cây, nhánh thứ i dùng:
$$
t_i=.16+.66i/(b-1),\quad a_i=phase+2.3999632i+U(-.12,.12),\quad L_i=C(1-t_i).9+.10.
$$
Root=(0,t_i H,0), tip=(cos(a_i)L_i,t_i H+.4L_i,sin(a_i)L_i). H là height, C là canopyWidth, b là branchCount. Ví dụ b=8,H=4,C=1.25,i=0 cho t=.16,L=1.045,rootY=.64 và tipY≈1.058.

Tán cây có 12 tầng với normalized heights [.22,.29,.34,.39,.45,.51,.56,.64,.71,.77,.84,.93] và widths [.70,1,.90,.78,.81,.72,.60,.60,.48,.40,.32,.14]. Mỗi tầng k có 10 sector:
$$
a=phase+2\pi i/10+.035\sin(1.7k+phase),
$$
$$
jitter=\operatorname{clamp}(sectorScale_i[1+.10\sin(3a+phase+1.4k)],.78,1.18),\quad r=C\,width_k\,jitter.
$$
CenterXZ=(.06sin(phase+3t),.05cos(phase+4t)); vertexY=Ht+.05H(1−t)sin(2a+phase+.6k). Các đỉnh cap đáy và tip khép kín tán. Không phải ghép nhiều cone rời; thay seed làm sectorScale/phase khác, không đổi số sector/tầng.

Đá dùng ba ring heights [−.55,−.04,.54], widths [.65,1,.67]; angle=a_i+.045k và radius=baseRadius_i×width_k×U(.94,1.06). LeanXZ nhân .4k,jitterY±.08 trừ tầng đáy. Đặt min localY=−.55; ví dụ rock scaleY=.6 và translateY=.33 cho min worldY=−.55×.6+.33=0.

Cỏ lấy angle=tuftAngle+blade×2π/3,side=(.045cos(angle),0,.045sin(angle)),tip offsetXZ=.6sideXZ và height=tuftHeight(1−.12blade). Blade0..2 có ba hướng và độ cao khác nhau,mỗi lá hai triangle đảo winding. 64×3×2=384 triangles.


<a id="m13"></a>

## F13 — Outline và transform nước

$$
r(a)=1+.10\sin3a+.055\cos5a+.035\sin(7a+.6),\quad q=c+\begin{bmatrix}\cos\theta&\sin\theta\\-\sin\theta&\cos\theta\end{bmatrix}(s\odot(r(a)\cos a,r(a)\sin a))
$$

**Ký hiệu và ý nghĩa:** a góc outline radian, θ yaw radian, c center XZ, s scale XZ; ⊙ nhân từng thành phần.

**Ví dụ:** a 0 ⇒r≈1+.055+.035 sin.6≈1.07476. Với s=(1.8, 1), trước quay điểm x≈1.93457.

**Nguồn:** [outlineRadius: src/Surface.cpp:15](../src/Surface.cpp#L15), [polygon::polygons@35: src/Surface.cpp:35](../src/Surface.cpp#L35), [waterInterior: shaders/object.frag:50](../shaders/object.frag#L50).

**Điều kiện/giới hạn:** Mesh 32 sample; shader tính radius trơn chỉ để blend màu bờ, không thay polygon thật. yaw dữ liệu degrees được radians trước dùng.


<a id="m14"></a>

## F14 — Gaussian cloud field, gradient và bisection

$$
D(p)=\sum_{i=1}^{5}e^{-2\|q_i\|^2},\quad q_i=(p-o_i)/s_i,\quad n\propto-\nabla D=4\sum_i e^{-2\|q_i\|^2}(p-o_i)/(s_i^2)
$$

**Ký hiệu và ý nghĩa:** s_i là lobe scale với Y×1.16. Tìm D(ray·radius)=.18, radius∈[0, 6].

**Ví dụ:** Một lobe đơn tại gốc, scale 1 ⇒r=√(−ln.18/2)≈.926.24 bisection cho khoảng radius≤6/2²⁴≈3.58e−7 trước sai số float.

**Nguồn:** [cloud: src/EnvironmentGeometry.cpp:121](../src/EnvironmentGeometry.cpp#L121), [cloud::field@129: src/EnvironmentGeometry.cpp:129](../src/EnvironmentGeometry.cpp#L129), [cloud::add@161: src/EnvironmentGeometry.cpp:161](../src/EnvironmentGeometry.cpp#L161).

**Điều kiện/giới hạn:** Thực tế tổng 5 lobe nên không dùng công thức đơn lobe để tìm skin. Bisection cần crossing và dạng star-shaped; float có thể giới hạn chính xác trước 24 vòng. Normal bỏ factor 4 vì chuẩn hóa.


<a id="m15"></a>

## F15 — Advection, wrap và coverage mây

$$
p\leftarrow p+(w_x,0,w_z)s\Delta t,\quad wrap(x)=x-20\lfloor(x+10)/20\rfloor,\quad coverage=S(0,1,\operatorname{clamp}((10-\max(|x|,|z|))/2.5,0,1))
$$

**Ký hiệu và ý nghĩa:** s cluster speed=.20+.025i; wrap ở [−10, 10), coverage theo tâm cả cloud.

**Ví dụ:** wind x=.65, s=.2, dt=.1⇒Δx=.013; wrap 10.2=−9.8; center x 8.75, z0⇒coverage=.5.

**Nguồn:** [CloudSystem::update: src/CloudSystem.cpp:34](../src/CloudSystem.cpp#L34), [wrap: src/CloudSystem.cpp:7](../src/CloudSystem.cpp#L7), [CloudSystem::coverage: src/CloudSystem.cpp:44](../src/CloudSystem.cpp#L44).

**Điều kiện/giới hạn:** Không bảo toàn density vật lý; fade che periodic seam bằng alpha. Toàn skin có thể extends ngoài 20×20 vì chỉ tâm bị wrap.


<a id="m16"></a>

## F16 — Fixed timestep và dt clamp

$$
a\leftarrow a+\min(\Delta t,.1),\quad h=1/120,\quad a+10^{-7}\ge h\Rightarrow update(h),\ a\leftarrow\max(0,a-h)
$$

**Ký hiệu và ý nghĩa:** a accumulator double, h giây; Application lấy dt từ steady_clock, clamp[0, .1].

**Ví dụ:** dt=.1⇒khoảng 12 substep; dt=.016⇒1 step và dư≈.007667, phần dư mang qua tick sau. Stall 10 giây vẫn chỉ cộng.1.

**Nguồn:** [Application::tick: src/Application.cpp:170](../src/Application.cpp#L170), [WeatherSystem::update: src/WeatherSystem.cpp:30](../src/WeatherSystem.cpp#L30).

**Điều kiện/giới hạn:** Giảm phụ thuộc FPS cho cùng thời gian mô phỏng được chấp nhận; không bảo đảm thời gian thực khi stall. Không nội suy render giữa hai state.


<a id="m17"></a>

## F17 — Euler bán ẩn, gió và terminal velocity

$$
v_y\leftarrow\max(v_y-9.81h,-17),\quad v_{xz}\leftarrow v_{xz}+(w_{xz}-v_{xz})\min(2h,1),\quad p\leftarrow p+vh
$$

**Ký hiệu và ý nghĩa:** h giây; cập nhật velocity trước position. Gió là relaxation rate≈2/s.

**Ví dụ:** vy=−8, h=1/120⇒vy≈−8.08175 và Δy≈−.067348; vx0, wind x.65⇒vx≈.010833.

**Nguồn:** [RainSystem::update: src/RainSystem.cpp:28](../src/RainSystem.cpp#L28), [SplashSystem::update: src/SplashSystem.cpp:38](../src/SplashSystem.cpp#L38).

**Điều kiện/giới hạn:** Mô hình gravity numeric đơn giản và hard terminal cap, không phương trình drag phi tuyến. Semi-implicit Euler có sai số toàn cục O(h) cho mô hình trơn; clamp/recycle là gián đoạn.


<a id="m18"></a>

## F18 — Rain streak và fade theo camera

$$
\ell=\operatorname{clamp}(|v|\,.024s_r,.12,.42),\quad tail=head-\widehat v\ell\operatorname{mix}(1.12,.62,S(4,30,d))
$$

**Ký hiệu và ý nghĩa:** s_r streakScale random.85..1.15; d từ head đến camera. Alpha=opacity×.78×S(.25, 1.4, d)×mix(1, .28, S(4, 32, d)).

**Ví dụ:** speed 10, s_r1⇒length.24; gần d≤4 (ngoài near fade)⇒length GPU.2688. Xa d≥32⇒length.1488 và alpha multiplier.78×.28=.2184.

**Nguồn:** [RainSystem::respawn: src/RainSystem.cpp:10](../src/RainSystem.cpp#L10), [RainSystem::update: src/RainSystem.cpp:28](../src/RainSystem.cpp#L28), [main: shaders/effects.vert:12](../shaders/effects.vert#L12).

**Điều kiện/giới hạn:** Length theo exposure .024s chỉ tạo vệt hình ảnh, không phải độ dài giọt. GL_LINES rộng 1 pixel; alpha tính vertex rồi nội suy, không per-fragment sort.


<a id="m19"></a>

## F19 — Giao đoạn với plane Y 0

$$
t=\frac{y_0}{y_0-y_1},\quad hit=(1-t)p_0+tp_1
$$

**Ký hiệu và ý nghĩa:** p0 previous, p1 updated; chỉ chạy khi p1.y≤0 và previous đang phía trên.

**Ví dụ:** p0=(1, .1, 2), p1=(1.2, −.1, 2)⇒t=.5, hit=(1.1, 0, 2).

**Nguồn:** [RainSystem::update: src/RainSystem.cpp:28](../src/RainSystem.cpp#L28).

**Điều kiện/giới hạn:** Nội suy theo đoạn Euler, không nghiệm parabol chính xác. XZ ngoài±10 không phát impact; rain ra biên mà chưa chạm đất bị recycle.


<a id="m20"></a>

## F20 — Ray crossing và khoảng cách cạnh polygon

$$
t_e=\operatorname{clamp}\frac{(p-a)\cdot e}{e\cdot e},\quad m=\min_{edges}\|p-(a+t_e e)\|
$$

**Ký hiệu và ý nghĩa:** e=b−a; ray ngang qua từng cạnh đổi parity inside, chỉ trả m khi inside.

**Ví dụ:** p=(.5, .2), segment(0, 0)→(1, 0):t=.5, distance=.2. Nếu ngoài polygon API trả 0 bất kể khoảng cách cạnh.

**Nguồn:** [waterMargin: src/Surface.cpp:52](../src/Surface.cpp#L52), [waterMargin::bounds@53: src/Surface.cpp:53](../src/Surface.cpp#L53).

**Điều kiện/giới hạn:** AABB reject nhanh; không finite validation đầu vào ở waterMargin, callers dùng hits hợp lệ. Shared polygon không có cạnh zero-length. Không dùng circle containment.


<a id="m21"></a>

## F21 — Splash lifetime và alpha

$$
life\leftarrow life-h,\quad\alpha=.85\frac{life}{initialLife},\quad v_0=(s\cos a,v_y,s\sin a)
$$

**Ký hiệu và ý nghĩa:** a∈[0, 2π), s.5..1.4, vy1.2..2.4, initialLife.16...28; 2 hạt/request accepted.

**Ví dụ:** initial.2, remaining.1⇒alpha.425. Vị trí y=.02 lúc emit; nếu y≤.01 hoặc remaining≤0 thì chết.

**Nguồn:** [SplashSystem::emit: src/SplashSystem.cpp:18](../src/SplashSystem.cpp#L18), [SplashSystem::update: src/SplashSystem.cpp:38](../src/SplashSystem.cpp#L38), [Renderer::draw: src/Renderer.cpp:84](../src/Renderer.cpp#L84).

**Điều kiện/giới hạn:** Sampling 45% bỏ request trước allocation; requests/filtered/rejected không cùng đơn vị với số hạt. Lifetime đếm lùi; hạn chế visual line.


<a id="m22"></a>

## F22 — Ripple radius và alpha decay

$$
t=\operatorname{clamp}(elapsed/lifetime,0,1),\quad r=(1-t).04+tr_{max},\quad\alpha=.5(1-t)^2,\quad r_{max}=\min(.45,.9m)
$$

**Ký hiệu và ý nghĩa:** lifetime=.55s là tổng đời, m waterMargin khi trong nước; trên đất rmax=.45.

**Ví dụ:** Tại elapsed.275⇒t=.5, r=.245 nếu rmax.45, alpha.125. Margin.2⇒rmax.18, vòng đủ nằm trong polygon nếu centered inside.

**Nguồn:** [RippleSystem::emit: src/RippleSystem.cpp:13](../src/RippleSystem.cpp#L13), [RippleSystem::update: src/RippleSystem.cpp:27](../src/RippleSystem.cpp#L27), [Renderer::draw: src/Renderer.cpp:84](../src/Renderer.cpp#L84).

**Điều kiện/giới hạn:** Bỏ emission rmax<.08; ring 12 cạnh line. Đây là animation bán kính, không nghiệm phương trình sóng; ground emit không kiểm wetness.


<a id="m23"></a>

## F23 — Midpoint displacement, constraints và taper

$$
M=(A+B)/2+\delta,\quad\delta=uU(-d,d)+vU(-d,d),\quad\|\delta\|\le .12\|B-A\|,\quad d_{next}=.4d
$$

**Ký hiệu và ý nghĩa:** u/v basis vuông góc AB, offset còn scale chung để ở box; depth 5 trunk, 3 branch.

**Ví dụ:** AB dài 8⇒offset tối đa.96 trước giới hạn biên.5 cấp⇒32 đoạn; 6 nhánh×8=48; tổng 80. Trunk width cuối=.095×.65=.06175.

**Nguồn:** [LightningSystem::subdivide: src/LightningSystem.cpp:26](../src/LightningSystem.cpp#L26), [LightningSystem::trigger: src/LightningSystem.cpp:55](../src/LightningSystem.cpp#L55).

**Điều kiện/giới hạn:** Seed deterministic theo thứ tự RNG; đoạn không phải discharge simulation. Taper branch=.032(1−.6j/7), cuối.0128.


<a id="m24"></a>

## F24 — Envelope flash và pulse gaps

$$
I(\tau)=I_p e^{-32\tau}\operatorname{clamp}((D_p-\tau)/.012,0,1),\quad 0\le\tau<D_p
$$

**Ký hiệu và ý nghĩa:** τ elapsed từ pulse.start, Dp duration pulse, Ip peak. Ngoài interval hoặc inactive I0.

**Ví dụ:** Ip1, τ.02, remaining≥.012⇒I=e−.64≈.5273. Duration strike.3/3 pulse⇒interval.1; duration pulse .055...07 tạo dark gap .03...045.

**Nguồn:** [LightningSystem::flashIntensity: src/LightningSystem.cpp:115](../src/LightningSystem.cpp#L115), [LightningSystem::trigger: src/LightningSystem.cpp:55](../src/LightningSystem.cpp#L55), [LightningSystem::update: src/LightningSystem.cpp:102](../src/LightningSystem.cpp#L102).

**Điều kiện/giới hạn:** Decay thị giác, không radiance đo thực. active vẫn true trong dark gap; nextStrike chỉ đếm lúc inactive.


<a id="m25"></a>

## F25 — Ambient, Lambert, point attenuation và warm lamp

$$
L_d=\max(N\cdot\widehat L,0),\quad A(d)=\frac1{1+.025d+.018d^2},\quad C_{flash}=(.70,.80,1)I A(d)(.45+.65L_d)
$$

**Ký hiệu và ý nghĩa:** Ambient/directional RGB theo environment; flash L từ point đến bolt head. Warm strength=.20 windows/(1+.85 d²) và normal fill .35+max dot.

**Ví dụ:** N=(0, 1, 0), L=+Y⇒Lambert 1. d10⇒attenuation 1/3.05≈.32787; flash I=1 gives scalar multiplier 1.1×.32787≈.36066 trước RGB.

**Nguồn:** [main: shaders/object.frag:65](../shaders/object.frag#L65), [LightingSystem::setEnvironment: src/LightingSystem.cpp:14](../src/LightingSystem.cpp#L14), [LightingSystem::attenuationAt: src/LightingSystem.cpp:58](../src/LightingSystem.cpp#L58).

**Điều kiện/giới hạn:** Không shadow hoặc inverse-square thuần; fill .45 giúp mặt quay khỏi light vẫn sáng. Shader source direction dùng−uLightDirection; đèn ấm không phải emission của mọi cửa sổ tích phân.


<a id="m26"></a>

## F26 — Blinn–Phong specular

$$
H=\widehat{V+L},\quad S=\max(N\cdot H,0)^n,\quad S_{sum}\le .45
$$

**Ký hiệu và ý nghĩa:** V hướng tới camera, L hướng tới light, n shininess. Strength theo ground/water/material response+wetGloss; directional max RGB scale sunSpec.

**Ví dụ:** dot=.9, n32⇒S≈.03434; dot 1⇒1 nhưng specular kết hợp bị cap.45. Water n72 tạo highlight hẹp hơn n18.

**Nguồn:** [main: shaders/object.frag:65](../shaders/object.frag#L65).

**Điều kiện/giới hạn:** Blinn–Phong không physical BRDF; không PBR roughness. Normalize dùng max(length, .001) tránh chia cho 0. Ánh specular chưa multiply RGB directional đầy đủ mà dùng tint cố định.


<a id="m27"></a>

## F27 — Schlick Fresnel và reflection direction

$$
F=.02+.98(1-\max(N\cdot V,0))^5,\quad R=\operatorname{reflect}(-V,N),\quad C\leftarrow\operatorname{mix}(C,C_{sky}(R_y),interior\min(.65F,.65))
$$

**Ký hiệu và ý nghĩa:** F0=.02; N/V đơn vị, interior mask bờ. Sky color mix horizon/top theo smoothstep(0, .7, clamp(Ry, 0, 1)).

**Ví dụ:** N·V1⇒F.02, tỷ lệ .013 trong interior 1. N·V0⇒F1, tỷ lệ.65. N·V.5⇒F=.050625.

**Nguồn:** [main: shaders/object.frag:65](../shaders/object.frag#L65).

**Điều kiện/giới hạn:** Chỉ phản xạ màu trời ước lượng, không trace cảnh. Schlick ở đây được dùng thẩm mỹ và thêm clamp; không đủ để gọi toàn bộ shader là PBR.


<a id="m28"></a>

## F28 — Exponential fog và sky gradient

$$
T=e^{-\rho k\max(d-6,0)},\quad C_{out}=(1-T)C_{fog}+TC_{lit},\quad C_{sky}=\operatorname{mix}(horizon,top,S(0,1,UV_y))
$$

**Ký hiệu và ý nghĩa:** ρ fogDensity default.018; k=1 cho object, cloud.65, rain.35, bolt.2, splash/ripple 1.

**Ví dụ:** d26, k1⇒T=e−.36≈.69768; d≤6⇒T1; rho0⇒1. SkyUVy.5⇒mix đều hai màu.

**Nguồn:** [LightingSystem::fogFactor: src/LightingSystem.cpp:71](../src/LightingSystem.cpp#L71), [main: shaders/object.frag:65](../shaders/object.frag#L65), [main: shaders/effects.frag:13](../shaders/effects.frag#L13), [main: shaders/sky.frag:6](../shaders/sky.frag#L6).

**Điều kiện/giới hạn:** Transmission không phải alpha effect. Cloud fog color còn nhân.8; sky gradient theo screen-space, không hướng world hoặc atmospheric optical depth thật.


<a id="m29"></a>

## F29 — Wetness, moisture, path và contact

$$
W\leftarrow\operatorname{clamp}(W+\lambda\min(h,.1),0,1),\quad m(p)=\operatorname{clamp}(.5+.28\sin(.34x+.8\sin(.21z))+.18\cos(.47z-.12x),0,1)
$$

**Ký hiệu và ý nghĩa:** λ=.06 mưa hoặc−.012 khô. localWet=Wmix(.42, 1, S(.15, .88, m)); slope perturb(.018 cos(.9 x+.3z), .014 sin(.8z−.2 x)).

**Ví dụ:** W.65, dt.1, mưa⇒.656; để khô 10 giây accepted dt tổng ⇒.53. Contact shading tối đa khoảng.11 gần nhà; soil darkening mix 1→.72 theo wet/path.

**Nguồn:** [LightingSystem::update: src/LightingSystem.cpp:46](../src/LightingSystem.cpp#L46), [moistureField: shaders/object.frag:34](../shaders/object.frag#L34), [soilColor: shaders/object.frag:39](../shaders/object.frag#L39), [main: shaders/object.frag:65](../shaders/object.frag#L65).

**Điều kiện/giới hạn:** groundMultiplier CPU=1−.24W chỉ helper test, không chính xác soil shader hiện tại. Water extent không đổi theoW, không mô hình evaporation/infiltration.


<a id="m30"></a>

## F30 — Clock, daylight và màu trời

$$
h\leftarrow\operatorname{wrap}_{24}\left(h+24\frac{\operatorname{fmod}(\Delta t,D)}D\right),\quad d=S(5,7,h),\ s=S(17,19,h),\ L=d(1-s),\ T_w=4[d(1-d)+s(1-s)]
$$

**Ký hiệu và ý nghĩa:** D giây cho 24 giờ, h giờ, L daylight, Tw twilight. Sun angle=(h−6)π/12, elevation=max(sin angle, 0); windows=1−S(.05, .85, L).

**Ví dụ:** D600, dt 10⇒.4 giờ=24 phút. h6⇒d.5, s0, L.5, Tw1. h12⇒L1, elevation 1, windows 0. h0⇒L0, windows 1.

**Nguồn:** [TimeOfDaySystem::setHour: src/TimeOfDaySystem.cpp:6](../src/TimeOfDaySystem.cpp#L6), [TimeOfDaySystem::update: src/TimeOfDaySystem.cpp:16](../src/TimeOfDaySystem.cpp#L16), [LightingSystem::setEnvironment: src/LightingSystem.cpp:14](../src/LightingSystem.cpp#L14), [TimeOfDaySystem::label: src/TimeOfDaySystem.cpp:21](../src/TimeOfDaySystem.cpp#L21).

**Điều kiện/giới hạn:** Clock double nhưng Lighting float; hour wrap và guard số âm rất nhỏ duy trì[0, 24). Hướng sun/moon/màu là nội suy nghệ thuật, không astronomy; pause/manual/reset như 06.
