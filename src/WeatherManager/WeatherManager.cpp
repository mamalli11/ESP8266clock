#include <ESP8266WiFi.h>

#include "WeatherManager.h"
#include "Config.h"
#include "bitmaps.h"

const String WeatherManager::apiUrl = "http://api.openweathermap.org/data/2.5/weather";

WeatherManager::WeatherManager(const String &city, const String &country)
{
    this->city = city.isEmpty() ? readCityFromEEPROM() : city;
    this->country = country.isEmpty() ? readCountryFromEEPROM() : country;

    // مقادیر پیش‌فرض اگر داده‌ای وجود نداشت
    if (this->city.isEmpty() || this->country.isEmpty())
    {
        this->city = "Tehran";
        this->country = "IR";
    }

    description = "";
    temperature = 0;
    humidity = 0;
    windSpeed = 0;
    weatherIcon = "";
    weatherBitmap = unknown_bitmap;
}

String WeatherManager::readCityFromEEPROM()
{
    EEPROM.begin(512);
    char buffer[50] = {0};
    EEPROM.get(EEPROM_CITY_ADDR, buffer);
    EEPROM.end();
    return String(buffer);
}

String WeatherManager::readCountryFromEEPROM()
{
    EEPROM.begin(512);
    char buffer[50] = {0};
    EEPROM.get(EEPROM_COUNTRY_ADDR, buffer);
    EEPROM.end();
    return String(buffer);
}

void WeatherManager::begin()
{
    update();
}

bool WeatherManager::update()
{
    HTTPClient http;
    WiFiClient client;

    Serial.println("🛜  Connecting to OpenWeatherMap API...");
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[Weather] WiFi not connected!");
        return false;
    }

    String url = apiUrl + "?q=" + city + "," + country + "&units=metric&appid=" + WEATHER_API_KEY;

    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();
        Serial.printf("Weather Response: %s\n", payload.c_str());

        return parseWeatherData(payload);
    }

    Serial.printf("[Weather] HTTP Error: %d\n", httpCode);
    http.end();
    return false;
}

bool WeatherManager::parseWeatherData(const String &jsonResponse)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error)
    {
        Serial.println("[Weather] JSON Parsing Failed!");
        return false;
    }

    // استخراج داده‌های اصلی
    description = doc["weather"][0]["description"].as<String>();
    temperature = doc["main"]["temp"];
    humidity = doc["main"]["humidity"];
    windSpeed = doc["wind"]["speed"];
    weatherIcon = doc["weather"][0]["icon"].as<String>();

    // نگاشت آیکون به بیت‌مپ
    weatherBitmap = mapIconToBitmap(weatherIcon.c_str());

    return true;
}

void WeatherManager::refreshLocation()
{
    city = readCityFromEEPROM();
    country = readCountryFromEEPROM();

    if (city.isEmpty() || country.isEmpty())
    {
        city = "Tehran";
        country = "IR";
    }
}

// توابع Getter
String WeatherManager::getDescription() const { return description; }
float WeatherManager::getTemperature() const { return temperature; }
float WeatherManager::getHumidity() const { return humidity; }
float WeatherManager::getWindSpeed() const { return windSpeed; }
String WeatherManager::getWeatherIcon() const { return weatherIcon; }
const uint8_t *WeatherManager::getWeatherBitmap() const { return weatherBitmap; }

const uint8_t *WeatherManager::mapIconToBitmap(const char *icon)
{
    // لیست کامل آیکون‌های OpenWeatherMap
    const struct IconMap
    {
        const char *code;
        const uint8_t *bitmap;
    } iconMap[] = {
        {"01d", sun_bitmap},
        {"01n", moon_bitmap},
        {"02d", cloud_sun_bitmap},
        {"02n", cloud_moon_bitmap},
        {"03", cloud_bitmap},       // برای تمام ورژن‌های 03xx
        {"04", cloud_dense_bitmap}, // برای تمام ورژن‌های 04xx
        {"09", rain_bitmap},
        {"10d", drizzle_bitmap},
        {"10n", drizzle_night_bitmap},
        {"11", storm_bitmap},
        {"13", snow_bitmap},
        {"50", fog_bitmap}};

    // جستجوی آیکون
    for (const auto &pair : iconMap)
    {
        if (strncmp(icon, pair.code, strlen(pair.code)) == 0)
        {
            return pair.bitmap;
        }
    }

    return unknown_bitmap;
}