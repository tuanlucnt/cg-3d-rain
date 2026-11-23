#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

namespace geometry {
// Triangle lists, counterclockwise from outside; flat normals per face.
std::vector<Vertex> ground(); // 20 x 20 on XZ, y = 0
std::vector<Vertex> box();    // centered unit box
std::vector<Vertex> roof();   // gabled prism, width/depth 1, height 1
std::vector<Vertex> cone(unsigned segments = 8); // radius 1, y in [0, 1]
std::vector<Vertex> rock();   // irregular closed low-poly octahedron
std::vector<Vertex> sphere(unsigned slices = 10, unsigned stacks = 6);
std::vector<Vertex> puddle();
std::vector<Vertex> screenTriangle();
}
