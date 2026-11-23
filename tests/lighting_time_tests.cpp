#include "LightingSystem.hpp"
#include "WeatherSystem.hpp"
#include <limits>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void lightingTests() {
    LightingSystem light;
    light.update(0.6f,0.1f,true);
    const float wet=light.wetness();
    for (float hour:{0.0f,5.0f,6.0f,7.0f,12.0f,17.0f,18.0f,19.0f,23.99f}) {
        light.setEnvironment(hour,0.9f);
        require(light.wetness()==wet && light.flash()==0.6f,"Changing hour must not reset wetness or flash");
        require(std::abs(glm::length(light.lightDirection())-1)<0.0001f,"Directional light must remain normalized");
        const auto a=light.ambientColor(),s=light.skyTop();
        require(std::isfinite(glm::length(a)) && glm::all(glm::greaterThan(a,glm::vec3(0))) &&
                glm::all(glm::lessThan(s,glm::vec3(1))),"Lighting colors must stay finite and bounded");
        LightingSystem adjacent;
        adjacent.setEnvironment(hour+0.001f,0.9f);
        require(glm::distance(a,adjacent.ambientColor())<0.002f,"Ambient must interpolate continuously at phase boundaries");
    }
    LightingSystem day,night,clear;
    day.setEnvironment(12,0.9f); night.setEnvironment(0,0.9f); clear.setEnvironment(12,0);
    require(glm::length(day.ambientColor())>glm::length(night.ambientColor()) &&
            glm::length(day.directionalColor())>glm::length(night.directionalColor()),"Day must be brighter than night");
    require(night.windowStrength()>0.99f && day.windowStrength()<0.01f,"Windows must fade out by day");
    require(glm::length(day.skyTop())<glm::length(clear.skyTop()),"Storm must darken the daytime sky");
    require(day.lightDirection().y< -0.8f,"Noon light must come from above");
    for (auto* l:{&day,&night}) {
        const auto before=l->skyTop(); l->update(1);
        require(glm::length(l->skyTop()-before)>0.2f,"Flash must contribute during day and night");
        l->update(0); require(glm::distance(l->skyTop(),before)<0.00001f,"Flash must not permanently alter sky lighting");
    }
    std::cout<<"PASS colored lighting: transitions, direction, windows, storm, transient flash and wetness preservation\n";
}
void clockTests() {
    TimeOfDaySystem time;
    require(time.hour()==18 && !time.autoAdvance() && time.dayDuration()==600,"Clock defaults must preserve a fixed storm evening");
    time.setHour(24); require(time.hour()==0 && time.label()=="00:00","24:00 must wrap to midnight");
    time.shiftHours(-1); require(time.hour()==23 && time.label()=="23:00","Negative hour shift must wrap");
    time.setHour(-std::numeric_limits<double>::denorm_min()); require(time.hour()<24,"Tiny negative input must stay in range");
    time.setHour(6.5); require(time.label()=="06:30","Clock label must show fractional hours as minutes");
    require(!time.setHour(std::numeric_limits<double>::infinity()) && time.hour()==6.5,"Reject invalid hours");
    require(!time.setDayDuration(0) && !time.setDayDuration(4000) && !time.setDayDuration(std::numeric_limits<double>::quiet_NaN()),"Day duration must remain bounded");
    TimeOfDaySystem one,many;
    one.setHour(23.8); many.setHour(23.8); one.setAutoAdvance(true); many.setAutoAdvance(true);
    one.update(10); for (int i=0;i<1000;++i) many.update(0.01);
    require(std::abs(one.hour()-many.hour())<1e-9 && one.hour()<1,"Auto time must be FPS independent and wrap continuously");
    const auto frozen=one.hour(); one.update(10,true); one.update(-1); one.update(std::numeric_limits<double>::quiet_NaN());
    require(one.hour()==frozen,"Pause and invalid dt must not advance time");
    one.update(1); require(one.hour()>frozen,"Resume must continue time");
    one.toggleDayNight(); require(one.hour()==0 || one.hour()==12,"Quick toggle must choose noon or midnight");
    LightingSystem before,after;
    before.setEnvironment(23.9999f,0.9f); after.setEnvironment(0.0001f,0.9f);
    require(glm::distance(before.skyTop(),after.skyTop())<0.0001f &&
            glm::distance(before.lightDirection(),after.lightDirection())<0.0001f,"No lighting jump at midnight");
    WeatherSystem weather;
    for (int i=0;i<10;++i) weather.update(0.1f);
    const auto rain=weather.rain.drops()[0].position;
    const auto splash=weather.splashes.activeCount(), ripple=weather.ripples.activeCount();
    const auto wet=weather.lighting.wetness();
    weather.timeOfDay.setHour(0); weather.refreshLighting();
    require(weather.rain.drops()[0].position==rain && weather.splashes.activeCount()==splash &&
            weather.ripples.activeCount()==ripple && weather.lighting.wetness()==wet && weather.valid(),"Changing hour must preserve weather and wetness");
    weather.timeOfDay.setAutoAdvance(true); weather.paused=true; weather.update(0.1f);
    require(weather.timeOfDay.hour()==0,"Space pause must also freeze auto time");
    weather.paused=false; weather.update(0.1f); require(weather.timeOfDay.hour()>0,"Weather resume must resume clock");
    weather.reset(); require(weather.timeOfDay.hour()==18 && !weather.timeOfDay.autoAdvance(),"R must restore hour and auto defaults");
    WeatherSystem fast,slow; fast.timeOfDay.setAutoAdvance(true); slow.timeOfDay.setAutoAdvance(true);
    for (int i=0;i<1000;++i) fast.update(0.01f);
    for (int i=0;i<100;++i) slow.update(0.1f);
    require(std::abs(fast.timeOfDay.hour()-slow.timeOfDay.hour())<1e-9,"Integrated clock must use the same fixed timestep as weather");
    std::cout<<"PASS time: wrap, bounds, FPS independence, pause/resume, midnight continuity, weather preservation and reset\n";
}
}
int main() {
    try { lightingTests(); clockTests(); return 0; }
    catch (const std::exception& e) { std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1; }
}
