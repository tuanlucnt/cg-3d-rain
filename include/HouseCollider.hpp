#pragma once

#include "ModelGeometry.hpp"
#include "Surface.hpp"
#include <limits>

namespace collision {
inline constexpr float EPSILON = 1e-5f;
inline constexpr float SURFACE_OFFSET = 0.002f;
// Maximum multiplier used by the distance-dependent rain streak shader.
inline constexpr float MAX_RAIN_STREAK_SCALE = 1.12f;

struct Aabb {
    glm::vec3 low{std::numeric_limits<float>::max()};
    glm::vec3 high{std::numeric_limits<float>::lowest()};
    void include(glm::vec3 point);
    bool overlaps(const Aabb& other) const;
};

struct Triangle {
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
    glm::vec3 a, edge1, edge2, normal;
    float planeD;
    Aabb bounds;
};

struct Hit {
    glm::vec3 position{}, normal{};
    float t = 1;
    models::HousePart part = models::HousePart::Roof;
    SurfaceType surface = SurfaceType::Roof;
    std::size_t objectIndex = 0;
};

// Two-sided Moller-Trumbore; t is a fraction of the segment, not a distance.
bool segmentTriangle(glm::vec3 start, glm::vec3 end, const Triangle& triangle, float& t);

class TriangleCollider {
public:
    // Earliest hit; incomingOnly rejects movement away from an outward face.
    bool intersect(glm::vec3 start, glm::vec3 end, Hit& hit, bool incomingOnly = true) const;
    const Aabb& bounds() const { return bounds_; }
    const std::vector<Triangle>& triangles() const { return triangles_; }

protected:
    void addMesh(const std::vector<Vertex>& mesh, const glm::mat4& model, SurfaceType surface,
                 models::HousePart part = models::HousePart::Roof, std::size_t objectIndex = 0);
    void build();

private:
    struct Node { Aabb bounds; std::size_t first, count, right; };
    std::size_t buildNode(std::size_t first, std::size_t end);
    Aabb bounds_;
    std::vector<Triangle> triangles_;
    struct Tag { models::HousePart part; SurfaceType surface; std::size_t objectIndex; };
    std::vector<Tag> tags_;
    std::vector<std::size_t> indices_;
    std::vector<Node> nodes_;
};

class HouseCollider : public TriangleCollider {
public:
    HouseCollider();
};

const HouseCollider& house();
// Clips a visual particle segment on either side of a surface, before upload.
glm::vec3 clipStreak(glm::vec3 head, glm::vec3 tail);
}
