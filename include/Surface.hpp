#pragma once
#include <array>
#include <glm/glm.hpp>

namespace surface {
struct Puddle { glm::vec3 center; glm::vec2 scale; float yaw; };
const std::array<Puddle, 3>& puddles();
const std::array<glm::vec2, 16>& outline();
// Distance to the nearest polygon edge for points inside water, zero outside.
float waterMargin(glm::vec3 hit);
}
