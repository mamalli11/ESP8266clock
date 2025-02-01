#ifndef CONFIG_H
#define CONFIG_H

// اعلام متغیرها با extern
extern const char* DEFAULT_SSID;
extern const char* DEFAULT_PASSWORD;

// API Keys
extern const char* WEATHER_API_KEY;         // کلید API آب‌وهوا
extern const char* PRAYER_TIMES_API_URL;    // آدرس API اوقات شرعی

// پین‌ها و تنظیمات دیگر
#define LED_PIN D5          // پین متصل به LED نئوپیکسل
#define BUZZER_PIN D7       // پین متصل به بوزر
#define LIGHT_SENSOR_PIN D8 // پین متصل به سنسور نور

// Display Settings
#define DISPLAY_WIDTH 128 // عرض نمایشگر
#define DISPLAY_HEIGHT 64 // ارتفاع نمایشگر

// Time Settings
const int TIMEZONE_OFFSET = 12600;  // افست زمانی بر حسب ثانیه (برای تهران: 3:30 * 3600)

#endif