#include <ESP8266WiFi.h>
#include <time.h>
#include "Config.h"
#include "ClockManager.h"

ClockManager::ClockManager(DisplayManager &display, WeatherManager &weather, PrayerTimesManager &prayer)
    : display(display), weather(weather), prayer(prayer), lastUpdateTime(0),
      lastWeatherUpdate(0), lastPrayerUpdate(0) {}

void ClockManager::begin()
{
    // Initialize time synchronization
    syncTime();
}

void ClockManager::update()
{
    time_t now = time(nullptr);

    // Update time every second
    if (now != lastUpdateTime)
    {
        lastUpdateTime = now;
        updateDisplay();
    }
}

void ClockManager::syncTime()
{
    configTime(TIMEZONE_OFFSET, 0, "pool.ntp.org", "time.nist.gov");
    while (time(nullptr) < 100000)
    {
        delay(500);
    }
}

void ClockManager::updateDisplay()
{
    time_t now = time(nullptr);
    struct tm *timeInfo = localtime(&now);

    static uint8_t displayState = 0;
    static time_t lastStateChange = 0;

    int displayDuration = (displayState == 0) ? 30 : 5; // 30s for clock, 5s for others

    if (now - lastStateChange >= displayDuration)
    {
        lastStateChange = now;
        displayState = (displayState + 1) % 3;
    }
    switch (displayState)
    {
    case 0:
        display.showClockPage(timeInfo, prayer.getShamsiDate());
        break;
    case 1:
        display.showWeatherPage(weather.getDescription(), weather.getTemperature(), weather.getHumidity());
        break;
    case 2:
        display.showPrayerTimesPage(prayer.getTimes());
        break;
    }
}
