#pragma once

#include "CloudSystem.hpp"
#include "RainSystem.hpp"
#include "LightningSystem.hpp"
#include "LightingSystem.hpp"
#include "SplashSystem.hpp"
#include "RippleSystem.hpp"

class WeatherSystem {
public:
    void reset();
    void update(float deltaTime);
    void triggerLightning();
    void toggleLightning();
    bool valid() const;
    void refreshLighting();
    glm::vec3 windVelocity{0.65f, 0.0f, 0.2f};
    glm::vec3 observerPosition{14, 10, 18};
    CloudSystem clouds;
    RainSystem rain;
    LightningSystem lightning;
    LightingSystem lighting;
    TimeOfDaySystem timeOfDay;
    SplashSystem splashes;
    RippleSystem ripples;
    bool cloudsEnabled = true;
    bool rainEnabled = true;
    bool lightningEnabled = true; // Automatic strikes; manual L always works.
    bool paused = false;
    bool fogEnabled = true;

private:
    double accumulator_ = 0;
};
