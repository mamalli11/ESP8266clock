#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <U8g2lib.h>
#include <time.h>
#include "../WeatherManager/WeatherManager.h"

class DisplayManager
{
public:
    DisplayManager();
    void initialize();
    void showClockPage(const tm *timeInfo, const String &shamsiDate);
    void showWeatherPage(const WeatherManager &weather);
    void showPrayerTimesPage(const String times[6]);
    void showMessage(const String &message, int duration = 2000);
    void showErrorMessage(const String &error);
    void clearDisplay();
    void adjustBrightness(int sensorValue);

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

    void drawTime(const tm *timeInfo, bool is24Hour);
    void drawDate(const String &date);
    void drawWeather(const WeatherManager &weather);
    void drawPrayerTimes(const String times[6]);
    bool readIs24HourFromEEPROM();
};

#endif
