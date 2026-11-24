#include "HouseCollider.hpp"

#include <algorithm>
#include <cmath>
#include <array>
#include <numeric>
#include <glm/gtc/matrix_transform.hpp>

namespace collision {
namespace {
bool finite(glm::vec3 p) {
    return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z);
}
}

void Aabb::include(glm::vec3 p) {
    low = glm::min(low, p);
    high = glm::max(high, p);
}

bool Aabb::overlaps(const Aabb& other) const {
    return low.x <= other.high.x + EPSILON && high.x >= other.low.x - EPSILON &&
           low.y <= other.high.y + EPSILON && high.y >= other.low.y - EPSILON &&
           low.z <= other.high.z + EPSILON && high.z >= other.low.z - EPSILON;
}

Triangle::Triangle(glm::vec3 p, glm::vec3 b, glm::vec3 c)
    : a(p), edge1(b - p), edge2(c - p),
      normal(glm::normalize(glm::cross(edge1, edge2))), planeD(-glm::dot(normal, p)) {
    bounds.include(p); bounds.include(b); bounds.include(c);
}

bool segmentTriangle(glm::vec3 start, glm::vec3 end, const Triangle& tri, float& t) {
    if (!finite(start) || !finite(end) || !finite(tri.normal)) return false;
    const auto direction = end - start;
    const auto p = glm::cross(direction, tri.edge2);
    const float determinant = glm::dot(tri.edge1, p);
    // Relative parallel tolerance also works for small splash displacements.
    const float scale = glm::length(tri.edge1) * glm::length(tri.edge2) * glm::length(direction);
    if (scale == 0 || std::abs(determinant) <= 1e-7f * scale) return false;
    const float inverse = 1.0f / determinant;
    const auto offset = start - tri.a;
    const float u = glm::dot(offset, p) * inverse;
    const auto q = glm::cross(offset, tri.edge1);
    const float v = glm::dot(direction, q) * inverse;
    const float fraction = glm::dot(tri.edge2, q) * inverse;
    if (u < -EPSILON || v < -EPSILON || u + v > 1 + EPSILON ||
        fraction < -EPSILON || fraction > 1 + EPSILON) return false;
    t = std::clamp(fraction, 0.0f, 1.0f);
    return true;
}

HouseCollider::HouseCollider() {
    const auto& mesh = models::houseGeometry();
    const auto model = glm::translate(glm::mat4(1), models::HOUSE_POSITION);
    for (std::size_t i = 0; i < mesh.size(); ++i)
        addMesh(mesh[i], model, SurfaceType::Roof, static_cast<models::HousePart>(i));
    build();
}

void TriangleCollider::addMesh(const std::vector<Vertex>& mesh, const glm::mat4& model,
                               SurfaceType surface, models::HousePart part, std::size_t objectIndex) {
    for (std::size_t k = 0; k < mesh.size(); k += 3) {
        const auto a = glm::vec3(model * glm::vec4(mesh[k].position, 1));
        const auto b = glm::vec3(model * glm::vec4(mesh[k + 1].position, 1));
        const auto c = glm::vec3(model * glm::vec4(mesh[k + 2].position, 1));
        triangles_.emplace_back(a, b, c);
        tags_.push_back({part, surface, objectIndex});
        for (const auto p : {a, b, c}) bounds_.include(p);
    }
}

void TriangleCollider::build() {
    const auto count = triangles_.size();
    indices_.resize(count);
    std::iota(indices_.begin(), indices_.end(), 0);
    nodes_.reserve(count * 2);
    buildNode(0, count);
}

std::size_t TriangleCollider::buildNode(std::size_t first, std::size_t end) {
    Aabb bounds, centers;
    for (std::size_t i = first; i < end; ++i) {
        const auto& box = triangles_[indices_[i]].bounds;
        bounds.include(box.low); bounds.include(box.high);
        centers.include((box.low + box.high) * 0.5f);
    }
    const std::size_t node = nodes_.size();
    nodes_.push_back({bounds, first, end - first, 0});
    if (end - first <= 8) return node;
    const auto extent = centers.high - centers.low;
    const int axis = extent.x > extent.y ? (extent.x > extent.z ? 0 : 2) : (extent.y > extent.z ? 1 : 2);
    const std::size_t middle = first + (end - first) / 2;
    std::nth_element(indices_.begin() + first, indices_.begin() + middle, indices_.begin() + end,
        [&](std::size_t a, std::size_t b) {
            const auto& x = triangles_[a].bounds;
            const auto& y = triangles_[b].bounds;
            return x.low[axis] + x.high[axis] < y.low[axis] + y.high[axis];
        });
    buildNode(first, middle); // Left child is immediately after its parent.
    const auto right = buildNode(middle, end);
    nodes_[node].count = 0;
    nodes_[node].right = right;
    return node;
}

bool TriangleCollider::intersect(glm::vec3 start, glm::vec3 end, Hit& hit, bool incomingOnly) const {
    if (nodes_.empty() || !finite(start) || !finite(end)) return false;
    Aabb swept;
    swept.include(start); swept.include(end);
    if (!bounds_.overlaps(swept)) return false;
    bool found = false;
    float earliest = 1;
    // Median splits keep depth logarithmic. The static mesh has <1500 faces;
    // a 64-entry traversal stack comfortably covers this balanced tree.
    std::array<std::size_t, 64> stack{};
    std::size_t pending = 1;
    while (pending) {
        const auto nodeIndex = stack[--pending];
        const auto& node = nodes_[nodeIndex];
        if (!node.bounds.overlaps(swept)) continue;
        if (!node.count) {
            stack[pending++] = node.right;
            stack[pending++] = nodeIndex + 1;
            continue;
        }
        for (std::size_t k = node.first; k < node.first + node.count; ++k) {
            const auto i = indices_[k];
            const auto& tri = triangles_[i];
            if (!tri.bounds.overlaps(swept) ||
                (incomingOnly && glm::dot(end - start, tri.normal) >= 0)) continue;
            float t;
            if (segmentTriangle(start, end, tri, t) && (!found || t < earliest)) {
                earliest = t;
                const auto& tag = tags_[i];
                hit = {glm::mix(start, end, t), tri.normal, t, tag.part, tag.surface, tag.objectIndex};
                found = true;
            }
        }
    }
    return found;
}

const HouseCollider& house() {
    static const HouseCollider collider;
    return collider;
}

}
