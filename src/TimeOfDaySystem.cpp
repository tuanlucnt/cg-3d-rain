#include "TimeOfDaySystem.hpp"
#include <cmath>
#include <cstdio>

void TimeOfDaySystem::reset() { hour_=DEFAULT_HOUR; dayDuration_=DEFAULT_DAY_SECONDS; autoAdvance_=false; }
bool TimeOfDaySystem::setHour(double hour) {
    if (!std::isfinite(hour)) return false;
    hour_=std::fmod(hour,24.0); if (hour_<0) hour_+=24;
    if (hour_>=24) hour_=0; // Rounding of a tiny negative hour can produce 24.
    return true;
}
bool TimeOfDaySystem::setDayDuration(double seconds) {
    if (!std::isfinite(seconds) || seconds<60 || seconds>3600) return false;
    dayDuration_=seconds; return true;
}
void TimeOfDaySystem::update(double seconds,bool paused) {
    if (!autoAdvance_ || paused || !std::isfinite(seconds) || seconds<=0) return;
    // Modulo first also bounds huge finite dt without multiplication overflow.
    setHour(hour_+std::fmod(seconds,dayDuration_)*24/dayDuration_);
}
std::string TimeOfDaySystem::label() const {
    const unsigned minute=static_cast<unsigned>(hour_*60)%1440;
    char label[6]{};
    std::snprintf(label,sizeof(label),"%02u:%02u",minute/60,minute%60);
    return label;
}
