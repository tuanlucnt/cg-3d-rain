#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <cstddef>
#include <cstdint>

struct EffectVertex {
    glm::vec3 position;
    glm::vec4 color;
    float edge = 0;
};

// One persistent VAO/VBO per batch, fixed capacity; no per-particle draw calls.
class DynamicBatch {
public:
    DynamicBatch(std::size_t capacity, GLenum primitive);
    ~DynamicBatch();
    DynamicBatch(const DynamicBatch&) = delete;
    DynamicBatch& operator=(const DynamicBatch&) = delete;
    void upload(const EffectVertex* vertices, std::size_t count);
    void draw() const;
    std::size_t vertexCount() const { return static_cast<std::size_t>(count_); }
    std::uint64_t uploadCount() const { return uploads_; }

private:
    GLuint vao_ = 0, vbo_ = 0;
    std::size_t capacity_;
    GLsizei count_ = 0;
    GLenum primitive_;
    std::uint64_t uploads_ = 0;
};
