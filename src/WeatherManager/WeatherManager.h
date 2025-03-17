#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

class WeatherManager
{
public:
    WeatherManager(const String &city = "", const String &country = "");
    void begin();
    bool update();
    void refreshLocation();

    // Getters
    String getDescription() const;
    float getTemperature() const;
    float getHumidity() const;
    float getWindSpeed() const;
    String getWeatherIcon() const;
    const uint8_t *getWeatherBitmap() const;

private:
    String city;
    String country;
    String description;
    float temperature;
    float humidity;
    float windSpeed;
    String weatherIcon;
    const uint8_t *weatherBitmap;

    String readCityFromEEPROM();
    String readCountryFromEEPROM();
    bool parseWeatherData(const String &jsonResponse);
    const uint8_t *mapIconToBitmap(const char *icon);

    static const String apiUrl;
    static constexpr int EEPROM_CITY_ADDR = 100;
    static constexpr int EEPROM_COUNTRY_ADDR = 150;
};

#endif