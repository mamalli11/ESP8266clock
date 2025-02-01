#include <ESP8266WiFi.h>

#include "PrayerTimesManager.h"
#include "config.h"

const String PrayerTimesManager::apiUrl = "https://prayer.aviny.com/api/prayertimes/311";

PrayerTimesManager::PrayerTimesManager()
{
    // مقداردهی اولیه
    for (int i = 0; i < 6; i++)
    {
        prayerTimes[i] = "--:--";
    }
    shamsiDate = "1403/01/01";
}

void PrayerTimesManager::begin()
{
    // مقداردهی اولیه (اگر نیاز است)
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
    http.begin(client, apiUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
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
        return false;
    }

    // فرض کنید ساختار JSON به این شکل است:
    // {"times": ["05:30", "06:45", "12:30", "16:15", "18:45", "20:00"], "date": "1403/07/15"}
    JsonArray times = doc["times"];
    for (int i = 0; i < 6; i++)
    {
        prayerTimes[i] = times[i].as<String>();
    }

    shamsiDate = doc["date"].as<String>();
    return true;
}

const String *PrayerTimesManager::getTimes() const
{
    return prayerTimes;
}

String PrayerTimesManager::getShamsiDate() const
{
    return shamsiDate;
}