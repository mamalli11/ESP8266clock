#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <U8g2lib.h>
#include <time.h>

class DisplayManager
{
public:
    DisplayManager();
    void initialize();
    void showClockPage(const tm *timeInfo, const String &shamsiDate);
    void showWeatherPage(const String &description, float temp, float humidity);
    void showPrayerTimesPage(const String times[6]);
    void adjustBrightness(int sensorValue);
    void drawWeatherIcon(const String &icon);

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

    void drawTime(const tm *timeInfo, bool is24Hour);
    void drawDate(const String &date);
    void drawWeather(const String &description, float temp, float humidity);
    void drawPrayerTimes(const String times[6]);
};

#endif