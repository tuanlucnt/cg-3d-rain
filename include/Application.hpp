#pragma once

// GLEW must precede FreeGLUT and any other OpenGL header.
#include "Renderer.hpp"
#include "Benchmark.hpp"
#include <array>
#include <chrono>
#include <memory>
#include <string>

struct RunOptions {
    bool smokeTest = false;
    std::string capturePath;
    std::string lightningCapturePath;
    std::string cloudWrapCapturePath;
    double benchmarkSeconds = 0;
    bool noFog = false;
    float fogDensity = 0.018f;
    std::string viewCaptureDirectory;
    bool unlocked = false;
};

class Application {
public:
    explicit Application(RunOptions options);
    ~Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    int run(char* executableName);

private:
    using Clock = std::chrono::steady_clock;
    void initialize(char* executableName);
    void display();
    void tick();
    void update(float deltaTime);
    void updateStatistics(Clock::time_point now);
    void reshape(int width, int height);
    void key(unsigned char value, bool pressed);
    void mouseButton(int button, int state, int x, int y);
    void mouseMotion(int x, int y);
    void releaseInput();
    void close();
    void requestExit();
    void fail(const char* message);
    void smokeStep();
    void inspectFramebuffer(const std::string& capturePath = {}) const;

    // FreeGLUT's C callbacks route to the single application instance.
    static Application* active_;
    static void displayCallback();
    static void timerCallback(int);
    static void idleCallback();
    static void reshapeCallback(int width, int height);
    static void keyDownCallback(unsigned char key, int, int);
    static void keyUpCallback(unsigned char key, int, int);
    static void mouseButtonCallback(int button, int state, int x, int y);
    static void mouseMotionCallback(int x, int y);
    static void entryCallback(int state);
    static void visibilityCallback(int state);
    static void closeCallback();

    RunOptions options_;
    Camera camera_;
    WeatherSystem weather_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<Benchmark> benchmark_;
    std::array<bool, 256> keys_{};
    Clock::time_point lastTick_{};
    Clock::time_point started_{};
    Clock::time_point statisticsTick_{};
    int statisticsFrames_ = 0;
    std::size_t maxDrawCalls_ = 0;
    std::size_t maxTriangles_ = 0, maxLines_ = 0;
    int window_ = 0;
    int width_ = 800;
    int height_ = 600;
    int mouseX_ = 0;
    int mouseY_ = 0;
    int frames_ = 0;
    int exitCode_ = 0;
    bool dragging_ = false;
    bool visible_ = true;
    bool exiting_ = false;
    bool smokeCompleted_ = false;
};
