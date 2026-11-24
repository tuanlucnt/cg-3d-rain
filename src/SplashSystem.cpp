#include "SplashSystem.hpp"
#include "SceneCollision.hpp"

#include <algorithm>
#include <cmath>

float SplashSystem::random(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random_);
}

void SplashSystem::reset() {
    random_.seed(2026);
    particles_.fill({});
    cursor_ = activeCount_ = 0;
    requests_ = rejected_ = filtered_ = 0;
    surfaceReturns_ = 0;
    ++revision_;
}

void SplashSystem::emit(glm::vec3 hit, bool sampled) {
    emitSurface(hit, {0, 1, 0}, {0, -7, 0}, SurfaceType::Ground, sampled);
}

void SplashSystem::emitSurface(glm::vec3 hit, glm::vec3 normal, glm::vec3 incoming,
                               SurfaceType surface, bool sampled) {
    ++requests_;
    const auto finite = [](glm::vec3 v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
    };
    if (!finite(hit) || !finite(normal) || !finite(incoming) || glm::length(normal) < collision::EPSILON ||
        std::abs(hit.x) > 10 || std::abs(hit.z) > 10) {
        ++rejected_; return;
    }
    // Preserve the existing sampling budget for numerous ground/water impacts.
    if (sampled && random(0, 1) > 0.45f) { ++filtered_; return; }
    const auto& profile = surface == SurfaceType::Roof ? splashConfig::ROOF :
        (surface == SurfaceType::Rock ? splashConfig::ROCK : splashConfig::GROUND);
    normal = glm::normalize(normal);
    const auto reference = std::abs(normal.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    const auto tangent = glm::normalize(glm::cross(normal, reference));
    const auto bitangent = glm::cross(normal, tangent);
    const auto reflected = glm::reflect(incoming, normal) * profile.reflection;
    const int count = std::uniform_int_distribution<int>(profile.minCount, profile.maxCount)(random_);
    if (activeCount_ + count > MAX_SPLASHES) { ++rejected_; return; }
    for (int i = 0; i < count; ++i) {
        while (particles_[cursor_].lifetime > 0) cursor_ = (cursor_ + 1) % MAX_SPLASHES;
        const float angle = random(0, 6.28318530718f);
        auto velocity = normal * random(profile.minNormal, profile.maxNormal) +
            (tangent * std::cos(angle) + bitangent * std::sin(angle)) *
                random(profile.minTangent, profile.maxTangent) + reflected;
        // Even glancing/wind-driven impacts must leave the surface outward.
        const float outward = glm::dot(velocity, normal);
        if (outward < profile.minNormal)
            velocity += normal * (profile.minNormal - outward);
        if (normal.y > 0.5f && velocity.y < 0.15f)
            velocity += normal * ((0.15f - velocity.y) / normal.y);
        const float life = random(profile.minLife, profile.maxLife);
        particles_[cursor_] = {hit + normal * collision::SURFACE_OFFSET, velocity, life, life,
            random(profile.minSize, profile.maxSize), splashConfig::INITIAL_ALPHA, surface};
        ++activeCount_;
        cursor_ = (cursor_ + 1) % MAX_SPLASHES;
    }
    ++revision_;
}

void SplashSystem::update(float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0 || !activeCount_) return;
    deltaTime = std::min(deltaTime, 0.1f);
    // Short trajectory chords also catch a particle that rises and returns to
    // a surface during a long render frame. No secondary emission on return.
    while (deltaTime > 0) {
        const float step = std::min(deltaTime, splashConfig::MAX_STEP);
        for (auto& p : particles_) {
            if (p.lifetime <= 0) continue;
            const float dt = std::min(step, p.lifetime);
            const auto next = p.position + p.velocity * dt -
                glm::vec3(0, 0.5f * splashConfig::GRAVITY * dt * dt, 0);
            collision::Hit hit;
            const bool returned = collision::earliestHit(p.position, next, hit);
            p.velocity.y -= splashConfig::GRAVITY * dt;
            p.lifetime = std::max(0.0f, p.lifetime - dt);
            p.position = returned ? hit.position + hit.normal * collision::SURFACE_OFFSET : next;
            if (returned || p.lifetime <= 0 || p.position.y <= 0) {
                p.lifetime = 0;
                --activeCount_;
                if (returned) ++surfaceReturns_;
            }
            p.alpha = splashConfig::INITIAL_ALPHA * std::clamp(p.lifetime / p.initialLifetime, 0.0f, 1.0f);
        }
        deltaTime = std::max(0.0f, deltaTime - step);
    }
    ++revision_;
}
