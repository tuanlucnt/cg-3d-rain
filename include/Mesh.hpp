#pragma once

#include "Geometry.hpp"
#include <GL/glew.h>

class Mesh {
public:
    explicit Mesh(const std::vector<Vertex>& vertices);
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    void draw() const;
    std::size_t triangleCount() const { return static_cast<std::size_t>(count_) / 3; }

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLsizei count_ = 0;
};
