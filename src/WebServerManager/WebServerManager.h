#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class WebServerManager
{
public:
    WebServerManager();
    void begin();
    void handleClient();
    void setWiFiCredentials(const String &ssid, const String &password);
    void setTimeSettings(const String &city, const String &country, bool is24Hour);
    void setAlarmSettings(const String &alarmsJson);
    void setDisplaySettings(int brightness, const String &themeColor);

private:
    ESP8266WebServer server;
    String wifiSSID;
    String wifiPassword;
    String city;
    String country;
    bool is24Hour;
    String alarmsJson;
    int brightness;
    String themeColor;

    void handleRoot();
    void handleGetSettings();
    void handleSetWiFi();
    void handleSetTime();
    void handleSetAlarm();
    void handleSetDisplay();
    void handleNotFound();
    void serveFile(const String &path, const String &contentType);
};

#endif