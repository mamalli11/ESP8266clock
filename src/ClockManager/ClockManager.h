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
    ClockManager(DisplayManager &display, AlarmManager &alarm, WeatherManager &weather, PrayerTimesManager &prayer, LEDController &leds);
    void begin();
    void update();

private:
    DisplayManager &display;
    AlarmManager &alarm;
    WeatherManager &weather;
    PrayerTimesManager &prayer;
    LEDController &leds;

    time_t lastUpdateTime;
    time_t lastWeatherUpdate;
    time_t lastPrayerUpdate;

    void syncTime();
    void updateDisplay();
    void updateAlarms();
    void updateWeather();
    void updatePrayerTimes();
    void updateLEDs();
};

#endif