#include "LightningSystem.hpp"

#include <algorithm>
#include <cmath>

float LightningSystem::random(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(random_);
}

void LightningSystem::reset(std::uint32_t seed) {
    random_.seed(seed);
    segmentCount_ = 0;
    strikeCount_ = 0;
    active_ = false;
    elapsed_ = duration_ = 0;
    nextStrike_ = random(2, 6);
    ++revision_;
}

void LightningSystem::cancel() {
    active_ = false;
    segmentCount_ = 0;
    nextStrike_ = random(2, 6);
}

void LightningSystem::subdivide(glm::vec3 a, glm::vec3 b, unsigned depth,
                                float displacement, float width, bool branch) {
    if (!depth) {
        if (segmentCount_ < MAX_SEGMENTS) segments_[segmentCount_++] = {a, b, width, branch};
        return;
    }
    const glm::vec3 direction = glm::normalize(b - a);
    const glm::vec3 reference = std::abs(direction.y) < 0.9f
        ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    const glm::vec3 side = glm::normalize(glm::cross(direction, reference));
    const glm::vec3 other = glm::normalize(glm::cross(direction, side));
    glm::vec3 offset = side * random(-displacement, displacement)
                     + other * random(-displacement, displacement);
    const float maxOffset = glm::distance(a, b) * 0.12f;
    if (glm::length(offset) > maxOffset) offset *= maxOffset / glm::length(offset);
    const glm::vec3 center = (a + b) * 0.5f;
    // Scale the entire perpendicular offset at the domain edge, preserving its
    // direction instead of clamping coordinates and introducing sharp corners.
    float amount = 1;
    const glm::vec3 low(-9.8f, 0.03f, -9.8f), high(9.8f, 10.0f, 9.8f);
    for (int axis = 0; axis < 3; ++axis) {
        if (offset[axis] > 0) amount = std::min(amount, (high[axis] - center[axis]) / offset[axis]);
        if (offset[axis] < 0) amount = std::min(amount, (low[axis] - center[axis]) / offset[axis]);
    }
    const glm::vec3 midpoint = center + offset * std::clamp(amount, 0.0f, 1.0f);
    subdivide(a, midpoint, depth - 1, displacement * 0.40f, width, branch);
    subdivide(midpoint, b, depth - 1, displacement * 0.40f, width, branch);
}

void LightningSystem::trigger(const CloudSystem& clouds, glm::vec3 observer) {
    segmentCount_ = 0;
    std::array<std::size_t, CloudSystem::COUNT> eligible{};
    std::size_t count = 0, farthest = 0;
    float farthestDistance = -1;
    for (std::size_t i = 0; i < CloudSystem::COUNT; ++i) {
        const auto p = clouds.clusters()[i].position;
        const float distance = glm::length(glm::vec2(p.x - observer.x, p.z - observer.z));
        if (CloudSystem::coverage(p) > 0.65f && distance > 4.5f) eligible[count++] = i;
        if (distance > farthestDistance) { farthestDistance = distance; farthest = i; }
    }
    const auto index = count ? eligible[std::uniform_int_distribution<std::size_t>(0, count - 1)(random_)]
                             : farthest;
    const glm::vec3 start = glm::clamp(clouds.clusters()[index].position +
        glm::vec3(random(-0.4f, 0.4f), 0.1f, random(-0.4f, 0.4f)),
        glm::vec3(-9.5f, 6.0f, -9.5f), glm::vec3(9.5f, 9.5f, 9.5f));
    const glm::vec3 end{std::clamp(start.x + random(-1.5f, 1.5f), -8.0f, 8.0f),
                        0.03f, std::clamp(start.z + random(-1.5f, 1.5f), -8.0f, 8.0f)};
    subdivide(start, end, MAIN_DEPTH, 0.85f, 0.095f, false);
    for (std::size_t i = 0; i < MAIN_SEGMENTS; ++i)
        segments_[i].width *= 1.0f - 0.35f * static_cast<float>(i) / static_cast<float>(MAIN_SEGMENTS - 1);
    // Every branch root is an exact shared vertex of the primary polyline.
    for (std::size_t i = 0; i < MAX_BRANCHES; ++i) {
        const auto root = segments_[5 + i * 4].end;
        const float side = i % 2 == 0 ? -1.0f : 1.0f;
        const glm::vec3 tip{std::clamp(root.x + side * random(0.9f, 2.0f), -9.5f, 9.5f),
                            std::max(0.08f, root.y - random(0.7f, 1.8f)),
                            std::clamp(root.z + random(-1.3f, 1.3f), -9.5f, 9.5f)};
        const auto first = segmentCount_;
        subdivide(root, tip, 3, 0.24f, 0.032f, true);
        for (std::size_t j = 0; j < 8; ++j)
            segments_[first + j].width *= 1.0f - 0.6f * static_cast<float>(j) / 7.0f;
    }
    elapsed_ = 0;
    duration_ = random(0.28f, 0.42f);
    pulseCount_ = std::uniform_int_distribution<unsigned>(1, 3)(random_);
    const float interval = duration_ / static_cast<float>(pulseCount_);
    for (unsigned i = 0; i < pulseCount_; ++i) {
        pulses_[i] = {interval * static_cast<float>(i), interval * random(0.55f, 0.70f),
                      i == 0 ? 1.0f : random(0.65f, 0.92f)};
    }
    nextStrike_ = random(2, 6);
    active_ = true;
    ++strikeCount_;
    ++revision_;
}

void LightningSystem::update(float deltaTime, const CloudSystem& clouds, bool randomEnabled,
                             glm::vec3 observer) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0) return;
    deltaTime = std::min(deltaTime, 0.1f);
    if (active_) {
        elapsed_ += deltaTime;
        if (elapsed_ >= duration_) active_ = false;
    } else if (randomEnabled) {
        nextStrike_ -= deltaTime;
        if (nextStrike_ <= 0) trigger(clouds, observer);
    }
}

float LightningSystem::flashIntensity() const {
    if (!active_) return 0;
    for (unsigned i = 0; i < pulseCount_; ++i) {
        const auto& pulse = pulses_[i];
        const float local = elapsed_ - pulse.start;
        if (local >= 0 && local < pulse.duration) {
            const float envelope = std::clamp((pulse.duration - local) / 0.012f, 0.0f, 1.0f);
            return pulse.peak * std::exp(-32.0f * local) * envelope;
        }
    }
    return 0; // A real dark interval separates successive pulses.
}
