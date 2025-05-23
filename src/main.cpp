#include <time.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>

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
NetworkManager networkManager(display);
WebServerManager webServer(alarm, networkManager);
WeatherManager weather;
PrayerTimesManager prayer;
LEDController leds;
ClockManager clockManager(display, weather, prayer);

void clearEEPROM()
{
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
  Serial.println("EEPROM cleared!");
}

void setup()
{
  Serial.begin(9600);
  // clearEEPROM();
  display.initialize();
  // EEPROM.begin(EEPROM_SIZE);

  pinMode(BUZZER_PIN, OUTPUT);

  // بررسی مقداردهی اولیه EEPROM
  if (networkManager.getSSID().isEmpty())
  {
    Serial.println("⚠️ EEPROM is empty! Setting default values...");
    networkManager.saveWiFiCredentials(DEFAULT_SSID, DEFAULT_PASSWORD);
  }

  // راه‌اندازی مدیریت شبکه
  networkManager.begin();
  webServer.begin();

  // راه‌اندازی سایر کامپوننت‌ها
  alarm.begin();
  leds.initialize();
  prayer.begin();
  weather.begin();
  clockManager.begin();

  tone(BUZZER_PIN, 1000);
  delay(200);
  tone(BUZZER_PIN, 1200);
  delay(200);
  tone(BUZZER_PIN, 800);
  delay(200);
  noTone(BUZZER_PIN);
  delay(100);
}

void loop()
{
  // به‌روزرسانی زمان
  time_t now = time(nullptr);
  struct tm *timeInfo = localtime(&now);

  // به‌روزرسانی کامپوننت‌ها
  alarm.checkAlarms(timeInfo);
  leds.update(timeInfo->tm_wday);
  clockManager.update();

  // مدیریت ارتباط شبکه
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 10000)
  {
    networkManager.checkWiFiConnection();
    lastWiFiCheck = millis();
  }

  // به‌روزرسانی آب‌وهوا هر ساعت (فقط اگر متصل باشیم)
  static unsigned long lastWeatherUpdate = 0;
  if (millis() - lastWeatherUpdate > 3600000 && networkManager.isConnected())
  {
    weather.update();
    lastWeatherUpdate = millis();
  }

  // به‌روزرسانی اوقات شرعی هر 6 ساعت (فقط اگر متصل باشیم)
  static unsigned long lastPrayerUpdate = 0;
  if (millis() - lastPrayerUpdate > 21600000 && networkManager.isConnected())
  {
    prayer.update();
    lastPrayerUpdate = millis();
  }

  // مدیریت درخواست‌های وب سرور
  webServer.handleClient();

  delay(100); // جلوگیری از اشغال بیش از حد CPU
}