#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <random>

struct Raindrop {
    glm::vec3 position;
    glm::vec3 velocity;
    float length;
    float lifetime;
    float streakScale;
    float opacity;
};

class RainSystem {
public:
    static constexpr std::size_t RAIN_COUNT = 1000;
    RainSystem() { reset(); }
    void reset(const glm::vec3& windVelocity = {0.65f, 0, 0.2f});
    void update(float deltaTime, const glm::vec3& windVelocity);
    const std::array<Raindrop, RAIN_COUNT>& drops() const { return drops_; }
    const std::array<glm::vec3, RAIN_COUNT>& impacts() const { return impacts_; }
    std::size_t impactCount() const { return impactCount_; }
    std::uint64_t recycledCount() const { return recycledCount_; }
    std::uint64_t revision() const { return revision_; }

private:
    float random(float low, float high);
    void respawn(Raindrop& drop, const glm::vec3& windVelocity, bool initial);
    std::array<Raindrop, RAIN_COUNT> drops_{};
    std::array<glm::vec3, RAIN_COUNT> impacts_{};
    std::mt19937 random_{1337};
    std::size_t impactCount_ = 0;
    std::uint64_t recycledCount_ = 0;
    std::uint64_t revision_ = 0;
};
