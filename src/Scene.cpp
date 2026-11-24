#include "Scene.hpp"
#include "Surface.hpp"
#include "ModelGeometry.hpp"
#include "EnvironmentGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

Scene::Scene()
    : ground_(geometry::ground()), puddle_(environment::puddles()) {
    objects_.reserve(32);
    modelMeshes_.reserve(21);
    add(ground_, {0, 0, 0}, {1, 1, 1}, surface::GROUND_COLOR);
    objects_.back().ground = true;
    add(puddle_, {0,0,0}, {1,1,1}, {0.115f,0.19f,0.22f});
    objects_.back().puddle = true;

    const auto& house = models::houseGeometry();
    const std::array<glm::vec3, 9> colors{{
        {0.70f,0.65f,0.53f}, {0.40f,0.42f,0.39f}, {0.40f,0.15f,0.09f},
        {0.32f,0.24f,0.15f}, {0.23f,0.13f,0.07f}, {0.20f,0.34f,0.39f},
        {0.21f,0.24f,0.25f}, {0.40f,0.23f,0.16f}, {0.035f,0.038f,0.04f}}};
    const std::array<Material, 9> materials{{
        {MaterialKind::Plaster, {1,1,0.015f,16},0},
        {MaterialKind::Stone, {1,0.95f,0.025f,20},0},
        {MaterialKind::RoofTile, {1,1,0.04f,40},0.16f},
        {MaterialKind::Wood, {1,1,0.025f,24},0.025f},
        {MaterialKind::Wood, {1,1,0.035f,28},0.025f},
        {MaterialKind::Glass, {0.9f,0.7f,0.32f,80},0.04f},
        {MaterialKind::Metal, {1,1,0.40f,72},0},
        {MaterialKind::Brick, {1,1,0.015f,20},0},
        {MaterialKind::Plain, {0.8f,0.4f,0,32},0}
    }};
    for (std::size_t i = 0; i < house.size(); ++i) modelPart(house[i], models::HOUSE_POSITION, colors[i], materials[i]);

    for (const auto& instance : models::treeLayout()) {
        const auto data=models::tree(instance.geometry);
        modelPart(data.bark,instance.position,{0.30f,0.19f,0.10f},
                  {MaterialKind::Wood,{1,1,0.01f,16},0},instance.geometry.height);
        modelPart(data.foliage,instance.position,glm::vec3(0.18f,0.37f,0.23f)*instance.shade,
                  {MaterialKind::Foliage,{1.12f,1,0,24},0},instance.geometry.height);
    }

    for (std::size_t i = 0; i < environment::rocks().size(); ++i) {
        const auto& instance = environment::rocks()[i];
        modelMeshes_.push_back(std::make_unique<Mesh>(environment::rockGeometry(i)));
        const auto model = environment::rockTransform(instance);
        objects_.push_back({modelMeshes_.back().get(), model, glm::transpose(glm::inverse(glm::mat3(model))), instance.color});
        objects_.back().material={MaterialKind::Rock,{1,1,0.012f,12},0.018f};
    }
    modelPart(environment::grass(),{0,0,0},{0.23f,0.35f,0.19f},{MaterialKind::Grass,{1,1,0,16},0});
}

void Scene::modelPart(const std::vector<Vertex>& vertices, glm::vec3 position,
                      glm::vec3 color, Material material, float treeHeight) {
    modelMeshes_.push_back(std::make_unique<Mesh>(vertices));
    add(*modelMeshes_.back(), position, {1,1,1}, color);
    objects_.back().material = material;
    objects_.back().treeHeight = treeHeight;
}

void Scene::add(const Mesh& mesh, glm::vec3 position, glm::vec3 scale,
                glm::vec3 color, float yaw) {
    const glm::mat4 model = glm::scale(
        glm::rotate(glm::translate(glm::mat4(1), position), glm::radians(yaw),
                    glm::vec3(0, 1, 0)), scale);
    objects_.push_back({&mesh, model, glm::transpose(glm::inverse(glm::mat3(model))), color});
}

std::size_t Scene::triangleCount() const {
    std::size_t count = 0;
    for (const auto& object : objects_) count += object.mesh->triangleCount();
    return count;
}

void Scene::update(float deltaTime, bool paused) {
    if (!paused && std::isfinite(deltaTime) && deltaTime > 0)
        animationTime_ += std::min(deltaTime,0.1f);
}
