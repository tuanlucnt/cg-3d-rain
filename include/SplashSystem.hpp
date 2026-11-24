#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <random>
#include "Surface.hpp"

// World units and seconds. All surfaces share the same fixed particle pool.
namespace splashConfig {
inline constexpr int MIN_ROOF_PARTICLES = 3, MAX_ROOF_PARTICLES = 6;
inline constexpr float MIN_LIFETIME = 0.12f, MAX_LIFETIME = 0.30f;
inline constexpr float MIN_SIZE = 0.015f, MAX_SIZE = 0.035f;
inline constexpr float MIN_NORMAL_SPEED = 0.45f, MAX_NORMAL_SPEED = 1.05f;
inline constexpr float MIN_TANGENT_SPEED = 0.25f, MAX_TANGENT_SPEED = 0.90f;
inline constexpr float REFLECTION_RETENTION = 0.06f;
inline constexpr float INITIAL_ALPHA = 0.85f;
inline constexpr float GRAVITY = 9.81f;
inline constexpr float MAX_STEP = 1.0f / 120.0f;
struct Profile {
    int minCount, maxCount;
    float minLife, maxLife, minSize, maxSize;
    float minNormal, maxNormal, minTangent, maxTangent, reflection;
};
inline constexpr Profile ROOF{MIN_ROOF_PARTICLES, MAX_ROOF_PARTICLES, MIN_LIFETIME, MAX_LIFETIME,
    MIN_SIZE, MAX_SIZE, MIN_NORMAL_SPEED, MAX_NORMAL_SPEED,
    MIN_TANGENT_SPEED, MAX_TANGENT_SPEED, REFLECTION_RETENTION};
inline constexpr Profile GROUND{2,4,0.10f,0.20f,0.010f,0.025f,0.30f,0.60f,0.15f,0.45f,0.02f};
inline constexpr Profile ROCK{3,5,0.12f,0.30f,0.015f,0.030f,0.40f,0.90f,0.20f,0.75f,0.05f};
}

struct SplashParticle {
    glm::vec3 position{};
    glm::vec3 velocity{};
    float lifetime = 0;
    float initialLifetime = 1;
    float length = 0.07f;
    float alpha = 0; // lifetime <= 0 is the inactive pool state.
    SurfaceType surface = SurfaceType::Ground;
};

class SplashSystem {
public:
    static constexpr std::size_t MAX_SPLASHES = 512;
    SplashSystem() { reset(); }
    void reset();
    void emit(glm::vec3 groundHit, bool sampled = false);
    void emitSurface(glm::vec3 hit, glm::vec3 normal, glm::vec3 incomingVelocity,
                     SurfaceType surface = SurfaceType::Roof, bool sampled = false);
    void update(float deltaTime);
    std::size_t activeCount() const { return activeCount_; }
    const std::array<SplashParticle, MAX_SPLASHES>& particles() const { return particles_; }
    std::uint64_t revision() const { return revision_; }
    std::uint64_t requests() const { return requests_; }
    std::uint64_t rejected() const { return rejected_; }
    std::uint64_t filtered() const { return filtered_; }
    std::uint64_t surfaceReturns() const { return surfaceReturns_; }

private:
    float random(float low, float high);
    std::array<SplashParticle, MAX_SPLASHES> particles_{};
    std::mt19937 random_{2026};
    std::size_t cursor_ = 0;
    std::size_t activeCount_ = 0;
    std::uint64_t revision_ = 0;
    std::uint64_t requests_ = 0, rejected_ = 0, filtered_ = 0;
    std::uint64_t surfaceReturns_ = 0;
};
