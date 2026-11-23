#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <random>

struct SplashParticle {
    glm::vec3 position{};
    glm::vec3 velocity{};
    float lifetime = 0;
    float initialLifetime = 1;
    float length = 0.07f;
};

class SplashSystem {
public:
    static constexpr std::size_t MAX_SPLASHES = 512;
    SplashSystem() { reset(); }
    void reset();
    void emit(glm::vec3 groundHit, bool sampled = false);
    void update(float deltaTime);
    std::size_t activeCount() const { return activeCount_; }
    const std::array<SplashParticle, MAX_SPLASHES>& particles() const { return particles_; }
    std::uint64_t revision() const { return revision_; }
    std::uint64_t requests() const { return requests_; }
    std::uint64_t rejected() const { return rejected_; }
    std::uint64_t filtered() const { return filtered_; }

private:
    float random(float low, float high);
    std::array<SplashParticle, MAX_SPLASHES> particles_{};
    std::mt19937 random_{2026};
    std::size_t cursor_ = 0;
    std::size_t activeCount_ = 0;
    std::uint64_t revision_ = 0;
    std::uint64_t requests_ = 0, rejected_ = 0, filtered_ = 0;
};
