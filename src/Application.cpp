#include "Application.hpp"

#include <GL/freeglut.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
void checkOpenGL(const char* where) {
    std::ostringstream errors;
    bool failed = false;
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
        errors << " 0x" << std::hex << error;
        failed = true;
    }
    if (failed) throw std::runtime_error(std::string(where) + ": OpenGL error" + errors.str());
}

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

std::vector<unsigned char> readColor(int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

std::size_t changedPixels(const std::vector<unsigned char>& a,
                          const std::vector<unsigned char>& b) {
    require(a.size() == b.size(), "Cannot compare different framebuffer sizes");
    std::size_t count = 0;
    for (std::size_t i = 0; i < a.size(); i += 3) {
        if (std::abs(int(a[i]) - int(b[i])) > 5 ||
            std::abs(int(a[i + 1]) - int(b[i + 1])) > 5 ||
            std::abs(int(a[i + 2]) - int(b[i + 2])) > 5) ++count;
    }
    return count;
}
}

Application* Application::active_ = nullptr;

Application::Application(RunOptions options) : options_(std::move(options)) {}

Application::~Application() {
    if (window_ != 0) {
        glutSetWindow(window_);
        // glutDestroyWindow may defer destruction until event processing/atexit.
        // A pending close callback must not retain this object's pointer.
        glutCloseFunc(nullptr);
        benchmark_.reset();
        renderer_.reset();
        scene_.reset();
        glutDestroyWindow(window_);
        window_ = 0;
    }
    active_ = nullptr;
}

int Application::run(char* executableName) {
    require(active_ == nullptr, "Only one FreeGLUT Application is supported");
    active_ = this;
    initialize(executableName);
    lastTick_ = started_ = statisticsTick_ = Clock::now();
    if (options_.unlocked) glutIdleFunc(idleCallback);
    else glutTimerFunc(16, timerCallback, 0);
    glutMainLoop();
    if (options_.smokeTest) {
        if (!smokeCompleted_) {
            std::cerr << "Smoke test interrupted before completion\n";
            exitCode_ = 1;
        } else if (exitCode_ == 0) {
            std::cout << "Smoke test passed: shaders, buffers, render, input handlers, reset, "
                         "weather pixels, pause, resize and cleanup; no OpenGL errors.\n";
        }
    }
    return exitCode_;
}

void Application::initialize(char* executableName) {
    int argc = 1;
    char* argv[]{executableName, nullptr};
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(width_, height_);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    window_ = glutCreateWindow("Storm");
    require(window_ > 0, "Cannot create an OpenGL 3.3 Core window");
    // Register close immediately, so partial initialization also cleans up safely.
    glutCloseFunc(closeCallback);

    glewExperimental = GL_TRUE;
    const GLenum status = glewInit();
    if (status != GLEW_OK) {
        throw std::runtime_error(std::string("GLEW initialization failed: ") +
            reinterpret_cast<const char*>(glewGetErrorString(status)));
    }
    // GLEW can probe a deprecated extension query during initialization in Core.
    // Drain only here; all subsequent errors are treated as failures.
    while (glGetError() != GL_NO_ERROR) {}
    GLint major = 0, minor = 0, profile = 0, depthBits = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH,
                                          GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits);
    checkOpenGL("Context capabilities");
    require((major > 3 || (major == 3 && minor >= 3)) && GLEW_VERSION_3_3 &&
            (profile & GL_CONTEXT_CORE_PROFILE_BIT),
            "OpenGL 3.3 Core Profile is required; no fallback renderer is used");
    require(depthBits >= 16, "A depth buffer of at least 16 bits is required");
    std::cout << "OpenGL: " << glGetString(GL_VERSION)
              << "\nRenderer: " << glGetString(GL_RENDERER)
              << "\nGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << "\nDepth buffer: " << depthBits << " bits\n";

    renderer_ = std::make_unique<Renderer>();
    scene_ = std::make_unique<Scene>();
    weather_.fogEnabled = !options_.noFog;
    weather_.lighting.fogDensity = options_.fogDensity;
    if (options_.benchmarkSeconds > 0) benchmark_ = std::make_unique<Benchmark>(options_.unlocked);
    reshape(width_, height_);
    checkOpenGL("Initialization (shader/mesh/state)");
    std::cout << "Scene: ground 20x20, 1 house, 4 trees, 3 rocks; "
              << scene_->objects().size() << " draw calls, "
              << scene_->triangleCount() << " triangles/frame\n"
              << "WASD: move | Q/E: down/up | left mouse drag: look | R: reset | Esc: exit\n";
    std::cout << "Weather: 1=rain (1000), 2=random lightning, 3=clouds, L=manual lightning, "
                 "Space=pause (camera remains free); splash pool=512\n";
    glutIgnoreKeyRepeat(1);
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyDownCallback);
    glutKeyboardUpFunc(keyUpCallback);
    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(mouseMotionCallback);
    glutEntryFunc(entryCallback);
    glutVisibilityFunc(visibilityCallback);
}

void Application::update(float deltaTime) {
    const glm::vec3 direction{
        static_cast<float>(keys_['d']) - static_cast<float>(keys_['a']),
        static_cast<float>(keys_['e']) - static_cast<float>(keys_['q']),
        static_cast<float>(keys_['w']) - static_cast<float>(keys_['s'])};
    camera_.move(direction, deltaTime);
    weather_.observerPosition = camera_.position();
    weather_.update(deltaTime);
    if (scene_) scene_->update(deltaTime, weather_.paused);
}

void Application::tick() {
    if (exiting_) return;
    const auto now = Clock::now();
    // Avoid teleporting after a breakpoint, stall, or a minimized window.
    const float dt = std::clamp(std::chrono::duration<float>(now - lastTick_).count(),
                                0.0f, 0.1f);
    lastTick_ = now;
    updateStatistics(now);
    if (options_.benchmarkSeconds > 0 &&
        std::chrono::duration<double>(now - started_).count() >= options_.benchmarkSeconds) {
        const double elapsed = std::chrono::duration<double>(now - started_).count();
        std::cout << "Benchmark: " << frames_ << " frames / " << elapsed << " s = "
                  << frames_ / elapsed << (options_.unlocked ? " FPS (timer removed); rain=" : " FPS (16 ms timer cap); rain=")
                  << weather_.rain.drops().size() << ", splash=" << weather_.splashes.activeCount()
                  << ", strikes=" << weather_.lightning.strikeCount()
                  << ", max draw calls=" << maxDrawCalls_ << ", max triangles=" << maxTriangles_
                  << ", max lines=" << maxLines_ << ", ripples=" << weather_.ripples.activeCount() << '\n';
        benchmark_->report(std::cout, elapsed, frames_);
        std::cout << "Splash requests=" << weather_.splashes.requests() << " filtered=" << weather_.splashes.filtered()
                  << " rejected=" << weather_.splashes.rejected() << "; ripple rejected=" << weather_.ripples.rejected() << '\n';
        requestExit();
        return;
    }
    if (benchmark_ && !options_.unlocked) glutTimerFunc(16, timerCallback, 0);
    if (visible_ || benchmark_) {
        update(dt);
        // Visibility notifications also include fully occluded windows. A timed
        // benchmark must keep simulating/rendering instead of measuring a pause.
        if (benchmark_) display();
        else glutPostRedisplay();
    }
    if (!options_.unlocked && !benchmark_) glutTimerFunc(16, timerCallback, 0);
}

void Application::display() {
    if (exiting_ || !scene_ || !renderer_) return;
    if (options_.smokeTest && frames_ == 0) {
        scene_->resetAnimation();
        // Deterministic half-second warmup makes real rain impacts/splashes visible.
        for (int i = 0; i < 5; ++i) weather_.update(0.1f);
    }
    if (benchmark_) benchmark_->beginFrame();
    renderer_->draw(*scene_, camera_, weather_, width_, height_);
    maxDrawCalls_ = std::max(maxDrawCalls_, renderer_->drawCalls());
    maxTriangles_ = std::max(maxTriangles_, renderer_->triangles());
    maxLines_ = std::max(maxLines_, renderer_->lines());
    checkOpenGL("Frame rendering");
    if (benchmark_) benchmark_->endFrame();
    ++frames_;
    if (options_.smokeTest) smokeStep();
    if (!exiting_) glutSwapBuffers();
}

void Application::reshape(int width, int height) {
    width_ = std::max(width, 1);
    height_ = std::max(height, 1);
    glViewport(0, 0, width_, height_);
}

void Application::key(unsigned char value, bool pressed) {
    const auto lower = static_cast<unsigned char>(std::tolower(value));
    keys_[lower] = pressed;
    if (!pressed) return;
    if (lower == 27) requestExit();
    if (lower == 'r') {
        camera_.reset();
        weather_.reset();
        if (scene_) scene_->resetAnimation();
        releaseInput();
    }
    if (lower == '3') weather_.cloudsEnabled = !weather_.cloudsEnabled;
    if (lower == '1') weather_.rainEnabled = !weather_.rainEnabled;
    if (lower == '2') weather_.toggleLightning();
    if (lower == 'l') {
        weather_.observerPosition = camera_.position();
        weather_.triggerLightning();
    }
    if (lower == ' ') weather_.paused = !weather_.paused;
    if (lower == 'f') weather_.fogEnabled = !weather_.fogEnabled;
    if (lower >= '4' && lower <= '6') {
        camera_.preset(lower - '4');
        weather_.observerPosition = camera_.position();
        releaseInput();
    }
}

void Application::updateStatistics(Clock::time_point now) {
    const double elapsed = std::chrono::duration<double>(now - statisticsTick_).count();
    if (elapsed < 1.0) return;
    const double fps = (frames_ - statisticsFrames_) / elapsed;
    if (benchmark_) {
        require(weather_.valid(), "Non-finite simulation data or inconsistent particle count");
        benchmark_->sample(weather_, std::chrono::duration<double>(now - started_).count(), fps);
    }
    std::array<char, 128> title{};
    std::snprintf(title.data(), title.size(),
        "Storm | %.0f FPS | R:%zu S:%zu P:%zu | L:%s C:%s F:%s%s",
        fps, weather_.rainEnabled ? RainSystem::RAIN_COUNT : std::size_t(0),
        weather_.splashes.activeCount(),
        weather_.ripples.activeCount(),
        weather_.lightning.active() ? "FLASH" : (weather_.lightningEnabled ? "ON" : "OFF"),
        weather_.cloudsEnabled ? "ON" : "OFF", weather_.fogEnabled ? "ON" : "OFF", weather_.paused ? " | PAUSE" : "");
    glutSetWindowTitle(title.data());
    statisticsTick_ = now;
    statisticsFrames_ = frames_;
}

void Application::mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        dragging_ = state == GLUT_DOWN;
        mouseX_ = x;
        mouseY_ = y;
    }
}

void Application::mouseMotion(int x, int y) {
    if (dragging_) {
        camera_.rotate(static_cast<float>(x - mouseX_), static_cast<float>(y - mouseY_));
        mouseX_ = x;
        mouseY_ = y;
    }
}

void Application::releaseInput() {
    keys_.fill(false);
    dragging_ = false;
}

void Application::close() {
    // FreeGLUT calls this while the closing window's GL context is still current.
    benchmark_.reset();
    renderer_.reset();
    scene_.reset();
    try {
        checkOpenGL("Resource cleanup");
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        exitCode_ = 1;
    }
    window_ = 0;
    exiting_ = true;
}

void Application::requestExit() {
    if (exiting_) return;
    exiting_ = true;
    releaseInput();
    benchmark_.reset();
    // Explicit release also covers Esc and successful smoke-test exit.
    renderer_.reset();
    scene_.reset();
    checkOpenGL("Resource cleanup");
    glutLeaveMainLoop();
}

void Application::fail(const char* message) {
    std::cerr << "Application error: " << message << '\n';
    exitCode_ = 1;
    if (window_) {
        // Do not throw across FreeGLUT's C callback boundary.
        exiting_ = true;
        benchmark_.reset();
        renderer_.reset();
        scene_.reset();
        glutLeaveMainLoop();
    }
}

void Application::inspectFramebuffer(const std::string& capturePath) const {
    require(renderer_->triangles() < 10000 && renderer_->drawCalls() < 80,
            "Scene exceeds the agreed model triangle/draw-call budget");
    const std::size_t pixels = static_cast<std::size_t>(width_) * height_;
    std::vector<float> depth(pixels);
    std::vector<unsigned char> color(pixels * 3);
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width_, height_, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, color.data());
    checkOpenGL("Framebuffer readback");
    std::size_t covered = 0;
    for (float d : depth) {
        require(d >= 0 && d <= 1, "Invalid depth value");
        if (d < 0.99999f) ++covered;
    }
    require(covered > pixels / 50 && covered < pixels * 49 / 50,
            "Framebuffer must contain both scene geometry and background");
    require(glIsEnabled(GL_DEPTH_TEST) && glIsEnabled(GL_CULL_FACE),
            "Depth test and face culling must remain enabled");
    GLboolean writesDepth = GL_FALSE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &writesDepth);
    require(writesDepth == GL_TRUE && !glIsEnabled(GL_BLEND),
            "Effect passes must restore depth writes and blending state");
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    require(viewport[2] == width_ && viewport[3] == height_, "Viewport resize failed");
    if (!capturePath.empty()) {
        std::ofstream file(capturePath, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot write capture: " + capturePath);
        file << "P6\n" << width_ << ' ' << height_ << "\n255\n";
        for (int row = height_ - 1; row >= 0; --row) {
            file.write(reinterpret_cast<const char*>(color.data() +
                       static_cast<std::size_t>(row) * width_ * 3), width_ * 3);
        }
        file.close();
        require(static_cast<bool>(file), "Failed writing framebuffer capture");
    }
    std::cout << "Framebuffer " << width_ << 'x' << height_ << ": " << covered
              << '/' << pixels << " pixels contain geometry\n";
}

void Application::smokeStep() {
    if (frames_ == 1) {
        if (!options_.viewCaptureDirectory.empty()) std::filesystem::create_directories(options_.viewCaptureDirectory);
        inspectFramebuffer(options_.capturePath);
        require(renderer_->renderedRainCount() == RainSystem::RAIN_COUNT &&
                renderer_->renderedSplashCount() == weather_.splashes.activeCount(),
                "Submitted particle counts must match active counts (not pool capacity)");
        const auto baseline = readColor(width_, height_);
        std::vector<float> baselineDepth(static_cast<std::size_t>(width_) * height_);
        glReadPixels(0, 0, width_, height_, GL_DEPTH_COMPONENT, GL_FLOAT, baselineDepth.data());
        const auto renderAndRead = [&] {
            renderer_->draw(*scene_, camera_, weather_, width_, height_);
            checkOpenGL("Isolated effect render");
            return readColor(width_, height_);
        };
        key('3', true); key('3', false);
        require(!weather_.cloudsEnabled, "Cloud input toggle failed");
        const auto cloudPixels = changedPixels(baseline, renderAndRead());
        require(cloudPixels > 1000, "Clouds must visibly contribute pixels");
        key('3', true); key('3', false);
        key('1', true); key('1', false);
        require(!weather_.rainEnabled && weather_.rain.drops().size() == 1000,
                "Rain toggle must preserve the 1000-particle pool");
        const auto rainPixels = changedPixels(baseline, renderAndRead());
        require(rainPixels > 250, "Rain must visibly contribute pixels");
        key('1', true); key('1', false);
        require(weather_.splashes.activeCount() > 0, "Real rain impacts must spawn splashes");
        const auto splashState = weather_.splashes;
        weather_.splashes.reset();
        const auto splashPixels = changedPixels(baseline, renderAndRead());
        require(splashPixels > 5, "Splashes must visibly contribute pixels");
        weather_.splashes = splashState;
        const auto rippleState = weather_.ripples;
        weather_.ripples.reset();
        const auto ripplePixels = changedPixels(baseline, renderAndRead());
        require(ripplePixels > 3, "Real ground impacts must render expanding ripple rings");
        weather_.ripples = rippleState;
        std::cout << "Ripple pixels=" << ripplePixels << ", splash active="
                  << weather_.splashes.activeCount() << "/512, rejected requests="
                  << weather_.splashes.rejected() << '\n';
        key('f', true); key('f', false);
        const auto fogPixels = changedPixels(baseline, renderAndRead());
        require(weather_.lighting.fogDensity == 0 ? fogPixels == 0 : fogPixels > 1000,
                "Fog toggle must reflect the configured density");
        key('f', true); key('f', false);

        const glm::vec3 initial = camera_.position();
        key('w', true);
        update(0.2f);
        key('w', false);
        require(glm::distance(initial, camera_.position()) > 0.5f, "W input did not move camera");
        const glm::vec3 stopped = camera_.position();
        update(0.2f);
        require(glm::distance(stopped, camera_.position()) < 0.0001f, "Key release did not stop movement");
        const glm::vec3 direction = camera_.forward();
        mouseButton(GLUT_LEFT_BUTTON, GLUT_DOWN, 100, 100);
        mouseMotion(160, 120);
        mouseButton(GLUT_LEFT_BUTTON, GLUT_UP, 160, 120);
        require(glm::distance(direction, camera_.forward()) > 0.01f, "Mouse input did not rotate camera");
        key('r', true);
        key('r', false);
        require(glm::distance(initial, camera_.position()) < 0.0001f, "Reset input failed");
        require(weather_.rainEnabled && weather_.cloudsEnabled && weather_.lightningEnabled &&
                !weather_.paused && weather_.splashes.activeCount() == 0 &&
                weather_.lighting.flash() == 0, "R must reset the entire weather state");
        for (int i = 0; i < 5; ++i) weather_.update(0.1f);
        key('2', true);
        key('2', false);
        require(!weather_.lightningEnabled, "Lightning toggle failed");
        key('l', true);
        key('l', false);
        require(weather_.lightning.active() && weather_.lighting.flash() > 0.9f,
                "L must trigger a flash even when random lightning is off");
        renderer_->draw(*scene_, camera_, weather_, width_, height_);
        checkOpenGL("Controlled lightning render");
        inspectFramebuffer(options_.lightningCapturePath);
        const auto flashPixels = readColor(width_, height_);
        const auto strikeState = weather_.lightning;
        weather_.lightning.cancel(); // Keep flash lighting to isolate bolt geometry.
        const auto boltPixels = changedPixels(flashPixels, renderAndRead());
        require(boltPixels > 20, "Lightning geometry must visibly contribute pixels");
        weather_.lightning = strikeState;
        const auto uploads = renderer_->lightningUploadCount();
        weather_.lightning.update(0.01f, weather_.clouds, false);
        weather_.lighting.update(weather_.lightning.flashIntensity());
        renderAndRead();
        require(renderer_->lightningUploadCount() == uploads,
                "Changing only pulse intensity must not upload bolt geometry again");
        camera_.rotate(1, 0);
        renderAndRead();
        require(renderer_->lightningUploadCount() == uploads + 1,
                "Camera changes must refresh camera-facing lightning quads");
        camera_.reset();
        weather_.lightning = strikeState;
        weather_.lighting.update(weather_.lightning.flashIntensity());
        std::size_t brightClouds = 0, brightScene = 0;
        std::size_t saturated = 0;
        for (std::size_t p = 0; p < baselineDepth.size(); ++p) {
            if (baselineDepth[p] >= 0.99999f) continue;
            const auto i = p * 3;
            if (flashPixels[i] > 248 && flashPixels[i + 1] > 248 && flashPixels[i + 2] > 248) ++saturated;
            const int brightness = int(flashPixels[i]) + flashPixels[i + 1] + flashPixels[i + 2];
            const int before = int(baseline[i]) + baseline[i + 1] + baseline[i + 2];
            if (brightness <= before + 20) continue;
            if (p / static_cast<std::size_t>(width_) > static_cast<std::size_t>(height_ * 7 / 10))
                ++brightClouds;
            else ++brightScene;
        }
        require(brightClouds > 1000 && brightScene > 1000,
                "Flash must brighten both cloud geometry and the ground/house/trees");
        require(saturated < baselineDepth.size() / 100,
                "Flash must not wash out the entire scene to white");
        std::cout << "Visible effect pixels: clouds=" << cloudPixels << ", rain=" << rainPixels
                  << ", splash=" << splashPixels << ", lightning=" << boltPixels
                  << "; brightened cloud/scene pixels=" << brightClouds << '/' << brightScene << '\n';

        key(' ', true); key(' ', false);
        const auto frozen = weather_.rain.drops()[0].position;
        const float strikeTime = weather_.lightning.elapsed();
        const float treeTime = scene_->animationTime();
        key('w', true); update(0.05f); key('w', false);
        require(weather_.paused && glm::distance(frozen, weather_.rain.drops()[0].position) == 0 &&
                weather_.lightning.elapsed() == strikeTime && scene_->animationTime() == treeTime,
                "Space must freeze weather and tree animation time");
        require(glm::distance(initial, camera_.position()) > 0.1f,
                "Camera must remain movable while simulation is paused");
        camera_.reset();
        key(' ', true); key(' ', false);
        for (int i = 0; i < 6; ++i) weather_.update(0.1f);
        require(!weather_.lightning.active() && weather_.lighting.flash() == 0,
                "Flash must return to background lighting after its lifetime");
        const auto staticTree = renderAndRead();
        const auto rainRevision = weather_.rain.revision();
        const auto triangles = scene_->triangleCount();
        for (int i = 0; i < 10; ++i) scene_->update(0.1f, false);
        const auto movingTree = renderAndRead();
        const auto swayPixels = changedPixels(staticTree, movingTree);
        require(swayPixels > 5, "Vertex shader tree sway must visibly change the framebuffer");
        require(weather_.rain.revision() == rainRevision && scene_->triangleCount() == triangles,
                "Tree sway must not alter rain state or recreate geometry");
        scene_->resetAnimation();
        std::cout << "Tree sway pixels=" << swayPixels << "; static triangles=" << triangles << '\n';
        // Freeze particle/camera state while varying only the environment's
        // lighting inputs. Median samples avoid a rain line crossing one pixel.
        const auto savedLighting=weather_.lighting;
        const auto sample=[&](const std::vector<unsigned char>& pixels,glm::vec3 world) {
            const auto clip=camera_.projection(static_cast<float>(width_)/height_)*camera_.view()*glm::vec4(world,1);
            require(clip.w>0,"Environment probe must be in front of the camera");
            const int x=static_cast<int>((clip.x/clip.w*0.5f+0.5f)*width_);
            const int y=static_cast<int>((clip.y/clip.w*0.5f+0.5f)*height_);
            std::vector<int> values;
            for (int dy=-2;dy<=2;++dy) for (int dx=-2;dx<=2;++dx) {
                require(x+dx>=0 && x+dx<width_ && y+dy>=0 && y+dy<height_,"Environment probe outside viewport");
                const auto i=(static_cast<std::size_t>(y+dy)*width_+x+dx)*3;
                values.push_back((int(pixels[i])+pixels[i+1]+pixels[i+2])/3);
            }
            std::sort(values.begin(),values.end());
            return values[values.size()/2];
        };
        for (int i=0;i<1000;++i) weather_.lighting.update(0,0.1f,false);
        const auto dryGround=renderAndRead();
        if (!options_.viewCaptureDirectory.empty())
            inspectFramebuffer((std::filesystem::path(options_.viewCaptureDirectory)/"ground-dry.ppm").string());
        for (int i=0;i<1000;++i) weather_.lighting.update(0,0.1f,true);
        const auto wetGround=renderAndRead();
        const int drySoil=sample(dryGround,{0,0,8}),wetSoil=sample(wetGround,{0,0,8});
        require(changedPixels(dryGround,wetGround)>1000 && drySoil>wetSoil+3,
                "Wetness must visibly darken soil while preserving a continuous surface");
        if (!options_.viewCaptureDirectory.empty())
            inspectFramebuffer((std::filesystem::path(options_.viewCaptureDirectory)/"ground-wet.ppm").string());
        weather_.lighting.setFlashPosition({-2,5,4.5f});
        weather_.lighting.update(1);
        const auto litWater=renderAndRead();
        const int waterBefore=sample(wetGround,{-2,0.012f,4.5f}),waterAfter=sample(litWater,{-2,0.012f,4.5f});
        require(waterAfter>waterBefore+5 && waterAfter<240,
                "Water must respond to flash without burning out");
        std::cout<<"Environment probe luminance: dry/wet soil="<<drySoil<<'/'<<wetSoil
                 <<", water before/flash="<<waterBefore<<'/'<<waterAfter<<'\n';
        weather_.lighting=savedLighting;
        for (unsigned i = 0; i < 3; ++i) {
            key(static_cast<unsigned char>('4' + i), true);
            renderer_->draw(*scene_, camera_, weather_, width_, height_);
            checkOpenGL("Camera preset render");
            if (!options_.viewCaptureDirectory.empty()) {
                std::filesystem::create_directories(options_.viewCaptureDirectory);
                inspectFramebuffer((std::filesystem::path(options_.viewCaptureDirectory) /
                    (std::string("camera-") + std::to_string(i + 4) + ".ppm")).string());
            }
        }
        if (!options_.viewCaptureDirectory.empty()) {
            key('l', true); key('l', false);
            renderer_->draw(*scene_, camera_, weather_, width_, height_);
            inspectFramebuffer((std::filesystem::path(options_.viewCaptureDirectory) / "ground-flash.ppm").string());
            Camera treeCamera;
            treeCamera.preset(2);
            treeCamera.move({-1,0,0},0.8f);
            const glm::vec3 target(-6.5f,2.0f,4.0f);
            const auto direction=glm::normalize(target-treeCamera.position());
            const float yaw=glm::degrees(std::atan2(direction.z,direction.x));
            const float pitch=glm::degrees(std::asin(direction.y));
            treeCamera.rotate((yaw-treeCamera.yaw())/0.12f,(treeCamera.pitch()-pitch)/0.12f);
            renderer_->draw(*scene_,treeCamera,weather_,width_,height_);
            inspectFramebuffer((std::filesystem::path(options_.viewCaptureDirectory) / "tree-detail.ppm").string());
        }
        key('4', true); key('4', false);
        glutReshapeWindow(640, 400);
    }
    if (frames_ >= 12) {
        require(width_ == 640 && height_ == 400, "FreeGLUT resize callback did not run");
        inspectFramebuffer();
        if (!options_.cloudWrapCapturePath.empty()) {
            weather_.reset();
            weather_.rainEnabled = false;
            weather_.lightningEnabled = false;
            // Reproduce the right cloud's partial wrap fade after 23 seconds.
            for (int i = 0; i < 230; ++i) weather_.clouds.update(0.1f, weather_.windVelocity);
            renderer_->draw(*scene_, camera_, weather_, width_, height_);
            checkOpenGL("Cloud wrap reproduction");
            inspectFramebuffer(options_.cloudWrapCapturePath);
            std::cout << "Right cloud wrap opacity: "
                      << CloudSystem::coverage(weather_.clouds.clusters()[2].position) << '\n';
            const auto fading=readColor(width_,height_);
            std::vector<float> fadeDepth(static_cast<std::size_t>(width_)*height_);
            glReadPixels(0,0,width_,height_,GL_DEPTH_COMPONENT,GL_FLOAT,fadeDepth.data());
            weather_.cloudsEnabled=false;
            renderer_->draw(*scene_,camera_,weather_,width_,height_);
            const auto clear=readColor(width_,height_);
            std::vector<float> clearDepth(fadeDepth.size());
            glReadPixels(0,0,width_,height_,GL_DEPTH_COMPONENT,GL_FLOAT,clearDepth.data());
            std::size_t transparentPixels=0;
            for (std::size_t p=0;p<fadeDepth.size();++p)
                if (fadeDepth[p]>0.99999f && clearDepth[p]>0.99999f &&
                    std::abs(int(fading[p*3])-int(clear[p*3]))>5) ++transparentPixels;
            require(transparentPixels>50,"Fading cloud must blend visibly without writing depth");
            weather_.cloudsEnabled=true;
            renderer_->draw(*scene_,camera_,weather_,width_,height_);
            require(changedPixels(fading,readColor(width_,height_))==0,"Cloud fade must be deterministic without stochastic holes");
            inspectFramebuffer();
            std::cout<<"Cloud fade pixels without depth writes="<<transparentPixels<<'\n';
        }
        smokeCompleted_ = true;
        requestExit();
    }
}

void Application::displayCallback() {
    try { active_->display(); }
    catch (const std::exception& e) { active_->fail(e.what()); }
}
void Application::timerCallback(int) {
    try { active_->tick(); }
    catch (const std::exception& e) { active_->fail(e.what()); }
}
void Application::idleCallback() { timerCallback(0); }
void Application::reshapeCallback(int w, int h) { active_->reshape(w, h); }
void Application::keyDownCallback(unsigned char k, int, int) {
    try { active_->key(k, true); }
    catch (const std::exception& e) { active_->fail(e.what()); }
}
void Application::keyUpCallback(unsigned char k, int, int) { active_->key(k, false); }
void Application::mouseButtonCallback(int b, int s, int x, int y) { active_->mouseButton(b, s, x, y); }
void Application::mouseMotionCallback(int x, int y) { active_->mouseMotion(x, y); }
void Application::entryCallback(int state) { if (state == GLUT_LEFT) active_->releaseInput(); }
void Application::visibilityCallback(int state) {
    active_->visible_ = state == GLUT_VISIBLE;
    if (!active_->visible_) active_->releaseInput();
}
void Application::closeCallback() { active_->close(); }
