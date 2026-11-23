#include "Scene.hpp"
#include "Surface.hpp"

#include <glm/gtc/matrix_transform.hpp>

Scene::Scene()
    : ground_(geometry::ground()), box_(geometry::box()), roof_(geometry::roof()),
      cone_(geometry::cone(8)), rock_(geometry::rock()), puddle_(geometry::puddle()) {
    objects_.reserve(32);
    add(ground_, {0, 0, 0}, {1, 1, 1}, {0.24f, 0.34f, 0.28f});
    objects_.back().ground = true;
    for (const auto& p : surface::puddles()) {
        add(puddle_, p.center, {p.scale.x, 1, p.scale.y}, {0.18f, 0.28f, 0.33f}, p.yaw);
        objects_.back().puddle = true;
    }

    // Front of the house faces +Z. Thin boxes put door/windows above the wall
    // surface with real separation, avoiding z-fighting.
    add(box_, {-1, 1.3f, -1}, {4, 2.6f, 3.6f}, {0.75f, 0.68f, 0.52f});
    add(roof_, {-1, 2.6f, -1}, {4.7f, 1.6f, 4.25f}, {0.43f, 0.20f, 0.15f});
    add(box_, {-1, 0.85f, 0.83f}, {0.85f, 1.7f, 0.06f}, {0.24f, 0.15f, 0.10f});
    for (const float x : {-2.25f, 0.25f}) {
        add(box_, {x, 1.6f, 0.84f}, {0.85f, 0.85f, 0.08f}, {0.27f, 0.20f, 0.14f});
        add(box_, {x, 1.6f, 0.89f}, {0.64f, 0.64f, 0.04f}, {0.46f, 0.70f, 0.76f});
    }
    add(box_, {1.04f, 1.6f, -1}, {0.08f, 0.85f, 0.85f}, {0.27f, 0.20f, 0.14f});
    add(box_, {1.09f, 1.6f, -1}, {0.04f, 0.64f, 0.64f}, {0.46f, 0.70f, 0.76f});

    tree({-6.8f, 0, -1}, 1.05f, 0.92f);
    tree({5, 0, -5}, 1.15f, 1.04f);
    tree({-6.5f, 0, 4}, 0.9f, 0.98f);
    tree({5.5f, 0, 4}, 1.0f, 1.08f);

    add(rock_, {1.5f, 0.33f, 4.0f}, {1.0f, 0.6f, 0.85f}, {0.43f, 0.46f, 0.46f}, 22);
    add(rock_, {-4.5f, 0.22f, 2.8f}, {0.7f, 0.4f, 0.7f}, {0.50f, 0.51f, 0.48f}, -15);
    add(rock_, {7.5f, 0.27f, 1.5f}, {0.8f, 0.49f, 0.9f}, {0.39f, 0.43f, 0.44f}, 48);
}

void Scene::add(const Mesh& mesh, glm::vec3 position, glm::vec3 scale,
                glm::vec3 color, float yaw) {
    const glm::mat4 model = glm::scale(
        glm::rotate(glm::translate(glm::mat4(1), position), glm::radians(yaw),
                    glm::vec3(0, 1, 0)), scale);
    objects_.push_back({&mesh, model, glm::transpose(glm::inverse(glm::mat3(model))), color});
}

void Scene::tree(glm::vec3 p, float s, float shade) {
    add(box_, p + glm::vec3(0, 0.7f * s, 0), {0.36f * s, 1.4f * s, 0.36f * s},
        {0.31f, 0.21f, 0.13f});
    add(cone_, p + glm::vec3(0, 1.0f * s, 0), {1.25f * s, 2.15f * s, 1.25f * s},
        glm::vec3(0.15f, 0.32f, 0.23f) * shade);
    add(cone_, p + glm::vec3(0, 2.0f * s, 0), {0.95f * s, 1.8f * s, 0.95f * s},
        glm::vec3(0.20f, 0.40f, 0.28f) * shade);
}

std::size_t Scene::triangleCount() const {
    std::size_t count = 0;
    for (const auto& object : objects_) count += object.mesh->triangleCount();
    return count;
}
