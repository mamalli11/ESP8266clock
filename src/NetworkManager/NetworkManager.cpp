// NetworkManager.cpp
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#include "NetworkManager.h"
#include "../WebServerManager/WebServerManager.h"
#include "../DisplayManager/DisplayManager.h"

NetworkManager::NetworkManager(DisplayManager &dispMgr) : server(80),
                                                          displayManager(dispMgr),
                                                          isAPMode(false),
                                                          wasEverConnected(false),
                                                          lastConnectionAttempt(0),
                                                          nextRetryDelay(0),
                                                          currentRetryStep(0) {}

void NetworkManager::begin()
{
    displayManager.showMessage("🔄 Initializing WiFi...", 2000);
    loadWiFiCredentials();

    if (wifiSSID.isEmpty())
    {
        displayManager.showMessage("⚠️ No WiFi found!\n\nStarting AP Mode", 3000);
        startAPMode();
    }
    else
    {
        connectToWiFi();
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

void NetworkManager::loadWiFiCredentials()
{
    Serial.println("📥 Loading WiFi credentials from EEPROM...");

    EEPROM.begin(EEPROM_SIZE);

    char ssidBuf[32] = {0}, passBuf[64] = {0};
    EEPROM.get(0, ssidBuf);
    EEPROM.get(32, passBuf);

    wifiSSID = String(ssidBuf);
    wifiPassword = String(passBuf);
    EEPROM.end();

    Serial.println("📥 Loading WiFi credentials from EEPROM...");
    Serial.print("🔍 Loaded SSID: ");
    Serial.println(wifiSSID);
    Serial.print("🔍 Loaded Password: ");
    Serial.println(wifiPassword);
}

void NetworkManager::saveWiFiCredentials(const String &ssid, const String &password)
{
    EEPROM.begin(EEPROM_SIZE);

    char ssidBuf[32] = {0}, passBuf[64] = {0};
    strncpy(ssidBuf, ssid.c_str(), sizeof(ssidBuf) - 1);
    strncpy(passBuf, password.c_str(), sizeof(passBuf) - 1);

    EEPROM.put(0, ssidBuf);
    EEPROM.put(32, passBuf);
    EEPROM.commit();
    delay(100);
    EEPROM.end();

    wifiSSID = ssid;
    wifiPassword = password;

    Serial.println("✅ Checking saved values...");
    char testSSID[32] = {0}, testPass[64] = {0};
    EEPROM.get(0, testSSID);
    EEPROM.get(32, testPass);

    Serial.printf("🔍 Saved SSID: %s\n", testSSID);
    Serial.printf("🔍 Saved Password: %s\n", testPass);
}

void NetworkManager::connectToWiFi()
{
    Serial.print("📶 Connecting to WiFi: ");
    Serial.println(wifiSSID);
    displayManager.showMessage("📶 Connecting to:\n\n" + wifiSSID, 2000);

    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts++ < 20)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("❌ Failed to connect to WiFi.");
        if (!wasEverConnected)
        {
            displayManager.showMessage("❌ WiFi Failed!\n\nStarting AP Mode", 3000);
            startAPMode();
        }
    }
    else
    {
        wasEverConnected = true;
        currentRetryStep = 0;
        Serial.print("✅ Connected to WiFi! IP Address: ");
        Serial.println(WiFi.localIP());
        displayManager.showMessage("✅ Connected!\n\nIP: " + WiFi.localIP().toString(), 3000);
        isAPMode = false;
    }
}

void NetworkManager::startAPMode()
{
    WiFi.softAP("SmartClockAP", "12345678");
    isAPMode = true;

    Serial.println("📡 AP Mode Started! SSID: SmartClockAP, Password: 12345678");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    displayManager.showMessage("📡 AP Mode Active\n\nSSID: SmartClockAP\n\nPass: 12345678", 4000);
}

void NetworkManager::checkWiFiConnection()
{
    static bool lastConnectionState = isConnected(); // وضعیت قبلی اتصال را ذخیره می‌کند

    bool currentConnectionState = isConnected();

    // اگر وضعیت اتصال تغییر کرده باشد
    if (currentConnectionState != lastConnectionState)
    {
        if (currentConnectionState)
        {
            // اتصال برقرار شده
            wasEverConnected = true;
            currentRetryStep = 0;
            Serial.println("\n✅ WiFi Connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            displayManager.showMessage("✅ WiFi Connected!\nIP: " + WiFi.localIP().toString(), 3000);
        }
        else
        {
            // اتصال قطع شده
            Serial.println("\n⚠️  WiFi Disconnected!");
            lastConnectionAttempt = millis();   // زمان قطع شدن را ذخیره می‌کند
            nextRetryDelay = retryIntervals[0]; // بازنشانی به اولین پله
            currentRetryStep = 0;
        }
        lastConnectionState = currentConnectionState;
    }

    // اگر در حالت AP نیستیم و اتصال قطع است
    if (!isAPMode && !currentConnectionState)
    {
        unsigned long currentTime = millis();

        if (currentTime - lastConnectionAttempt >= nextRetryDelay)
        {
            Serial.println("🔄 Attempting to reconnect...");

            WiFi.reconnect();
            lastConnectionAttempt = currentTime;

            // محاسبه زمان تلاش بعدی
            if (currentRetryStep < 7)
            {
                nextRetryDelay = retryIntervals[currentRetryStep];
                currentRetryStep++;
            }
            else
            {
                nextRetryDelay = retryIntervals[7]; // 48 ساعت
            }

            Serial.print("⏱ Next attempt in: ");
            Serial.print(nextRetryDelay / 1000);
            Serial.println(" seconds");
        }
    }
}