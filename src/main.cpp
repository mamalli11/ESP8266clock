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
#include <HTTPClient.h>
#include <DHT.h>

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);
ESP8266WebServer server(80);

// WiFi Credentials
const char *ssid = "Your_SSID";
const char *password = "Your_PASSWORD";

// NTP Config
int timezone = 3 * 3600 + 30 * 60; // Tehran time zone
int dst = 0;

// Pin Definitions
#define LED_PIN D5
#define DHT_PIN D4
#define LIGHT_SENSOR_PIN A0

Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
DHT dht(DHT_PIN, DHT11);

// متغیرها و تنظیمات
bool nightMode = false;
bool isPersian = true;
int brightness = 255;
int alarmHour = -1;
int alarmMinute = -1;
bool alarmActive = false;
int timerSeconds = -1;

// تابع ذخیره تنظیمات
void saveSettings()
{
  EEPROM.begin(512);
  EEPROM.write(0, nightMode);
  EEPROM.write(1, isPersian);
  EEPROM.write(2, brightness);
  EEPROM.write(3, alarmHour);
  EEPROM.write(4, alarmMinute);
  EEPROM.write(5, alarmActive);
  EEPROM.commit();
  EEPROM.end();
}

// تابع بازیابی تنظیمات
void loadSettings()
{
  EEPROM.begin(512);
  nightMode = EEPROM.read(0);
  isPersian = EEPROM.read(1);
  brightness = EEPROM.read(2);
  alarmHour = EEPROM.read(3);
  alarmMinute = EEPROM.read(4);
  alarmActive = EEPROM.read(5);
  EEPROM.end();
}

// تنظیم روشنایی نمایشگر
void adjustBrightness()
{
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(nightMode ? 50 : brightness);
}

// مدیریت صفحه وب
void handleRoot()
{
  String html = R"rawliteral(
    <html>
    <head>
      <title>تنظیمات</title>
    </head>
    <body>
      <h1>تنظیمات دستگاه</h1>
      <form action="/setSettings" method="POST">
        <label for="language">زبان:</label>
        <select id="language" name="language">
          <option value="0">انگلیسی</option>
          <option value="1">فارسی</option>
        </select><br>
        <label for="brightness">روشنایی:</label>
        <input type="range" id="brightness" name="brightness" min="50" max="255"><br>
        <input type="submit" value="ذخیره">
      </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// ذخیره تنظیمات از صفحه وب
void handleSetSettings()
{
  if (server.hasArg("language"))
  {
    isPersian = server.arg("language").toInt();
  }
  if (server.hasArg("brightness"))
  {
    brightness = server.arg("brightness").toInt();
  }
  saveSettings();
  server.send(200, "text/html", "تنظیمات ذخیره شد. دستگاه را مجددا راه‌اندازی کنید.");
}

// خواندن سنسور نور
int readLightSensor()
{
  // کدی برای خواندن نور از سنسور
  return analogRead(A0); // مثال، فرض بر اینکه سنسور به A0 متصل است
}

// مدیریت آلارم
void checkAlarm(struct tm *p_tm)
{
  if (alarmActive && p_tm->tm_hour == alarmHour && p_tm->tm_min == alarmMinute)
  {
    // اجرای کد برای هشدار
    Serial.println("Alarm!");
    alarmActive = false; // غیرفعال کردن آلارم
    saveSettings();
  }
}

// دریافت اطلاعات آب‌وهوا
void fetchWeather()
{
  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://api.weatherapi.com/v1/current.json?key=API_KEY&q=Tehran&lang=fa");
  int httpCode = http.GET();
  if (httpCode == 200)
  {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    String weather = doc["current"]["condition"]["text"];
    float temp = doc["current"]["temp_c"];
    Serial.printf("Weather: %s, Temp: %.1f°C\n", weather.c_str(), temp);
  }
  http.end();
}

// تابع برای اسلاید کردن متن از راست به چپ
void displayTimeWithSlide(struct tm *p_tm)
{
  for (int x = 128; x >= 0; x -= 4)
  { // انیمیشن اسلاید از راست به چپ
    display.clearDisplay();

    // نمایش ساعت
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(x, 0);
    if (p_tm->tm_hour < 10)
      display.print("0");
    display.print(p_tm->tm_hour);
    display.print(":");
    if (p_tm->tm_min < 10)
      display.print("0");
    display.print(p_tm->tm_min);

    // نمایش تاریخ
    display.setTextSize(2);
    display.setCursor(x, 25);
    display.print(p_tm->tm_mday);
    display.print("/");
    display.print(p_tm->tm_mon + 1);
    display.print("/");
    display.print(p_tm->tm_year + 1900);

    display.display();
    delay(50); // کنترل سرعت انیمیشن
  }
}

// تابع برای انیمیشن فید (روشن و خاموش شدن تدریجی)
void displayTimeWithFade(struct tm *p_tm)
{
  for (int brightness = 0; brightness <= 255; brightness += 15)
  { // فید ورودی
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(20, 0);
    if (p_tm->tm_hour < 10)
      display.print("0");
    display.print(p_tm->tm_hour);
    display.print(":");
    if (p_tm->tm_min < 10)
      display.print("0");
    display.print(p_tm->tm_min);
    display.display();

    analogWrite(OLED_RESET, brightness); // تغییر روشنایی
    delay(50);
  }

  delay(1000); // مکث برای نمایش ساعت

  for (int brightness = 255; brightness >= 0; brightness -= 15)
  { // فید خروجی
    analogWrite(OLED_RESET, brightness);
    delay(50);
  }
}

void setup()
{
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  while (!time(nullptr))
  {
    delay(1000);
  }

  server.on("/", handleRoot);
  server.on("/setSettings", HTTP_POST, handleSetSettings);
  server.begin();

  loadSettings();
}

void loop()
{
  server.handleClient();

  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);

  checkAlarm(p_tm);
  adjustBrightness();

  if (millis() % 60000 == 0)
  { // هر دقیقه آب‌وهوا را به‌روزرسانی کن
    fetchWeather();
  }

  delay(1000);
}
