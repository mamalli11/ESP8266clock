
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <FS.h>
#include <LittleFS.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

// Pin Definitions
#define LED_PIN D5
#define DHT_PIN D4
#define LIGHT_SENSOR_PIN A0

Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
DHT dht(DHT_PIN, DHT11);

// WiFi Credentials
const char *ssid = "Your_SSID";
const char *password = "Your_PASSWORD";

// NTP Config
int timezone = 3 * 3600 + 30 * 60; // Tehran time zone
int dst = 0;

// Web Server
ESP8266WebServer server(80);

// Configuration
struct Config
{
  String city;
  bool autoBrightness;
  float brightness;
  String alarms[5]; // Max 5 alarms
};
Config config;

// Weather Info
String weatherInfo;

// Utility Functions
void saveConfig()
{
  File file = LittleFS.open("/config.json", "w");
  if (!file)
  {
    Serial.println("Failed to open config file for writing.");
    return;
  }
  StaticJsonDocument<512> doc;
  doc["city"] = config.city;
  doc["autoBrightness"] = config.autoBrightness;
  doc["brightness"] = config.brightness;
  for (int i = 0; i < 5; i++)
  {
    doc["alarms"][i] = config.alarms[i];
  }
  serializeJson(doc, file);
  file.close();
}

void loadConfig()
{
  File file = LittleFS.open("/config.json", "r");
  if (!file)
  {
    Serial.println("No config file found, using defaults.");
    return;
  }
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Failed to read config file, using defaults.");
    return;
  }
  config.city = doc["city"].as<String>();
  config.autoBrightness = doc["autoBrightness"];
  config.brightness = doc["brightness"];
  for (int i = 0; i < 5; i++)
  {
    config.alarms[i] = doc["alarms"][i].as<String>();
  }
  file.close();
}

void adjustBrightness()
{
  if (config.autoBrightness)
  {
    int lightLevel = analogRead(LIGHT_SENSOR_PIN);
    float brightness = map(lightLevel, 0, 1023, 0, 255) / 255.0;
    analogWrite(OLED_RESET, brightness * 255);
  }
  else
  {
    analogWrite(OLED_RESET, config.brightness * 255);
  }
}

void fetchWeather()
{
  if (WiFi.status() == WL_CONNECTED && config.city.length() > 0)
  {
    HTTPClient http;
    WiFiClient client;
    String apiUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + config.city + "&appid=Your_API_Key&units=metric";
    http.begin(client, apiUrl);
    int httpCode = http.GET();
    if (httpCode == 200)
    {
      weatherInfo = http.getString();
    }
    http.end();
  }
}

void checkAlarms(struct tm *p_tm)
{
  String currentTime = String(p_tm->tm_hour) + ":" + String(p_tm->tm_min);
  for (String alarm : config.alarms)
  {
    if (alarm == currentTime)
    {
      pixel.fill(pixel.Color(255, 0, 0));
      pixel.show();
      delay(1000);
    }
  }
}

void handleRoot()
{
  String html = "<html><body>";
  html += "<h1>ESP8266 Clock Settings</h1>";
  html += "<form action='/save' method='POST'>";
  html += "City: <input type='text' name='city' value='" + config.city + "'><br>";
  html += "Auto Brightness: <input type='checkbox' name='autoBrightness' " + String(config.autoBrightness ? "checked" : "") + "><br>";
  html += "Brightness: <input type='range' name='brightness' min='0' max='1' step='0.1' value='" + String(config.brightness) + "'><br>";
  for (int i = 0; i < 5; i++)
  {
    html += "Alarm " + String(i + 1) + ": <input type='time' name='alarm" + String(i) + "' value='" + config.alarms[i] + "'><br>";
  }
  html += "<input type='submit' value='Save'>";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSave()
{
  config.city = server.arg("city");
  config.autoBrightness = server.hasArg("autoBrightness");
  config.brightness = server.arg("brightness").toFloat();
  for (int i = 0; i < 5; i++)
  {
    config.alarms[i] = server.arg("alarm" + String(i));
  }
  saveConfig();
  server.send(200, "text/plain", "Settings saved. Rebooting...");
  ESP.restart();
}

void setup()
{
  Serial.begin(115200);

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed.");
    return;
  }

  loadConfig();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  dht.begin();
  pixel.begin();

  fetchWeather();
}

void loop()
{
  server.handleClient();

  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);

  checkAlarms(p_tm);
  adjustBrightness();

  if (millis() % 6000000 == 0)
  {
    fetchWeather();
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.printf("%02d:%02d:%02d", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.printf("%d/%d/%d", p_tm->tm_mday, p_tm->tm_mon + 1, p_tm->tm_year + 1900);

  display.setCursor(0, 30);
  display.print(weatherInfo);

  display.display();

  delay(1000);
}
