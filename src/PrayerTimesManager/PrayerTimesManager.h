#ifndef PRAYERTIMESMANAGER_H
#define PRAYERTIMESMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

class PrayerTimesManager
{
public:
    PrayerTimesManager();
    void begin();
    bool update();
    const String *getTimes() const;
    String getShamsiDate() const;

private:
    static const String apiUrl;
    String prayerTimes[6]; // Fajr, Sunrise, Dhuhr, Asr, Maghrib, Isha
    String shamsiDate;

    bool parsePrayerTimes(const String &jsonResponse);
};

#endif