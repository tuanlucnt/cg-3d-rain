#include "ModelGeometry.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace {
using MeshData = std::vector<Vertex>;
void tri(MeshData& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 outward) {
    if (glm::dot(glm::cross(b - a, c - a), outward) < 0) std::swap(b, c);
    const auto n = glm::normalize(glm::cross(b - a, c - a));
    for (auto p : {a, b, c}) v.push_back({p, n});
}
void quad(MeshData& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 outward) {
    tri(v, a, b, c, outward); tri(v, a, c, d, outward);
}
void box(MeshData& v, glm::vec3 position, glm::vec3 scale, const glm::mat4& parent = glm::mat4(1)) {
    static const auto unit = geometry::box();
    const auto model = glm::scale(glm::translate(parent, position), scale);
    const auto normal = glm::transpose(glm::inverse(glm::mat3(model)));
    for (const auto& p : unit) v.push_back({glm::vec3(model * glm::vec4(p.position, 1)), glm::normalize(normal * p.normal)});
}
void beam(MeshData& v, glm::vec3 a, glm::vec3 b, float width, float depth) {
    const auto axis = glm::normalize(b - a);
    const auto side = glm::normalize(glm::cross(axis, glm::vec3(0, 0, 1)));
    const auto back = glm::cross(side, axis);
    const glm::mat4 basis(glm::vec4(side, 0), glm::vec4(axis, 0), glm::vec4(back, 0), glm::vec4((a + b) * 0.5f, 1));
    box(v, {0, 0, 0}, {width, glm::distance(a, b), depth}, basis);
}
void tube(MeshData& v, glm::vec3 a, glm::vec3 b, float ra, float rb, unsigned sides) {
    const auto axis = glm::normalize(b - a);
    const auto reference = std::abs(axis.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    const auto u = glm::normalize(glm::cross(axis, reference));
    const auto w = glm::cross(u, axis);
    for (unsigned i = 0; i < sides; ++i) {
        const float x = static_cast<float>(i) * 6.2831853f / static_cast<float>(sides);
        const float y = static_cast<float>(i + 1) * 6.2831853f / static_cast<float>(sides);
        const auto p = u * std::cos(x) + w * std::sin(x);
        const auto q = u * std::cos(y) + w * std::sin(y);
        quad(v, a + p * ra, b + p * rb, b + q * rb, a + q * ra, p + q);
        tri(v, a, a + p * ra, a + q * ra, -axis);
        tri(v, b, b + q * rb, b + p * rb, axis);
    }
}
void roofShell(MeshData& v) {
    constexpr float z = 2.15f;
    const glm::vec3 l{-2.38f, 2.56f, z}, m{0, 4.20f, z}, r{2.38f, 2.56f, z};
    const glm::vec3 li{-2.38f, 2.44f, z}, mi{0, 4.08f, z}, ri{2.38f, 2.44f, z};
    const auto back = [](glm::vec3 p) { p.z = -z; return p; };
    quad(v, l, back(l), back(m), m, {-1, 1, 0});
    quad(v, m, back(m), back(r), r, {1, 1, 0});
    quad(v, li, mi, back(mi), back(li), {1, -1, 0});
    quad(v, mi, ri, back(ri), back(mi), {-1, -1, 0});
    quad(v, l, li, back(li), back(l), {-1, 0, 0});
    quad(v, r, back(r), back(ri), ri, {1, 0, 0});
    quad(v, l, m, mi, li, {0, 0, 1});
    quad(v, m, r, ri, mi, {0, 0, 1});
    quad(v, back(l), back(li), back(mi), back(m), {0, 0, -1});
    quad(v, back(m), back(mi), back(ri), back(r), {0, 0, -1});
}
void gable(MeshData& v) {
    const float edge = 4.08f - 2.0f * (4.08f - 2.44f) / 2.38f;
    const std::array<glm::vec2, 5> profile{{{-2, 2.6f}, {2, 2.6f}, {2, edge}, {0, 4.08f}, {-2, edge}}};
    const auto point = [&](std::size_t i, float z) { return glm::vec3(profile[i], z); };
    for (std::size_t i = 1; i + 1 < profile.size(); ++i) {
        tri(v, point(0, 1.8f), point(i, 1.8f), point(i + 1, 1.8f), {0, 0, 1});
        tri(v, point(0, -1.8f), point(i + 1, -1.8f), point(i, -1.8f), {0, 0, -1});
    }
    for (std::size_t i = 0; i < profile.size(); ++i) {
        const auto j = (i + 1) % profile.size();
        const auto d = profile[j] - profile[i];
        quad(v, point(i, 1.8f), point(i, -1.8f), point(j, -1.8f), point(j, 1.8f), {d.y, -d.x, 0});
    }
}
}

namespace models {
House house() {
    House h;
    for (auto& part : h) part.reserve(1536);
    const auto part = [&](HousePart p) -> MeshData& { return h[static_cast<std::size_t>(p)]; };
    auto& walls = part(HousePart::Walls);
    box(walls, {0, 1.3f, 0}, {4, 2.6f, 3.6f}); gable(walls);
    auto& stone = part(HousePart::Stone);
    box(stone, {0, 0.12f, 0}, {4.2f, 0.24f, 3.8f});
    box(stone, {0, 0.1f, 2.22f}, {1.4f, 0.2f, 0.72f});
    box(stone, {0, 0.22f, 1.99f}, {1.22f, 0.18f, 0.44f});
    auto& roof = part(HousePart::Roof);
    roofShell(roof);
    box(roof, {0, 4.2f, 0}, {0.14f, 0.12f, 4.4f});
    auto& trim = part(HousePart::Trim);
    for (float x : {-2.03f, 2.03f}) for (float z : {-1.83f, 1.83f})
        box(trim, {x, 1.42f, z}, {0.10f, 2.34f, 0.10f});
    for (float x : {-2.38f, 2.38f}) box(trim, {x, 2.50f, 0}, {0.13f, 0.18f, 4.47f});
    for (float z : {-2.22f, 2.22f}) {
        beam(trim, {-2.38f, 2.51f, z}, {0, 4.15f, z}, 0.10f, 0.12f);
        beam(trim, {0, 4.15f, z}, {2.38f, 2.51f, z}, 0.10f, 0.12f);
    }
    for (float x : {-0.53f, 0.53f}) box(trim, {x, 1.20f, 1.88f}, {0.14f, 1.90f, 0.20f});
    box(trim, {0, 2.16f, 1.88f}, {1.20f, 0.14f, 0.20f});
    auto& wood = part(HousePart::Door);
    box(wood, {0, 1.19f, 1.83f}, {0.90f, 1.74f, 0.10f});
    for (float x : {-0.215f, 0.215f}) for (float y : {0.77f, 1.56f})
        box(wood, {x, y, 1.895f}, {0.33f, 0.65f, 0.025f});
    auto& metal = part(HousePart::Metal);
    box(metal, {0.31f, 1.17f, 1.92f}, {0.1f, 0.18f, 0.025f});
    tube(metal, {0.31f, 1.17f, 1.93f}, {0.31f, 1.17f, 2.03f}, 0.045f, 0.045f, 6);
    const auto window = [&](glm::vec3 center, float yaw) {
        const auto parent = glm::rotate(glm::translate(glm::mat4(1), center), glm::radians(yaw), glm::vec3(0, 1, 0));
        for (float x : {-0.415f, 0.415f}) box(trim, {x, 0, 0.07f}, {0.10f, 0.9f, 0.14f}, parent);
        for (float y : {-0.4f, 0.4f}) box(trim, {0, y, 0.07f}, {0.74f, 0.10f, 0.14f}, parent);
        box(trim, {0, 0, 0.11f}, {0.045f, 0.74f, 0.08f}, parent);
        box(trim, {0, 0, 0.11f}, {0.74f, 0.045f, 0.08f}, parent);
        box(part(HousePart::Glass), {0, 0, 0.035f}, {0.74f, 0.74f, 0.035f}, parent);
        box(stone, {0, -0.49f, 0.11f}, {1.06f, 0.10f, 0.28f}, parent);
    };
    window({-1.25f, 1.6f, 1.8f}, 0); window({1.25f, 1.6f, 1.8f}, 0);
    window({2.0f, 1.6f, 0}, 90);
    auto& brick = part(HousePart::Chimney);
    for (float x : {-1.06f, -0.64f}) box(brick, {x, 3.99f, -0.65f}, {0.08f, 1.36f, 0.55f});
    for (float z : {-0.885f, -0.415f}) box(brick, {-0.85f, 3.99f, z}, {0.34f, 1.36f, 0.08f});
    for (float x : {-1.14f, -0.56f}) box(stone, {x, 4.70f, -0.65f}, {0.12f, 0.14f, 0.75f});
    for (float z : {-0.965f, -0.335f}) box(stone, {-0.85f, 4.70f, z}, {0.46f, 0.14f, 0.12f});
    quad(part(HousePart::Cavity), {-1.08f, 4.60f, -0.9f}, {-1.08f, 4.60f, -0.4f},
         {-0.62f, 4.60f, -0.4f}, {-0.62f, 4.60f, -0.9f}, {0, 1, 0});
    return h;
}

const std::array<TreeInstance, 4>& treeLayout() {
    static const std::array<TreeInstance, 4> layout{{
        {{-6.8f,0,-1},0.92f,{3.99f,0.189f,1.3125f,8,11}},
        {{5,0,-5},1.04f,{4.37f,0.207f,1.4375f,9,29}},
        {{-6.5f,0,4},0.98f,{3.42f,0.162f,1.125f,7,47}},
        {{5.5f,0,4},1.08f,{3.8f,0.18f,1.25f,10,83}}
    }};
    return layout;
}

Tree tree(const TreeParameters& p) {
    if (!std::isfinite(p.height) || !std::isfinite(p.trunkRadius) || !std::isfinite(p.canopyWidth) ||
        p.height < 2 || p.height > 6 || p.trunkRadius <= 0 || p.trunkRadius > 0.35f ||
        p.canopyWidth < 0.6f || p.canopyWidth > 2 || p.branchCount < 4 || p.branchCount > 12)
        throw std::invalid_argument("Tree parameters exceed the low-poly model budget");
    Tree result;
    result.bark.reserve((32 + p.branchCount * 24) * 3);
    result.foliage.reserve(720);
    std::mt19937 rng(p.seed);
    const auto random = [&](float low, float high) { return std::uniform_real_distribution<float>(low, high)(rng); };
    const float phase = random(0, 6.2831853f);
    tube(result.bark, {0,0,0}, {0,p.height*0.88f,0}, p.trunkRadius, p.trunkRadius*0.18f, 8);
    for (unsigned i=0; i<p.branchCount; ++i) {
        const float t=0.16f + 0.66f*static_cast<float>(i)/static_cast<float>(p.branchCount-1);
        const float angle=phase + static_cast<float>(i)*2.3999632f + random(-0.12f,0.12f);
        const float length=p.canopyWidth*(1-t)*0.9f+0.10f;
        const glm::vec3 root(0,t*p.height,0);
        const glm::vec3 tip(std::cos(angle)*length, root.y+length*0.40f, std::sin(angle)*length);
        tube(result.bark, root, tip, p.trunkRadius*(0.35f-0.18f*t), p.trunkRadius*0.045f, 6);
    }
    constexpr unsigned sectors=10, levels=12;
    const std::array<float,levels> heights{{0.22f,0.29f,0.34f,0.39f,0.45f,0.51f,0.56f,0.64f,0.71f,0.77f,0.84f,0.93f}};
    const std::array<float,levels> widths{{0.70f,1,0.90f,0.78f,0.81f,0.72f,0.60f,0.60f,0.48f,0.40f,0.32f,0.14f}};
    std::array<float,sectors> sectorScale{};
    for (auto& s : sectorScale) s=random(0.86f,1.10f);
    std::array<std::array<glm::vec3,sectors>,levels> rings{};
    for (unsigned k=0;k<levels;++k) {
        const float t=heights[k];
        const glm::vec2 center(0.06f*std::sin(phase+t*3),0.05f*std::cos(phase+t*4));
        for (unsigned i=0;i<sectors;++i) {
            const float angle=phase+static_cast<float>(i)*6.2831853f/sectors + 0.035f*std::sin(k*1.7f+phase);
            const float irregularity=std::clamp(sectorScale[i]*(1+0.10f*std::sin(angle*3+phase+k*1.4f)),0.78f,1.18f);
            const float radius=p.canopyWidth*widths[k]*irregularity;
            rings[k][i]={center.x+std::cos(angle)*radius,
                p.height*t+0.05f*p.height*(1-t)*std::sin(angle*2+phase+k*0.6f),
                center.y+std::sin(angle)*radius};
        }
    }
    for (unsigned k=0;k+1<levels;++k) for (unsigned i=0;i<sectors;++i) {
        const auto j=(i+1)%sectors;
        const glm::vec3 outward(rings[k][i].x+rings[k][j].x,0,rings[k][i].z+rings[k][j].z);
        quad(result.foliage,rings[k][i],rings[k+1][i],rings[k+1][j],rings[k][j],outward);
    }
    const glm::vec3 tip(0.06f*std::sin(phase+3),p.height,0.05f*std::cos(phase+4));
    const glm::vec3 bottom(0,p.height*heights[0],0);
    for (unsigned i=0;i<sectors;++i) {
        const auto j=(i+1)%sectors;
        tri(result.foliage,bottom,rings[0][i],rings[0][j],{0,-1,0});
        tri(result.foliage,tip,rings[levels-1][j],rings[levels-1][i],{0,1,0});
    }
    return result;
}
}
