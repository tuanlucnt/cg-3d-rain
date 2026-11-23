#pragma once

#include <glm/glm.hpp>
#include "TimeOfDaySystem.hpp"

class LightingSystem {
public:
    static constexpr float DEFAULT_HOUR = static_cast<float>(TimeOfDaySystem::DEFAULT_HOUR);
    static constexpr float AMBIENT = 0.27f;
    static constexpr float DIFFUSE = 0.53f;
    static constexpr float LINEAR_ATTENUATION = 0.025f;
    static constexpr float QUADRATIC_ATTENUATION = 0.018f;
    LightingSystem() { reset(); }
    void reset();
    // Changes only background lighting; wetness and transient flash stay intact.
    void setEnvironment(float hour, float storminess);
    glm::vec3 ambientColor() const { return ambient_; }
    glm::vec3 directionalColor() const { return directional_; }
    glm::vec3 lightDirection() const { return direction_; }
    glm::vec3 particleColor() const;
    float windowStrength() const { return windows_; }
    float daylight() const { return daylight_; }
    void update(float flashIntensity, float deltaTime = 0, bool raining = true);
    float flash() const { return flash_; }
    void setFlashPosition(glm::vec3 position) { flashPosition_ = position; }
    glm::vec3 flashPosition() const { return flashPosition_; }
    float attenuationAt(glm::vec3 point) const;
    glm::vec3 skyColor() const;
    glm::vec3 skyTop() const;
    glm::vec3 fogColor() const;
    float fogFactor(float distance) const;
    float fogDensity = 0.018f;
    float wetness() const { return wetness_; }
    float groundMultiplier() const { return 1.0f - wetness_ * 0.24f; }

private:
    float flash_ = 0;
    float wetness_ = 0.65f;
    glm::vec3 flashPosition_{0, 8, 0};
    glm::vec3 ambient_{}, directional_{}, direction_{};
    glm::vec3 top_{}, horizon_{};
    float daylight_ = 0, windows_ = 0;
};
