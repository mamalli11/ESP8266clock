// NetworkManager.h
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
    void checkWiFiConnection();
    void saveWiFiCredentials(const String &ssid, const String &password);
    String getSSID() const { return wifiSSID; }
    String getPassword() const { return wifiPassword; }

private:
    ESP8266WebServer server;
    DisplayManager &displayManager;
    String wifiSSID;
    String wifiPassword;
    bool isAPMode;

    bool wasEverConnected; // Flag to track if we ever had a successful connection
    unsigned long lastConnectionAttempt;
    unsigned long nextRetryDelay;
    const unsigned long retryIntervals[8] = {30000, 60000, 120000, 300000, 600000, 1200000, 2400000, 17280000};
    int currentRetryStep;

    void loadWiFiCredentials();
    void saveWiFiCredentials();
    void startAPMode();
    void connectToWiFi();
    void initWebServer();
    void serveFile(const String &path, const String &contentType);
};

#endif