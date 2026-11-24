#include "SceneCollision.hpp"
#include "WeatherSystem.hpp"
#include "EnvironmentGeometry.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
void near(float a, float b, const char* message) { require(std::abs(a - b) < 0.0002f, message); }
glm::vec3 world(glm::vec3 local) { return local + models::HOUSE_POSITION; }
float roofY(float x) { return 4.20f - std::abs(x) * 1.64f / 2.38f; }

collision::Hit vertical(float localX, float localZ) {
    collision::Hit hit;
    require(collision::house().intersect(world({localX, 9, localZ}), world({localX, -1, localZ}), hit),
            "Expected a vertical house collision");
    return hit;
}

void triangleTests() {
    const collision::Triangle tri({0, 0, 0}, {0, 0, 2}, {2, 0, 0});
    near(tri.normal.y, 1, "Outward normal comes from the winding");
    float t = -1;
    require(collision::segmentTriangle({0.5f, 10, 0.5f}, {0.5f, -10, 0.5f}, tri, t), "Fast segment must hit");
    near(t, 0.5f, "Segment fraction");
    for (const auto p : {glm::vec3(0), glm::vec3(1, 0, 1), glm::vec3(0, 0, 2), glm::vec3(2, 0, 0)}) {
        require(collision::segmentTriangle(p + glm::vec3(0, 1, 0), p, tri, t), "Vertices/edges/endpoints included");
        near(t, 1, "Endpoint hit");
    }
    require(collision::segmentTriangle({0.5f, -1, 0.5f}, {0.5f, 1, 0.5f}, tri, t), "Primitive is two-sided");
    require(!collision::segmentTriangle({1.5f, 1, 1.5f}, {1.5f, -1, 1.5f}, tri, t), "Reject outside triangle");
    require(!collision::segmentTriangle({0.5f, 1, 0.5f}, {0.5f, 0.1f, 0.5f}, tri, t), "Reject beyond segment");
    require(!collision::segmentTriangle({0.5f, 1, 0.5f}, {0.8f, 1, 0.5f}, tri, t), "Parallel miss");
    require(!collision::segmentTriangle({0.5f, 0, 0.5f}, {0.8f, 0, 0.5f}, tri, t), "Coplanar motion is not an impact");
    require(!collision::segmentTriangle({0, 1, 0}, {0, 1, 0}, tri, t), "Stationary miss");
    require(!collision::segmentTriangle({NAN, 1, 0}, {0, -1, 0}, tri, t), "Reject nonfinite input");
    require(!collision::segmentTriangle({0, 1, 0}, {0, -1, 0}, collision::Triangle({0,0,0},{0,0,0},{0,0,0}), t),
            "Degenerate triangle must not hit");
    std::cout << "PASS segment-triangle: high speed, barycentric bounds, vertices, edges, parallel/invalid input\n";
}

void houseTests() {
    const auto& collider = collision::house();
    const auto& mesh = models::houseGeometry();
    std::size_t index = 0;
    for (const auto& part : mesh) for (std::size_t i = 0; i < part.size(); i += 3) {
        const auto& tri = collider.triangles()[index++];
        require(glm::distance(tri.a, world(part[i].position)) < 1e-6f, "Collider shares exact render vertices/translation");
        require(glm::dot(tri.normal, part[i].normal) > 0.9999f, "Collider normals agree with rendering");
        near(glm::dot(tri.normal, tri.a) + tri.planeD, 0, "Triangle lies on its plane");
    }
    require(index == collider.triangles().size(), "Every static house triangle included once");
    for (const float x : {-1.0f, 1.0f, -2.25f, 2.25f}) {
        const auto hit = vertical(x, 0.8f);
        near(hit.position.y, roofY(x), "Both sloping roof planes include the wall overhang");
        require(hit.normal.y > 0.8f && hit.normal.x * x > 0, "Roof normals point upward/outward");
    }
    near(vertical(0, 0.8f).position.y, 4.26f, "Ridge hits the visible raised ridge cap first");
    for (float x : {-0.071f, 0.071f}) near(vertical(x, 0.8f).position.y, roofY(x), "No gap beside ridge cap");
    near(vertical(-1.19f, 0).position.y, roofY(-1.19f), "Shared diagonal cannot leak");
    for (float z : {-2.15f, 2.15f}) near(vertical(1, z).position.y, roofY(1), "Front/back shell boundary included");
    for (float x : {-2.38f, 2.38f}) near(vertical(x, 0).position.y, 2.59f, "Eaves hit visible wooden trim first");
    near(vertical(-1.14f, -0.65f).position.y, 4.77f, "Chimney rim precedes lower roof");
    near(vertical(-0.85f, -0.65f).position.y, 4.60f, "Chimney opening ends on visible cavity surface");
    collision::Hit hit;
    require(collider.intersect(world({-2, 4.45f, 0.8f}), world({2, 1.5f, 0.8f}), hit), "Strong diagonal wind hits roof");
    near(hit.position.y, roofY(hit.position.x - models::HOUSE_POSITION.x), "Diagonal intersection is on slope");
    require(collider.intersect(world({1, 1000, 0.8f}), world({1, -1000, 0.8f}), hit), "CCD handles very fast travel");
    near(hit.position.y, roofY(1), "Earliest surface rather than ground/underside");
    require(collider.intersect(world({-3, 4.3f, -0.65f}), world({0, 4.3f, -0.65f}), hit), "Wind hits chimney side");
    require(hit.normal.x < -0.99f, "Chimney side normal");
    require(!collider.intersect(world({3, 8, 0}), world({3, -2, 0}), hit), "Outside house reaches ground");
    const auto roof = vertical(1, 0.8f);
    const auto head = roof.position + roof.normal * 0.01f;
    const auto tail = collision::clipStreak(head, head - roof.normal * 0.2f);
    require(glm::dot(tail - roof.position, roof.normal) > 0, "Splash/rain visual tails stay outside roof");
    require(!collider.intersect(head, tail, hit, false), "Clipped streak never intersects house geometry");
    const glm::vec3 farHead(8, 1, 8), farTail(8, 1.2f, 8);
    require(collision::clipStreak(farHead, farTail) == farTail, "Outside streak unchanged");
    std::cout << "PASS house: shared world geometry, both slopes, ridge/cap, seam, overhang, eaves, wind, chimney, earliest hit\n";
}

// The underlying RainSystem is mutable. Inject known trajectories into its
// existing pool only in tests, without adding a production mutation API.
Raindrop& fixtureDrop(RainSystem& rain, std::size_t i) {
    return const_cast<Raindrop&>(rain.drops()[i]);
}
void rainTests() {
    RainSystem rain;
    collision::Hit hit;
    for (const auto& p : rain.drops())
        require(!collision::house().intersect({p.position.x, 8.5f, p.position.z}, p.position, hit),
                "Initial rain cannot be inside or beneath a roof");
    for (std::size_t i = 0; i < RainSystem::RAIN_COUNT; ++i) {
        auto& p = fixtureDrop(rain, i);
        p.position = {8, 8, 8}; p.velocity = {0, -7, 0};
    }
    const auto first = vertical(-1, 0.8f), second = vertical(1, 0.8f);
    fixtureDrop(rain, 0).position = first.position + glm::vec3(0, 0.04f, 0);
    fixtureDrop(rain, 1).position = second.position + glm::vec3(0, 0.04f, 0);
    fixtureDrop(rain, 2).position = {8, 0.04f, 8};
    fixtureDrop(rain, 3).position = world({-1.14f, 4.80f, -0.65f});
    rain.update(10, {0, 0, 0});
    require(rain.impactCount() == 4 && rain.recycledCount() == 4 &&
            rain.impacts()[0].surface == SurfaceType::Roof && rain.impacts()[1].surface == SurfaceType::Roof &&
            rain.impacts()[2].surface == SurfaceType::Ground && rain.impacts()[3].surface == SurfaceType::Roof,
            "One hit per drop; stalled dt hits roof/chimney separately from ground");
    near(rain.impacts()[0].position.y, first.position.y, "Left rain stops at actual hit");
    near(rain.impacts()[1].position.y, second.position.y, "Right rain stops at actual hit");
    near(rain.impacts()[3].position.y, 4.77f, "Rain stops at chimney");
    for (int i = 0; i < 4; ++i) {
        const auto& p = rain.drops()[i];
        require(p.position.y >= 7.6f && p.position == p.previousPosition, "Old drop removed; new drop has no teleport streak");
    }
    rain.update(0.001f, {0, 0, 0});
    require(rain.impactCount() == 0, "Hits are not emitted twice");
    std::cout << "PASS rain lifecycle: initial shelter, continuous hit, respawn, no duplicate or ground hit under roof\n";
}

void splashTests() {
    SplashSystem splashes;
    const auto* storage = splashes.particles().data();
    const auto hit = vertical(1, 0.8f);
    splashes.emitSurface(hit.position, hit.normal, {4, -17, -3});
    require(splashes.activeCount() >= 3 && splashes.activeCount() <= 6, "3..6 roof droplets per impact");
    glm::vec3 firstVelocity{};
    bool different = false;
    for (const auto& p : splashes.particles()) if (p.lifetime > 0) {
        require(glm::dot(p.velocity, hit.normal) > 0 && p.velocity.y > 0, "Splash initially leaves roof upward/outward");
        require(p.lifetime >= splashConfig::MIN_LIFETIME && p.lifetime <= splashConfig::MAX_LIFETIME,
                "Roof lifetime bounds");
        require(p.length >= splashConfig::MIN_SIZE && p.length <= splashConfig::MAX_SIZE && p.length < 0.12f,
                "Splash is significantly smaller than rain");
        require(glm::dot(p.position - hit.position, hit.normal) > 0, "Spawn outside roof");
        if (firstVelocity != glm::vec3(0)) different |= glm::distance(firstVelocity, p.velocity) > 0.01f;
        firstVelocity = p.velocity;
    }
    require(different, "Random tangent directions must differ");
    splashes.update(1.0f / 120.0f);
    for (const auto& p : splashes.particles()) if (p.lifetime > 0)
        require(p.alpha > 0 && p.alpha < splashConfig::INITIAL_ALPHA, "Alpha fades with remaining lifetime");
    // Low incoming speed exercises short arcs that return before expiring.
    splashes.reset();
    for (int i = 0; i < 40; ++i) splashes.emitSurface(hit.position, hit.normal, {0, -0.1f, 0});
    const auto requests = splashes.requests();
    bool curved = false;
    for (int i = 0; i < 48; ++i) {
        const auto previous = splashes.particles();
        splashes.update(1.0f / 120.0f);
        for (std::size_t k = 0; k < previous.size(); ++k) {
            const auto& p = splashes.particles()[k];
            if (p.lifetime <= 0) { require(p.alpha == 0, "Inactive droplets are fully transparent"); continue; }
            curved |= p.velocity.y < 0;
            collision::Hit crossed;
            require(!collision::house().intersect(previous[k].position, p.position, crossed),
                    "No live splash crosses back into the house");
        }
    }
    require(curved && splashes.surfaceReturns() > 0, "Gravity curves splashes and roof return kills them");
    require(splashes.activeCount() == 0 && splashes.requests() == requests, "Expired splashes never emit secondary splashes");
    for (int i = 0; i < 3000; ++i) splashes.emitSurface(hit.position, hit.normal, {0, -10, 0});
    require(splashes.activeCount() <= SplashSystem::MAX_SPLASHES && splashes.rejected() > 0 &&
            storage == splashes.particles().data(), "High density has a fixed bounded pool");
    for (int i = 0; i < 4; ++i) splashes.update(0.1f);
    require(splashes.activeCount() == 0, "Saturated pool fully drains");
    std::cout << "PASS roof splash: count, size, normal/reflection, fading, gravity, roof return, no cascade, pool cap\n";
}

void waterBoundaryTests() {
    RippleSystem dry;
    dry.emit({0, 0, 0});
    dry.emit({1, 0, 1});
    for (int i = 0; i < 100; ++i) dry.emit({8, 0, 8});
    dry.emit({NAN, 0, 0});
    require(dry.activeCount() == 0, "Dry land and invalid input must never create rings");
    std::size_t edges = 0;
    for (std::size_t index = 0; index < surface::puddles().size(); ++index) {
        const auto& puddle = surface::puddles()[index];
        const auto& polygon = surface::polygon(index);
        const auto sample = surface::waterAt(puddle.center);
        require(sample && sample.puddle == static_cast<int>(index), "Puddle center classification");
        collision::Hit centerHit;
        require(collision::earliestHit(puddle.center + glm::vec3(0, 100, 0),
                                     puddle.center - glm::vec3(0, 100, 0), centerHit), "Fast water collision");
        require(centerHit.surface == SurfaceType::Water, "Water takes precedence over lower ground");
        near(centerHit.position.y, puddle.center.y, "Use the actual rendered water height");
        dry.emit({puddle.center.x, 1, puddle.center.z});
        require(dry.activeCount() == 0, "A solid hit above a puddle cannot emit a water ripple");
        for (std::size_t i = 0; i < polygon.size(); ++i) {
            const auto midpoint = (polygon[i] + polygon[(i + 1) % polygon.size()]) * 0.5f;
            const auto edge = polygon[(i + 1) % polygon.size()] - polygon[i];
            const auto inward = glm::normalize(glm::vec2(-edge.y, edge.x));
            const auto inside = midpoint + inward * 0.01f;
            const auto outside = midpoint - inward * 0.01f;
            const glm::vec3 in(inside.x, puddle.center.y, inside.y), out(outside.x, puddle.center.y, outside.y);
            require(surface::waterAt(in) && !surface::waterAt(out), "Exact inside/outside shoreline classification");
            RippleSystem ripple;
            ripple.emit(out);
            require(ripple.activeCount() == 0, "No ripple just outside the water polygon");
            ripple.emit(in);
            require(ripple.activeCount() == 1, "Small ripple just inside the shoreline");
            for (int step = 0; step < 67; ++step) {
                for (const auto& r : ripple.particles()) if (r.lifetime > 0) {
                    require(r.maxRadius < 0.011f && r.radius <= r.maxRadius, "Even the initial ring fits near the edge");
                    // Sample the circle and interior of all 12 rendered chords.
                    for (int j = 0; j < 96; ++j) {
                        const float a = j * 6.2831853f / 96;
                        require(surface::waterAt(r.center + glm::vec3(std::cos(a),0,std::sin(a)) * r.radius).puddle ==
                                static_cast<int>(index), "The entire expanding ring stays inside its puddle");
                    }
                    for (int j = 0; j < 12; ++j) {
                        const float a = j * 6.2831853f / 12, b = (j + 1) * 6.2831853f / 12;
                        const auto p = r.center + glm::vec3(std::cos(a),0,std::sin(a)) * r.radius;
                        const auto q = r.center + glm::vec3(std::cos(b),0,std::sin(b)) * r.radius;
                        require(surface::waterAt(glm::mix(p, q, 0.5f)).puddle == static_cast<int>(index),
                                "Ring line segments do not cross concave shores");
                    }
                }
                ripple.update(1.0f / 120.0f);
            }
            require(ripple.activeCount() == 0, "Shoreline ripple fades completely");
            collision::Hit outsideHit;
            require(collision::earliestHit(out + glm::vec3(0, 0.1f, 0), out - glm::vec3(0, 0.1f, 0), outsideHit),
                    "Outside shoreline still hits a surface");
            require(outsideHit.surface == SurfaceType::Ground, "Outside shoreline is dry ground");
            ++edges;
        }
    }
    std::cout << "PASS water boundary: dry rejection, actual heights, all " << edges
              << " shoreline edges, full circles/chords, fading\n";
}

void rockTests() {
    const auto& collider = collision::rocks();
    require(collider.triangles().size() == environment::rocks().size() * 48, "All rock meshes included");
    for (std::size_t i = 0; i < environment::rocks().size(); ++i) {
        const auto& instance = environment::rocks()[i];
        const auto& mesh = environment::rockGeometry(i);
        const auto model = environment::rockTransform(instance);
        const auto normals = glm::transpose(glm::inverse(glm::mat3(model)));
        for (std::size_t k = 0; k < mesh.size(); k += 3) {
            const auto& tri = collider.triangles()[i * 48 + k / 3];
            require(glm::distance(tri.a, glm::vec3(model * glm::vec4(mesh[k].position,1))) < 1e-6f,
                    "Rock collider shares exact translated/rotated/scaled render vertices");
            require(glm::dot(tri.normal, glm::normalize(normals * mesh[k].normal)) > 0.9999f,
                    "Nonuniform scale preserves outward rock normal");
            const auto center = tri.a + (tri.edge1 + tri.edge2) / 3.0f;
            if (center.y < 0.01f) continue;
            collision::Hit hit;
            require(collider.intersect(center + tri.normal * 0.01f, center - tri.normal * 0.01f, hit),
                    "Every exposed low-poly rock face can be hit");
            require(hit.surface == SurfaceType::Rock && hit.objectIndex == i, "Correct rock identified");
            require(glm::distance(hit.position, center) < 0.0002f && glm::dot(hit.normal, tri.normal) > 0.999f,
                    "Rock contact and face normal agree");
        }
        collision::Hit hit;
        require(collision::earliestHit(instance.position + glm::vec3(0, 100, 0),
                                     instance.position - glm::vec3(0, 100, 0), hit), "High speed rock hit");
        require(hit.surface == SurfaceType::Rock && hit.objectIndex == i && hit.position.y > 0.1f,
                "Rock precedes ground along a very fast trajectory");
        collision::Hit windHit;
        require(collider.intersect(instance.position + glm::vec3(-4,3,-3),
                                   instance.position + glm::vec3(4,-3,3), windHit) && windHit.objectIndex == i,
                "Strong diagonal wind cannot tunnel through any rock");
        const auto head = hit.position + hit.normal * 0.01f;
        const auto tail = collision::clipStreak(head, head - hit.normal * 0.2f);
        collision::Hit crossed;
        require(!collision::earliestHit(head, tail, crossed, false), "Visual streak stays outside rock");

        WeatherSystem weather;
        weather.windVelocity = {0, 0, 0};
        for (std::size_t k = 0; k < RainSystem::RAIN_COUNT; ++k) {
            auto& p = fixtureDrop(weather.rain, k);
            p.position = {8, 8, 8}; p.velocity = {0,-7,0};
        }
        fixtureDrop(weather.rain, 0).position = hit.position + glm::vec3(0,0.025f,0);
        weather.update(1.0f/120.0f);
        require(weather.rain.impactCount() == 1 && weather.rain.impacts()[0].surface == SurfaceType::Rock,
                "One rock event, no ground event from the same raindrop");
        require(weather.splashes.requests() == 1 && weather.splashes.activeCount() >= 3 &&
                weather.splashes.activeCount() <= 5 && weather.ripples.activeCount() == 0,
                "Rock creates only its small spray, no ground splash or ripple");
        for (const auto& p : weather.splashes.particles()) if (p.lifetime > 0)
            require(p.surface == SurfaceType::Rock && glm::dot(p.velocity, hit.normal) > 0,
                    "Rock droplets initially leave the surface outward");
        weather.rainEnabled = false;
        const auto requests = weather.splashes.requests();
        for (int j = 0; j < 48; ++j) weather.update(1.0f/120.0f);
        require(weather.splashes.activeCount() == 0 && weather.splashes.requests() == requests &&
                weather.ripples.activeCount() == 0, "Secondary droplets never create additional effects");
        SplashSystem returning;
        for (int j = 0; j < 40; ++j)
            returning.emitSurface(hit.position, hit.normal, {0,-0.1f,0}, SurfaceType::Rock);
        const auto emitted = returning.requests();
        for (int j = 0; j < 48; ++j) returning.update(1.0f/120.0f);
        require(returning.activeCount() == 0 && returning.surfaceReturns() > 0 && returning.requests() == emitted,
                "Splash returning to rock terminates without secondary emission");
    }
    std::cout << "PASS rocks: 144 transformed faces, contact normals, fast travel, streak clipping, single impact, spray only\n";
}

void surfaceSplashTests() {
    for (const auto type : {SurfaceType::Ground, SurfaceType::Rock, SurfaceType::Roof, SurfaceType::Water}) {
        const auto& profile = type == SurfaceType::Roof ? splashConfig::ROOF :
            (type == SurfaceType::Rock ? splashConfig::ROCK : splashConfig::GROUND);
        SplashSystem spray;
        const auto normal = type == SurfaceType::Rock ? glm::normalize(glm::vec3(1, 1, 0)) : glm::vec3(0, 1, 0);
        spray.emitSurface({8, 0, 8}, normal, {2,-17,1}, type);
        require(spray.activeCount() >= static_cast<std::size_t>(profile.minCount) &&
                spray.activeCount() <= static_cast<std::size_t>(profile.maxCount), "Per-surface splash count");
        const auto before = spray.particles();
        for (const auto& p : before) if (p.lifetime > 0) {
            require(p.surface == type && p.lifetime >= profile.minLife && p.lifetime <= profile.maxLife,
                    "Per-surface lifetime and source classification");
            require(p.length >= profile.minSize && p.length <= profile.maxSize && glm::dot(p.velocity, normal) > 0,
                    "Small droplets leave their surface");
            if (type == SurfaceType::Ground) require(p.velocity.y < 1.0f, "Ground spray has a low arc");
        }
        spray.update(1.0f/120.0f);
        for (std::size_t i = 0; i < before.size(); ++i) if (spray.particles()[i].lifetime > 0) {
            const auto& p = spray.particles()[i];
            near(p.velocity.y, before[i].velocity.y - splashConfig::GRAVITY/120.0f, "Splash gravity");
            near(p.alpha, splashConfig::INITIAL_ALPHA * p.lifetime/p.initialLifetime, "Lifetime-based alpha");
        }
    }
    std::cout << "PASS surface spray: ground/water/rock/roof profiles, reduced energy, gravity, alpha\n";
}

void enduranceTests() {
    WeatherSystem weather;
    const auto* rainStorage = weather.rain.drops().data();
    const auto* splashStorage = weather.splashes.particles().data();
    std::array<std::size_t,4> hits{};
    std::size_t peak = 0;
    // 120 seconds, alternating wind, all 1000 raindrops and natural lightning.
    for (int step = 0; step < 14400; ++step) {
        if (step % 1200 == 0) weather.windVelocity = step % 2400 ? glm::vec3(-7,0,-3) : glm::vec3(5,0,2);
        weather.update(1.0f / 120.0f);
        for (std::size_t i = 0; i < weather.rain.impactCount(); ++i)
            ++hits[static_cast<std::size_t>(weather.rain.impacts()[i].surface)];
        peak = std::max(peak, weather.splashes.activeCount());
        require(weather.valid(), "Long-run weather state remains finite and bounded");
        for (const auto& r : weather.ripples.particles()) if (r.lifetime > 0) {
            const auto water = surface::waterAt(r.center);
            require(water && r.radius <= water.margin, "Every live ring has a disk fully inside water");
        }
        for (const auto& p : weather.rain.drops()) {
            collision::Hit crossed;
            require(!collision::earliestHit(p.previousPosition, p.position, crossed),
                    "No surviving drop segment crosses the house");
            const auto tail = p.position - glm::normalize(p.velocity) *
                std::min(p.length, p.streakLengthLimit) * collision::MAX_RAIN_STREAK_SCALE;
            require(!collision::earliestHit(p.position, tail, crossed, false), "Max-length rain streak cannot cross a surface");
        }
    }
    for (const auto count : hits) require(count > 100, "Endurance covers every surface classification");
    require(weather.lightning.strikeCount() > 0, "Endurance includes lightning");
    require(weather.rain.drops().data() == rainStorage && weather.splashes.particles().data() == splashStorage,
            "Pool allocations remain stable throughout endurance run");
    weather.rainEnabled = false;
    for (int i = 0; i < 60; ++i) weather.update(1.0f / 120.0f);
    require(weather.splashes.activeCount() == 0, "Endurance pool drains after stopping rain");
    std::cout << "PASS endurance: 120s, 1000 drops, strong/reversed wind, ground/rock/roof/water="
              << hits[0] << '/' << hits[1] << '/' << hits[2] << '/' << hits[3]
              << ", splash peak=" << peak << '/' << SplashSystem::MAX_SPLASHES << '\n';
}
}

int main() {
    try {
        triangleTests(); houseTests(); rainTests(); splashTests();
        waterBoundaryTests(); rockTests(); surfaceSplashTests(); enduranceTests();
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
