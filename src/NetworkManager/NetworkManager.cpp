#include <EEPROM.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>

#include "NetworkManager.h"

NetworkManager::NetworkManager() : server(80), is24Hour(true) {}

void NetworkManager::begin()
{
    loadCredentials(); // Load saved WiFi credentials
    initWebServer();   // Initialize the web server

    if (wifiSSID.isEmpty() || wifiPassword.isEmpty())
    {
        startAPMode(); // Start AP mode if no credentials are saved
    }
    else
    {
        WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts++ < 20)
        {
            delay(500);
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            startAPMode(); // Fallback to AP mode if connection fails
        }
    }
}

void NetworkManager::handleClient()
{
    server.handleClient();
}

bool NetworkManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String NetworkManager::getLocalIP()
{
    return WiFi.localIP().toString();
}

void NetworkManager::loadCredentials()
{
    EEPROM.begin(512);
    char ssid[32], password[64];
    EEPROM.get(0, ssid);
    EEPROM.get(32, password);
    wifiSSID = ssid;
    wifiPassword = password;
    EEPROM.end();
}

void NetworkManager::saveCredentials()
{
    EEPROM.begin(512);
    EEPROM.put(0, wifiSSID.c_str());
    EEPROM.put(32, wifiPassword.c_str());
    EEPROM.commit();
    EEPROM.end();
}

void NetworkManager::startAPMode()
{
    WiFi.softAP("SmartClockAP", "12345678");
    Serial.print("AP Mode IP: ");
    Serial.println(WiFi.softAPIP());
}

void NetworkManager::initWebServer()
{
    LittleFS.begin(); // Initialize LittleFS

    // Serve static files
    server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");

    // API Endpoints
    server.on("/", HTTP_GET, [this]()
              { handleRoot(); });
    server.on("/scan", HTTP_POST, [this]()
              { handleScan(); });
    server.on("/save", HTTP_POST, [this]()
              { handleSaveConfig(); });
    server.on("/get-settings", HTTP_GET, [this]()
              { handleGetSettings(); });
    server.on("/set-wifi", HTTP_POST, [this]()
              { handleSetWifi(); });
    server.on("/set-time", HTTP_POST, [this]()
              { handleSetTime(); });
    server.on("/set-alarm", HTTP_POST, [this]()
              { handleSetAlarm(); });
    server.on("/set-display", HTTP_POST, [this]()
              { handleSetDisplay(); });

    server.begin();
}

void NetworkManager::handleRoot()
{
    File file = LittleFS.open("/www/index.html", "r");
    if (file)
    {
        server.streamFile(file, "text/html");
        file.close();
    }
    else
    {
        server.send(404, "text/plain", "File not found");
    }
}

void NetworkManager::handleScan()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() > 0 && password.length() > 0)
    {
        wifiSSID = ssid;
        wifiPassword = password;
        saveCredentials();
        server.send(200, "text/plain", "Credentials saved. Rebooting...");
        delay(1000);
        ESP.restart();
    }
    else
    {
        server.send(400, "text/plain", "Invalid SSID or password");
    }
}

void NetworkManager::handleSaveConfig()
{
    city = server.arg("city");
    country = server.arg("country");
    is24Hour = server.arg("timeFormat") == "24";
    server.send(200, "text/plain", "Settings saved");
}

void NetworkManager::handleGetSettings()
{
    JsonDocument doc;
    doc["wifi"]["ssid"] = wifiSSID;
    doc["wifi"]["password"] = wifiPassword;
    doc["time"]["city"] = city;
    doc["time"]["country"] = country;
    doc["time"]["format"] = is24Hour ? "24" : "12";

    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void NetworkManager::handleSetWifi()
{
    wifiSSID = server.arg("ssid");
    wifiPassword = server.arg("password");
    saveCredentials();
    server.send(200, "text/plain", "WiFi settings saved. Rebooting...");
    delay(1000);
    ESP.restart();
}

void NetworkManager::handleSetTime()
{
    city = server.arg("city");
    country = server.arg("country");
    is24Hour = server.arg("timeFormat") == "24";
    server.send(200, "text/plain", "Time settings saved");
}

void NetworkManager::handleSetAlarm()
{
    // Parse and save alarm settings
    server.send(200, "text/plain", "Alarm settings saved");
}

void NetworkManager::handleSetDisplay()
{
    // Parse and save display settings
    server.send(200, "text/plain", "Display settings saved");
}