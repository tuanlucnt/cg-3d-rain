#pragma once
#include "HouseCollider.hpp"

namespace collision {
class RockCollider : public TriangleCollider {
public:
    RockCollider();
};
const RockCollider& rocks();
// One world-space query used by rain, secondary droplets and visual streaks.
// Includes the exact water heights and the bounded 20x20 ground plane.
bool earliestHit(glm::vec3 start, glm::vec3 end, Hit& hit, bool incomingOnly = true);
}
