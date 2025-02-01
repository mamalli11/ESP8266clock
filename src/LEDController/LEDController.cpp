#include "LEDController.h"

LEDController::LEDController() : pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800) {}

void LEDController::initialize()
{
    pixels.begin();
    pixels.show(); // Initialize all pixels to 'off'
}

void LEDController::update(uint8_t dayOfWeek)
{
    uint32_t color = getColorForDay(dayOfWeek);
    pixels.setPixelColor(0, dimColor(color, 0.1f)); // Dim the color for better visibility
    pixels.show();
}

void LEDController::setColor(uint8_t red, uint8_t green, uint8_t blue)
{
    pixels.setPixelColor(0, pixels.Color(red, green, blue));
    pixels.show();
}

void LEDController::turnOff()
{
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Turn off the LED
    pixels.show();
}

uint32_t LEDController::getColorForDay(uint8_t day) const
{
    const uint32_t colors[7] = {
        pixels.Color(255, 0, 0),   // Sunday - Red
        pixels.Color(0, 255, 0),   // Monday - Green
        pixels.Color(0, 0, 255),   // Tuesday - Blue
        pixels.Color(255, 255, 0), // Wednesday - Yellow
        pixels.Color(255, 0, 255), // Thursday - Magenta
        pixels.Color(0, 255, 255), // Friday - Cyan
        pixels.Color(255, 165, 0)  // Saturday - Orange
    };
    return colors[day % 7];
}

uint32_t LEDController::dimColor(uint32_t color, float factor) const
{
    uint8_t r = (color >> 16) * factor;
    uint8_t g = (color >> 8) * factor;
    uint8_t b = color * factor;
    return pixels.Color(r, g, b);
}