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

float outlineRadius(float a) {
    return 1 + 0.10f * std::sin(3 * a) + 0.055f * std::cos(5 * a) + 0.035f * std::sin(7 * a + 0.6f);
}

const std::array<glm::vec2, OUTLINE_POINTS>& outline() {
    static const auto points = [] {
        std::array<glm::vec2, OUTLINE_POINTS> result{};
        for (std::size_t i = 0; i < result.size(); ++i) {
            const float a = static_cast<float>(i) * 6.2831853f / static_cast<float>(result.size());
            const float r = outlineRadius(a);
            result[i] = glm::vec2(std::cos(a), std::sin(a)) * r;
        }
        return result;
    }();
    return points;
}

const std::array<glm::vec2, OUTLINE_POINTS>& polygon(std::size_t index) {
    // Static world polygons shared by geometry and rain-impact queries. No
    // per-impact trigonometry or rebuilding the shoreline arrays.
    static const auto polygons = [] {
        std::array<std::array<glm::vec2, OUTLINE_POINTS>, 3> result{};
        for (std::size_t j = 0; j < puddles().size(); ++j) {
            const auto& puddle = puddles()[j];
            const float a = glm::radians(puddle.yaw);
            for (std::size_t i = 0; i < outline().size(); ++i) {
                const auto p = outline()[i] * puddle.scale;
                result[j][i] = glm::vec2(puddle.center.x, puddle.center.z) +
                    glm::vec2(std::cos(a) * p.x + std::sin(a) * p.y,
                              -std::sin(a) * p.x + std::cos(a) * p.y);
            }
        }
        return result;
    }();
    return polygons.at(index);
}

WaterSample waterAt(glm::vec3 hit) {
    if (!std::isfinite(hit.x) || !std::isfinite(hit.z)) return {};
    static const auto bounds=[] {
        std::array<glm::vec4,3> result{};
        for (std::size_t j=0;j<result.size();++j) {
            glm::vec2 low(std::numeric_limits<float>::max()),high(-std::numeric_limits<float>::max());
            for (const auto& p:polygon(j)) { low=glm::min(low,p); high=glm::max(high,p); }
            result[j]={low.x,low.y,high.x,high.y};
        }
        return result;
    }();
    const glm::vec2 point(hit.x, hit.z);
    for (std::size_t index = 0; index < puddles().size(); ++index) {
        const auto& b=bounds[index];
        if (point.x<b.x || point.y<b.y || point.x>b.z || point.y>b.w) continue;
        const auto& polygon = surface::polygon(index);
        bool inside = false;
        float margin = std::numeric_limits<float>::max();
        for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
            const auto p = polygon[i], q = polygon[j];
            if ((p.y > point.y) != (q.y > point.y) &&
                point.x < (q.x - p.x) * (point.y - p.y) / (q.y - p.y) + p.x) inside = !inside;
            const auto edge = q - p;
            const float t = std::clamp(glm::dot(point - p, edge) / glm::dot(edge, edge), 0.0f, 1.0f);
            margin = std::min(margin, glm::distance(point, p + edge * t));
        }
        if (inside && margin > 0) return {static_cast<int>(index), margin, puddles()[index].center.y};
    }
    return {};
}

float waterMargin(glm::vec3 hit) {
    return waterAt(hit).margin;
}
}
