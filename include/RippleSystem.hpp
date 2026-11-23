#pragma once
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <random>

struct Ripple {
    glm::vec3 center{};
    float radius = 0.04f;
    float maxRadius = 0.45f;
    float elapsed = 0;
    float lifetime = 0;
    float alpha = 0;
};

class RippleSystem {
public:
    static constexpr std::size_t CAPACITY = 48;
    RippleSystem() { reset(); }
    void reset();
    void emit(glm::vec3 hit, bool sampled = false, float waterMargin = 0);
    void update(float deltaTime);
    const std::array<Ripple, CAPACITY>& particles() const { return particles_; }
    std::size_t activeCount() const { return active_; }
    std::uint64_t revision() const { return revision_; }
    std::uint64_t rejected() const { return rejected_; }
private:
    std::array<Ripple, CAPACITY> particles_{};
    std::mt19937 random_{76};
    std::size_t active_ = 0, cursor_ = 0;
    std::uint64_t revision_ = 0, rejected_ = 0;
};
