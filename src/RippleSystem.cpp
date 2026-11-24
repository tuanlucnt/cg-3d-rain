#include "RippleSystem.hpp"
#include "Surface.hpp"
#include <algorithm>
#include <cmath>

void RippleSystem::reset() {
    particles_.fill({});
    active_ = cursor_ = 0;
    rejected_ = 0;
    random_.seed(76);
    ++revision_;
}

void RippleSystem::emit(glm::vec3 hit, bool sampled) {
    const auto water = surface::waterAt(hit);
    if (!water || !std::isfinite(hit.y) || std::abs(hit.y - water.height) > 0.001f) return;
    if (sampled && std::uniform_real_distribution<float>(0, 1)(random_) > 0.35f) return;
    // An inscribed disk is entirely inside even a concave shoreline. Every
    // GL_LINES chord of the ring is a subset of that disk, so it also fits.
    const float maxRadius = std::min(0.45f, water.margin * 0.9f);
    if (maxRadius < 0.002f) return;
    if (active_ == CAPACITY) { ++rejected_; return; }
    while (particles_[cursor_].lifetime > 0) cursor_ = (cursor_ + 1) % CAPACITY;
    const float initialRadius = std::min(0.04f, maxRadius * 0.25f);
    particles_[cursor_] = {{hit.x, water.height + HEIGHT_OFFSET, hit.z}, initialRadius, maxRadius, 0, 0.55f, 0.50f};
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
        p.radius = glm::mix(std::min(0.04f, p.maxRadius * 0.25f), p.maxRadius, t);
        p.alpha = 0.5f * (1 - t) * (1 - t);
        if (t >= 1) { p.lifetime = 0; --active_; }
    }
    ++revision_;
}
