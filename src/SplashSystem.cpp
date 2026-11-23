#include "SplashSystem.hpp"

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
    ++revision_;
}

void SplashSystem::emit(glm::vec3 hit, bool sampled) {
    ++requests_;
    if (!std::isfinite(hit.x) || !std::isfinite(hit.z) ||
        std::abs(hit.x) > 10 || std::abs(hit.z) > 10) { ++rejected_; return; }
    if (sampled && random(0, 1) > 0.45f) { ++filtered_; return; }
    if (activeCount_ > MAX_SPLASHES - 2) { ++rejected_; return; }
    for (int i = 0; i < 2; ++i) {
        while (particles_[cursor_].lifetime > 0) cursor_ = (cursor_ + 1) % MAX_SPLASHES;
        auto& p = particles_[cursor_];
        ++activeCount_;
        const float angle = random(0, 6.28318530718f), speed = random(0.5f, 1.4f);
        const float lifetime = random(0.16f, 0.28f);
        p = {{hit.x, 0.02f, hit.z},
             {std::cos(angle) * speed, random(1.2f, 2.4f), std::sin(angle) * speed},
             lifetime, lifetime, random(0.05f, 0.10f)};
        cursor_ = (cursor_ + 1) % MAX_SPLASHES;
    }
    ++revision_;
}

void SplashSystem::update(float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0 || !activeCount_) return;
    deltaTime = std::min(deltaTime, 0.1f);
    for (auto& p : particles_) {
        if (p.lifetime <= 0) continue;
        p.velocity.y -= 9.81f * deltaTime;
        p.position += p.velocity * deltaTime;
        p.lifetime -= deltaTime;
        if (p.lifetime <= 0 || p.position.y <= 0.01f) {
            p.lifetime = 0;
            --activeCount_;
        }
    }
    ++revision_;
}
