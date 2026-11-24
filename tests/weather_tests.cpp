#include "WeatherSystem.hpp"
#include "Surface.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void cloudTests() {
    WeatherSystem weather;
    const auto start = weather.clouds.clusters()[0].position;
    weather.update(0.1f);
    const auto displacement = weather.clouds.clusters()[0].position - start;
    require(displacement.x > 0 && displacement.z > 0 && displacement.y == 0,
            "Clouds must move horizontally with the shared wind");
    weather.cloudsEnabled = false;
    const auto stopped = weather.clouds.clusters()[0].position;
    weather.update(0.1f);
    require(glm::distance(stopped, weather.clouds.clusters()[0].position) == 0,
            "Disabled clouds must stop");
    weather.reset();
    require(glm::distance(start, weather.clouds.clusters()[0].position) == 0,
            "Reset restores clouds");
    for (int i = 0; i < 10000; ++i) weather.clouds.update(0.1f, weather.windVelocity);
    for (const auto& cluster : weather.clouds.clusters()) {
        require(std::abs(cluster.position.x) <= 10 && std::abs(cluster.position.z) <= 10,
                "Cloud wrapping bounds");
    }
    require(CloudSystem::coverage({10, 8, 0}) == 0 &&
            CloudSystem::coverage({-10, 8, 0}) == 0 && CloudSystem::coverage({0, 8, 0}) == 1,
            "Cloud wrap seam must fade smoothly to zero");
    CloudSystem one, many;
    for (const auto& cluster : one.clusters()) {
        std::array<bool, 5> connected{{true, false, false, false, false}};
        for (int pass = 0; pass < 5; ++pass) {
            for (std::size_t i = 0; i < 5; ++i) {
                if (!connected[i]) continue;
                for (std::size_t j = 0; j < 5; ++j) {
                    if (i == j) continue;
                    const auto delta = cluster.lobes[j].offset - cluster.lobes[i].offset;
                    const float a = glm::length(delta / cluster.lobes[i].scale);
                    const float b = glm::length(delta / cluster.lobes[j].scale);
                    if (a < 0.00001f || b < 0.00001f || 1.0f / a + 1.0f / b >= 1.0f)
                        connected[j] = true;
                }
            }
        }
        for (bool attached : connected) require(attached, "Cloud lobes must overlap in one connected mass");
    }
    require(one.clusters()[0].lobes[3].scale.y != one.clusters()[1].lobes[3].scale.y,
            "Cloud profiles must differ across clusters");
    one.update(0.1f, {1, 0, 1});
    for (int i = 0; i < 10; ++i) many.update(0.01f, {1, 0, 1});
    require(glm::distance(one.clusters()[0].position, many.clusters()[0].position) < 0.00001f,
            "Cloud motion must use deltaTime");
    std::cout << "PASS clouds: movement, shared wind, toggle, wrapping, reset, deltaTime\n";
}

void rainTests() {
    RainSystem rain;
    require(rain.drops().size() == 1000, "Rain pool must contain exactly 1000 particles");
    const auto* pool = rain.drops().data();
    std::array<int, 16> bins{};
    float minLength = 1, maxLength = 0, minOpacity = 1, maxOpacity = 0;
    for (const auto& drop : rain.drops()) {
        const int x = std::min(3, static_cast<int>((drop.position.x + 10) / 5));
        const int z = std::min(3, static_cast<int>((drop.position.z + 10) / 5));
        ++bins[z * 4 + x];
        minLength = std::min(minLength, drop.length); maxLength = std::max(maxLength, drop.length);
        minOpacity = std::min(minOpacity, drop.opacity); maxOpacity = std::max(maxOpacity, drop.opacity);
    }
    for (int count : bins) require(count >= 25 && count <= 100,
                                  "Rain must cover all 16 ground regions, without a concentrated corner");
    require(maxLength > minLength * 1.3f && maxOpacity - minOpacity > 0.15f,
            "Rain needs visible variation in length and brightness");
    std::size_t index = 0;
    while (rain.drops()[index].position.y < 2) ++index;
    const auto start = rain.drops()[index];
    rain.update(1.0f / 120.0f, {1, 0, 0.5f});
    const auto moved = rain.drops()[index];
    require(moved.position.y < start.position.y && moved.velocity.y < start.velocity.y,
            "Gravity accelerates rain downward");
    require(moved.length > start.length, "Accelerating drops must form longer bounded streaks");
    require(moved.position.x > start.position.x && moved.position.z > start.position.z,
            "Wind tilts falling rain");
    std::size_t impacts = 0;
    for (int i = 0; i < 2400; ++i) {
        rain.update(1.0f / 120.0f, {0.65f, 0, 0.2f});
        impacts += rain.impactCount();
        for (std::size_t k = 0; k < rain.impactCount(); ++k) {
            const auto& hit = rain.impacts()[k];
            require(std::abs(hit.position.x) <= 10 && std::abs(hit.position.z) <= 10 &&
                    hit.t >= 0 && hit.t <= 1, "Impact must lie on the segment and bounded scene");
            if (hit.surface == SurfaceType::Ground) require(hit.position.y == 0, "Ground impact height");
            if (hit.surface == SurfaceType::Water)
                require(std::abs(hit.position.y - surface::waterAt(hit.position).height) < 1e-5f,
                        "Water impact must be on the rendered water plane");
        }
    }
    require(rain.recycledCount() > 1000 && impacts > 1000, "Rain must recycle repeatedly");
    require(pool == rain.drops().data(), "Rain pool storage must remain fixed");
    for (const auto& drop : rain.drops()) {
        require(drop.position.y > 0 && drop.position.y <= 8.4f && std::abs(drop.position.x) <= 10 &&
                std::abs(drop.position.z) <= 10, "Recycled rain must stay inside its volume");
        require(drop.velocity.y >= -17 && drop.velocity.y < 0, "Terminal velocity bound");
        require(drop.length >= 0.12f && drop.length <= 0.42f, "Rain streak length bound");
    }
    RainSystem still;
    still.reset({0, 0, 0});
    const auto stillStart = still.drops()[index].position;
    still.update(0.01f, {0, 0, 0});
    require(still.drops()[index].position.x == stillStart.x &&
            still.drops()[index].position.z == stillStart.z, "No wind means vertical rain");
    for (int i = 0; i < 120; ++i) still.update(1.0f / 120.0f, {-1, 0, -0.5f});
    for (const auto& drop : still.drops())
        require(drop.velocity.x < 0 && drop.velocity.z < 0, "Rain follows reversed shared wind");
    WeatherSystem a, b;
    for (int i = 0; i < 10; ++i) a.update(0.1f);
    for (int i = 0; i < 100; ++i) b.update(0.01f);
    for (std::size_t i = 0; i < 1000; ++i)
        require(glm::distance(a.rain.drops()[i].position, b.rain.drops()[i].position) < 0.0001f,
                "Fixed simulation steps must make rain independent of render FPS");
    a.rainEnabled = false;
    const auto stopped = a.rain.drops()[0].position;
    a.update(0.1f);
    require(glm::distance(stopped, a.rain.drops()[0].position) == 0, "Rain toggle freezes pool");
    std::cout << "PASS rain: 1000 particles, gravity, wind, recycling, fixed storage, FPS independence\n";
}

void lightningTests() {
    CloudSystem clouds;
    LightningSystem lightning;
    require(lightning.nextStrike() >= 2 && lightning.nextStrike() <= 6,
            "Random strike interval must be 2..6 seconds");
    lightning.trigger(clouds);
    require(lightning.active() && lightning.flashIntensity() == 1, "Immediate manual flash");
    require(lightning.segmentCount() == 80 && lightning.segmentCount() <= LightningSystem::MAX_SEGMENTS,
            "Bounded 32-segment trunk and six 8-segment branches");
    const auto& segments = lightning.segments();
    const glm::vec3 start = segments[0].start;
    const glm::vec3 end = segments[LightningSystem::MAIN_SEGMENTS - 1].end;
    const glm::vec3 axis = glm::normalize(end - start);
    bool displaced = false;
    for (std::size_t i = 0; i < LightningSystem::MAIN_SEGMENTS; ++i) {
        require(!segments[i].branch, "Primary polyline precedes branches");
        require(glm::distance(segments[i].start, segments[i].end) > 0.001f, "Nondegenerate lightning");
        if (i) require(glm::distance(segments[i - 1].end, segments[i].start) < 0.00001f,
                       "Primary segment connectivity");
        if (glm::length(glm::cross(segments[i].end - start, axis)) > 0.05f) displaced = true;
    }
    require(displaced, "Midpoint displacement must produce a zigzag, not a straight line");
    for (std::size_t branch = 0; branch < LightningSystem::MAX_BRANCHES; ++branch) {
        const std::size_t first = LightningSystem::MAIN_SEGMENTS + branch * 8;
        bool attached = false;
        for (std::size_t i = 0; i < LightningSystem::MAIN_SEGMENTS; ++i)
            attached |= glm::distance(segments[first].start, segments[i].end) < 0.00001f;
        require(attached, "Branch must attach exactly to the trunk");
        for (std::size_t i = first; i < first + 8; ++i) {
            require(segments[i].branch && segments[i].width < segments[0].width,
                    "Secondary branches must be thinner");
            if (i > first) require(glm::distance(segments[i - 1].end, segments[i].start) < 0.00001f,
                                   "Branch segment connectivity");
        }
    }
    const auto oldPoint = segments[1].end;
    lightning.update(0.01f, clouds, false);
    const float firstFlash = lightning.flashIntensity();
    lightning.update(0.01f, clouds, false);
    require(lightning.flashIntensity() < firstFlash, "Flash decays exponentially within a pulse");
    for (int i = 0; i < 80; ++i) {
        lightning.update(1.0f / 120.0f, clouds, false);
        require(lightning.flashIntensity() >= 0 && lightning.flashIntensity() <= 1,
                "Flash intensity bounds");
    }
    require(!lightning.active() && lightning.flashIntensity() == 0, "Strike expires and removes flash");
    lightning.trigger(clouds);
    require(glm::distance(oldPoint, lightning.segments()[1].end) > 0.01f,
            "Every strike must generate different geometry");
    lightning.cancel();
    require(!lightning.active() && lightning.flashIntensity() == 0, "Disabling cancels a strike");
    const auto count = lightning.strikeCount();
    for (int i = 0; i < 800; ++i) lightning.update(1.0f / 120.0f, clouds, true);
    require(lightning.strikeCount() > count, "Automatic strikes must trigger without input");
    LightingSystem lighting;
    lighting.setFlashPosition({0, 8, 0});
    require(lighting.attenuationAt({0, 8, 0}) == 1 &&
            lighting.attenuationAt({1, 8, 0}) > lighting.attenuationAt({10, 8, 0}),
            "Nearby clouds must receive more flash illumination than distant clouds");
    const auto sky = lighting.skyColor();
    lighting.update(1);
    require(lighting.skyColor().r > sky.r, "Flash also brightens the sky");
    lighting.update(0);
    require(glm::distance(sky, lighting.skyColor()) == 0, "Background lighting restores exactly");
    bool foundMultiplePulses = false;
    for (std::uint32_t seed = 0; seed < 100; ++seed) {
        LightningSystem sample;
        sample.reset(seed);
        sample.trigger(clouds);
        float trunkLength = 0;
        for (std::size_t i = 0; i < sample.segmentCount(); ++i) {
            const auto& s = sample.segments()[i];
            require(glm::distance(s.start, s.end) > 0.0001f, "Generated segments must remain nondegenerate");
            for (auto p : {s.start, s.end})
                require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) &&
                        std::abs(p.x) <= 9.8f && std::abs(p.z) <= 9.8f && p.y >= 0.029f && p.y <= 10,
                        "Lightning geometry must stay in the bounded scene volume");
            if (i < LightningSystem::MAIN_SEGMENTS) trunkLength += glm::distance(s.start, s.end);
            if (i && i < LightningSystem::MAIN_SEGMENTS) {
                const auto& previous = sample.segments()[i - 1];
                require(glm::dot(glm::normalize(previous.end - previous.start),
                                 glm::normalize(s.end - s.start)) > -0.05f,
                        "Lightning trunk must not form sharp reversing kinks");
            }
        }
        for (std::size_t branch = 0; branch < LightningSystem::MAX_BRANCHES; ++branch) {
            float length = 0;
            const std::size_t first = LightningSystem::MAIN_SEGMENTS + branch * 8;
            for (std::size_t j = first; j < first + 8; ++j)
                length += glm::distance(sample.segments()[j].start, sample.segments()[j].end);
            require(length < trunkLength, "Branches must be shorter than the primary bolt");
            require(sample.segments()[first].width > sample.segments()[first + 7].width,
                    "Branch widths must taper toward their tips");
        }
        if (sample.pulseCount() > 1) {
            foundMultiplePulses = true;
            const auto first = sample.pulses()[0], second = sample.pulses()[1];
            require(first.start + first.duration < second.start && first.peak != second.peak,
                    "Lightning pulses need dark gaps and different intensities");
            const float gap = (first.start + first.duration + second.start) * 0.5f;
            while (sample.elapsed() < gap) sample.update(std::min(0.001f, gap - sample.elapsed()), clouds, false);
            require(sample.flashIntensity() == 0 && sample.active(), "Real dark gap within an active strike");
            const auto revision = sample.revision();
            while (sample.elapsed() < second.start + 0.002f) sample.update(0.001f, clouds, false);
            require(sample.flashIntensity() > 0.1f && sample.revision() == revision,
                    "Next pulse reuses the same bolt geometry");
        }
    }
    require(foundMultiplePulses, "Deterministic seeds must exercise multiple lightning pulses");
    for (const auto& cloud : clouds.clusters()) {
        LightningSystem sample;
        const glm::vec3 observer = cloud.position;
        sample.trigger(clouds, observer);
        for (std::size_t i = 0; i < sample.segmentCount(); ++i) {
            const auto& s = sample.segments()[i];
            const auto v = s.end - s.start;
            const float t = std::clamp(glm::dot(observer - s.start, v) / glm::dot(v, v), 0.0f, 1.0f);
            require(glm::distance(observer, s.start + t * v) > 0.75f,
                    "A nearby observer must not be chosen as the strike location");
        }
    }
    std::cout << "PASS lightning: midpoint zigzag, connectivity, branches, lifecycle, random/manual, flash\n";
}

void splashTests() {
    SplashSystem splashes;
    const auto* pool = splashes.particles().data();
    splashes.emit({8, 0, 8});
    require(splashes.activeCount() >= 2 && splashes.activeCount() <= 4, "Two to four droplets per ground impact");
    const auto start = splashes.particles()[0].position;
    splashes.update(1.0f / 120.0f);
    require(splashes.particles()[0].position.y > start.y, "Splashes initially move upward");
    for (int i = 0; i < 2000; ++i) splashes.emit({8, 0, 8});
    require(splashes.activeCount() >= SplashSystem::MAX_SPLASHES - 3 &&
            splashes.activeCount() <= SplashSystem::MAX_SPLASHES,
            "Full splash pool must reuse existing particles, never exceed its cap");
    for (int i = 0; i < 60; ++i) splashes.update(1.0f / 120.0f);
    require(splashes.activeCount() == 0, "All splash particles expire without new rain");
    require(pool == splashes.particles().data(), "Splash storage remains fixed");
    splashes.emit({11, 0, 0});
    require(splashes.activeCount() == 0, "Do not splash beyond the ground");
    splashes.emit({0, 0, 0});
    splashes.reset();
    require(splashes.activeCount() == 0, "Splash reset clears every particle");
    WeatherSystem weather;
    std::size_t peak = 0;
    for (int i = 0; i < 1200; ++i) {
        weather.update(1.0f / 120.0f);
        std::size_t active = 0;
        for (const auto& p : weather.splashes.particles()) {
            if (p.lifetime > 0) {
                ++active;
                require(std::isfinite(p.lifetime) && p.lifetime <= p.initialLifetime &&
                        p.initialLifetime <= 0.32f, "Splash lifetime must be finite and bounded");
            }
        }
        require(active == weather.splashes.activeCount(), "Active count must match live slots exactly");
        peak = std::max(peak, active);
    }
    require(peak > 0 && peak < SplashSystem::MAX_SPLASHES && weather.splashes.filtered() > 0,
            "Sampled impacts must keep the steady splash pool below saturation");
    weather.rainEnabled = false;
    for (int i = 0; i < 60; ++i) weather.update(1.0f / 120.0f);
    require(weather.splashes.activeCount() == 0, "A saturated pool must drain when emissions stop");
    std::cout << "PASS splash: emission, upward motion, capacity, recycling, lifetime, reset\n";
}

void integrationTests() {
    WeatherSystem weather;
    require(weather.lightningEnabled && !weather.lightning.active() &&
            weather.lightning.nextStrike() >= 2 && weather.lightning.nextStrike() <= 6,
            "Lightning defaults to enabled/idle with a valid countdown");
    for (int i = 0; i < 120; ++i) weather.update(1.0f / 120.0f);
    require(weather.splashes.activeCount() > 0, "Rain ground impacts feed the splash system");
    weather.triggerLightning();
    const auto rain = weather.rain.drops()[0].position;
    const auto cloud = weather.clouds.clusters()[0].position;
    const auto splash = weather.splashes.particles()[0].position;
    const float flash = weather.lighting.flash();
    weather.paused = true;
    weather.update(0.1f);
    require(glm::distance(rain, weather.rain.drops()[0].position) == 0 &&
            glm::distance(cloud, weather.clouds.clusters()[0].position) == 0 &&
            glm::distance(splash, weather.splashes.particles()[0].position) == 0 &&
            weather.lighting.flash() == flash, "Pause freezes all weather systems and lighting");
    weather.paused = false;
    weather.rainEnabled = false;
    for (int i = 0; i < 60; ++i) weather.update(1.0f / 120.0f);
    require(weather.splashes.activeCount() == 0, "Existing splashes expire when rain is disabled");
    require(glm::distance(rain, weather.rain.drops()[0].position) == 0,
            "Disabling rain retains the original particle pool state");
    require(weather.lighting.flash() == 0, "Flash illumination restores after a strike");
    weather.toggleLightning();
    weather.triggerLightning();
    require(!weather.lightningEnabled && weather.lightning.active(),
            "Manual lightning works independently of automatic strikes");
    weather.reset();
    require(weather.rainEnabled && weather.cloudsEnabled && weather.lightningEnabled && !weather.paused &&
            weather.lightning.strikeCount() == 0 && weather.splashes.activeCount() == 0 &&
            weather.lighting.flash() == 0, "Full reset restores every system and toggle");

    WeatherSystem clamped, normal;
    clamped.update(10);
    normal.update(0.1f);
    require(glm::distance(clamped.rain.drops()[0].position, normal.rain.drops()[0].position) == 0,
            "Stalled deltaTime must be clamped to 0.1 seconds");
    const auto before = normal.rain.drops()[0].position;
    normal.update(-1);
    normal.update(std::numeric_limits<float>::quiet_NaN());
    normal.update(std::numeric_limits<float>::infinity());
    require(glm::distance(before, normal.rain.drops()[0].position) == 0,
            "Invalid deltaTime must not corrupt the simulation");

    WeatherSystem fast, slow;
    for (int i = 0; i < 600; ++i) fast.update(0.01f);
    for (int i = 0; i < 60; ++i) slow.update(0.1f);
    require(fast.lightning.strikeCount() == slow.lightning.strikeCount() &&
            fast.splashes.activeCount() == slow.splashes.activeCount(),
            "Render FPS must not affect strike scheduling or splash lifecycle");
    for (std::size_t i = 0; i < 1000; ++i)
        require(glm::distance(fast.rain.drops()[i].position, slow.rain.drops()[i].position) < 0.0001f,
                "Integrated rain remains deterministic across frame rates");
    for (std::size_t i = 0; i < SplashSystem::MAX_SPLASHES; ++i)
        require(glm::distance(fast.splashes.particles()[i].position,
                              slow.splashes.particles()[i].position) < 0.0001f,
                "Integrated splash remains deterministic across frame rates");
    require(fast.lighting.wetness() > 0.65f && fast.lighting.groundMultiplier() < 1,
            "Rain gradually darkens wet ground");
    std::cout << "PASS integration: impacts, pause, reset, toggles, deltaTime, lighting, FPS independence\n";
}

void rippleTests() {
    RippleSystem ripple;
    const auto* pool = ripple.particles().data();
    const auto water = surface::puddles()[0].center;
    ripple.emit(water);
    const auto first = ripple.particles()[0];
    ripple.update(0.1f);
    require(ripple.particles()[0].radius > first.radius && ripple.particles()[0].alpha < first.alpha,
            "Ripple expands while fading");
    for (int i = 0; i < 200; ++i) ripple.emit(water);
    require(ripple.activeCount() == RippleSystem::CAPACITY && ripple.rejected() > 0,
            "Ripple pool enforces its fixed cap");
    for (int i = 0; i < 10; ++i) ripple.update(0.1f);
    require(ripple.activeCount() == 0 && ripple.particles().data() == pool, "Ripple expires, storage stays fixed");
    WeatherSystem w;
    for (int i = 0; i < 120; ++i) w.update(1.0f / 120.0f);
    require(w.ripples.activeCount() > 0, "Real rain must produce ripples");
    w.reset();
    require(w.ripples.activeCount() == 0, "R resets ripples");
}

void surfaceTests() {
    LightingSystem wet, dry;
    const float initial = wet.wetness();
    wet.update(0, 0.1f, true); dry.update(0, 0.1f, false);
    require(wet.wetness() > initial && dry.wetness() < initial, "Wetness follows rain state");
    for (int i = 0; i < 1000; ++i) { wet.update(0, 0.1f, true); dry.update(0, 0.1f, false); }
    require(wet.wetness() == 1 && dry.wetness() == 0, "Wetness clamps to [0,1]");
    for (const auto& p : surface::puddles()) {
        const float margin = surface::waterMargin(p.center);
        require(margin > 0.3f, "Puddle center lies inside its polygon");
        RippleSystem ripple;
        ripple.emit(p.center);
        for (int i = 0; i < 4; ++i) ripple.update(0.1f);
        require(ripple.particles()[0].radius <= margin && ripple.particles()[0].center.y > p.center.y,
                "Water ripple fits its puddle and stays above its surface");
    }
    require(surface::waterMargin({9, 0, 9}) == 0, "Dry ground is not a puddle");
    LightingSystem fog;
    require(fog.fogFactor(0) == 1 && fog.fogFactor(10) > fog.fogFactor(30) && fog.fogFactor(30) > 0.5f,
            "Fog must preserve nearby objects and fade distant geometry moderately");
    fog.fogDensity = 0;
    require(fog.fogFactor(50) == 1, "Zero density disables fog");
}
}

int main() {
    try {
        cloudTests();
        rainTests();
        lightningTests();
        splashTests();
        integrationTests();
        rippleTests();
        surfaceTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
