#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../DisplayManager/DisplayManager.h"

class NetworkManager
{
public:
    NetworkManager(DisplayManager &dispMgr);
    void begin();
    void handleClient();
    bool isConnected();
    String getLocalIP();
    void saveWiFiCredentials(const String &ssid, const String &password);
    String getSSID() const { return wifiSSID; }
    String getPassword() const { return wifiPassword; }

private:
    ESP8266WebServer server;
    DisplayManager &displayManager;
    String wifiSSID;
    String wifiPassword;
    bool isAPMode;

    void loadWiFiCredentials();
    void saveWiFiCredentials();
    void startAPMode();
    void connectToWiFi();
    void checkWiFiConnection();
    void initWebServer();
    void serveFile(const String &path, const String &contentType);
};

#endif
