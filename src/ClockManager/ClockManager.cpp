#include <ESP8266WiFi.h>
#include <time.h>
#include "config.h"
#include "ClockManager.h"

ClockManager::ClockManager(DisplayManager &display, AlarmManager &alarm,
                           WeatherManager &weather, PrayerTimesManager &prayer,
                           LEDController &leds)
    : display(display), alarm(alarm), weather(weather),
      prayer(prayer), leds(leds), lastUpdateTime(0),
      lastWeatherUpdate(0), lastPrayerUpdate(0) {}

void ClockManager::begin()
{
    // Initialize time synchronization
    syncTime();

    // Initialize other components
    display.initialize();
    alarm.begin();
    weather.begin();
    prayer.begin();
    leds.initialize();
}

void ClockManager::update()
{
    time_t now = time(nullptr);

    // Update time every second
    if (now != lastUpdateTime)
    {
        lastUpdateTime = now;
        updateDisplay();
        updateAlarms();
        updateLEDs();
    }

    // Update weather every hour
    if (now - lastWeatherUpdate >= 3600)
    {
        lastWeatherUpdate = now;
        updateWeather();
    }

    // Update prayer times every 6 hours
    if (now - lastPrayerUpdate >= 21600)
    {
        lastPrayerUpdate = now;
        updatePrayerTimes();
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

    // Display clock page
    display.showClockPage(timeInfo, prayer.getShamsiDate());

    // Rotate between clock, weather, and prayer times
    static uint8_t displayState = 0;
    static time_t lastStateChange = 0;

    if (now - lastStateChange >= 10)
    { // Change state every 10 seconds
        lastStateChange = now;
        displayState = (displayState + 1) % 3;

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
}

void ClockManager::updateAlarms()
{
    time_t now = time(nullptr);
    struct tm *timeInfo = localtime(&now);
    alarm.checkAlarms(timeInfo);
}

void ClockManager::updateWeather()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        weather.update();
    }
}

void ClockManager::updatePrayerTimes()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        prayer.update();
    }
}

void ClockManager::updateLEDs()
{
    time_t now = time(nullptr);
    struct tm *timeInfo = localtime(&now);
    leds.update(timeInfo->tm_wday);
}