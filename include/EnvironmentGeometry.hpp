#pragma once
#include "Geometry.hpp"
#include "CloudSystem.hpp"
#include <cstdint>

namespace environment {
// Three static world-space water polygons in one draw, no transparent layering.
std::vector<Vertex> puddles();
std::vector<Vertex> rock(std::uint32_t seed);
struct RockInstance { glm::vec3 position, scale, color; float yaw; std::uint32_t seed; };
const std::array<RockInstance,3>& rocks();
// One closed skin extracted from the five overlapping lobe density fields.
// Expensive field evaluation happens only at renderer initialization.
std::vector<Vertex> cloud(const CloudCluster& cluster);
}
