#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <EEPROM.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

#include "../AlarmManager/AlarmManager.h"
#include "../NetworkManager/NetworkManager.h"

class WebServerManager
{
public:
    WebServerManager(AlarmManager &alarmManager, NetworkManager &networkManager);
    void begin();
    void handleClient();
    void loadSettings();
    void saveTimeSettings(const String &city, const String &country, bool is24Hour);
    void saveDisplaySettings(int brightness, const String &theme); // اضافه شد
    void saveAlarmSettings(const String &alarmsJson);
    String getConfigAsJson() const;

private:
    ESP8266WebServer server;
    AlarmManager &alarmManager;
    NetworkManager &networkManager;
    String currentCity;
    String currentCountry;
    bool is24HourFormat;
    int displayBrightness;
    String displayTheme;

    void initWebRoutes();
};

#endif
