#pragma once
#include <GL/glew.h>
#include "WeatherSystem.hpp"
#include <array>
#include <chrono>
#include <iosfwd>

// Optional diagnostics only. All frame storage and GPU queries have fixed caps.
class Benchmark {
public:
    explicit Benchmark(bool unlocked);
    ~Benchmark();
    Benchmark(const Benchmark&) = delete;
    Benchmark& operator=(const Benchmark&) = delete;
    void beginFrame();
    void endFrame();
    void sample(const WeatherSystem& weather, double seconds, double fps);
    void report(std::ostream& output, double seconds, int frames) const;
private:
    using Clock = std::chrono::steady_clock;
    std::array<GLuint, 4> queries_{};
    std::array<bool, 4> pending_{};
    int active_ = -1;
    std::array<double, 65536> intervals_{};
    std::size_t intervalCount_ = 0, gpuCount_ = 0, renderCount_ = 0;
    double gpuMs_ = 0, submitMs_ = 0, cpuStart_ = 0;
    Clock::time_point previous_{}, frameStart_{};
    std::size_t rssStart_ = 0, rssLast_ = 0, splashPeak_ = 0, ripplePeak_ = 0;
    std::size_t saturatedSamples_ = 0, stateSamples_ = 0;
    int minute_ = -1;
};
