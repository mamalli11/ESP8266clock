# 🚀 **ESP8266 Smart Wi-Fi Clock with OLED Display and WS2812 LED**

This project is a **smart and interactive clock** built using the ESP8266 module. It combines essential timekeeping features with a modern design and advanced functionalities, such as alarms, weather updates, dynamic animations, and web-based configuration. The device uses an OLED display for a sleek time and date presentation, while the WS2812 LED enhances the experience with colorful visual cues.

---

## ✨ **Features Overview**

### 🌐 **Wi-Fi Connectivity**

The ESP8266 connects to your Wi-Fi network, enabling it to:

- Fetch real-time time data from an NTP server.
- Retrieve live weather information via APIs.
- Host a web-based configuration interface for user settings.

Once connected, the clock displays its IP address on the OLED screen.

---

### ⏰ **Clock and Date Display**

The OLED screen showcases:

- **Time:** Hours and minutes are prominently displayed.
- **Seconds:** Updated dynamically for precision.
- **Date:** Day, month, and year are shown in an elegant format.

The display transitions smoothly between time, date, and other features using eye-catching animations.

---

### 🎨 **Dynamic LED Indicator**

A WS2812 RGB LED provides visual feedback for the day of the week:

- Each day is represented by a unique color, such as red for Sunday, green for Monday, and blue for Tuesday.
- The LED brightness is adjustable and complements the clock's aesthetic.

---

### 🔆 **Ambient Light Sensing and Brightness Control**

The clock includes an ambient light sensor to adjust the OLED brightness automatically based on the surrounding light. Users can also manually configure brightness levels through the web interface.

---

### ⏳ **Alarms Timer**

The clock supports multiple alarms, all configurable via the web interface:

- **Alarms:** Set daily alarms with customizable times.

The alarms and timers are saved to persistent memory, ensuring settings remain intact even after power loss.

---

### 🌤️ **Weather Updates**

Stay informed about local weather conditions with live data fetched from an API:

- **Temperature:** Displayed in degrees Celsius.
- **Humidity:** Monitored to provide current atmospheric conditions.
- **Weather Conditions:** Indications like sunny, cloudy, or rainy weather.

Weather data is updated periodically whenever the device connects to the internet.

---

### 🌐 **Web-Based Control Panel**

A user-friendly web interface allows you to:

- Configure Wi-Fi credentials.
- Set alarms.
- Adjust display brightness and other settings.
- Enable or disable specific features.

Simply connect to the clock's IP address to access the control panel from any browser.

---

### 💾 **Persistent Settings with LittleFS**

All user settings, including alarms, Wi-Fi credentials, and brightness preferences, are stored using the ESP8266's LittleFS file system. This ensures:

- Settings are preserved across reboots.
- Users don’t need to reconfigure the device after power loss.

---

## 🔧 **How to Set Up**

### 1️⃣ **Hardware Requirements:**

- ESP8266 (e.g., NodeMCU)
- OLED Display (e.g., SSD1306)
- WS2812 RGB LED
- Ambient Light Sensor
- Buzzer (for alarms)

### 2️⃣ **Software Requirements:**

- Arduino IDE with the following libraries installed:
  - `olikraus/U8g2@^2.36.2`
  - `bblanchon/ArduinoJson@^7.2.0`
  - `adafruit/Adafruit NeoPixel@^1.12.3`
  - `adafruit/Adafruit Unified Sensor@^1.1.14`
  - `arduino-libraries/ArduinoHttpClient@^0.6.1`
  - `rweather/Crypto@^0.4.0`
  - `espressif8266`
  - `LittleFS`

### 3️⃣ **Steps:**

1. Rename the `Config.Example.txt` file to `Config.cpp` and enter the requested values.
2. Connect your hardware components as per the wiring diagram.
3. Edit the code to include your Wi-Fi credentials and preferred settings.
4. Upload the code to your ESP8266 using Arduino IDE.
5. Access the web interface to personalize your clock settings.

---

## 📜 **Why This Clock?**

This smart clock is more than just a timepiece. It’s a versatile, visually engaging, and highly functional device that brings together:

- Cutting-edge technology.
- Aesthetic design.
- Practical usability for everyday life.

Build this project and make your workspace or room smarter and more connected!

---

### ❤️ **Contribute & Improve**

Feel free to fork this repository, add your features, and share your updates! Let’s make this project even better together. 😊

---

🌟 **Enjoy your smart clock** 🌟
