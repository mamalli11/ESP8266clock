#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

class WeatherManager
{
public:
    WeatherManager(const String &city, const String &country);
    void begin();
    bool update();
    String getDescription() const;
    float getTemperature() const;
    float getHumidity() const;
    String getWeatherIcon() const;

private:
    String city;
    String country;
    String description;
    float temperature;
    float humidity;
    String weatherIcon;

    static const String apiUrl;
    static const String apiKey;

    bool parseWeatherData(const String &jsonResponse);
    String mapWeatherToIcon(const String &weatherDescription) const;
};

#endif