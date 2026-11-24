#pragma once
#include <array>
#include <glm/glm.hpp>

// Physical response is independent of the visual material/shader category.
enum class SurfaceType { Ground, Rock, Roof, Water };

namespace surface {
inline constexpr std::size_t OUTLINE_POINTS = 32;
inline constexpr glm::vec3 GROUND_COLOR{0.23f, 0.30f, 0.26f};
struct Puddle { glm::vec3 center; glm::vec2 scale; float yaw; };
const std::array<Puddle, 3>& puddles();
float outlineRadius(float angle);
const std::array<glm::vec2, OUTLINE_POINTS>& outline();
const std::array<glm::vec2, OUTLINE_POINTS>& polygon(std::size_t index);
struct WaterSample {
    int puddle = -1;
    float margin = 0;
    float height = 0;
    explicit operator bool() const { return puddle >= 0; }
};
// Exact world XZ polygon query, also returning clearance to the shoreline.
WaterSample waterAt(glm::vec3 hit);
// Distance to the nearest polygon edge for points inside water, zero outside.
float waterMargin(glm::vec3 hit);
}
