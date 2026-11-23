#include "Camera.hpp"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() { reset(); }

void Camera::reset() {
    position_ = {14.0f, 10.0f, 18.0f};
    yaw_ = -128.0f;
    pitch_ = -24.0f;
    updateDirection();
}

void Camera::preset(unsigned index) {
    if (index == 0) { reset(); return; }
    if (index > 2) return;
    position_ = index == 1 ? glm::vec3(5, 3.2f, 8) : glm::vec3(-0.4f, 0.85f, 7);
    const auto target = index == 1 ? glm::vec3(-1, 1.4f, -1) : glm::vec3(-2, 0.12f, 4.5f);
    const auto direction = glm::normalize(target - position_);
    yaw_ = glm::degrees(std::atan2(direction.z, direction.x));
    pitch_ = std::clamp(glm::degrees(std::asin(direction.y)), -89.0f, 89.0f);
    updateDirection();
}

void Camera::updateDirection() {
    const float yaw = glm::radians(yaw_);
    const float pitch = glm::radians(pitch_);
    forward_ = glm::normalize(glm::vec3{
        std::cos(yaw) * std::cos(pitch), std::sin(pitch),
        std::sin(yaw) * std::cos(pitch)});
}

void Camera::rotate(float deltaX, float deltaY) {
    yaw_ = std::remainder(yaw_ + deltaX * 0.12f, 360.0f);
    pitch_ = std::clamp(pitch_ - deltaY * 0.12f, -89.0f, 89.0f);
    updateDirection();
}

void Camera::move(const glm::vec3& localDirection, float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f || glm::dot(localDirection, localDirection) == 0.0f) {
        return;
    }
    const glm::vec3 right = glm::normalize(glm::cross(forward_, up_));
    glm::vec3 direction = right * localDirection.x + up_ * localDirection.y
                        + forward_ * localDirection.z;
    if (glm::dot(direction, direction) > 0.000001f) {
        position_ += glm::normalize(direction) * speed_ * deltaTime;
    }
    position_ = glm::clamp(position_, glm::vec3(-16.0f, 0.35f, -16.0f),
                          glm::vec3(16.0f, 14.0f, 18.0f));
}

glm::mat4 Camera::view() const {
    return glm::lookAt(position_, position_ + forward_, up_);
}

glm::mat4 Camera::projection(float aspect) const {
    return glm::perspective(glm::radians(fov_), std::max(aspect, 0.001f), 0.1f, 80.0f);
}
