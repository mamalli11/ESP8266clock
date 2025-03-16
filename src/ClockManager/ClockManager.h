#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#include <time.h>
#include "../AlarmManager/AlarmManager.h"
#include "../LEDController/LEDController.h"
#include "../WeatherManager/WeatherManager.h"
#include "../DisplayManager/DisplayManager.h"
#include "../PrayerTimesManager/PrayerTimesManager.h"

class ClockManager
{
public:
    ClockManager(DisplayManager &display, WeatherManager &weather, PrayerTimesManager &prayer);
    void begin();
    void update();

private:
    DisplayManager &display;
    WeatherManager &weather;
    PrayerTimesManager &prayer;

    time_t lastUpdateTime;
    time_t lastWeatherUpdate;
    time_t lastPrayerUpdate;

    void syncTime();
    void updateDisplay();
    void updateWeather();
    void updatePrayerTimes();
};

#endif