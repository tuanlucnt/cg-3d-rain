#pragma once

#include "Mesh.hpp"
#include <vector>

struct SceneObject {
    const Mesh* mesh;
    glm::mat4 model;
    glm::mat3 normal;
    glm::vec3 color;
    bool ground = false;
    bool puddle = false;
};

class Scene {
public:
    Scene();
    const std::vector<SceneObject>& objects() const { return objects_; }
    std::size_t triangleCount() const;

private:
    void add(const Mesh& mesh, glm::vec3 position, glm::vec3 scale,
             glm::vec3 color, float yaw = 0.0f);
    void tree(glm::vec3 position, float scale, float shade);
    Mesh ground_, box_, roof_, cone_, rock_, puddle_;
    std::vector<SceneObject> objects_;
};
