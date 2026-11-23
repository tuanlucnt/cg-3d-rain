#pragma once

#include <array>
#include <glm/glm.hpp>

struct CloudLobe {
    glm::vec3 offset;
    glm::vec3 scale;
    float shade;
};

struct CloudCluster {
    glm::vec3 position;
    float speed;
    std::array<CloudLobe, 5> lobes;
};

// Simulation data only: the renderer owns the shared sphere mesh.
class CloudSystem {
public:
    static constexpr std::size_t COUNT = 4;
    CloudSystem() { reset(); }
    void reset();
    void update(float deltaTime, const glm::vec3& windVelocity);
    const std::array<CloudCluster, COUNT>& clusters() const { return clusters_; }
    static float coverage(glm::vec3 center);

private:
    std::array<CloudCluster, COUNT> clusters_{};
};
