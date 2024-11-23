#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <time.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// تنظیمات نمایشگر SSD1315
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// تنظیمات LED
#define LED_PIN D5
Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// Function to dim a color
uint32_t dimColor(uint8_t red, uint8_t green, uint8_t blue, float dimFactor)
{
  red = static_cast<uint8_t>(red * dimFactor);
  green = static_cast<uint8_t>(green * dimFactor);
  blue = static_cast<uint8_t>(blue * dimFactor);
  return pixel.Color(red, green, blue);
}

// تنظیمات سنسور نور
#define LIGHT_SENSOR_PIN D8

// تنظیمات buzzer
#define BUZZER_PIN D7

// تنظیمات وب سرور
ESP8266WebServer server(80);
// Function prototypes
void startHotspot();

// تنظیمات شبکه
const char *my_default_ssid = "TP-Mjm";
const char *my_default_password = "MJMoe99_$@M87";

String wifi_ssid;
String wifi_password;

// تنظیمات کاربر
String city = "Tehran";
String country = "IR";
int timezone_offset = 3 * 3600 + 30 * 60;
bool is_24_hour_format = true;

// تنظیمات API آب‌وهوا
const char *weather_api_key = "a287e26533d2ceee3e022d769575453e"; // کلید API خود را اینجا قرار دهید
String weather_city = "Esfahan";
String weather_country = "IR";
String weather_description;
float weather_temperature;
float weather_humidity;

// متغیرهای زمان‌بندی
unsigned long lastWeatherUpdate = 0;
unsigned long lastFetchTime = 0;

struct PrayerTimes
{
  char azan_sobh[6];
  char tolu_aftab[6];
  char azan_zohr[6];
  char ghorub_aftab[6];
  char azan_maghreb[6];
  char nimeshab[6];
};

// متغیر برای ذخیره اوقات شرعی
PrayerTimes prayerTimes;

// تنظیمات آلارم و تایمر
struct Alarm
{
  int hour;
  int minute;
  bool active;
};
Alarm alarms[3];
unsigned long timer_duration = 0;
unsigned long timer_start_time = 0;

// ذخیره تنظیمات در حافظه
void saveSettings()
{
  EEPROM.begin(512);
  EEPROM.put(0, wifi_ssid);
  EEPROM.put(50, wifi_password);
  EEPROM.put(100, city);
  EEPROM.put(150, country);
  EEPROM.commit();
}

// بارگذاری تنظیمات از حافظه
void loadSettings()
{
  EEPROM.begin(512);
  char ssid_buffer[50];
  char password_buffer[50];
  char city_buffer[50];
  char country_buffer[50];
  EEPROM.get(0, ssid_buffer);
  EEPROM.get(50, password_buffer);
  EEPROM.get(100, city_buffer);
  EEPROM.get(150, country_buffer);
  wifi_ssid = String(ssid_buffer);
  wifi_password = String(password_buffer);
  city = String(city_buffer);
  country = String(country_buffer);
}

void handleScan()
{
  // دریافت نام و رمز عبور وای فای جدید از فرم
  String new_ssid = server.arg("ssid");
  String new_password = server.arg("password");

  // اتصال به وای فای جدید با SSID و رمز عبور دریافتی
  WiFi.begin(new_ssid.c_str(), new_password.c_str());

  // انتظار برای اتصال به وای فای جدید
  int connection_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && connection_attempts < 10)
  {
    delay(1000);
    Serial.print(".");
    connection_attempts++;
  }

  // بررسی وضعیت اتصال
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to new WiFi");
    // بستن وب سرور
    server.close();
    // خاموش کردن وای فای داخلی
    WiFi.softAPdisconnect(true);
    // نمایش آی پی وای فای جدید
    Serial.print("New IP address: ");
    Serial.println(WiFi.localIP());
    // ارسال صفحه وب با پیام موفقیت
    server.send(200, "text/html", "<html><body><p>Connected to new WiFi. New IP address: " + WiFi.localIP().toString() + "</p></body></html>");
  }
  else
  {
    Serial.println("Failed to connect to new WiFi");
    // ارسال صفحه وب با پیام خطا
    server.send(200, "text/html", "<html><body><p>Failed to connect to new WiFi. Please try again.</p></body></html>");
  }
}

// تابع نمایش صفحه HTML
void handleRoot()
{
  String page = "<html><body><form action=\"/scan\" method=\"POST\"><input type=\"text\" name=\"ssid\" placeholder=\"SSID\"><br><input type=\"password\" name=\"password\" placeholder=\"Password\"><br><input type=\"submit\" value=\"Connect\"></form></body></html>";
  server.send(200, "text/html", page);
}

// تابع ایجاد Hotspot
void startHotspot()
{
  WiFi.softAP(my_default_ssid, my_default_password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10, "Connect to AP:");
  u8g2.drawStr(0, 30, my_default_ssid);
  u8g2.sendBuffer();

  server.on("/", handleRoot);
  server.on("/scan", HTTP_POST, handleScan);
  server.begin();
  Serial.println("HTTP server started");
}

// تابع اتصال به WiFi
void connectToWiFi()
{
  Serial.print("Get Config Wifi");
  Serial.println(wifi_ssid);
  Serial.print("Defult Wifi");
  Serial.println(my_default_ssid);

  if (wifi_ssid.length() != 0 || wifi_password.length() != 0) // Check if the values are empty
  {
    WiFi.begin(my_default_ssid, my_default_password);
  }
  else
  {
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  u8g2.drawStr(2, 40, "Connecting to WiFi...");
  u8g2.sendBuffer();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    u8g2.clearBuffer();
    u8g2.drawStr(5, 10, "WiFi Connected!");
    String ip = "IP: " + WiFi.localIP().toString();
    u8g2.drawStr(5, 30, ip.c_str());
    u8g2.sendBuffer();
    delay(1000);
  }
  else
  {
    Serial.println("Failed to connect to WiFi");
    startHotspot();
  }
}

// تابع دریافت اطلاعات آب‌وهوا از API
void fetchWeather()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String api_url = "http://api.openweathermap.org/data/2.5/weather?q=" + weather_city + "," + weather_country + "&units=metric&appid=" + weather_api_key;

    WiFiClient client;           // Create a WiFiClient object
    http.begin(client, api_url); // Pass the WiFiClient object and the URL to the begin method
    int httpCode = http.GET();

    if (httpCode == 200)
    {
      String payload = http.getString();
      StaticJsonDocument<1024> doc; // استفاده از StaticJsonDocument
      deserializeJson(doc, payload);

      weather_description = doc["weather"][0]["description"].as<String>();
      weather_temperature = doc["main"]["temp"].as<float>();
      weather_humidity = doc["main"]["humidity"].as<float>();

      Serial.println("Weather updated:");
      Serial.println("Description: " + weather_description);
      Serial.println("Temperature: " + String(weather_temperature) + "°C");
      Serial.println("Humidity: " + String(weather_humidity) + "%");

      lastFetchTime = millis(); // ثبت زمان آخرین دریافت
    }
    else
    {
      Serial.println("Failed to fetch weather data");
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected!");
  }
}

// تابع نمایش اطلاعات آب‌وهوا روی نمایشگر
void displayWeather()
{
  u8g2.clearBuffer();

  // تنظیم فونت برای آیکن‌ها
  u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);

  // نمایش آیکن وضعیت آب‌وهوا
  if (weather_description.indexOf("clear") >= 0)
  {
    u8g2.drawStr(5, 30, "A"); // آیکن آفتابی
  }
  else if (weather_description.indexOf("cloud") >= 0)
  {
    u8g2.drawStr(5, 30, "C"); // آیکن ابری
  }
  else if (weather_description.indexOf("rain") >= 0)
  {
    u8g2.drawStr(5, 30, "R"); // آیکن بارانی
  }
  else if (weather_description.indexOf("snow") >= 0)
  {
    u8g2.drawStr(5, 30, "S"); // آیکن برفی
  }
  else
  {
    u8g2.drawStr(5, 30, "?"); // آیکن نامشخص
  }

  // تنظیم فونت برای متن
  u8g2.setFont(u8g2_font_helvB08_tf); // فونت خوانا و فانتزی

  // نمایش توضیحات وضعیت آب‌وهوا
  u8g2.drawStr(30, 15, "Weather:");
  u8g2.drawStr(30, 30, weather_description.c_str());

  // نمایش دما و رطوبت
  char tempStr[16];
  snprintf(tempStr, sizeof(tempStr), "Temp: %.1f C", weather_temperature);
  u8g2.drawStr(30, 45, tempStr);

  char humStr[16];
  snprintf(humStr, sizeof(humStr), "Hum: %.1f%%", weather_humidity);
  u8g2.drawStr(30, 60, humStr);

  u8g2.sendBuffer();
}

// تابع دریافت زمان از NTP
void updateTime()
{
  configTime(timezone_offset, 0, "pool.ntp.org", "time.nist.gov");
  while (!time(nullptr))
  {
    delay(1000);
  }
}

// تابع کنترل روشنایی
void adjustBrightness()
{
  int light_level = analogRead(LIGHT_SENSOR_PIN);
  int brightness = map(light_level, 0, 1023, 0, 255);

  u8g2.setContrast(brightness);
}

// تابع وب سرویس برای تنظیمات
void handleWebConfig()
{
  String html = "<html><body><h1>Clock Settings</h1>";
  html += "<form action='/save' method='POST'>";
  html += "WiFi SSID: <input type='text' name='ssid'><br>";
  html += "WiFi Password: <input type='text' name='password'><br>";
  html += "City: <input type='text' name='city'><br>";
  html += "Country: <input type='text' name='country'><br>";
  html += "<input type='submit' value='Save'></form></body></html>";
  server.send(200, "text/html", html);
}

// تابع ذخیره تنظیمات از طریق وب
void handleSaveConfig()
{
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("password");
  city = server.arg("city");
  country = server.arg("country");
  saveSettings();
  server.send(200, "text/plain", "Settings saved! Restarting...");
  ESP.restart();
}

// تابع نمایش آلارم
void checkAlarms()
{
  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);
  for (int i = 0; i < 3; i++)
  {
    if (alarms[i].active && alarms[i].hour == p_tm->tm_hour && alarms[i].minute == p_tm->tm_min)
    {
      for (int j = 0; j < 60; j++)
      {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
        delay(500);
      }
    }
  }
}

// تابع تنظیم LED بر اساس روز هفته
void updateLED(int dayOfWeek, float dimFactor = 0.1)
{
  switch (dayOfWeek)
  {
  case 0:
    pixel.fill(dimColor(255, 0, 0, dimFactor));
    break; // Red
  case 1:
    pixel.fill(dimColor(0, 255, 0, dimFactor));
    break; // Green
  case 2:
    pixel.fill(dimColor(0, 0, 255, dimFactor));
    break; // Blue
  case 3:
    pixel.fill(dimColor(255, 255, 0, dimFactor));
    break; // Yellow
  case 4:
    pixel.fill(dimColor(255, 0, 255, dimFactor));
    break; // Magenta
  case 5:
    pixel.fill(dimColor(0, 255, 255, dimFactor));
    break; // Cyan
  case 6:
    pixel.fill(dimColor(255, 165, 0, dimFactor));
    break; // Orange
  }
  pixel.show();
}

// تابع به‌روزرسانی نمایشگر
void updateDisplay(struct tm *p_tm)
{
  u8g2.clearBuffer();

  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), is_24_hour_format ? "%02d:%02d" : "%02d:%02d %s",
           is_24_hour_format ? p_tm->tm_hour : (p_tm->tm_hour % 12), p_tm->tm_min, p_tm->tm_hour >= 12 ? "PM" : "AM");

  u8g2.setFont(u8g2_font_logisoso30_tr);
  u8g2.drawStr(20, 40, timeStr);

  char dateStr[11];
  snprintf(dateStr, sizeof(dateStr), "%04d/%02d/%02d", p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday);
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(35, 60, dateStr);

  u8g2.sendBuffer();
}

void fetchPrayerTimes()
{
  if (WiFi.status() == WL_CONNECTED)
  {

    HTTPClient http;
    WiFiClient client;                                                                                       // Create a WiFiClient object
    String api_url = "https://api.keybit.ir/owghat/?city=%D8%AE%D9%85%DB%8C%D9%86%DB%8C+%D8%B4%D9%87%D8%B1"; // URL کدگذاری‌شده

    Serial.println("Connecting to API...");
    http.begin(client, api_url);
    int httpCode = http.GET(); // ارسال درخواست GET

    Serial.println(http.errorToString(1));
    if (httpCode == 200)
    {
      String payload = http.getString();
      Serial.println("API Response: " + payload);

      // تجزیه JSON
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error)
      {
        Serial.print("JSON Parsing Error: ");
        Serial.println(error.c_str());
        return;
      }

      // ذخیره مقادیر در متغیرهای ساختار
      strcpy(prayerTimes.azan_sobh, doc["result"]["azan_sobh"] | "N/A");
      strcpy(prayerTimes.tolu_aftab, doc["result"]["tolu_aftab"] | "N/A");
      strcpy(prayerTimes.azan_zohr, doc["result"]["azan_zohr"] | "N/A");
      strcpy(prayerTimes.ghorub_aftab, doc["result"]["ghorub_aftab"] | "N/A");
      strcpy(prayerTimes.azan_maghreb, doc["result"]["azan_maghreb"] | "N/A");
      strcpy(prayerTimes.nimeshab, doc["result"]["nimeshab"] | "N/A");

      Serial.println("Prayer times updated successfully!");
    }
    else
    {
      Serial.printf("Failed to fetch prayer times, HTTP code: %d\n", httpCode);
    }

    http.end(); // پایان درخواست HTTP
  }
}

void displayPrayerTimes()
{
  u8g2.clearBuffer();

  u8g2.setCursor(5, 0);
  u8g2.println("Prayer Times:");

  u8g2.setCursor(5, 10);
  u8g2.printf("Sobh: %s\n", prayerTimes.azan_sobh);

  u8g2.setCursor(5, 20);
  u8g2.printf("Tolu: %s\n", prayerTimes.tolu_aftab);

  u8g2.setCursor(5, 30);
  u8g2.printf("Zohr: %s\n", prayerTimes.azan_zohr);

  u8g2.setCursor(5, 40);
  u8g2.printf("Maghrib: %s\n", prayerTimes.azan_maghreb);

  u8g2.setCursor(5, 50);
  u8g2.printf("Midnight: %s\n", prayerTimes.nimeshab);

  u8g2.sendBuffer();
}

// تنظیمات اولیه
void setup()
{
  Serial.begin(9600);
  u8g2.begin();
  pixel.begin(); // Initialize the pixel
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  loadSettings();
  connectToWiFi();
  updateTime();

  server.on("/", handleWebConfig);
  server.on("/save", handleSaveConfig);
  server.begin();

  fetchWeather();     // دریافت اولین اطلاعات آب‌وهوا
  fetchPrayerTimes(); // دریافت اولین اطلاعات اوقات شرعی
}

unsigned long previousMillis = 0;

// حالت‌ها برای نمایش چرخه‌ای
enum DisplayState
{
  DISPLAY_TIME,
  DISPLAY_WEATHER,
  DISPLAY_PRAYER_TIMES
};

DisplayState currentState = DISPLAY_TIME; // حالت فعلی
unsigned long lastStateChange = 0;        // زمان آخرین تغییر حالت

const unsigned long TIME_DISPLAY_DURATION = 15000;   // مدت نمایش ساعت (15 ثانیه)
const unsigned long WEATHER_DISPLAY_DURATION = 5000; // مدت نمایش آب و هوا (5 ثانیه)
const unsigned long PRAYER_DISPLAY_DURATION = 5000;  // مدت نمایش اوقات شرعی (5 ثانیه)

void loop()
{
  unsigned long currentMillis = millis();
  time_t now = time(nullptr);
  struct tm *p_tm = localtime(&now);

  // بررسی زمان و تغییر حالت‌ها
  switch (currentState)
  {
  case DISPLAY_TIME:
    updateDisplay(p_tm); // نمایش ساعت
    if (currentMillis - lastStateChange >= TIME_DISPLAY_DURATION)
    {
      currentState = DISPLAY_WEATHER; // تغییر به حالت نمایش آب و هوا
      lastStateChange = currentMillis;
    }
    break;

  case DISPLAY_WEATHER:
    displayWeather(); // نمایش آب و هوا
    if (currentMillis - lastStateChange >= WEATHER_DISPLAY_DURATION)
    {
      currentState = DISPLAY_PRAYER_TIMES; // تغییر به حالت نمایش اوقات شرعی
      lastStateChange = currentMillis;
    }
    break;

  case DISPLAY_PRAYER_TIMES:
    displayPrayerTimes(); // نمایش اوقات شرعی
    if (currentMillis - lastStateChange >= PRAYER_DISPLAY_DURATION)
    {
      currentState = DISPLAY_TIME; // بازگشت به حالت نمایش ساعت
      lastStateChange = currentMillis;
    }
    break;
  }

  // تنظیم LED بر اساس روز هفته شمسی
  updateLED(p_tm->tm_wday);

  // بررسی و تنظیم روشنایی
  adjustBrightness();

  // بررسی هشدارها
  checkAlarms();

  // به‌روزرسانی اطلاعات آب و هوا و اوقات شرعی هر 6 ساعت
  if (millis() - lastFetchTime >= 6L * 60L * 60L * 1000L)
  {
    fetchPrayerTimes();
    fetchWeather();
    lastFetchTime = millis();
  }

  // مدیریت وب سرور
  server.handleClient();

  delay(10); // برای جلوگیری از اشغال بیش از حد CPU
}