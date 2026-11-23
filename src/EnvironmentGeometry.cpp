#include "EnvironmentGeometry.hpp"
#include "Surface.hpp"
#include "ModelGeometry.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <stdexcept>

namespace {
void triangle(std::vector<Vertex>& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 outward) {
    if (glm::dot(glm::cross(b-a,c-a),outward)<0) std::swap(b,c);
    const auto normal=glm::normalize(glm::cross(b-a,c-a));
    for (auto p : {a,b,c}) mesh.push_back({p,normal});
}
}

namespace environment {
bool grassAllowed(glm::vec2 p) {
    if (std::abs(p.x)>9.6f || std::abs(p.y)>9.6f) return false;
    if (p.x> -3.5f && p.x<1.5f && p.y> -3.3f && p.y<1.95f) return false;
    const float pathX=-1+0.13f*std::sin(p.y*0.7f);
    if (p.y>1.2f && std::abs(p.x-pathX)<0.95f) return false;
    for (const auto& tree:models::treeLayout())
        if (glm::distance(p,glm::vec2(tree.position.x,tree.position.z))<0.7f) return false;
    for (const auto& rock:rocks())
        if (glm::distance(p,glm::vec2(rock.position.x,rock.position.z))<std::max(rock.scale.x,rock.scale.z)*1.2f+0.25f) return false;
    for (glm::vec2 offset:{glm::vec2(0),glm::vec2(0.2f,0),glm::vec2(-0.2f,0),glm::vec2(0,0.2f),glm::vec2(0,-0.2f)})
        if (surface::waterMargin({p.x+offset.x,0,p.y+offset.y})>0) return false;
    return true;
}

const std::array<GrassTuft,64>& grassTufts() {
    static const auto tufts=[] {
        std::array<GrassTuft,64> result{};
        std::mt19937 rng(824);
        std::uniform_real_distribution<float> coordinate(-9.5f,9.5f),unit(0,1);
        std::size_t count=0;
        for (unsigned attempt=0;count<result.size() && attempt<4096;++attempt) {
            const glm::vec2 p(coordinate(rng),coordinate(rng));
            if (!grassAllowed(p)) continue;
            result[count++]={{p.x,0.005f,p.y},0.10f+unit(rng)*0.12f,unit(rng)*6.2831853f};
        }
        if (count!=result.size()) throw std::runtime_error("Cannot fit bounded grass tufts in the environment");
        return result;
    }();
    return tufts;
}

std::vector<Vertex> grass() {
    std::vector<Vertex> mesh;
    mesh.reserve(grassTufts().size()*18);
    for (const auto& tuft:grassTufts()) for (unsigned blade=0;blade<3;++blade) {
        const float angle=tuft.angle+blade*2.0943951f;
        const glm::vec3 side(std::cos(angle)*0.045f,0,std::sin(angle)*0.045f);
        const auto a=tuft.position-side,b=tuft.position+side;
        const auto c=tuft.position+glm::vec3(side.x*0.6f,tuft.height*(1-blade*0.12f),side.z*0.6f);
        // Both sides are explicit triangles: keep culling enabled globally.
        const auto n=glm::normalize(glm::cross(b-a,c-a));
        triangle(mesh,a,b,c,n); triangle(mesh,c,b,a,-n);
    }
    return mesh;
}

std::vector<Vertex> puddles() {
    std::vector<Vertex> mesh;
    mesh.reserve(surface::puddles().size()*surface::OUTLINE_POINTS*3);
    for (std::size_t j=0;j<surface::puddles().size();++j) {
        const auto& p=surface::puddles()[j];
        const auto& outline=surface::polygon(j);
        for (std::size_t i=0;i<outline.size();++i) {
            const auto a=outline[i], b=outline[(i+1)%outline.size()];
            triangle(mesh,p.center,{a.x,p.center.y,a.y},{b.x,p.center.y,b.y},{0,1,0});
        }
    }
    return mesh;
}

const std::array<RockInstance,3>& rocks() {
    static const std::array<RockInstance,3> instances{{
        {{1.5f,0.33f,4},{1,0.6f,0.85f},{0.43f,0.46f,0.46f},22,137},
        {{-4.5f,0.22f,2.8f},{0.7f,0.4f,0.7f},{0.46f,0.47f,0.43f},-15,251},
        {{7.5f,0.27f,1.5f},{0.8f,0.49f,0.9f},{0.37f,0.41f,0.42f},48,389}
    }};
    return instances;
}

std::vector<Vertex> rock(std::uint32_t seed) {
    constexpr unsigned sides=8;
    std::mt19937 rng(seed);
    const auto random=[&](float low,float high) { return std::uniform_real_distribution<float>(low,high)(rng); };
    std::array<float,sides> angles{},radii{};
    for (unsigned i=0;i<sides;++i) {
        angles[i]=i*6.2831853f/sides+random(-0.12f,0.12f);
        radii[i]=random(0.82f,1.12f);
    }
    std::array<std::array<glm::vec3,sides>,3> rings{};
    const std::array<float,3> height{{-0.55f,-0.04f,0.54f}}, width{{0.65f,1,0.67f}};
    const glm::vec2 lean(random(-0.15f,0.15f),random(-0.15f,0.15f));
    for (unsigned k=0;k<3;++k) for (unsigned i=0;i<sides;++i) {
        const float a=angles[i]+k*0.045f;
        const float r=radii[i]*width[k]*random(0.94f,1.06f);
        rings[k][i]={std::cos(a)*r+lean.x*k*0.4f,
                     height[k]+(k==0?0:random(-0.08f,0.08f)),std::sin(a)*r+lean.y*k*0.4f};
    }
    std::vector<Vertex> mesh;
    mesh.reserve(48*3);
    const auto add=[&](glm::vec3 a,glm::vec3 b,glm::vec3 c) { triangle(mesh,a,b,c,(a+b+c)/3.0f); };
    for (unsigned i=0;i<sides;++i) {
        const auto j=(i+1)%sides;
        for (unsigned k=0;k<2;++k) {
            add(rings[k][i],rings[k+1][i],rings[k+1][j]);
            add(rings[k][i],rings[k+1][j],rings[k][j]);
        }
        add({0,-0.55f,0},rings[0][j],rings[0][i]);
        add({lean.x,0.95f,lean.y},rings[2][i],rings[2][j]);
    }
    return mesh;
}

std::vector<Vertex> cloud(const CloudCluster& cluster) {
    // Blend five compact ellipsoidal density fields, then extract a single
    // star-shaped outer skin. No internal lobe surfaces or alpha intersections.
    // This is an offline mesh builder, never a per-frame volumetric shader.
    for (const auto& lobe:cluster.lobes)
        if (glm::any(glm::lessThanEqual(lobe.scale,glm::vec3(0))) ||
            !std::isfinite(glm::length(lobe.scale)) || !std::isfinite(glm::length(lobe.offset)))
            throw std::invalid_argument("Cloud lobes require finite positive scales");
    const auto field=[&](glm::vec3 p,glm::vec3* normal=nullptr) {
        float density=0;
        glm::vec3 gradient(0);
        for (const auto& lobe:cluster.lobes) {
            const auto scale=lobe.scale*glm::vec3(1,1.16f,1);
            const auto delta=p-lobe.offset;
            const float value=std::exp(-2*glm::dot(delta/scale,delta/scale));
            density+=value;
            gradient+=value*delta/(scale*scale); // Negative density gradient.
        }
        if (normal) *normal=glm::normalize(gradient);
        return density;
    };
    constexpr unsigned slices=28,stacks=14;
    constexpr float pi=3.14159265359f, iso=0.18f;
    std::array<std::array<Vertex,slices>,stacks+1> rings{};
    for (unsigned row=0;row<=stacks;++row) for (unsigned col=0;col<slices;++col) {
        const float latitude=pi*row/stacks,longitude=2*pi*col/slices;
        glm::vec3 ray(std::sin(latitude)*std::cos(longitude),std::cos(latitude),std::sin(latitude)*std::sin(longitude));
        if (row==0) ray={0,1,0};
        if (row==stacks) ray={0,-1,0};
        float low=0,high=6;
        for (int j=0;j<24;++j) {
            const float radius=(low+high)*0.5f;
            if (field(ray*radius)>iso) low=radius; else high=radius;
        }
        auto& vertex=rings[row][col];
        vertex.position=ray*((low+high)*0.5f);
        field(vertex.position,&vertex.normal);
    }
    std::vector<Vertex> mesh;
    mesh.reserve(2*slices*(stacks-1)*3);
    const auto add=[&](Vertex a,Vertex b,Vertex c) {
        auto n=glm::cross(b.position-a.position,c.position-a.position);
        if (glm::dot(n,a.normal+b.normal+c.normal)<0) { std::swap(b,c); n=-n; }
        n=glm::normalize(n);
        // Smooth the lobe junctions while retaining a restrained low-poly facet.
        for (auto p:{a,b,c}) {
            p.normal=glm::normalize(p.normal*0.85f+n*0.15f);
            mesh.push_back(p);
        }
    };
    for (unsigned row=0;row<stacks;++row) for (unsigned col=0;col<slices;++col) {
        const auto next=(col+1)%slices;
        const auto a=rings[row][col], b=rings[row+1][col], c=rings[row+1][next], d=rings[row][next];
        if (row!=stacks-1) add(a,c,b);
        if (row!=0) add(a,d,c);
    }
    return mesh;
}
}
