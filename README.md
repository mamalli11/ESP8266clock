# ESP8266 Wi-Fi Clock with OLED Display and WS2812 LED Indicator

این کد برای اتصال یک ماژول ESP8266 به شبکه Wi-Fi و دریافت زمان از یک سرور NTP (شبکه پروتکل زمانی) طراحی شده است. زمان و تاریخ به‌دست‌آمده از سرور روی یک نمایشگر OLED به‌طور زیبایی نمایش داده می‌شود و LED WS2812 رنگ مخصوص هر روز از هفته را نشان می‌دهد.

## ویژگی‌ها

### اتصال به Wi-Fi
در بخش اولیه‌ی کد، ماژول به شبکه Wi-Fi مشخص‌شده متصل می‌شود و پس از اتصال، آدرس IP ماژول روی نمایشگر OLED نمایش داده می‌شود.

### دریافت زمان از سرور NTP
پس از اتصال به اینترنت، زمان محلی از طریق سرورهای NTP تنظیم می‌شود (با تنظیم UTC+3:30 برای تهران).

### نمایش ساعت و تاریخ
- **ساعت و دقیقه** به شکلی خوانا و زیبا در مرکز نمایشگر نمایش داده می‌شوند.
- **ثانیه‌ها** نیز در کنار ساعت نمایش داده می‌شوند.
- **تاریخ روز** در پایین صفحه قرار گرفته است.

### تغییر رنگ LED بر اساس روز هفته
LED به رنگ خاصی برای هر روز هفته تغییر می‌کند (مثلاً قرمز برای یکشنبه، سبز برای دوشنبه، و غیره) که این امر می‌تواند در محیط‌های کم‌نور مفید باشد.

---

## English Code Explanation

This code is designed to connect an ESP8266 module to a Wi-Fi network and obtain the current time from an NTP (Network Time Protocol) server. The received date and time are displayed on an OLED screen in an elegant format, while an attached WS2812 LED lights up with a specific color for each day of the week.

### Features

#### Connecting to Wi-Fi
Initially, the ESP8266 module connects to the specified Wi-Fi network, and after a successful connection, the module’s IP address is displayed on the OLED screen.

#### Fetching Time from NTP Server
Once connected to the internet, the local time is synchronized from NTP servers (with an offset for UTC+3:30, set for Tehran).

#### Displaying Date and Time
- **Hour and Minute** are clearly and beautifully displayed in the center of the OLED screen.
- **Seconds** are displayed alongside the hour and minute.
- **Date** is displayed at the bottom of the screen.

#### LED Color Change Based on Day
The LED lights up with a specific color depending on the day of the week (e.g., red for Sunday, green for Monday, etc.), adding a visual cue that could be useful in low-light settings.

---


