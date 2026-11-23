#include "RippleSystem.hpp"
#include <algorithm>
#include <cmath>

void RippleSystem::reset() {
    particles_.fill({});
    active_ = cursor_ = 0;
    rejected_ = 0;
    random_.seed(76);
    ++revision_;
}

void RippleSystem::emit(glm::vec3 hit, bool sampled, float waterMargin) {
    if (!std::isfinite(hit.x) || !std::isfinite(hit.z) ||
        std::abs(hit.x) > 9.5f || std::abs(hit.z) > 9.5f) return;
    if (sampled && std::uniform_real_distribution<float>(0, 1)(random_) > (waterMargin > 0 ? 0.35f : 0.055f)) return;
    const float maxRadius = waterMargin > 0 ? std::min(0.45f, waterMargin * 0.9f) : 0.45f;
    if (maxRadius < 0.08f) return;
    if (active_ == CAPACITY) { ++rejected_; return; }
    while (particles_[cursor_].lifetime > 0) cursor_ = (cursor_ + 1) % CAPACITY;
    particles_[cursor_] = {{hit.x, std::max(0.028f, hit.y + 0.01f), hit.z}, 0.04f, maxRadius, 0, 0.55f, 0.50f};
    cursor_ = (cursor_ + 1) % CAPACITY;
    ++active_;
    ++revision_;
}

void RippleSystem::update(float dt) {
    if (!std::isfinite(dt) || dt <= 0 || !active_) return;
    dt = std::min(dt, 0.1f);
    for (auto& p : particles_) {
        if (p.lifetime <= 0) continue;
        p.elapsed += dt;
        const float t = std::clamp(p.elapsed / p.lifetime, 0.0f, 1.0f);
        p.radius = glm::mix(0.04f, p.maxRadius, t);
        p.alpha = 0.5f * (1 - t) * (1 - t);
        if (t >= 1) { p.lifetime = 0; --active_; }
    }
    ++revision_;
}
