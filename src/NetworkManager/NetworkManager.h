#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class NetworkManager
{
public:
    NetworkManager();
    void begin();
    void handleClient();
    bool isConnected();
    String getLocalIP();

private:
    ESP8266WebServer server;
    String wifiSSID;
    String wifiPassword;
    String city;
    String country;
    bool is24Hour;

    void loadCredentials();
    void saveCredentials();
    void startAPMode();
    void initWebServer();
    void handleRoot();
    void handleScan();
    void handleSaveConfig();
    void handleGetSettings();
    void handleSetWifi();
    void handleSetTime();
    void handleSetAlarm();
    void handleSetDisplay();
    void serveFile(const String &path, const String &contentType);
};

#endif