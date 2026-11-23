#include "Renderer.hpp"
#include "Surface.hpp"
#include "EnvironmentGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

Renderer::Renderer()
    : shader_(std::string(RAIN_SHADER_DIR) + "/object.vert",
              std::string(RAIN_SHADER_DIR) + "/object.frag"),
      skyShader_(std::string(RAIN_SHADER_DIR) + "/sky.vert", std::string(RAIN_SHADER_DIR) + "/sky.frag"),
      skyMesh_(geometry::screenTriangle()),
      skyTop_(skyShader_.uniform("uTop")), skyHorizon_(skyShader_.uniform("uHorizon")),
      effectsShader_(std::string(RAIN_SHADER_DIR) + "/effects.vert",
                     std::string(RAIN_SHADER_DIR) + "/effects.frag"),
      rainBatch_(RainSystem::RAIN_COUNT * 2, GL_LINES),
      lightningBatch_(LightningSystem::MAX_SEGMENTS * 12, GL_TRIANGLES),
      splashBatch_(SplashSystem::MAX_SPLASHES * 2, GL_LINES),
      rippleBatch_(RippleSystem::CAPACITY * 24, GL_LINES),
      effectsViewProjection_(effectsShader_.uniform("uViewProjection")),
      effectsCameraPosition_(effectsShader_.uniform("uCameraPosition")),
      effectsDistanceFade_(effectsShader_.uniform("uDistanceFade")),
      effectsIntensity_(effectsShader_.uniform("uIntensity")),
      effectsSoftEdge_(effectsShader_.uniform("uSoftEdge")),
      model_(shader_.uniform("uModel")), view_(shader_.uniform("uView")),
      projection_(shader_.uniform("uProjection")), normal_(shader_.uniform("uNormalMatrix")),
      color_(shader_.uniform("uColor")), coverage_(shader_.uniform("uCoverage")),
      flash_(shader_.uniform("uFlash")), flashPosition_(shader_.uniform("uFlashPosition")),
      cloud_(shader_.uniform("uCloud")), cloudHeightRange_(shader_.uniform("uCloudHeightRange")),
      wetness_(shader_.uniform("uWetness")), surface_(shader_.uniform("uSurface")),
      objectCamera_(shader_.uniform("uCameraPosition")),
      objectFogColor_(shader_.uniform("uFogColor")), objectFogDensity_(shader_.uniform("uFogDensity")),
      effectsFogColor_(effectsShader_.uniform("uFogColor")),
      effectsFogDensity_(effectsShader_.uniform("uFogDensity")), effectsFogScale_(effectsShader_.uniform("uFogScale")),
      materialKind_(shader_.uniform("uMaterialKind")), materialResponse_(shader_.uniform("uMaterialResponse")),
      materialWetGloss_(shader_.uniform("uMaterialWetGloss")),
      treeHeight_(shader_.uniform("uTreeHeight")), modelTime_(shader_.uniform("uModelTime")),
      treeWind_(shader_.uniform("uTreeWind")) {
    const CloudSystem layout;
    for (std::size_t i=0;i<CloudSystem::COUNT;++i) {
        const auto mesh=environment::cloud(layout.clusters()[i]);
        auto range=glm::vec2(100,-100);
        for (const auto& vertex:mesh) { range.x=std::min(range.x,vertex.position.y); range.y=std::max(range.y,vertex.position.y); }
        cloudHeightRanges_[i]=range;
        cloudMeshes_[i]=std::make_unique<Mesh>(mesh);
    }
    glEnable(GL_DEPTH_TEST);
    for (std::size_t i = 0; i < ring_.size(); ++i) {
        const float a = static_cast<float>(i) * 6.2831853f / 12.0f;
        ring_[i] = {std::cos(a), std::sin(a)};
    }
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.08f, 0.11f, 0.16f, 1.0f);
    shader_.use();
    glUniform3f(shader_.uniform("uLightDirection"), -0.5f, -1.0f, -0.35f);
    glUniform1f(shader_.uniform("uAmbient"), LightingSystem::AMBIENT);
    glUniform1f(shader_.uniform("uDiffuse"), LightingSystem::DIFFUSE);
    glUniform2f(shader_.uniform("uFlashAttenuation"), LightingSystem::LINEAR_ATTENUATION,
                LightingSystem::QUADRATIC_ATTENUATION);
    glUniform3fv(shader_.uniform("uGroundColor"), 1, glm::value_ptr(surface::GROUND_COLOR));
    for (std::size_t i=0;i<surface::puddles().size();++i) {
        const auto& p=surface::puddles()[i];
        const float a=glm::radians(p.yaw);
        glUniform4f(shader_.uniform(("uPuddles["+std::to_string(i)+"]").c_str()),p.center.x,p.center.z,p.scale.x,p.scale.y);
        glUniform2f(shader_.uniform(("uPuddleRotation["+std::to_string(i)+"]").c_str()),std::cos(a),std::sin(a));
    }
    glUseProgram(0);
}

void Renderer::draw(const Scene& scene, const Camera& camera, const WeatherSystem& weather,
                    int width, int height) {
    drawCalls_ = rainRendered_ = splashRendered_ = 0;
    triangles_ = lines_ = 0;
    const auto sky = weather.lighting.skyColor();
    glClearColor(sky.r, sky.g, sky.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const auto top = weather.lighting.skyTop(), horizon = weather.lighting.fogColor();
    skyShader_.use();
    glUniform3fv(skyTop_, 1, glm::value_ptr(top));
    glUniform3fv(skyHorizon_, 1, glm::value_ptr(horizon));
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    skyMesh_.draw();
    ++triangles_;
    ++drawCalls_;
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    effectsShader_.use();
    const float density = weather.fogEnabled ? weather.lighting.fogDensity : 0.0f;
    glUniform3fv(effectsFogColor_, 1, glm::value_ptr(horizon));
    glUniform1f(effectsFogDensity_, density);
    glUniform3fv(effectsCameraPosition_, 1, glm::value_ptr(camera.position()));
    glUniform1f(effectsFogScale_, 1.0f);
    shader_.use();
    glUniform3fv(objectFogColor_, 1, glm::value_ptr(horizon));
    glUniform1f(objectFogDensity_, density);
    glUniform1f(flash_, weather.lighting.flash());
    glUniform3fv(flashPosition_, 1, glm::value_ptr(weather.lighting.flashPosition()));
    glUniform1f(cloud_, 0.0f);
    glUniform1f(wetness_, weather.lighting.wetness());
    glUniform1f(modelTime_, scene.animationTime());
    glUniform2f(treeWind_, weather.windVelocity.x, weather.windVelocity.z);
    glUniform3fv(objectCamera_, 1, glm::value_ptr(camera.position()));
    glUniform2f(cloudHeightRange_, 0.0f, 1.0f);
    const glm::mat4 view = camera.view();
    const glm::mat4 projection = camera.projection(static_cast<float>(width) /
                                                   static_cast<float>(height));
    glUniformMatrix4fv(view_, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projection_, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(coverage_, 1.0f);
    for (const auto& object : scene.objects()) {
        glUniformMatrix4fv(model_, 1, GL_FALSE, glm::value_ptr(object.model));
        glUniformMatrix3fv(normal_, 1, GL_FALSE, glm::value_ptr(object.normal));
        glUniform1f(surface_, object.puddle ? 2.0f : (object.ground ? 1.0f : 0.0f));
        glUniform3fv(color_, 1, glm::value_ptr(object.color));
        glUniform1i(materialKind_, static_cast<int>(object.material.kind));
        glUniform4fv(materialResponse_, 1, glm::value_ptr(object.material.response));
        glUniform1f(materialWetGloss_, object.material.wetGloss);
        glUniform1f(treeHeight_, object.treeHeight);
        object.mesh->draw();
        triangles_ += object.mesh->triangleCount();
        ++drawCalls_;
    }
    if (weather.cloudsEnabled) {
        glUniform1i(materialKind_, 0);
        glUniform4f(materialResponse_, 1, 1, 0, 32);
        glUniform1f(materialWetGloss_, 0);
        glUniform1f(treeHeight_, 0);
        glUniform1f(surface_, 0.0f);
        glUniform1f(cloud_, 1.0f);
        struct FadingCloud { std::size_t index; float depth; };
        std::array<FadingCloud,CloudSystem::COUNT> fading{};
        std::size_t fadingCount=0;
        const auto drawCloud=[&](std::size_t i) {
            const auto& cloud=weather.clouds.clusters()[i];
            const auto height=cloudHeightRanges_[i]+glm::vec2(cloud.position.y);
            glUniform2f(cloudHeightRange_,height.x,height.y);
            glUniform1f(coverage_,CloudSystem::coverage(cloud.position));
            const auto model=glm::translate(glm::mat4(1),cloud.position);
            const glm::mat3 normal(1);
            glUniformMatrix4fv(model_,1,GL_FALSE,glm::value_ptr(model));
            glUniformMatrix3fv(normal_,1,GL_FALSE,glm::value_ptr(normal));
            glUniform3f(color_,0.40f,0.44f,0.49f);
            cloudMeshes_[i]->draw();
            triangles_+=cloudMeshes_[i]->triangleCount();
            ++drawCalls_;
        };
        for (std::size_t i=0;i<CloudSystem::COUNT;++i) {
            const auto& cloud=weather.clouds.clusters()[i];
            const float opacity=CloudSystem::coverage(cloud.position);
            if (opacity<=0) continue;
            if (opacity>=1.0f) drawCloud(i);
            else fading[fadingCount++]={i,glm::dot(cloud.position-camera.position(),camera.forward())};
        }
        if (fadingCount) {
            std::sort(fading.begin(),fading.begin()+fadingCount,[](const auto& a,const auto& b) { return a.depth>b.depth; });
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            for (std::size_t i=0;i<fadingCount;++i) drawCloud(fading[i].index);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
    }

    if (weather.rainEnabled) {
        effectsShader_.use();
        glUniform1f(effectsFogScale_, 0.35f);
        glUniform1f(effectsIntensity_, 1.0f);
        glUniform1f(effectsSoftEdge_, 0.0f);
        glUniform3fv(effectsCameraPosition_, 1, glm::value_ptr(camera.position()));
        glUniform1f(effectsDistanceFade_, 1.0f);
        const auto vp = projection * view;
        glUniformMatrix4fv(effectsViewProjection_, 1, GL_FALSE, glm::value_ptr(vp));
        if (rainRevision_ != weather.rain.revision()) {
            std::size_t i = 0;
            for (const auto& drop : weather.rain.drops()) {
                const glm::vec4 color(0.66f, 0.80f, 0.94f, drop.opacity);
                rainVertices_[i++] = {drop.position, color};
                rainVertices_[i++] = {drop.position - glm::normalize(drop.velocity) * drop.length, color};
            }
            rainBatch_.upload(rainVertices_.data(), rainVertices_.size());
            rainRevision_ = weather.rain.revision();
        }
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glLineWidth(1.0f);
        rainBatch_.draw();
        rainRendered_ = rainBatch_.vertexCount() / 2;
        lines_ += rainRendered_;
        ++drawCalls_;
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    if (weather.splashes.activeCount()) {
        effectsShader_.use();
        glUniform1f(effectsFogScale_, 1.0f);
        glUniform1f(effectsIntensity_, 1.0f);
        glUniform1f(effectsSoftEdge_, 0.0f);
        glUniform1f(effectsDistanceFade_, 0.0f);
        const auto vp = projection * view;
        glUniformMatrix4fv(effectsViewProjection_, 1, GL_FALSE, glm::value_ptr(vp));
        if (splashRevision_ != weather.splashes.revision()) {
            std::size_t count = 0;
            for (const auto& p : weather.splashes.particles()) {
                if (p.lifetime <= 0) continue;
                const glm::vec4 color(0.65f, 0.83f, 0.95f, 0.85f * p.lifetime / p.initialLifetime);
                const auto tail = p.position - glm::normalize(p.velocity) * p.length;
                splashVertices_[count++] = {p.position, color};
                splashVertices_[count++] = {{tail.x, std::max(tail.y, 0.015f), tail.z}, color};
            }
            splashBatch_.upload(splashVertices_.data(), count);
            splashRevision_ = weather.splashes.revision();
        }
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glLineWidth(1.0f);
        splashBatch_.draw();
        splashRendered_ = splashBatch_.vertexCount() / 2;
        lines_ += splashRendered_;
        ++drawCalls_;
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    if (weather.ripples.activeCount()) {
        effectsShader_.use();
        glUniform1f(effectsFogScale_, 1.0f);
        glUniform1f(effectsIntensity_, 1.0f);
        glUniform1f(effectsSoftEdge_, 0.0f);
        glUniform1f(effectsDistanceFade_, 0.0f);
        const auto vp = projection * view;
        glUniformMatrix4fv(effectsViewProjection_, 1, GL_FALSE, glm::value_ptr(vp));
        if (rippleRevision_ != weather.ripples.revision()) {
            std::size_t count = 0;
            for (const auto& p : weather.ripples.particles()) {
                if (p.lifetime <= 0) continue;
                const bool water = surface::waterMargin(p.center) > 0;
                const glm::vec4 color = water ? glm::vec4(0.56f,0.77f,0.86f,std::min(0.72f,p.alpha*1.4f)) :
                                               glm::vec4(0.46f,0.66f,0.78f,p.alpha);
                for (std::size_t i = 0; i < ring_.size(); ++i) {
                    for (std::size_t j : {i, (i + 1) % ring_.size()}) {
                        const auto v = ring_[j] * p.radius;
                        rippleVertices_[count++] = {p.center + glm::vec3(v.x, 0, v.y),color};
                    }
                }
            }
            rippleBatch_.upload(rippleVertices_.data(), count);
            rippleRevision_ = weather.ripples.revision();
        }
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        rippleBatch_.draw();
        lines_ += rippleBatch_.vertexCount() / 2;
        ++drawCalls_;
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    if (weather.lightning.active() && weather.lightning.flashIntensity() > 0) {
        const bool changed = lightningRevision_ != weather.lightning.revision() ||
            glm::any(glm::notEqual(lightningCameraPosition_, camera.position())) ||
            glm::any(glm::notEqual(lightningCameraForward_, camera.forward()));
        if (changed) {
            std::size_t count = 0;
            for (std::size_t i = 0; i < weather.lightning.segmentCount(); ++i) {
                const auto& segment = weather.lightning.segments()[i];
                glm::vec3 side = glm::cross(segment.end - segment.start,
                    camera.position() - (segment.start + segment.end) * 0.5f);
                if (glm::dot(side, side) < 0.000001f)
                    side = glm::cross(camera.forward(), glm::vec3(0, 1, 0));
                side = glm::normalize(side);
                for (int layer = 0; layer < 2; ++layer) {
                    const glm::vec3 offset = side * segment.width * (layer == 0 ? 1.35f : 0.5f);
                    const glm::vec4 color = layer == 0
                        ? glm::vec4(0.35f, 0.58f, 1.0f, 0.28f)
                        : glm::vec4(0.96f, 0.98f, 1.0f, 0.98f);
                    const auto a = segment.start - offset, b = segment.start + offset;
                    const auto c = segment.end + offset, d = segment.end - offset;
                    lightningVertices_[count++] = {a, color, -1};
                    lightningVertices_[count++] = {b, color, 1};
                    lightningVertices_[count++] = {c, color, 1};
                    lightningVertices_[count++] = {a, color, -1};
                    lightningVertices_[count++] = {c, color, 1};
                    lightningVertices_[count++] = {d, color, -1};
                }
            }
            lightningBatch_.upload(lightningVertices_.data(), count);
            lightningRevision_ = weather.lightning.revision();
            lightningCameraPosition_ = camera.position();
            lightningCameraForward_ = camera.forward();
        }
        effectsShader_.use();
        const auto vp = projection * view;
        glUniform1f(effectsFogScale_, 0.2f);
        glUniform1f(effectsDistanceFade_, 0.0f);
        glUniform1f(effectsIntensity_, weather.lightning.flashIntensity());
        glUniform1f(effectsSoftEdge_, 1.0f);
        glUniform3fv(effectsCameraPosition_, 1, glm::value_ptr(camera.position()));
        glUniformMatrix4fv(effectsViewProjection_, 1, GL_FALSE, glm::value_ptr(vp));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        lightningBatch_.draw();
        triangles_ += lightningBatch_.vertexCount() / 3;
        ++drawCalls_;
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}
