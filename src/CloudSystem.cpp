#include "CloudSystem.hpp"

#include <algorithm>
#include <cmath>

namespace {
float wrap(float x) {
    return x - 20.0f * std::floor((x + 10.0f) / 20.0f);
}
}

void CloudSystem::reset() {
    const std::array<glm::vec3, COUNT> centers{{
        {-5.5f, 7.3f, -4.5f}, {1.5f, 8.0f, -5.5f},
        {5.0f, 7.6f, 1.5f}, {-3.5f, 7.5f, 3.7f}}};
    const std::array<glm::vec3, COUNT> proportions{{
        {1.03f, 0.90f, 0.96f}, {0.88f, 1.18f, 1.12f},
        {1.10f, 1.02f, 0.90f}, {0.96f, 0.86f, 1.04f}}};
    const std::array<float, COUNT> skew{{-0.28f, 0.25f, 0.12f, -0.15f}};
    for (std::size_t i = 0; i < COUNT; ++i) {
        const auto p = proportions[i];
        clusters_[i] = {centers[i], 0.20f + static_cast<float>(i) * 0.025f, {{
            {glm::vec3(-1.15f, -0.1f, -0.08f) * p, glm::vec3(1.65f, 0.62f, 1.12f) * p, 0.96f},
            {glm::vec3(0, -0.1f, 0) * p, glm::vec3(2.0f, 0.72f, 1.38f) * p, 1.0f},
            {glm::vec3(1.2f, -0.1f, 0.10f) * p, glm::vec3(1.55f, 0.61f, 1.05f) * p, 0.97f},
            {glm::vec3(-0.55f + skew[i], 0.52f, -0.18f) * p,
             glm::vec3(1.25f, 0.92f + skew[i] * 0.25f, 1.12f) * p, 1.02f},
            {glm::vec3(0.8f, 0.38f - skew[i] * 0.3f, 0.16f) * p,
             glm::vec3(1.05f, 0.75f, 0.92f) * p, 1.0f}
        }}};
    }
}

void CloudSystem::update(float deltaTime, const glm::vec3& windVelocity) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0) return;
    deltaTime = std::min(deltaTime, 0.1f);
    for (auto& cloud : clusters_) {
        cloud.position += glm::vec3(windVelocity.x, 0, windVelocity.z) * cloud.speed * deltaTime;
        cloud.position.x = wrap(cloud.position.x);
        cloud.position.z = wrap(cloud.position.z);
    }
}

float CloudSystem::coverage(glm::vec3 center) {
    // Fade on both sides of the periodic seam, so wrapping cannot pop in view.
    const float edge = 10.0f - std::max(std::abs(center.x), std::abs(center.z));
    const float t = std::clamp(edge / 2.5f, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
