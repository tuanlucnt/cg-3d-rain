#pragma once

#include "Camera.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "WeatherSystem.hpp"
#include "DynamicBatch.hpp"

class Renderer {
public:
    Renderer();
    void draw(const Scene& scene, const Camera& camera, const WeatherSystem& weather,
              int width, int height);
    std::size_t drawCalls() const { return drawCalls_; }
    std::size_t renderedRainCount() const { return rainRendered_; }
    std::size_t renderedSplashCount() const { return splashRendered_; }
    std::size_t renderedRippleCount() const { return rippleRendered_; }
    std::uint64_t lightningUploadCount() const { return lightningBatch_.uploadCount(); }
    std::uint64_t rainUploadCount() const { return rainBatch_.uploadCount(); }
    std::size_t triangles() const { return triangles_; }
    std::size_t lines() const { return lines_; }

private:
    Shader shader_;
    Shader skyShader_;
    Mesh skyMesh_;
    GLint skyTop_, skyHorizon_;
    std::array<std::unique_ptr<Mesh>,CloudSystem::COUNT> cloudMeshes_;
    std::array<glm::vec2,CloudSystem::COUNT> cloudHeightRanges_{};
    Shader effectsShader_;
    DynamicBatch rainBatch_;
    DynamicBatch lightningBatch_;
    DynamicBatch splashBatch_;
    DynamicBatch rippleBatch_;
    std::array<EffectVertex, RainSystem::RAIN_COUNT * 2> rainVertices_{};
    std::array<EffectVertex, LightningSystem::MAX_SEGMENTS * 12> lightningVertices_{};
    std::array<EffectVertex, SplashSystem::MAX_SPLASHES * 2> splashVertices_{};
    std::array<EffectVertex, RippleSystem::CAPACITY * 24> rippleVertices_{};
    std::array<glm::vec2, 12> ring_{};
    std::uint64_t rippleRevision_ = 0;
    std::uint64_t rainRevision_ = 0;
    std::uint64_t splashRevision_ = 0;
    std::uint64_t lightningRevision_ = 0;
    glm::vec3 lightningCameraPosition_{}, lightningCameraForward_{};
    GLint effectsViewProjection_;
    GLint effectsCameraPosition_, effectsDistanceFade_;
    GLint effectsIntensity_, effectsSoftEdge_;
    GLint model_, view_, projection_, normal_, color_, coverage_, flash_;
    GLint flashPosition_, cloud_, cloudHeightRange_;
    GLint wetness_, surface_, objectCamera_;
    GLint objectFogColor_, objectFogDensity_;
    GLint effectsFogColor_, effectsFogDensity_, effectsFogScale_;
    GLint materialKind_, materialResponse_, materialWetGloss_;
    GLint treeHeight_, modelTime_, treeWind_;
    GLint lightDirection_, ambientColor_, directionalColor_, windowStrength_, weatherLight_;
    GLint skyReflection_;
    GLint rainPass_;
    std::size_t drawCalls_ = 0, rainRendered_ = 0, splashRendered_ = 0, rippleRendered_ = 0;
    std::size_t triangles_ = 0, lines_ = 0;
};
