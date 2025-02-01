#include "DisplayManager.h"
#include <U8g2lib.h>

DisplayManager::DisplayManager() : u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE) {}

void DisplayManager::initialize()
{
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "Initializing...");
    u8g2.sendBuffer();
}

void DisplayManager::showClockPage(const tm *timeInfo, const String &shamsiDate)
{
    u8g2.clearBuffer();

    // Draw time
    drawTime(timeInfo, true); // true for 24-hour format

    // Draw date
    drawDate(shamsiDate);

    u8g2.sendBuffer();
}

void DisplayManager::showWeatherPage(const String &description, float temp, float humidity)
{
    u8g2.clearBuffer();

    // Draw weather information
    drawWeather(description, temp, humidity);

    u8g2.sendBuffer();
}

void DisplayManager::showPrayerTimesPage(const String times[6])
{
    u8g2.clearBuffer();

    // Draw prayer times
    drawPrayerTimes(times);

    u8g2.sendBuffer();
}

void DisplayManager::adjustBrightness(int sensorValue)
{
    int brightness = map(sensorValue, 0, 1023, 0, 255);
    u8g2.setContrast(brightness);
}

void DisplayManager::drawTime(const tm *timeInfo, bool is24Hour)
{
    char timeStr[9];
    if (is24Hour)
    {
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeInfo);
    }
    else
    {
        strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", timeInfo);
    }

    u8g2.setFont(u8g2_font_logisoso24_tn);
    u8g2.drawStr(10, 40, timeStr);
}

void DisplayManager::drawDate(const String &date)
{
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(15, 60, date.c_str());
}

void DisplayManager::drawWeather(const String &description, float temp, float humidity)
{
    u8g2.setFont(u8g2_font_6x10_tr);

    // Draw weather description
    u8g2.drawStr(5, 10, ("Weather: " + description).c_str());

    // Draw temperature
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "Temp: %.1f C", temp);
    u8g2.drawStr(5, 25, tempStr);

    // Draw humidity
    char humStr[16];
    snprintf(humStr, sizeof(humStr), "Hum: %.1f%%", humidity);
    u8g2.drawStr(5, 40, humStr);
}

void DisplayManager::drawPrayerTimes(const String times[6])
{
    u8g2.setFont(u8g2_font_6x10_tr);

    // Draw prayer times
    u8g2.drawStr(5, 10, ("Fajr: " + times[0]).c_str());
    u8g2.drawStr(5, 20, ("Sunrise: " + times[1]).c_str());
    u8g2.drawStr(5, 30, ("Dhuhr: " + times[2]).c_str());
    u8g2.drawStr(5, 40, ("Asr: " + times[3]).c_str());
    u8g2.drawStr(5, 50, ("Maghrib: " + times[4]).c_str());
    u8g2.drawStr(5, 60, ("Isha: " + times[5]).c_str());
}

void DisplayManager::drawWeatherIcon(const String &icon)
{
    u8g2.setFont(u8g2_font_unifont_t_weather);
    if (icon == "☀️")
    {
        u8g2.drawGlyph(0, 20, 0x2600);
    }
    else if (icon == "☁️")
    {
        u8g2.drawGlyph(0, 20, 0x2601);
    }
    else if (icon == "🌧️")
    {
        u8g2.drawGlyph(0, 20, 0x2614);
    }
    else if (icon == "❄️")
    {
        u8g2.drawGlyph(0, 20, 0x2744);
    }
    else if (icon == "⛈️")
    {
        u8g2.drawGlyph(0, 20, 0x26C8);
    }
    else
    {
        u8g2.drawGlyph(0, 20, 0x2753);
    }
}