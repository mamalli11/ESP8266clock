#include <LittleFS.h>
#include <ArduinoJson.h>

#include "WebServerManager.h"
#include "../AlarmManager/AlarmManager.h"
#include "../NetworkManager/NetworkManager.h"
#include "../DisplayManager/DisplayManager.h"

// WebServerManager::WebServerManager(AlarmManager &alarmMgr) : server(80), alarmManager(alarmMgr) {}
WebServerManager::WebServerManager(AlarmManager &alarmMgr, NetworkManager &netMgr)
    : server(80), alarmManager(alarmMgr), networkManager(netMgr) {}

void WebServerManager::begin()
{
    if (!LittleFS.begin())
    {
        Serial.println("❌ Failed to mount LittleFS. Formatting...");
        LittleFS.format();
        if (!LittleFS.begin())
        {
            Serial.println("❌ LittleFS mount failed after format!");
            return;
        }
    }

    // 🚀 بررسی موجود بودن `index.html`
    if (!LittleFS.exists("/www/index.html"))
    {
        Serial.println("⚠️ index.html not found in LittleFS!");
    }
    else
    {
        Serial.println("✅ index.html found in LittleFS.");
    }

    Serial.println("✅ Starting Web Server...");
    loadSettings();
    initWebRoutes();
    server.begin();
}

void WebServerManager::handleClient()
{
    server.handleClient();
}

void WebServerManager::initWebRoutes()
{
    server.on("/", HTTP_GET, [this]()
              {
        File file = LittleFS.open("/www/index.html", "r");
        if (!file)
        {
            server.send(404, "text/plain", "File Not Found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close(); });

    server.serveStatic("/css", LittleFS, "/www/css");
    server.serveStatic("/js", LittleFS, "/www/js");

    // دریافت تنظیمات ذخیره‌شده
    server.on("/api/get/config", HTTP_GET, [this]()
              {
        String jsonConfig = getConfigAsJson();
        server.send(200, "application/json", jsonConfig); });

    // ذخیره تنظیمات WiFi
    server.on("/api/update/wifi", HTTP_POST, [this]()
              {


        if (!server.hasArg("ssid") || !server.hasArg("password"))
        {
            server.send(400, "text/plain", "Missing parameters");
            return;
        }
        
        Serial.println(server.arg("ssid"));
        Serial.println(server.arg("password"));

        networkManager.saveWiFiCredentials(server.arg("ssid"), server.arg("password"));
        server.send(200, "text/plain", "WiFi Updated. Restarting...");

        delay(1000);
        ESP.restart(); });

    // ذخیره تنظیمات آلارم
    server.on("/api/update/alarms", HTTP_POST, [this]()
              {
        if (!server.hasArg("plain"))
        {
            server.send(400, "text/plain", "Missing parameters");
            return;
        }

        saveAlarmSettings(server.arg("plain"));
        server.send(200, "text/plain", "Alarms updated!"); });

    Serial.println("✅ WebServer initialized.");

    // 🚀 **مسیر صحیح برای ذخیره تنظیمات زمان**
    server.on("/api/update/time", HTTP_POST, [this]()
              {
        if (!server.hasArg("plain"))
        {
            server.send(400, "text/plain", "Missing parameters");
            return;
        }

        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, server.arg("plain"));

        if (error)
        {
            server.send(400, "text/plain", "Invalid JSON");
            return;
        }

        String city = doc["city"].as<String>();
        String country = doc["country"].as<String>();
        bool is24Hour = doc["timeFormat"].as<String>() == "24";

        saveTimeSettings(city, country, is24Hour);
        server.send(200, "text/plain", "Time settings updated!"); });

    server.begin();
}

void WebServerManager::loadSettings()
{
    EEPROM.begin(EEPROM_SIZE);

    char city[50] = {0}, country[50] = {0}, theme[20] = {0};

    EEPROM.get(100, city);
    city[sizeof(city) - 1] = '\0';
    EEPROM.get(150, country);
    country[sizeof(country) - 1] = '\0';
    EEPROM.get(200, is24HourFormat);
    EEPROM.get(250, displayBrightness);
    EEPROM.get(300, theme);
    theme[sizeof(theme) - 1] = '\0';

    currentCity = String(city);
    currentCountry = String(country);
    displayTheme = String(theme);

    alarmManager.loadAlarms();

    EEPROM.end();
    Serial.println("✅ WebServerManager Settings loaded successfully.");
}

void WebServerManager::saveTimeSettings(const String &city, const String &country, bool is24Hour)
{
    EEPROM.begin(EEPROM_SIZE);

    char cityBuf[50] = {0}, countryBuf[50] = {0};
    strncpy(cityBuf, city.c_str(), sizeof(cityBuf) - 1);
    strncpy(countryBuf, country.c_str(), sizeof(countryBuf) - 1);

    EEPROM.put(100, cityBuf);
    EEPROM.put(150, countryBuf);
    EEPROM.put(200, is24Hour);
    EEPROM.commit();
    EEPROM.end();

    Serial.println("✅ Time settings saved to EEPROM.");
}

void WebServerManager::saveAlarmSettings(const String &alarmsJson)
{
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, alarmsJson);

    if (error)
    {
        Serial.println("❌ Failed to parse alarm settings JSON.");
        return;
    }

    uint8_t count = doc.size();
    if (count > MAX_ALARMS)
        count = MAX_ALARMS;

    // 🚀 فقط آلارم‌های معتبر را ذخیره کن
    uint8_t validAlarmCount = 0;
    for (uint8_t i = 0; i < count; i++)
    {
        Alarm alarm;
        strncpy(alarm.name, doc[i]["name"] | "", sizeof(alarm.name) - 1);
        alarm.name[sizeof(alarm.name) - 1] = '\0';

        alarm.hour = doc[i]["hour"];
        alarm.minute = doc[i]["minute"];
        alarm.active = doc[i]["active"];

        for (uint8_t j = 0; j < 7; j++)
        {
            alarm.days[j] = doc[i]["days"][j];
        }

        // 🚀 **بررسی حذف آلارم‌های خالی**
        if (alarm.hour == 0 && alarm.minute == 0 && !alarm.active)
            continue; // 👈 ذخیره نکن

        alarmManager.setAlarm(validAlarmCount++, alarm.hour, alarm.minute, alarm.active, alarm.name, alarm.days);
    }

    Serial.println("✅ Alarms saved successfully.");
}

void WebServerManager::saveDisplaySettings(int brightness, const String &theme)
{
    EEPROM.begin(EEPROM_SIZE);

    EEPROM.put(250, brightness);
    EEPROM.put(300, theme.c_str());
    EEPROM.commit();
    EEPROM.end();

    Serial.println("Display settings saved to EEPROM.");
}

String WebServerManager::getConfigAsJson() const
{

    DynamicJsonDocument doc(2048);

    doc["wifi"]["ssid"] = networkManager.getSSID();
    doc["wifi"]["password"] = networkManager.getPassword();
    doc["time"]["city"] = currentCity;
    doc["time"]["country"] = currentCountry;
    doc["time"]["24h"] = is24HourFormat;
    doc["display"]["brightness"] = displayBrightness;
    doc["display"]["theme"] = displayTheme;

    JsonArray alarmsArray = doc.createNestedArray("alarms");
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        Alarm alarm = alarmManager.getAlarm(i);
        if (alarm.hour == 0 && alarm.minute == 0 && !alarm.active)
            continue;

        JsonObject alarmObj = alarmsArray.createNestedObject();
        alarmObj["name"] = String(alarm.name);
        alarmObj["hour"] = alarm.hour;
        alarmObj["minute"] = alarm.minute;
        alarmObj["active"] = alarm.active;

        JsonArray daysArray = alarmObj.createNestedArray("days");
        for (uint8_t j = 0; j < 7; j++)
        {
            daysArray.add(alarm.days[j]);
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}
