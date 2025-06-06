#include <ESP8266WiFi.h>

#include "PrayerTimesManager.h"
#include "Config.h"

PrayerTimesManager::PrayerTimesManager()
{
    // مقداردهی اولیه
    for (int i = 0; i < 6; i++)
    {
        prayerTimes[i] = "--:--";
    }
    shamsiDate = "1404/01/01";
}

void PrayerTimesManager::begin()
{
    update();
}

bool PrayerTimesManager::update()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure(); // غیرفعال کردن بررسی SSL (موقت)

    HTTPClient http;
    http.begin(client, PRAYER_TIMES_API_URL);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        Serial.println("✅ PrayerTimesManager updated successfully!");
        String payload = http.getString();
        return parsePrayerTimes(payload);
    }

    return false;
}

bool PrayerTimesManager::parsePrayerTimes(const String &jsonResponse)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error)
    {
        Serial.println("❌ JSON parsing failed!");
        return false;
    }

    // استخراج زمان‌های نماز از JSON دریافتی
    prayerTimes[0] = doc["Imsaak"].as<String>().substring(0, 5);   // اذان صبح
    prayerTimes[1] = doc["Sunrise"].as<String>().substring(0, 5);  // طلوع آفتاب
    prayerTimes[2] = doc["Noon"].as<String>().substring(0, 5);     // ظهر
    prayerTimes[3] = doc["Sunset"].as<String>().substring(0, 5);   // غروب
    prayerTimes[4] = doc["Maghreb"].as<String>().substring(0, 5);  // مغرب
    prayerTimes[5] = doc["Midnight"].as<String>().substring(0, 5); // نیمه‌شب شرعی

    // استخراج تاریخ شمسی
    String todayStr = doc["Today"].as<String>();
    int dashIndex = todayStr.indexOf(" - "); // موقعیت اولین " - " در رشته

    if (dashIndex != -1)
    {
        shamsiDate = todayStr.substring(0, dashIndex); // استخراج فقط بخش تاریخ
    }
    else
    {
        shamsiDate = todayStr; // اگر "-" نبود، کل مقدار را در نظر بگیر
    }

    return true;
}

const String *PrayerTimesManager::getTimes() const
{
    return prayerTimes;
}

String PrayerTimesManager::getShamsiDate() const
{
    return shamsiDate.substring(0, 10);
}