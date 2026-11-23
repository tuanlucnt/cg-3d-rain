#include "EnvironmentGeometry.hpp"
#include "Surface.hpp"
#include "RippleSystem.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <stdexcept>

namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
using Point=std::array<long,3>;
Point point(glm::vec3 p) { return {std::lround(p.x*100000),std::lround(p.y*100000),std::lround(p.z*100000)}; }
void valid(const std::vector<Vertex>& mesh, bool closed, bool smooth=false) {
    require(!mesh.empty() && mesh.size()%3==0,"Incomplete environment triangle list");
    std::map<std::pair<Point,Point>,std::pair<int,int>> edges;
    for (std::size_t i=0;i<mesh.size();i+=3) {
        const auto n=glm::cross(mesh[i+1].position-mesh[i].position,mesh[i+2].position-mesh[i].position);
        require(glm::length(n)>0.000001f,"Degenerate environment triangle");
        for (int j=0;j<3;++j) {
            const auto& v=mesh[i+j];
            require(std::isfinite(v.position.x) && std::isfinite(v.position.y) && std::isfinite(v.position.z),"Non-finite environment vertex");
            require(std::abs(glm::length(v.normal)-1)<0.0001f && glm::dot(glm::normalize(n),v.normal)>(smooth?0.45f:0.999f),
                    "Invalid normal/winding in environment mesh");
            auto a=point(v.position),b=point(mesh[i+(j+1)%3].position);
            const int sign=a<b?1:-1;
            if (b<a) std::swap(a,b);
            auto& edge=edges[{a,b}]; ++edge.first; edge.second+=sign;
        }
    }
    if (closed) for (const auto& e:edges)
        require(e.second.first==2 && e.second.second==0,"Open or inconsistent environment skin");
}
void waterTests() {
    const auto mesh=environment::puddles();
    valid(mesh,false);
    require(mesh.size()/3==surface::OUTLINE_POINTS*3,"All three puddles must be in one static mesh");
    for (const auto& v:mesh) require(v.normal.y>0.999f && v.position.y>0 && v.position.y<0.02f,
                                    "Water must face up, above the ground without changing impact height");
    for (std::size_t index=0;index<3;++index) {
        const auto& p=surface::puddles()[index];
        const auto& polygon=surface::polygon(index);
        require(surface::waterMargin(p.center)>0.3f,"Water center must be inside shared collision polygon");
        // Interior samples near different sections of the concave shoreline.
        for (const auto& edge:polygon) {
            const glm::vec2 xz=glm::mix(glm::vec2(p.center.x,p.center.z),edge,0.65f);
            const glm::vec3 hit(xz.x,0,xz.y);
            const float margin=surface::waterMargin(hit);
            require(margin>0,"Interior water samples must match the rendered polygon");
            RippleSystem ripple;
            ripple.emit(hit,false,margin);
            for (int t=0;t<6;++t) {
                for (const auto& r:ripple.particles()) if (r.lifetime>0) {
                    require(r.center.y>p.center.y,"Ripple must not Z-fight the water");
                    for (int j=0;j<24;++j) {
                        const float a=j*6.2831853f/24;
                        require(surface::waterMargin(r.center+glm::vec3(std::cos(a),0,std::sin(a))*r.radius)>0,
                                "Water ripple escaped the shoreline");
                    }
                }
                ripple.update(0.1f);
            }
        }
    }
    require(surface::waterMargin({9,0,9})==0,"Dry land classified as water");
    std::cout<<"PASS water: three batched shorelines, shared geometry/collision, ripples remain inside\n";
}
void rockTests() {
    require(environment::rocks().size()==3,"Scene must retain exactly three rocks");
    for (const auto& instance:environment::rocks()) {
        const auto mesh=environment::rock(instance.seed), repeat=environment::rock(instance.seed);
        valid(mesh,true);
        require(mesh.size()/3==48,"Rock polygon budget changed");
        float minimum=100;
        for (std::size_t i=0;i<mesh.size();++i) {
            require(mesh[i].position==repeat[i].position,"Rock seed must be deterministic");
            minimum=std::min(minimum,mesh[i].position.y*instance.scale.y+instance.position.y);
        }
        require(minimum>=-0.0001f && minimum<0.001f,"Rock must sit on ground without hovering or Z-fighting");
    }
    require(environment::rock(137)[0].position!=environment::rock(251)[0].position,"Rock silhouettes must differ");
    std::cout<<"PASS rocks: three distinct seeded closed 48-triangle meshes, grounded placement\n";
}
void cloudTests() {
    CloudSystem clouds;
    glm::vec3 previous(0);
    for (const auto& cluster:clouds.clusters()) {
        const auto mesh=environment::cloud(cluster);
        valid(mesh,true,true);
        require(mesh.size()/3==728,"Cloud mesh exceeded the per-cluster budget");
        float low=100,high=-100;
        for (const auto& v:mesh) {
            low=std::min(low,v.position.y); high=std::max(high,v.position.y);
            float density=0;
            glm::vec3 outward(0);
            for (const auto& lobe:cluster.lobes) {
                const auto scale=lobe.scale*glm::vec3(1,1.16f,1),delta=v.position-lobe.offset;
                const float value=std::exp(-2*glm::dot(delta/scale,delta/scale));
                density+=value; outward+=value*delta/(scale*scale);
            }
            require(std::abs(density-0.18f)<0.00001f,"Cloud skin must follow the blended density field");
            require(glm::dot(glm::normalize(outward),v.normal)>0.95f,"Cloud gradient normals must point outside");
        }
        require(low< -0.7f && high>1.0f,"Cloud should have a broad base and a raised crown");
        require(mesh[0].position!=previous,"Cloud profiles must retain distinct silhouettes");
        previous=mesh[0].position;
    }
    std::cout<<"PASS clouds: four closed blended skins, density/normal agreement, 728 triangles each\n";
}
void grassTests() {
    const auto mesh=environment::grass();
    valid(mesh,false);
    require(mesh.size()/3==384,"Grass must remain one bounded mesh, six triangles per tuft");
    for (const auto& tuft:environment::grassTufts()) {
        require(environment::grassAllowed({tuft.position.x,tuft.position.z}),"Grass overlaps a protected footprint or water");
        require(tuft.height>=0.10f && tuft.height<=0.22f,"Grass must remain low and bounded");
    }
    for (const auto& v:mesh) require(v.position.y>=0.005f && v.position.y<0.23f &&
        std::abs(v.position.x)<10 && std::abs(v.position.z)<10,"Grass exceeded the ground footprint");
    require(!environment::grassAllowed({-1,2}) && !environment::grassAllowed({-2,4.5f}) &&
            !environment::grassAllowed({-1,0}),"Grass exclusions must cover path, puddles and house");
    std::cout<<"PASS grass: 64 cached tufts in one 384-triangle mesh, placement exclusions\n";
}
}
int main() {
    try { waterTests(); rockTests(); cloudTests(); grassTests(); return 0; }
    catch (const std::exception& e) { std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1; }
}
