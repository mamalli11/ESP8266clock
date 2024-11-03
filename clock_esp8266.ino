#include <ESP8266WiFi.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

const char* ssid = "------"; // نام شبکه Wi-Fi
const char* password = "------"; // پسورد شبکه Wi-Fi

int ledPin = D5;
Adafruit_NeoPixel pixel(1, ledPin, NEO_GRB + NEO_KHZ800);

int timezone = 3 * 3600 + 30 * 60;
int dst = 0;

uint32_t dimColor(uint8_t red, uint8_t green, uint8_t blue, float dimFactor) {
    return pixel.Color(red * dimFactor, green * dimFactor, blue * dimFactor);
}

void setup() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    pixel.begin();
    pixel.show();

    display.clearDisplay();
    display.display();

    Serial.begin(115200);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Connecting to WiFi...");
    display.display();

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        display.print(".");
        display.display();
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(1000);

    configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");

    while (!time(nullptr)) {
        Serial.print("*");
        delay(1000);
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.println("Ready!");
    display.display();
    delay(1000);

    display.clearDisplay();
    display.display();
}

void loop() {
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);

    display.clearDisplay();

    // نمایش ساعت
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(20, 0); // موقعیت مرکز برای ساعت
    if (p_tm->tm_hour < 10) display.print("0");
    display.print(p_tm->tm_hour);
    display.print(":");
    if (p_tm->tm_min < 10) display.print("0");
    display.print(p_tm->tm_min);

    // نمایش ثانیه‌ها
    display.setTextSize(1,2);
    display.setCursor(110, 7);
    display.print(":");
    if (p_tm->tm_sec < 10) display.print("0");
    display.print(p_tm->tm_sec);

    // نمایش تاریخ در پایین صفحه
    display.setTextSize(2,1);
    display.setCursor(10, 25);
    display.print(p_tm->tm_mday);
    display.print("/");
    display.print(p_tm->tm_mon + 1);
    display.print("/");
    display.print(p_tm->tm_year + 1900);

    display.display();

    // تنظیم رنگ LED بر اساس روز هفته
    int dayOfWeek = p_tm->tm_wday;
    float dimFactor = 0.1;
    switch (dayOfWeek) {
        case 0: pixel.fill(dimColor(255, 0, 0, dimFactor)); break;
        case 1: pixel.fill(dimColor(0, 255, 0, dimFactor)); break;
        case 2: pixel.fill(dimColor(0, 0, 255, dimFactor)); break;
        case 3: pixel.fill(dimColor(255, 255, 0, dimFactor)); break;
        case 4: pixel.fill(dimColor(255, 0, 255, dimFactor)); break;
        case 5: pixel.fill(dimColor(0, 255, 255, dimFactor)); break;
        case 6: pixel.fill(dimColor(255, 165, 0, dimFactor)); break;
    }
    pixel.show();

    delay(1000);
}
