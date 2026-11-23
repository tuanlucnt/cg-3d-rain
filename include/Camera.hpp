#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera();
    void reset();
    void preset(unsigned index); // 0=overview, 1=house, 2=ground; camera only.
    // Local x: right, y: world up, z: forward. Diagonals keep the same speed.
    void move(const glm::vec3& localDirection, float deltaTime);
    void rotate(float deltaX, float deltaY);
    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    const glm::vec3& position() const { return position_; }
    const glm::vec3& forward() const { return forward_; }
    float pitch() const { return pitch_; }
    float yaw() const { return yaw_; }

private:
    void updateDirection();
    glm::vec3 position_{};
    glm::vec3 forward_{};
    const glm::vec3 up_{0.0f, 1.0f, 0.0f};
    float yaw_ = -128.0f;
    float pitch_ = -24.0f;
    float speed_ = 5.0f;
    float fov_ = 55.0f;
};
