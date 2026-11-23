#include "Camera.hpp"
#include "Geometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(float a, float b, float epsilon = 0.0001f) { return std::abs(a - b) < epsilon; }

using Point = std::array<long, 3>;
Point point(glm::vec3 p) {
    return {std::lround(p.x * 100000), std::lround(p.y * 100000), std::lround(p.z * 100000)};
}

void mesh(const std::string& name, const std::vector<Vertex>& vertices,
          glm::vec3 interior, bool closed = true) {
    require(!vertices.empty() && vertices.size() % 3 == 0, name + ": triangle list");
    // Welding positions lets us verify manifold topology even with flat normals.
    std::map<std::pair<Point, Point>, std::pair<int, int>> edges;
    for (std::size_t i = 0; i < vertices.size(); i += 3) {
        const glm::vec3 a = vertices[i].position;
        const glm::vec3 b = vertices[i + 1].position;
        const glm::vec3 c = vertices[i + 2].position;
        const glm::vec3 cross = glm::cross(b - a, c - a);
        require(glm::length(cross) > 0.00001f, name + ": degenerate triangle");
        const glm::vec3 normal = glm::normalize(cross);
        for (int k = 0; k < 3; ++k) {
            const auto& v = vertices[i + k];
            require(near(glm::length(v.normal), 1), name + ": unit normal");
            require(glm::dot(normal, v.normal) > 0.9999f, name + ": normal/winding mismatch");
            Point from = point(vertices[i + k].position);
            Point to = point(vertices[i + (k + 1) % 3].position);
            const int sign = from < to ? 1 : -1;
            if (to < from) std::swap(from, to);
            auto& edge = edges[{from, to}];
            ++edge.first;
            edge.second += sign;
        }
        if (closed) {
            require(glm::dot(normal, (a + b + c) / 3.0f - interior) > 0,
                    name + ": triangle faces inward (would be culled)");
        } else {
            require(normal.y > 0.9999f, name + ": ground must face +Y");
        }
    }
    if (closed) {
        for (const auto& edge : edges) {
            require(edge.second.first == 2 && edge.second.second == 0,
                    name + ": open or inconsistently oriented mesh edge");
        }
    }
    std::cout << "PASS " << name << ": " << vertices.size() / 3 << " triangles\n";
}

void geometryTests() {
    const auto ground = geometry::ground();
    mesh("ground", ground, {}, false);
    mesh("puddle", geometry::puddle(), {}, false);
    float minX = 100, maxX = -100, minZ = 100, maxZ = -100;
    for (const auto& v : ground) {
        require(v.position.y == 0, "Ground must lie at Y=0");
        minX = std::min(minX, v.position.x); maxX = std::max(maxX, v.position.x);
        minZ = std::min(minZ, v.position.z); maxZ = std::max(maxZ, v.position.z);
    }
    require(minX == -10 && maxX == 10 && minZ == -10 && maxZ == 10,
            "Ground must be exactly 20x20");
    mesh("box", geometry::box(), {});
    mesh("roof", geometry::roof(), {0, 0.25f, 0});
    for (unsigned n : {3u, 8u, 16u, 64u})
        mesh("cone " + std::to_string(n), geometry::cone(n), {0, 0.25f, 0});
    mesh("rock", geometry::rock(), {});
    mesh("cloud sphere", geometry::sphere(), {});
    bool rejected = false;
    try { geometry::cone(2); }
    catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "Reject invalid cone geometry");
}

void cameraTests() {
    Camera camera;
    const glm::vec3 start = camera.position();
    require(start.y > 0 && start.z > 10, "Initial camera must be outside the scene objects");
    const glm::vec4 eye = camera.view() * glm::vec4(start, 1);
    const glm::vec4 target = camera.view() * glm::vec4(start + camera.forward(), 1);
    require(glm::length(glm::vec3(eye)) < 0.0001f, "View maps camera to origin");
    require(near(target.x, 0) && near(target.y, 0) && near(target.z, -1),
            "View looks along camera forward");

    Camera oneStep, manySteps;
    oneStep.move({0, 0, 1}, 1.0f);
    for (int i = 0; i < 100; ++i) manySteps.move({0, 0, 1}, 0.01f);
    require(glm::distance(oneStep.position(), manySteps.position()) < 0.001f,
            "Movement must not depend on FPS");
    require(near(glm::distance(oneStep.position(), start), 5), "Camera speed");
    camera.move({1, 1, 1}, 0.1f);
    require(near(glm::distance(camera.position(), start), 0.5f), "Diagonal speed normalization");
    camera.rotate(1000000, -1000000);
    require(camera.pitch() == 89 && std::abs(camera.yaw()) <= 180, "Pitch/yaw bounds");
    camera.rotate(0, 1000000);
    require(camera.pitch() == -89, "Negative pitch bound");
    require(near(glm::length(camera.forward()), 1), "Normalized forward direction");
    camera.reset();
    camera.move({0, -1, 0}, 100);
    require(near(camera.position().y, 0.35f), "Camera floor constraint");
    camera.move({0, 1, 0}, 100);
    require(near(camera.position().y, 14), "Camera ceiling constraint");
    camera.move({1, 0, 1}, 100);
    require(std::abs(camera.position().x) <= 16 && camera.position().z >= -16 &&
            camera.position().z <= 18, "Horizontal camera bounds");
    camera.reset();
    camera.move({0, 0, 1}, 0);
    require(glm::distance(camera.position(), start) == 0, "Zero dt preserves position");
    camera.move({0, 0, 1}, -1);
    require(glm::distance(camera.position(), start) == 0, "Negative dt is rejected");

    const auto wide = camera.projection(16.0f / 9.0f);
    const auto narrow = camera.projection(4.0f / 3.0f);
    require(near(wide[1][1], narrow[1][1]) && wide[0][0] < narrow[0][0],
            "Resize changes horizontal projection, preserves vertical FOV");
    require(wide[2][3] == -1 && wide[3][3] == 0, "Perspective projection");
    for (unsigned i = 0; i < 3; ++i) {
        camera.preset(i);
        const auto p = camera.position();
        require(p.y > 0.35f && std::abs(camera.pitch()) <= 89, "Preset stays above ground, pitch is safe");
        require(!(p.x > -3 && p.x < 1 && p.z > -2.8f && p.z < 0.8f && p.y < 4.2f),
                "Preset cannot be inside the house");
        for (glm::vec2 trunk : {glm::vec2(-6.8f, -1), glm::vec2(5, -5),
                               glm::vec2(-6.5f, 4), glm::vec2(5.5f, 4)})
            require(p.y > 4 || glm::distance(glm::vec2(p.x, p.z), trunk) > 1.5f,
                    "Preset cannot be inside a tree");
    }
    camera.preset(0);
    require(glm::distance(camera.position(), start) == 0, "Overview preset preserves default reset position");
    camera.move({0, 0, 1}, std::numeric_limits<float>::quiet_NaN());
    require(glm::distance(camera.position(), start) == 0, "Camera rejects invalid deltaTime");
    std::cout << "PASS camera: view, projection, deltaTime, speed, input limits, reset\n";
}
}

int main() {
    try {
        geometryTests();
        cameraTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
