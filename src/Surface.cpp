#include "Surface.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace surface {
const std::array<Puddle, 3>& puddles() {
    static const std::array<Puddle, 3> data{{
        {{-2, 0.012f, 4.5f}, {1.8f, 1.0f}, 15},
        {{3.6f, 0.012f, 1.9f}, {1.2f, 0.75f}, -22},
        {{-5, 0.012f, -4}, {0.95f, 0.65f}, 35}}};
    return data;
}

const std::array<glm::vec2, 16>& outline() {
    static const auto points = [] {
        std::array<glm::vec2, 16> result{};
        for (std::size_t i = 0; i < result.size(); ++i) {
            const float a = static_cast<float>(i) * 6.2831853f / 16;
            const float r = 1 + 0.12f * std::sin(3 * a) + 0.07f * std::cos(5 * a);
            result[i] = glm::vec2(std::cos(a), std::sin(a)) * r;
        }
        return result;
    }();
    return points;
}

float waterMargin(glm::vec3 hit) {
    const glm::vec2 point(hit.x, hit.z);
    for (const auto& puddle : puddles()) {
        std::array<glm::vec2, 16> polygon{};
        const float a = glm::radians(puddle.yaw);
        for (std::size_t i = 0; i < polygon.size(); ++i) {
            const auto p = outline()[i] * puddle.scale;
            polygon[i] = glm::vec2(puddle.center.x, puddle.center.z) +
                glm::vec2(std::cos(a) * p.x + std::sin(a) * p.y,
                          -std::sin(a) * p.x + std::cos(a) * p.y);
        }
        bool inside = false;
        float margin = std::numeric_limits<float>::max();
        for (std::size_t i = 0, j = 15; i < polygon.size(); j = i++) {
            const auto p = polygon[i], q = polygon[j];
            if ((p.y > point.y) != (q.y > point.y) &&
                point.x < (q.x - p.x) * (point.y - p.y) / (q.y - p.y) + p.x) inside = !inside;
            const auto edge = q - p;
            const float t = std::clamp(glm::dot(point - p, edge) / glm::dot(edge, edge), 0.0f, 1.0f);
            margin = std::min(margin, glm::distance(point, p + edge * t));
        }
        if (inside) return margin;
    }
    return 0;
}
}
