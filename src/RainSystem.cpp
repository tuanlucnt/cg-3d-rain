#include "RainSystem.hpp"
#include "SceneCollision.hpp"

#include <algorithm>
#include <cmath>

float RainSystem::random(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random_);
}

void RainSystem::respawn(Raindrop& drop, const glm::vec3& wind, bool initial) {
    drop.position = {random(-9.95f, 9.95f), initial ? random(0.05f, 8.4f) : random(7.6f, 8.4f),
                     random(-9.95f, 9.95f)};
    // Initial distribution must not seed rain inside/under a solid surface.
    collision::Hit hit;
    if (initial && collision::earliestHit({drop.position.x, 8.4f, drop.position.z}, drop.position, hit))
        drop.position.y = random(7.6f, 8.4f);
    drop.previousPosition = drop.position;
    drop.velocity = {wind.x, random(-9.0f, -6.0f), wind.z};
    drop.streakScale = random(0.85f, 1.15f);
    drop.opacity = random(0.55f, 0.82f);
    updateStreak(drop);
    drop.lifetime = 0;
}

void RainSystem::updateStreak(Raindrop& drop) {
    drop.length = std::clamp(glm::length(drop.velocity) * 0.024f * drop.streakScale, 0.12f, 0.42f);
    const auto tail = drop.position - glm::normalize(drop.velocity) *
        drop.length * collision::MAX_RAIN_STREAK_SCALE;
    drop.streakLengthLimit = glm::distance(drop.position, collision::clipStreak(drop.position, tail)) /
        collision::MAX_RAIN_STREAK_SCALE;
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
        drop.previousPosition = drop.position;
        drop.velocity += glm::vec3(0, -9.81f, 0) * deltaTime;
        drop.velocity.y = std::max(drop.velocity.y, -17.0f);
        const float drag = std::min(2.0f * deltaTime, 1.0f);
        drop.velocity.x += (wind.x - drop.velocity.x) * drag;
        drop.velocity.z += (wind.z - drop.velocity.z) * drag;
        const auto next = drop.position + drop.velocity * deltaTime;
        drop.lifetime += deltaTime;
        collision::Hit hit;
        if (collision::earliestHit(drop.previousPosition, next, hit)) {
            impacts_[impactCount_++] = {hit.position, hit.normal, drop.velocity, hit.surface, hit.t};
            respawn(drop, wind, false);
            ++recycledCount_;
            continue; // Exactly one primary event before respawning, for every surface.
        }
        drop.position = next;
        if (drop.position.y <= 0 || std::abs(drop.position.x) > 10 || std::abs(drop.position.z) > 10) {
            respawn(drop, wind, false);
            ++recycledCount_;
        } else {
            updateStreak(drop);
        }
    }
    ++revision_;
}
