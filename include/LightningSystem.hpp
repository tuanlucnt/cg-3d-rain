#pragma once

#include "CloudSystem.hpp"
#include <array>
#include <cstdint>
#include <random>

struct LightningSegment {
    glm::vec3 start;
    glm::vec3 end;
    float width;
    bool branch;
};

struct LightningPulse {
    float start = 0;
    float duration = 0;
    float peak = 0;
};

class LightningSystem {
public:
    static constexpr unsigned MAIN_DEPTH = 5;
    static constexpr std::size_t MAIN_SEGMENTS = 1u << MAIN_DEPTH;
    static constexpr std::size_t MAX_BRANCHES = 6;
    static constexpr std::size_t MAX_SEGMENTS = 128;
    LightningSystem() { reset(); }
    void reset(std::uint32_t seed = 6174);
    void trigger(const CloudSystem& clouds, glm::vec3 observer = {14, 10, 18});
    void cancel();
    void update(float deltaTime, const CloudSystem& clouds, bool randomEnabled,
                glm::vec3 observer = {14, 10, 18});
    float flashIntensity() const;
    bool active() const { return active_; }
    float duration() const { return duration_; }
    float elapsed() const { return elapsed_; }
    float nextStrike() const { return nextStrike_; }
    std::uint64_t strikeCount() const { return strikeCount_; }
    std::uint64_t revision() const { return revision_; }
    unsigned pulseCount() const { return pulseCount_; }
    const std::array<LightningPulse, 3>& pulses() const { return pulses_; }
    glm::vec3 lightPosition() const {
        return segmentCount_ ? segments_[0].start : glm::vec3(0, 8, 0);
    }
    const std::array<LightningSegment, MAX_SEGMENTS>& segments() const { return segments_; }
    std::size_t segmentCount() const { return segmentCount_; }

private:
    float random(float low, float high);
    void subdivide(glm::vec3 start, glm::vec3 end, unsigned depth, float displacement,
                   float width, bool branch);
    std::array<LightningSegment, MAX_SEGMENTS> segments_{};
    std::mt19937 random_{6174};
    std::size_t segmentCount_ = 0;
    std::uint64_t strikeCount_ = 0;
    std::uint64_t revision_ = 0;
    float elapsed_ = 0;
    float duration_ = 0;
    float nextStrike_ = 0;
    unsigned pulseCount_ = 1;
    std::array<LightningPulse, 3> pulses_{};
    bool active_ = false;
};
