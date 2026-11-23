#include "RainSystem.hpp"

#include <algorithm>
#include <cmath>

float RainSystem::random(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random_);
}

void RainSystem::respawn(Raindrop& drop, const glm::vec3& wind, bool initial) {
    drop.position = {random(-9.95f, 9.95f), initial ? random(0.05f, 8.4f) : random(7.6f, 8.4f),
                     random(-9.95f, 9.95f)};
    drop.velocity = {wind.x, random(-9.0f, -6.0f), wind.z};
    drop.streakScale = random(0.85f, 1.15f);
    drop.opacity = random(0.55f, 0.82f);
    drop.length = std::clamp(glm::length(drop.velocity) * 0.024f * drop.streakScale, 0.12f, 0.42f);
    drop.lifetime = 0;
}

void RainSystem::reset(const glm::vec3& wind) {
    random_.seed(1337);
    for (auto& drop : drops_) respawn(drop, wind, true);
    impactCount_ = 0;
    recycledCount_ = 0;
    ++revision_;
}

void RainSystem::update(float deltaTime, const glm::vec3& wind) {
    impactCount_ = 0;
    if (!std::isfinite(deltaTime) || deltaTime <= 0) return;
    deltaTime = std::min(deltaTime, 0.1f);
    for (auto& drop : drops_) {
        const glm::vec3 previous = drop.position;
        drop.velocity += glm::vec3(0, -9.81f, 0) * deltaTime;
        drop.velocity.y = std::max(drop.velocity.y, -17.0f);
        const float drag = std::min(2.0f * deltaTime, 1.0f);
        drop.velocity.x += (wind.x - drop.velocity.x) * drag;
        drop.velocity.z += (wind.z - drop.velocity.z) * drag;
        drop.position += drop.velocity * deltaTime;
        // A short exposure interval converts speed to a bounded streak length.
        drop.length = std::clamp(glm::length(drop.velocity) * 0.024f * drop.streakScale, 0.12f, 0.42f);
        drop.lifetime += deltaTime;
        if (drop.position.y <= 0) {
            const float t = previous.y / (previous.y - drop.position.y);
            glm::vec3 hit = glm::mix(previous, drop.position, t);
            hit.y = 0;
            if (std::abs(hit.x) <= 10 && std::abs(hit.z) <= 10) {
                impacts_[impactCount_++] = hit;
            }
            respawn(drop, wind, false);
            ++recycledCount_;
        } else if (std::abs(drop.position.x) > 10 || std::abs(drop.position.z) > 10) {
            respawn(drop, wind, false);
            ++recycledCount_;
        }
    }
    ++revision_;
}
