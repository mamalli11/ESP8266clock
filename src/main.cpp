#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "Config.h"

#include "./WebServerManager/WebServerManager.h"
#include "./AlarmManager/AlarmManager.h"
#include "./ClockManager/ClockManager.h"
#include "./LEDController/LEDController.h"
#include "./NetworkManager/NetworkManager.h"
#include "./DisplayManager/DisplayManager.h"
#include "./WeatherManager/WeatherManager.h"
#include "./PrayerTimesManager/PrayerTimesManager.h"

// تعریف اشیاء کامپوننت‌ها
DisplayManager display;
AlarmManager alarm;
WeatherManager weather("Tehran", "IR");
PrayerTimesManager prayer;
LEDController leds;
WebServerManager webServer;

void setup()
{
  Serial.begin(9600);

  // راه‌اندازی WiFi
  WiFi.begin(DEFAULT_SSID, DEFAULT_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // راه‌اندازی کامپوننت‌ها
  display.initialize();
  alarm.begin();
  weather.begin();
  prayer.begin();
  leds.initialize();
  webServer.begin();

  // تنظیمات اولیه وب سرور
  webServer.setWiFiCredentials(DEFAULT_SSID, DEFAULT_PASSWORD);
  webServer.setTimeSettings("Tehran", "IR", true);
  webServer.setAlarmSettings("[{\"hour\":8,\"minute\":30,\"active\":true}]");
  webServer.setDisplaySettings(128, "#0000FF");

  // همگام‌سازی زمان با سرور NTP
  configTime(TIMEZONE_OFFSET, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000)
  {
    delay(500);
  }
}

void loop()
{
  // به‌روزرسانی زمان
  time_t now = time(nullptr);
  struct tm *timeInfo = localtime(&now);

  // به‌روزرسانی کامپوننت‌ها
  display.showClockPage(timeInfo, prayer.getShamsiDate());
  alarm.checkAlarms(timeInfo);
  leds.update(timeInfo->tm_wday);

  // به‌روزرسانی آب‌وهوا هر ساعت
  static unsigned long lastWeatherUpdate = 0;
  if (millis() - lastWeatherUpdate > 3600000)
  { // 1 ساعت
    weather.update();
    lastWeatherUpdate = millis();
  }

  // به‌روزرسانی اوقات شرعی هر 6 ساعت
  static unsigned long lastPrayerUpdate = 0;
  if (millis() - lastPrayerUpdate > 21600000)
  { // 6 ساعت
    prayer.update();
    lastPrayerUpdate = millis();
  }

  // مدیریت درخواست‌های وب سرور
  webServer.handleClient();

  delay(100); // جلوگیری از اشغال بیش از حد CPU
}