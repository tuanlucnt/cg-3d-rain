#include "LightingSystem.hpp"

#include <algorithm>
#include <cmath>

void LightingSystem::reset() {
    flash_ = 0;
    wetness_ = 0.65f;
    flashPosition_ = {0, 8, 0};
    fogDensity = 0.018f;
}

void LightingSystem::update(float flashIntensity, float deltaTime, bool raining) {
    flash_ = std::isfinite(flashIntensity) ? std::clamp(flashIntensity, 0.0f, 1.0f) : 0;
    if (std::isfinite(deltaTime) && deltaTime > 0) {
        wetness_ = std::clamp(wetness_ + (raining ? 0.06f : -0.012f) *
                              std::min(deltaTime, 0.1f), 0.0f, 1.0f);
    }
}

glm::vec3 LightingSystem::skyColor() const {
    return glm::vec3(0.08f, 0.11f, 0.16f) + glm::vec3(0.14f, 0.16f, 0.20f) * flash_;
}

float LightingSystem::attenuationAt(glm::vec3 point) const {
    const float d = glm::distance(point, flashPosition_);
    return 1.0f / (1.0f + LINEAR_ATTENUATION * d + QUADRATIC_ATTENUATION * d * d);
}

glm::vec3 LightingSystem::skyTop() const {
    return glm::vec3(0.045f, 0.065f, 0.105f) + glm::vec3(0.14f, 0.16f, 0.20f) * flash_;
}

glm::vec3 LightingSystem::fogColor() const {
    return glm::vec3(0.115f, 0.155f, 0.20f) + glm::vec3(0.14f, 0.16f, 0.20f) * flash_;
}

float LightingSystem::fogFactor(float distance) const {
    if (!std::isfinite(distance)) return 0;
    return std::exp(-std::max(fogDensity, 0.0f) * std::max(distance - 6.0f, 0.0f));
}
