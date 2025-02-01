#include <EEPROM.h>
#include <ESP8266WebServer.h>

#include "WebServerManager.h"

WebServerManager::WebServerManager() : server(80), is24Hour(true), brightness(255), themeColor("#FFFFFF") {}

void WebServerManager::begin()
{
    LittleFS.begin(); // Initialize LittleFS

    // Serve static files
    server.serveStatic("/", LittleFS, "/www/");
    server.on("/", HTTP_GET, [this]()
              { serveFile("/www/index.html", "text/html"); });

    // API Endpoints
    server.on("/", HTTP_GET, [this]()
              { handleRoot(); });
    server.on("/get-settings", HTTP_GET, [this]()
              { handleGetSettings(); });
    server.on("/set-wifi", HTTP_POST, [this]()
              { handleSetWiFi(); });
    server.on("/set-time", HTTP_POST, [this]()
              { handleSetTime(); });
    server.on("/set-alarm", HTTP_POST, [this]()
              { handleSetAlarm(); });
    server.on("/set-display", HTTP_POST, [this]()
              { handleSetDisplay(); });
    server.onNotFound([this]()
                      { handleNotFound(); });

    server.begin();
}

void WebServerManager::handleClient()
{
    server.handleClient();
}

void WebServerManager::setWiFiCredentials(const String &ssid, const String &password)
{
    wifiSSID = ssid;
    wifiPassword = password;
}

void WebServerManager::setTimeSettings(const String &city, const String &country, bool is24Hour)
{
    this->city = city;
    this->country = country;
    this->is24Hour = is24Hour;
}

void WebServerManager::setAlarmSettings(const String &alarmsJson)
{
    this->alarmsJson = alarmsJson;
}

void WebServerManager::setDisplaySettings(int brightness, const String &themeColor)
{
    this->brightness = brightness;
    this->themeColor = themeColor;
}

void WebServerManager::handleRoot()
{
    serveFile("/www/index.html", "text/html");
}

void WebServerManager::handleGetSettings()
{
    JsonDocument doc;
    doc["wifi"]["ssid"] = wifiSSID;
    doc["wifi"]["password"] = wifiPassword;
    doc["time"]["city"] = city;
    doc["time"]["country"] = country;
    doc["time"]["format"] = is24Hour ? "24" : "12";
    doc["alarms"] = alarmsJson;
    doc["display"]["brightness"] = brightness;
    doc["display"]["themeColor"] = themeColor;

    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void WebServerManager::handleSetWiFi()
{
    wifiSSID = server.arg("ssid");
    wifiPassword = server.arg("password");
    server.send(200, "text/plain", "WiFi settings saved. Rebooting...");
    delay(1000);
    ESP.restart();
}

void WebServerManager::handleSetTime()
{
    city = server.arg("city");
    country = server.arg("country");
    is24Hour = server.arg("timeFormat") == "24";
    server.send(200, "text/plain", "Time settings saved");
}

void WebServerManager::handleSetAlarm()
{
    alarmsJson = server.arg("alarms");
    server.send(200, "text/plain", "Alarm settings saved");
}

void WebServerManager::handleSetDisplay()
{
    brightness = server.arg("brightness").toInt();
    themeColor = server.arg("themeColor");
    server.send(200, "text/plain", "Display settings saved");
}

void WebServerManager::handleNotFound()
{
    serveFile("/www/404.html", "text/html");
}

void WebServerManager::serveFile(const String &path, const String &contentType)
{
    File file = LittleFS.open(path, "r");
    if (file)
    {
        server.streamFile(file, contentType);
        file.close();
    }
    else
    {
        server.send(404, "text/plain", "File not found");
    }
}