#pragma once
#include "Geometry.hpp"
#include <array>
#include <cstdint>

namespace models {
enum class HousePart : std::size_t { Walls, Stone, Roof, Trim, Door, Glass, Metal, Chimney, Cavity, Count };
using House = std::array<std::vector<Vertex>, static_cast<std::size_t>(HousePart::Count)>;
// Local origin is ground-level center. Scene translates the model to (-1,0,-1).
House house();

struct TreeParameters {
    float height = 3.8f;
    float trunkRadius = 0.18f;
    float canopyWidth = 1.25f;
    unsigned branchCount = 8;
    std::uint32_t seed = 1;
};
struct Tree {
    std::vector<Vertex> bark;
    std::vector<Vertex> foliage;
};
struct TreeInstance { glm::vec3 position; float shade; TreeParameters geometry; };
const std::array<TreeInstance, 4>& treeLayout();
Tree tree(const TreeParameters& parameters);
}
