#pragma once
#include <glm/glm.hpp>

enum class MaterialKind { Plain, Plaster, RoofTile, Wood, Glass, Stone, Foliage, Brick, Metal, Rock, Grass };

struct Material {
    MaterialKind kind = MaterialKind::Plain;
    // Ambient multiplier, diffuse multiplier, specular strength, shininess.
    glm::vec4 response{1.0f, 1.0f, 0.0f, 32.0f};
    float wetGloss = 0;
};
