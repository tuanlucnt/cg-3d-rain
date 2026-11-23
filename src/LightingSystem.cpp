#include "LightingSystem.hpp"

#include <algorithm>
#include <cmath>

void LightingSystem::reset() {
    flash_ = 0;
    wetness_ = 0.65f;
    flashPosition_ = {0, 8, 0};
    fogDensity = 0.018f;
    setEnvironment(DEFAULT_HOUR,0.9f);
}

void LightingSystem::setEnvironment(float hour, float storminess) {
    if (!std::isfinite(hour) || !std::isfinite(storminess)) return;
    hour=std::fmod(hour,24.0f); if (hour<0) hour+=24;
    const float storm=std::clamp(storminess,0.0f,1.0f);
    const auto smooth=[](float a,float b,float x) { const float t=std::clamp((x-a)/(b-a),0.0f,1.0f); return t*t*(3-2*t); };
    const float dawn=smooth(5,7,hour), dusk=smooth(17,19,hour);
    daylight_=dawn*(1-dusk);
    const float twilight=4*(dawn*(1-dawn)+dusk*(1-dusk));
    windows_=1-smooth(0.05f,0.85f,daylight_);
    ambient_=glm::mix(glm::vec3(0.25f,0.30f,0.40f),glm::vec3(0.59f,0.63f,0.68f),daylight_);
    ambient_+=glm::vec3(0.14f,0.095f,0.045f)*twilight;
    ambient_*=1-0.18f*storm;
    const float angle=(hour-6)*3.14159265359f/12;
    const float elevation=std::max(std::sin(angle),0.0f);
    const glm::vec3 sun=glm::normalize(glm::vec3(-std::cos(angle),std::max(0.16f,elevation),0.32f));
    const glm::vec3 moon=glm::normalize(glm::vec3(0.35f,0.8f,-0.4f));
    direction_=-glm::normalize(glm::mix(moon,sun,daylight_));
    const auto sunColor=glm::mix(glm::vec3(0.96f,0.94f,0.85f),glm::vec3(1.0f,0.66f,0.40f),twilight*0.65f);
    directional_=glm::vec3(0.10f,0.15f,0.24f)*(1-daylight_)+
        sunColor*daylight_*(0.30f+0.42f*elevation)*(1-0.30f*storm);
    const auto dayTop=glm::mix(glm::vec3(0.24f,0.43f,0.63f),glm::vec3(0.095f,0.155f,0.235f),storm);
    const auto dayHorizon=glm::mix(glm::vec3(0.49f,0.61f,0.69f),glm::vec3(0.27f,0.34f,0.40f),storm);
    top_=glm::mix(glm::vec3(0.025f,0.043f,0.085f),dayTop,daylight_);
    horizon_=glm::mix(glm::vec3(0.075f,0.105f,0.165f),dayHorizon,daylight_);
    top_+=glm::vec3(0.045f,0.025f,0.035f)*twilight*(1-0.5f*storm);
    horizon_+=glm::vec3(0.14f,0.055f,0.02f)*twilight*(1-0.45f*storm);
}

glm::vec3 LightingSystem::particleColor() const {
    return glm::mix(glm::vec3(0.48f,0.64f,0.83f),glm::vec3(0.88f,0.94f,1.0f),daylight_)+glm::vec3(0.20f)*flash_;
}

void LightingSystem::update(float flashIntensity, float deltaTime, bool raining) {
    flash_ = std::isfinite(flashIntensity) ? std::clamp(flashIntensity, 0.0f, 1.0f) : 0;
    if (std::isfinite(deltaTime) && deltaTime > 0) {
        wetness_ = std::clamp(wetness_ + (raining ? 0.06f : -0.012f) *
                              std::min(deltaTime, 0.1f), 0.0f, 1.0f);
    }
}

glm::vec3 LightingSystem::skyColor() const {
    return glm::mix(top_,horizon_,0.45f) + glm::vec3(0.14f, 0.16f, 0.20f) * flash_;
}

float LightingSystem::attenuationAt(glm::vec3 point) const {
    const float d = glm::distance(point, flashPosition_);
    return 1.0f / (1.0f + LINEAR_ATTENUATION * d + QUADRATIC_ATTENUATION * d * d);
}

glm::vec3 LightingSystem::skyTop() const {
    return top_ + glm::vec3(0.14f, 0.16f, 0.20f) * flash_;
}

glm::vec3 LightingSystem::fogColor() const {
    return horizon_ + glm::vec3(0.14f, 0.16f, 0.20f) * flash_;
}

float LightingSystem::fogFactor(float distance) const {
    if (!std::isfinite(distance)) return 0;
    return std::exp(-std::max(fogDensity, 0.0f) * std::max(distance - 6.0f, 0.0f));
}
