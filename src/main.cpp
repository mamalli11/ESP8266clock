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
#include <WiFiClientSecure.h>

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
const char *my_default_ssid = "Wifi_SSD";
const char *my_default_password = "Wifi_Password";

String wifi_ssid;
String wifi_password;

// تنظیمات کاربر
String city = "Tehran";
String country = "IR";
int timezone_offset = 3 * 3600 + 30 * 60;
bool is_24_hour_format = true;

// تنظیمات API آب‌وهوا
const char *weather_api_key = "API_KEY"; // کلید API خود را اینجا قرار دهید
String weather_city = "Esfahan";
String weather_country = "IR";
String weather_description;
float weather_temperature;
float weather_humidity;

// متغیرهای زمان‌بندی
unsigned long lastWeatherUpdate = 0;
unsigned long lastFetchTime = 0;

// API URL
const String apiUrl = "https://prayer.aviny.com/api/prayertimes/311";

// متغیرهای اوقات شرعی
String CityLName, imsaakTime, sunriseTime, noonTime, sunsetTime, maghrebTime, midnightTime, todayDate, todayQamari, simultaneityKaaba;

// Alarm Configuration
struct Alarm
{
  int hour;
  int minute;
  bool active;
};
Alarm alarms[3];

// Load/Save Alarms
void saveAlarms()
{
  EEPROM.begin(512);
  EEPROM.put(0, alarms);
  EEPROM.commit();
}

void loadAlarms()
{
  EEPROM.begin(512);
  EEPROM.get(0, alarms);
}

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
    String ip = "IP : " + WiFi.localIP().toString();
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

// Serve Index.html
void serveIndex()
{
  if (SPIFFS.exists("/index.html"))
  {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  }
  else
  {
    server.send(404, "text/plain", "File not found");
  }
}

// Handle Alarm Setup via Web
void handleSetAlarm()
{
  if (server.hasArg("alarm") && server.hasArg("hour") && server.hasArg("minute"))
  {
    int alarmIdx = server.arg("alarm").toInt();
    if (alarmIdx >= 0 && alarmIdx < 3)
    {
      alarms[alarmIdx].hour = server.arg("hour").toInt();
      alarms[alarmIdx].minute = server.arg("minute").toInt();
      alarms[alarmIdx].active = true;
      saveAlarms();
      server.send(200, "text/plain", "Alarm set successfully!");
    }
    else
    {
      server.send(400, "text/plain", "Invalid alarm index");
    }
  }
  else
  {
    server.send(400, "text/plain", "Missing parameters");
  }
}

// Check and Trigger Alarms
void checkAlarms()
{
  time_t now = time(nullptr);
  struct tm *timeInfo = localtime(&now);
  for (int i = 0; i < 3; i++)
  {
    if (alarms[i].active &&
        alarms[i].hour == timeInfo->tm_hour &&
        alarms[i].minute == timeInfo->tm_min)
    {
      // Trigger alarm
      digitalWrite(D7, HIGH); // Buzzer ON
      delay(1000);
      digitalWrite(D7, LOW); // Buzzer OFF
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

// متغیرهای حالت
bool showShamsiDate = true;                // برای تعیین اینکه تاریخ شمسی یا میلادی نمایش داده شود
unsigned long lastSwitchTime = 0;          // زمان آخرین تغییر نمایش
const unsigned long switchInterval = 7500; // بازه تغییر به میلی‌ثانیه (7.5 ثانیه)

// تابع به‌روزرسانی نمایشگر
void updateDisplay(struct tm *p_tm)
{
  u8g2.clearBuffer();

  // نمایش زمان (همراه با ثانیه)
  // نمایش ساعت
  char hourStr[3];
  snprintf(hourStr, sizeof(hourStr), "%02d",
           is_24_hour_format ? p_tm->tm_hour : (p_tm->tm_hour % 12 == 0 ? 12 : p_tm->tm_hour % 12));
  u8g2.setFont(u8g2_font_logisoso20_tr); // فونت دلخواه برای ساعت
  u8g2.drawStr(15, 40, hourStr);

  // نمایش جداکننده ":" بین ساعت و دقیقه

  u8g2.setFont(u8g2_font_logisoso30_tr); // فونت دلخواه برای جداکننده
  u8g2.drawStr(45, 40, ":");

  // نمایش دقیقه
  char minuteStr[3];
  snprintf(minuteStr, sizeof(minuteStr), "%02d", p_tm->tm_min);
  u8g2.setFont(u8g2_font_logisoso20_tr); // فونت دلخواه برای دقیقه
  u8g2.drawStr(55, 40, minuteStr);

  // نمایش جداکننده ":" بین دقیقه و ثانیه
  u8g2.setFont(u8g2_font_6x10_tr); // فونت دلخواه برای جداکننده
  u8g2.drawStr(85, 35, ":");

  // نمایش AM/PM در صورت فعال بودن حالت 12 ساعته
  if (!is_24_hour_format)
  {
    // نمایش ثانیه
    char secondStr[3];
    snprintf(secondStr, sizeof(secondStr), "%02d", p_tm->tm_sec);
    u8g2.setFont(u8g2_font_6x10_tr); // فونت دلخواه برای ثانیه
    u8g2.drawStr(92, 30, secondStr);

    char amPmStr[3];
    snprintf(amPmStr, sizeof(amPmStr), "%s", p_tm->tm_hour >= 12 ? "pm" : "am");
    u8g2.setFont(u8g2_font_6x10_tr); // فونت دلخواه برای AM/PM
    u8g2.drawStr(92, 40, amPmStr);   // تنظیم مکان دلخواه برای AM/PM
  }
  else
  {
    // نمایش ثانیه
    char secondStr[3];
    snprintf(secondStr, sizeof(secondStr), "%02d", p_tm->tm_sec);
    u8g2.setFont(u8g2_font_6x13_tr); // فونت دلخواه برای ثانیه
    u8g2.drawStr(92, 35, secondStr);
  }

  // مدیریت تغییر بین تاریخ شمسی و میلادی
  unsigned long currentMillis = millis();
  if (currentMillis - lastSwitchTime > switchInterval)
  {
    showShamsiDate = !showShamsiDate;
    lastSwitchTime = currentMillis;
  }

  // نمایش تاریخ
  char dateStr[20];
  if (showShamsiDate)
  {
    // نمایش تاریخ شمسی
    if (todayDate.isEmpty())
    {
      snprintf(dateStr, sizeof(dateStr), "1111/22/33"); // مقدار پیش‌فرض در صورت خالی بودن
    }
    else
    {
      snprintf(dateStr, sizeof(dateStr), "%s", todayDate.c_str());
    }
  }
  else
  {
    // نمایش تاریخ میلادی
    snprintf(dateStr, sizeof(dateStr), "%04d/%02d/%02d", p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday);
  }

  u8g2.setFont(u8g2_font_6x13_tr);
  u8g2.drawStr(30, 60, dateStr);

  // ارسال به نمایشگر-
  u8g2.sendBuffer();
}

// تابع دریافت داده‌های اوقات شرعی
bool fetchPrayerTimes()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    client.setInsecure(); // غیرفعال کردن بررسی SSL

    HTTPClient http;
    http.begin(client, apiUrl);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
      String payload = http.getString();
      Serial.println("API Response:");
      Serial.println(payload);

      // استخراج داده‌های JSON
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      if (!error)
      {
        imsaakTime = doc["Imsaak"].as<String>().substring(0, 5);
        sunriseTime = doc["Sunrise"].as<String>().substring(0, 5);
        noonTime = doc["Noon"].as<String>().substring(0, 5);
        sunsetTime = doc["Sunset"].as<String>().substring(0, 5);
        maghrebTime = doc["Maghreb"].as<String>().substring(0, 5);
        midnightTime = doc["Midnight"].as<String>().substring(0, 5);
        todayDate = doc["Today"].as<String>().substring(0, 8);
        todayQamari = doc["TodayQamari"].as<String>();
        CityLName = doc["CityLName"].as<String>();
        return true;
      }
      else
      {
        Serial.println("Error parsing JSON!");
      }
    }
    else
    {
      Serial.print("Error connecting to API: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected!");
  }
  return false;
}

void displayPrayerTimes()
{
  static unsigned long previousMillis = 0; // زمان آخرین تغییر صفحه
  static bool showFirstPage = true;        // وضعیت نمایش صفحه
  unsigned long currentMillis = millis();  // زمان فعلی

  // تغییر صفحه هر ۲.۵ ثانیه
  if (currentMillis - previousMillis >= 3500)
  {
    previousMillis = currentMillis; // به‌روزرسانی زمان
    showFirstPage = !showFirstPage; // تغییر وضعیت صفحه
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr); // تنظیم فونت متن

  if (showFirstPage)
  {
    // صفحه دوم = اوقات عصر و تاریخ
    u8g2.drawStr(5, 10, ("Maghreb = " + maghrebTime).c_str());
    u8g2.drawStr(5, 25, ("Nimeh shab = " + midnightTime).c_str());
    u8g2.drawStr(5, 40, ("Qamari = " + todayQamari).c_str());
    u8g2.drawStr(5, 60, ("City : " + CityLName).c_str());
  }
  else
  {
    // صفحه اول: اوقات صبح و ظهر
    u8g2.drawStr(5, 10, ("Sobh = " + imsaakTime).c_str());
    u8g2.drawStr(5, 25, ("Toloe = " + sunriseTime).c_str());
    u8g2.drawStr(5, 40, ("Zohr = " + noonTime).c_str());
    u8g2.drawStr(5, 55, ("ghrob = " + sunsetTime).c_str());
  }

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

  // Initialize File System
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  server.on("/", handleWebConfig);
  server.on("/save", handleSaveConfig);
  server.on("/setAlarm", HTTP_POST, handleSetAlarm);
  server.begin();

  loadAlarms();
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
const unsigned long PRAYER_DISPLAY_DURATION = 7000;  // مدت نمایش اوقات شرعی (5 ثانیه)

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
    checkAlarms();
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