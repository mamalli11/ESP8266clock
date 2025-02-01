#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Adafruit_NeoPixel.h>
#include "Config.h"

class LEDController
{
public:
    LEDController();
    void initialize();
    void update(uint8_t dayOfWeek);
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void turnOff();

private:
    Adafruit_NeoPixel pixels;

    uint32_t getColorForDay(uint8_t day) const;
    uint32_t dimColor(uint32_t color, float factor) const;
};

#endif