#pragma once
#include <string>

class TimeOfDaySystem {
public:
    static constexpr double DEFAULT_HOUR=18.0;
    static constexpr double DEFAULT_DAY_SECONDS=600.0;
    void reset();
    bool setHour(double hour);
    bool setDayDuration(double seconds);
    void shiftHours(double hours) { setHour(hour_+hours); }
    void toggleDayNight() { setHour(hour_>=6 && hour_<18 ? 0 : 12); }
    void setAutoAdvance(bool enabled) { autoAdvance_=enabled; }
    void update(double seconds,bool paused=false);
    double hour() const { return hour_; }
    double dayDuration() const { return dayDuration_; }
    bool autoAdvance() const { return autoAdvance_; }
    std::string label() const;
private:
    double hour_=DEFAULT_HOUR,dayDuration_=DEFAULT_DAY_SECONDS;
    bool autoAdvance_=false;
};
