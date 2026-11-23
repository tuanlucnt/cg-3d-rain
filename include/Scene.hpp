#pragma once

#include "Mesh.hpp"
#include "Material.hpp"
#include <memory>
#include <vector>

struct SceneObject {
    const Mesh* mesh;
    glm::mat4 model;
    glm::mat3 normal;
    glm::vec3 color;
    bool ground = false;
    bool puddle = false;
    Material material{};
    float treeHeight = 0;
};

class Scene {
public:
    Scene();
    const std::vector<SceneObject>& objects() const { return objects_; }
    std::size_t triangleCount() const;
    void update(float deltaTime, bool paused);
    void resetAnimation() { animationTime_ = 0; }
    float animationTime() const { return animationTime_; }

private:
    void add(const Mesh& mesh, glm::vec3 position, glm::vec3 scale,
             glm::vec3 color, float yaw = 0.0f);
    void modelPart(const std::vector<Vertex>& vertices, glm::vec3 position,
                   glm::vec3 color, Material material, float treeHeight = 0);
    Mesh ground_, puddle_;
    std::vector<std::unique_ptr<Mesh>> modelMeshes_;
    std::vector<SceneObject> objects_;
    float animationTime_ = 0;
};
