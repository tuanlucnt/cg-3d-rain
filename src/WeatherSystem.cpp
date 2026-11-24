#include "WeatherSystem.hpp"
#include "Surface.hpp"

#include <algorithm>
#include <cmath>

void WeatherSystem::reset() {
    windVelocity = {0.65f, 0.0f, 0.2f};
    clouds.reset();
    rain.reset(windVelocity);
    lightning.reset();
    lighting.reset();
    timeOfDay.reset();
    splashes.reset();
    ripples.reset();
    cloudsEnabled = true;
    rainEnabled = true;
    lightningEnabled = true;
    paused = false;
    fogEnabled = true;
    accumulator_ = 0;
    refreshLighting();
}

void WeatherSystem::refreshLighting() {
    const float storm=cloudsEnabled ? (rainEnabled ? 0.9f : 0.55f) : (rainEnabled ? 0.45f : 0.0f);
    lighting.setEnvironment(static_cast<float>(timeOfDay.hour()),storm);
}

void WeatherSystem::update(float deltaTime) {
    if (paused || !std::isfinite(deltaTime) || deltaTime <= 0) { refreshLighting(); return; }
    accumulator_ += std::min(deltaTime, 0.1f);
    constexpr double step = 1.0 / 120.0;
    while (accumulator_ + 1e-7 >= step) {
        timeOfDay.update(step);
        if (cloudsEnabled) clouds.update(static_cast<float>(step), windVelocity);
        if (rainEnabled) rain.update(static_cast<float>(step), windVelocity);
        lightning.update(static_cast<float>(step), clouds, lightningEnabled, observerPosition);
        lighting.setFlashPosition(lightning.lightPosition());
        splashes.update(static_cast<float>(step));
        ripples.update(static_cast<float>(step));
        if (rainEnabled) {
            for (std::size_t i = 0; i < rain.impactCount(); ++i) {
                const auto& hit = rain.impacts()[i];
                const bool sampled = hit.surface == SurfaceType::Ground || hit.surface == SurfaceType::Water;
                splashes.emitSurface(hit.position, hit.normal, hit.velocity, hit.surface, sampled);
                if (hit.surface == SurfaceType::Water) ripples.emit(hit.position, true);
            }
        }
        lighting.update(lightning.flashIntensity(), static_cast<float>(step), rainEnabled);
        accumulator_ = std::max(0.0, accumulator_ - step);
    }
    refreshLighting();
}

void WeatherSystem::triggerLightning() {
    lightning.trigger(clouds, observerPosition);
    lighting.setFlashPosition(lightning.lightPosition());
    lighting.update(lightning.flashIntensity());
}

void WeatherSystem::toggleLightning() {
    lightningEnabled = !lightningEnabled;
    if (!lightningEnabled) lightning.cancel();
    lighting.update(lightning.flashIntensity());
}

bool WeatherSystem::valid() const {
    if (!std::isfinite(timeOfDay.hour()) || timeOfDay.hour()<0 || timeOfDay.hour()>=24) return false;
    const auto finite = [](glm::vec3 p) { return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z); };
    if (!std::isfinite(lighting.wetness()) || lighting.wetness() < 0 || lighting.wetness() > 1 ||
        !std::isfinite(lighting.flash()) || lighting.flash() < 0 || lighting.flash() > 1) return false;
    for (const auto& p : rain.drops())
        if (!finite(p.position) || !finite(p.velocity) || !std::isfinite(p.length) ||
            p.position.y <= 0 || p.position.y > 8.4f || std::abs(p.position.x) > 10 || std::abs(p.position.z) > 10) return false;
    std::size_t count = 0;
    for (const auto& p : splashes.particles()) {
        if (!finite(p.position) || !finite(p.velocity) || !std::isfinite(p.lifetime)) return false;
        if (p.lifetime > 0) ++count;
    }
    if (count != splashes.activeCount()) return false;
    count = 0;
    for (const auto& p : ripples.particles()) {
        if (!finite(p.center) || !std::isfinite(p.radius) || !std::isfinite(p.alpha)) return false;
        if (p.lifetime > 0) ++count;
    }
    if (count != ripples.activeCount()) return false;
    for (const auto& c : clouds.clusters())
        if (!finite(c.position) || std::abs(c.position.x) > 10 || std::abs(c.position.z) > 10) return false;
    for (std::size_t i = 0; i < lightning.segmentCount(); ++i)
        if (!finite(lightning.segments()[i].start) || !finite(lightning.segments()[i].end)) return false;
    return true;
}
