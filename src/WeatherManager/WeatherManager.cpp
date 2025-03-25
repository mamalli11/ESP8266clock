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
        {"01d", sun_bitmap},            // آفتابی روز
        {"01n", moon_bitmap},           // آفتابی شب
        {"02d", cloud_sun_bitmap},      // نیمه ابری روز
        {"02n", cloud_moon_bitmap},     // نیمه ابری شب
        {"03d", cloud_d_bitmap},        // ابری پراکنده روز
        {"03n", cloud_n_bitmap},        // ابری پراکنده شب
        {"04d", cloud_dense_d_bitmap},  // ابری سنگین روز
        {"04n", cloud_dense_n_bitmap},  // ابری سنگین شب
        {"09d", rain_d_bitmap},         // بارش پراکنده روز
        {"09n", rain_n_bitmap},         // بارش پراکنده شب
        {"10d", drizzle_d_bitmap},      // باران روز
        {"10n", drizzle_n_bitmap},      // باران شب
        {"11d", thunderstorm_d_bitmap}, // رعد و برق روز
        {"11n", thunderstorm_n_bitmap}, // رعد و برق شب
        {"13d", snow_d_bitmap},         // برف روز
        {"13n", snow_n_bitmap},         // برف شب
        {"50d", fog_d_bitmap},          // مه روز
        {"50n", fog_n_bitmap}           // مه شب
    };

    // جستجوی آیکون
    for (const auto &pair : iconMap)
    {
        if (strncmp(icon, pair.code, strlen(pair.code)) == 0)
        {
            return pair.bitmap;
        }
    }

    return unknown_bitmap; // آیکون نامشخص
}