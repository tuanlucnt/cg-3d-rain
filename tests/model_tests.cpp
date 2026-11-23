#include "ModelGeometry.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>

namespace {
void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }
void vertices(const std::vector<Vertex>& mesh) {
    require(!mesh.empty() && mesh.size() % 3 == 0, "Model triangle list is incomplete");
    for (std::size_t i = 0; i < mesh.size(); i += 3) {
        const auto cross = glm::cross(mesh[i+1].position - mesh[i].position, mesh[i+2].position - mesh[i].position);
        require(glm::length(cross) > 0.0000001f, "Degenerate model triangle");
        for (int j = 0; j < 3; ++j) {
            const auto& p = mesh[i+j];
            require(std::isfinite(p.position.x) && std::isfinite(p.position.y) && std::isfinite(p.position.z),
                    "Non-finite model vertex");
            require(std::abs(glm::length(p.normal) - 1) < 0.0001f &&
                    glm::dot(glm::normalize(cross), p.normal) > 0.999f, "Model normal and CCW winding disagree");
        }
    }
}
using Point = std::array<long, 3>;
Point key(glm::vec3 p) { return {std::lround(p.x*100000), std::lround(p.y*100000), std::lround(p.z*100000)}; }
void closed(const std::vector<Vertex>& mesh) {
    std::map<std::pair<Point,Point>, std::pair<int,int>> edges;
    for (std::size_t i=0; i<mesh.size(); i+=3) for (int j=0; j<3; ++j) {
        auto a=key(mesh[i+j].position), b=key(mesh[i+(j+1)%3].position);
        const int direction=a<b ? 1 : -1;
        if (b<a) std::swap(a,b);
        auto& edge=edges[{a,b}]; ++edge.first; edge.second+=direction;
    }
    for (const auto& edge : edges) require(edge.second.first==2 && edge.second.second==0,
                                           "Closed model has an open or inconsistently oriented edge");
}
float undersideAt(const std::vector<Vertex>& mesh, float x, float z) {
    float result=std::numeric_limits<float>::infinity();
    const glm::vec2 p(x,z);
    for (std::size_t i=0; i<mesh.size(); i+=3) {
        if (mesh[i].normal.y >= -0.1f) continue;
        const auto a=mesh[i].position,b=mesh[i+1].position,c=mesh[i+2].position;
        const glm::vec2 ab(b.x-a.x,b.z-a.z), ac(c.x-a.x,c.z-a.z), ap=p-glm::vec2(a.x,a.z);
        const float determinant=ab.x*ac.y-ab.y*ac.x;
        if (std::abs(determinant)<0.000001f) continue;
        const float u=(ap.x*ac.y-ap.y*ac.x)/determinant;
        const float v=(ab.x*ap.y-ab.y*ap.x)/determinant;
        if (u>=-0.0001f && v>=-0.0001f && u+v<=1.0001f) result=std::min(result,a.y+u*(b.y-a.y)+v*(c.y-a.y));
    }
    return result;
}
void houseTests() {
    const auto house=models::house();
    std::size_t triangles=0;
    for (const auto& part : house) {
        vertices(part); triangles+=part.size()/3;
        for (const auto& v : part)
            require(std::abs(v.position.x)<2.6f && v.position.y>=-0.00001f && v.position.y<4.9f &&
                    v.position.z>-2.4f && v.position.z<2.7f, "House exceeded its compact footprint");
    }
    const auto& roof=house[static_cast<std::size_t>(models::HousePart::Roof)];
    closed(roof);
    const auto& walls=house[static_cast<std::size_t>(models::HousePart::Walls)];
    for (const auto& v : walls) if (v.position.y>2.601f && std::abs(v.position.z)>1.79f)
        require(std::abs(undersideAt(roof,v.position.x,v.position.z)-v.position.y)<0.0001f,
                "Gable must meet roof underside without a gap or penetration");
    const auto& glass=house[static_cast<std::size_t>(models::HousePart::Glass)];
    for (const auto& v : glass) if (v.position.z>1.0f)
        require(v.position.z<1.94f, "Glass must be recessed behind window frames");
    require(triangles<1500, "House geometry budget exceeded");
    std::cout << "PASS house: " << triangles << " triangles in " << house.size() << " material batches\n";
}
void treeTests() {
    std::size_t total=0;
    const auto& layout=models::treeLayout();
    require(layout.size()==4,"Must retain exactly four trees");
    for (const auto& instance : layout) {
        const auto data=models::tree(instance.geometry);
        const auto repeat=models::tree(instance.geometry);
        vertices(data.bark); vertices(data.foliage); closed(data.foliage);
        require(data.foliage.size()==repeat.foliage.size(),"Tree seed must reproduce vertex count");
        for (std::size_t i=0;i<data.foliage.size();++i)
            require(data.foliage[i].position==repeat.foliage[i].position,"Tree geometry must be deterministic");
        for (const auto& v : data.foliage) {
            require(v.position.y>0.6f && v.position.y<=instance.geometry.height+0.001f,
                    "Canopy must taper below its apex");
            const auto world=v.position+instance.position;
            require(std::abs(world.x)<10 && std::abs(world.z)<10,"Tree canopy must stay within the ground");
        }
        const auto triangles=(data.bark.size()+data.foliage.size())/3;
        require(triangles<700,"Per-tree triangle budget exceeded");
        total+=triangles;
    }
    auto a=layout[0].geometry,b=a; ++b.seed;
    require(models::tree(a).foliage[1].position!=models::tree(b).foliage[1].position,
            "Different seeds must change tree silhouettes");
    bool rejected=false;
    try { a.branchCount=10000; models::tree(a); } catch (const std::invalid_argument&) { rejected=true; }
    require(rejected,"Reject parameters that would create excessive geometry");
    std::cout<<"PASS four trees: "<<total<<" triangles, deterministic silhouettes, closed canopies\n";
}
}
int main() {
    try { houseTests(); treeTests(); return 0; }
    catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
