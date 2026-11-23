#include "Geometry.hpp"
#include "Surface.hpp"

#include <cmath>
#include <stdexcept>

namespace {
void triangle(std::vector<Vertex>& vertices, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    const glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
    vertices.push_back({a, normal});
    vertices.push_back({b, normal});
    vertices.push_back({c, normal});
}

void quad(std::vector<Vertex>& vertices, glm::vec3 a, glm::vec3 b,
          glm::vec3 c, glm::vec3 d) {
    triangle(vertices, a, b, c);
    triangle(vertices, a, c, d);
}
}

namespace geometry {
std::vector<Vertex> screenTriangle() {
    return {{{-1, -1, 0}, {0, 0, 1}}, {{3, -1, 0}, {0, 0, 1}}, {{-1, 3, 0}, {0, 0, 1}}};
}
std::vector<Vertex> puddle() {
    std::vector<Vertex> v;
    v.reserve(48);
    const auto& p = surface::outline();
    for (std::size_t i = 0; i < p.size(); ++i) {
        const auto a = p[i], b = p[(i + 1) % p.size()];
        triangle(v, {0, 0, 0}, {b.x, 0, b.y}, {a.x, 0, a.y});
    }
    return v;
}
std::vector<Vertex> ground() {
    std::vector<Vertex> v;
    v.reserve(6);
    quad(v, {-10, 0, -10}, {-10, 0, 10}, {10, 0, 10}, {10, 0, -10});
    return v;
}

std::vector<Vertex> box() {
    std::vector<Vertex> v;
    v.reserve(36);
    constexpr float a = 0.5f;
    quad(v, {-a, -a, a}, {a, -a, a}, {a, a, a}, {-a, a, a});
    quad(v, {a, -a, -a}, {-a, -a, -a}, {-a, a, -a}, {a, a, -a});
    quad(v, {a, -a, a}, {a, -a, -a}, {a, a, -a}, {a, a, a});
    quad(v, {-a, -a, -a}, {-a, -a, a}, {-a, a, a}, {-a, a, -a});
    quad(v, {-a, a, a}, {a, a, a}, {a, a, -a}, {-a, a, -a});
    quad(v, {-a, -a, -a}, {a, -a, -a}, {a, -a, a}, {-a, -a, a});
    return v;
}

std::vector<Vertex> roof() {
    std::vector<Vertex> v;
    v.reserve(24);
    const glm::vec3 lf{-0.5f, 0, 0.5f}, rf{0.5f, 0, 0.5f}, tf{0, 1, 0.5f};
    const glm::vec3 lb{-0.5f, 0, -0.5f}, rb{0.5f, 0, -0.5f}, tb{0, 1, -0.5f};
    triangle(v, lf, rf, tf);
    triangle(v, rb, lb, tb);
    quad(v, lf, tf, tb, lb);
    quad(v, tf, rf, rb, tb);
    quad(v, lb, rb, rf, lf);
    return v;
}

std::vector<Vertex> cone(unsigned segments) {
    if (segments < 3 || segments > 64) {
        throw std::invalid_argument("Cone must have 3 to 64 segments");
    }
    std::vector<Vertex> v;
    v.reserve(segments * 6);
    constexpr float tau = 6.28318530718f;
    for (unsigned i = 0; i < segments; ++i) {
        const float a = tau * static_cast<float>(i) / static_cast<float>(segments);
        const float b = tau * static_cast<float>(i + 1) / static_cast<float>(segments);
        const glm::vec3 p{std::cos(a), 0, std::sin(a)};
        const glm::vec3 q{std::cos(b), 0, std::sin(b)};
        triangle(v, p, {0, 1, 0}, q);
        triangle(v, {0, 0, 0}, p, q);
    }
    return v;
}

std::vector<Vertex> rock() {
    std::vector<Vertex> v;
    v.reserve(24);
    const glm::vec3 rim[]{{1, 0, 0}, {0, 0, -0.8f}, {-0.9f, 0, 0}, {0, 0, 1}};
    for (int i = 0; i < 4; ++i) {
        triangle(v, {0.15f, 1, -0.1f}, rim[i], rim[(i + 1) % 4]);
        triangle(v, {-0.1f, -0.55f, 0}, rim[(i + 1) % 4], rim[i]);
    }
    return v;
}

std::vector<Vertex> sphere(unsigned slices, unsigned stacks) {
    if (slices < 3 || slices > 32 || stacks < 2 || stacks > 16) {
        throw std::invalid_argument("Low-poly sphere: slices 3..32, stacks 2..16");
    }
    constexpr float pi = 3.14159265359f;
    const auto point = [=](unsigned row, unsigned column) {
        const float latitude = pi * static_cast<float>(row) / static_cast<float>(stacks);
        const float longitude = 2 * pi * static_cast<float>(column) / static_cast<float>(slices);
        return glm::vec3(std::sin(latitude) * std::cos(longitude), std::cos(latitude),
                         std::sin(latitude) * std::sin(longitude));
    };
    std::vector<Vertex> v;
    v.reserve(6 * slices * (stacks - 1));
    for (unsigned row = 0; row < stacks; ++row) {
        for (unsigned col = 0; col < slices; ++col) {
            const auto a = point(row, col), b = point(row + 1, col);
            const auto c = point(row + 1, col + 1), d = point(row, col + 1);
            if (row != stacks - 1) triangle(v, a, c, b);
            if (row != 0) triangle(v, a, d, c);
        }
    }
    return v;
}
}
