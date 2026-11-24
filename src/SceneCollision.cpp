#include "SceneCollision.hpp"
#include "EnvironmentGeometry.hpp"
#include <algorithm>
#include <cmath>

namespace collision {
RockCollider::RockCollider() {
    for (std::size_t i = 0; i < environment::rocks().size(); ++i)
        addMesh(environment::rockGeometry(i), environment::rockTransform(environment::rocks()[i]),
                SurfaceType::Rock, models::HousePart::Roof, i);
    build();
}

const RockCollider& rocks() {
    static const RockCollider data;
    return data;
}

bool earliestHit(glm::vec3 start, glm::vec3 end, Hit& hit, bool incomingOnly) {
    for (int axis = 0; axis < 3; ++axis)
        if (!std::isfinite(start[axis]) || !std::isfinite(end[axis])) return false;
    bool found = house().intersect(start, end, hit, incomingOnly);
    Hit candidate;
    if (rocks().intersect(start, end, candidate, incomingOnly) && (!found || candidate.t < hit.t)) {
        hit = candidate; found = true;
    }
    const float dy = end.y - start.y;
    if (dy == 0 || (incomingOnly && dy >= 0)) return found;
    const auto planeHit = [&](float height, SurfaceType type) {
        const float t = (height - start.y) / dy;
        if (t < 0 || t > 1) return;
        auto p = glm::mix(start, end, t);
        p.y = height;
        if (std::abs(p.x) > 10 || std::abs(p.z) > 10) return;
        if (type == SurfaceType::Water) {
            const auto water = surface::waterAt(p);
            if (!water || std::abs(water.height - height) > EPSILON) return;
        }
        // A water surface coincident with ground takes precedence at a tie.
        if (!found || t < hit.t || (type == SurfaceType::Water && hit.surface == SurfaceType::Ground &&
                                   std::abs(t - hit.t) <= EPSILON)) {
            hit = {p, {0, 1, 0}, t, models::HousePart::Roof, type, 0};
            found = true;
        }
    };
    planeHit(0, SurfaceType::Ground);
    // All current puddles share a height. Query each distinct height only once.
    for (std::size_t i = 0; i < surface::puddles().size(); ++i) {
        const float height = surface::puddles()[i].center.y;
        bool duplicate = false;
        for (std::size_t j = 0; j < i; ++j) duplicate |= surface::puddles()[j].center.y == height;
        if (!duplicate) planeHit(height, SurfaceType::Water);
    }
    return found;
}

glm::vec3 clipStreak(glm::vec3 head, glm::vec3 tail) {
    Hit hit;
    if (!earliestHit(head, tail, hit, false)) return tail;
    const float length = glm::distance(head, tail);
    const float t = std::max(0.0f, hit.t - SURFACE_OFFSET / std::max(length, EPSILON));
    return glm::mix(head, tail, t);
}
}
