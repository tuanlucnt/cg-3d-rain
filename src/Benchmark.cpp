#include "Benchmark.hpp"
#include <GL/glxew.h>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <sys/resource.h>
#include <unistd.h>
#include <vector>

namespace {
double cpuSeconds() {
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6 +
           usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
}
std::size_t residentBytes() {
    FILE* file = std::fopen("/proc/self/statm", "r");
    unsigned long total = 0, resident = 0;
    if (file) { if (std::fscanf(file, "%lu %lu", &total, &resident) != 2) resident = 0; std::fclose(file); }
    return resident * static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
}
std::size_t highWaterBytes() {
    FILE* file = std::fopen("/proc/self/status", "r");
    char line[256]{};
    unsigned long kb = 0;
    if (file) {
        while (std::fgets(line, sizeof(line), file))
            if (std::sscanf(line, "VmHWM: %lu kB", &kb) == 1) break;
        std::fclose(file);
    }
    return kb * 1024;
}
}

Benchmark::Benchmark(bool unlocked) {
    glGenQueries(static_cast<GLsizei>(queries_.size()), queries_.data());
    cpuStart_ = cpuSeconds();
    rssStart_ = rssLast_ = residentBytes();
    if (unlocked) {
        std::cout << "Timer cap removed; requesting process-local swap interval 0.\n";
        if (glxewInit() == GLEW_OK && glXGetCurrentDisplay()) {
            if (GLXEW_EXT_swap_control) {
                auto* display = glXGetCurrentDisplay();
                const auto drawable = glXGetCurrentDrawable();
                glXSwapIntervalEXT(display, drawable, 0);
                unsigned interval = 0;
                glXQueryDrawable(display, drawable, GLX_SWAP_INTERVAL_EXT, &interval);
                std::cout << "GLX reported swap interval=" << interval << '\n';
            } else if (GLXEW_MESA_swap_control) {
                glXSwapIntervalMESA(0);
                std::cout << "MESA reported swap interval=" << glXGetSwapIntervalMESA() << '\n';
            } else std::cout << "Swap control unavailable; VSync may still cap presentation.\n";
        }
        std::cout << "Compositor/presentation limits may remain; FPS is not automatically a GPU limit.\n";
    }
}

Benchmark::~Benchmark() {
    if (active_ >= 0) glEndQuery(GL_TIME_ELAPSED);
    glDeleteQueries(static_cast<GLsizei>(queries_.size()), queries_.data());
}

void Benchmark::beginFrame() {
    const auto now = Clock::now();
    if (previous_ != Clock::time_point{}) {
        intervals_[intervalCount_ % intervals_.size()] = std::chrono::duration<double, std::milli>(now - previous_).count();
        ++intervalCount_;
    }
    previous_ = frameStart_ = now;
    for (std::size_t i = 0; i < queries_.size(); ++i) {
        if (pending_[i]) {
            GLint ready = GL_FALSE;
            glGetQueryObjectiv(queries_[i], GL_QUERY_RESULT_AVAILABLE, &ready);
            if (ready) {
                GLuint64 ns = 0;
                glGetQueryObjectui64v(queries_[i], GL_QUERY_RESULT, &ns);
                gpuMs_ += static_cast<double>(ns) / 1e6;
                ++gpuCount_;
                pending_[i] = false;
            }
        }
    }
    for (std::size_t i = 0; i < queries_.size(); ++i) {
        if (!pending_[i]) { active_ = static_cast<int>(i); glBeginQuery(GL_TIME_ELAPSED, queries_[i]); break; }
    }
}

void Benchmark::endFrame() {
    if (active_ >= 0) {
        glEndQuery(GL_TIME_ELAPSED);
        pending_[static_cast<std::size_t>(active_)] = true;
        active_ = -1;
    }
    submitMs_ += std::chrono::duration<double, std::milli>(Clock::now() - frameStart_).count();
    ++renderCount_;
}

void Benchmark::sample(const WeatherSystem& weather, double seconds, double fps) {
    rssLast_ = residentBytes();
    splashPeak_ = std::max(splashPeak_, weather.splashes.activeCount());
    ripplePeak_ = std::max(ripplePeak_, weather.ripples.activeCount());
    ++stateSamples_;
    if (weather.splashes.activeCount() == SplashSystem::MAX_SPLASHES) ++saturatedSamples_;
    if (static_cast<int>(seconds / 60) != minute_) {
        minute_ = static_cast<int>(seconds / 60);
        std::cout << "TRACE seconds=" << seconds << " fps=" << fps << " rss_MiB=" << rssLast_ / 1048576.0
                  << " splash=" << weather.splashes.activeCount() << " ripple=" << weather.ripples.activeCount()
                  << " strikes=" << weather.lightning.strikeCount() << " wetness=" << weather.lighting.wetness()
                  << " splash_rejected=" << weather.splashes.rejected() << " validation=ok" << std::endl;
    }
}

void Benchmark::report(std::ostream& output, double seconds, int frames) const {
    const auto count = std::min(intervalCount_, intervals_.size());
    std::vector<double> sorted(intervals_.begin(), intervals_.begin() + count);
    std::sort(sorted.begin(), sorted.end());
    const auto quantile = [&](double p) { return sorted.empty() ? 0 : sorted[static_cast<std::size_t>((sorted.size() - 1) * p)]; };
    output << "PROFILE avg_frame_ms=" << seconds * 1000 / std::max(frames, 1)
           << " p95_ms=" << quantile(0.95) << " p99_ms=" << quantile(0.99) << " interval_samples=" << count
           << " submit_cpu_wall_ms=" << submitMs_ / std::max<std::size_t>(renderCount_, 1)
           << " gpu_command_ms=" << gpuMs_ / std::max<std::size_t>(gpuCount_, 1) << " gpu_samples=" << gpuCount_
           << " cpu_percent_one_core=" << (cpuSeconds() - cpuStart_) / seconds * 100
           << " rss_start_MiB=" << rssStart_ / 1048576.0 << " rss_last_MiB=" << rssLast_ / 1048576.0
           << " peak_rss_MiB=" << highWaterBytes() / 1048576.0
           << " splash_peak=" << splashPeak_ << " ripple_peak=" << ripplePeak_
           << " saturated_samples=" << saturatedSamples_ << '/' << stateSamples_ << std::endl;
}
