#include "DynamicBatch.hpp"

#include <limits>
#include <stdexcept>
#include <type_traits>

DynamicBatch::DynamicBatch(std::size_t capacity, GLenum primitive)
    : capacity_(capacity), primitive_(primitive) {
    static_assert(std::is_standard_layout<EffectVertex>::value, "Vertex layout");
    if (!capacity || capacity > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()))
        throw std::invalid_argument("Invalid dynamic batch capacity");
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(capacity * sizeof(EffectVertex)),
                 nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EffectVertex),
                          reinterpret_cast<void*>(offsetof(EffectVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(EffectVertex),
                          reinterpret_cast<void*>(offsetof(EffectVertex, color)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(EffectVertex),
                          reinterpret_cast<void*>(offsetof(EffectVertex, edge)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

DynamicBatch::~DynamicBatch() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}

void DynamicBatch::upload(const EffectVertex* vertices, std::size_t count) {
    if (count > capacity_) throw std::length_error("Dynamic batch capacity exceeded");
    count_ = static_cast<GLsizei>(count);
    if (!count) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(EffectVertex)), vertices);
    ++uploads_;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DynamicBatch::draw() const {
    if (!count_) return;
    glBindVertexArray(vao_);
    glDrawArrays(primitive_, 0, count_);
}
