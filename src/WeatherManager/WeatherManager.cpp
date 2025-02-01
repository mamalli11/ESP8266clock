#include "WeatherManager.h"
#include <ESP8266WiFi.h>

const String WeatherManager::apiUrl = "http://api.openweathermap.org/data/2.5/weather";
const String WeatherManager::apiKey = "YOUR_API_KEY"; // کلید API خود را اینجا قرار دهید

WeatherManager::WeatherManager(const String &city, const String &country)
    : city(city), country(country), description(""), temperature(0), humidity(0), weatherIcon("") {}

void WeatherManager::begin()
{
}

bool WeatherManager::update()
{
    HTTPClient http;
    WiFiClient client;

    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    String url = apiUrl + "?q=" + city + "," + country + "&units=metric&appid=" + apiKey;

    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();
        return parseWeatherData(payload);
    }

    return false;
}

bool WeatherManager::parseWeatherData(const String &jsonResponse)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error)
    {
        return false;
    }

    // استخراج اطلاعات آب‌وهوا
    description = doc["weather"][0]["description"].as<String>();
    temperature = doc["main"]["temp"].as<float>();
    humidity = doc["main"]["humidity"].as<float>();

    // تعیین آیکن بر اساس شرایط آب‌وهوایی
    weatherIcon = mapWeatherToIcon(doc["weather"][0]["main"].as<String>());

    return true;
}

String WeatherManager::getDescription() const
{
    return description;
}

float WeatherManager::getTemperature() const
{
    return temperature;
}

float WeatherManager::getHumidity() const
{
    return humidity;
}

String WeatherManager::getWeatherIcon() const
{
    return weatherIcon;
}

String WeatherManager::mapWeatherToIcon(const String &weatherDescription) const
{
    if (weatherDescription == "Clear")
    {
        return "☀️"; // آفتابی
    }
    else if (weatherDescription == "Clouds")
    {
        return "☁️"; // ابری
    }
    else if (weatherDescription == "Rain")
    {
        return "🌧️"; // بارانی
    }
    else if (weatherDescription == "Snow")
    {
        return "❄️"; // برفی
    }
    else if (weatherDescription == "Thunderstorm")
    {
        return "⛈️"; // رعد و برق
    }
    else if (weatherDescription == "Drizzle")
    {
        return "🌦️"; // نم نم باران
    }
    else if (weatherDescription == "Mist" || weatherDescription == "Fog")
    {
        return "🌫️"; // مه
    }
    else
    {
        return "❓"; // نامشخص
    }
}