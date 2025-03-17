#include "DisplayManager.h"
#include <U8g2lib.h>

DisplayManager::DisplayManager() : u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE) {}

void DisplayManager::initialize()
{
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    showMessage("✅ Initializing...", 2000);
}

void DisplayManager::showClockPage(const tm *timeInfo, const String &shamsiDate)
{
    u8g2.clearBuffer();
    drawTime(timeInfo, true); // نمایش ۲۴ ساعته
    drawDate(shamsiDate);
    u8g2.sendBuffer();
}

void DisplayManager::showWeatherPage(const WeatherManager &weather)
{
    u8g2.clearBuffer();
    drawWeather(weather);
    u8g2.sendBuffer();
}

void DisplayManager::showPrayerTimesPage(const String times[6])
{
    u8g2.clearBuffer();
    drawPrayerTimes(times);
    u8g2.sendBuffer();
}

void DisplayManager::showMessage(const String &message, int duration)
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);

    int y = 20;
    int lineHeight = 10;

    size_t start = 0;
    while (start < message.length()) // اصلاح شده
    {
        size_t end = message.indexOf('\n', start);
        if (end == -1)
            end = message.length();

        u8g2.drawStr(5, y, message.substring(start, end).c_str());
        y += lineHeight;
        start = end + 1;
    }

    u8g2.sendBuffer();
    delay(duration);
}

void DisplayManager::showErrorMessage(const String &error)
{
    showMessage("❌ ERROR:\n" + error, 3000);
}

void DisplayManager::clearDisplay()
{
    u8g2.clearBuffer();
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

void DisplayManager::drawWeather(const WeatherManager &weather)
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);

    // نمایش توضیحات وضعیت آب‌وهوا
    u8g2.drawStr(5, 10, ("Weather: " + weather.getDescription()).c_str());

    // نمایش دما
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "Temp: %.1f C", weather.getTemperature());
    u8g2.drawStr(5, 25, tempStr);

    // نمایش رطوبت
    char humStr[16];
    snprintf(humStr, sizeof(humStr), "Hum: %.1f%%", weather.getHumidity());
    u8g2.drawStr(5, 40, humStr);

    // نمایش سرعت باد
    char windStr[16];
    snprintf(windStr, sizeof(windStr), "Wind: %.1f m/s", weather.getWindSpeed());
    u8g2.drawStr(5, 55, windStr);

    // نمایش آیکون آب‌وهوا
    if (weather.getWeatherBitmap())
    {
        // u8g2.drawXBMP(90, 10, 32, 32, weather.getWeatherBitmap());
        u8g2.drawXBMP(90, 20, 25, 25, weather.getWeatherBitmap());
    }

    u8g2.sendBuffer();
}

void DisplayManager::drawPrayerTimes(const String times[6])
{
    u8g2.setFont(u8g2_font_6x10_tr);

    u8g2.drawStr(5, 10, ("Fajr: " + times[0]).c_str());
    u8g2.drawStr(5, 20, ("Sunrise: " + times[1]).c_str());
    u8g2.drawStr(5, 30, ("Dhuhr: " + times[2]).c_str());
    u8g2.drawStr(5, 40, ("Asr: " + times[3]).c_str());
    u8g2.drawStr(5, 50, ("Maghrib: " + times[4]).c_str());
    u8g2.drawStr(5, 60, ("Isha: " + times[5]).c_str());
}
