#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>
#include "driver/ledc.h"
#include "esp_err.h"


class Rgb_Led
{
public:
    Rgb_Led(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin);
    void setRed(uint8_t value);
    void setGreen(uint8_t value);
    void setBlue(uint8_t value);
private:
    const int redPWMFreq = 5000; /* 5 KHz */
    const int redPWMChannel = LEDC_CHANNEL_0;
    const int redPWMResolution = LEDC_TIMER_8_BIT;
    const int RED_MAX_DUTY_CYCLE = (int)(pow(2, redPWMResolution) - 1);

    const int greenPWMFreq = 5000; /* 5 KHz */
    const int greenPWMChannel = LEDC_CHANNEL_2;
    const int greenPWMResolution = LEDC_TIMER_8_BIT;
    const int GREEN_MAX_DUTY_CYCLE = (int)(pow(2, greenPWMResolution) - 1);

    const int bluePWMFreq = 5000; /* 5 KHz */
    const int bluePWMChannel = LEDC_CHANNEL_4;
    const int bluePWMResolution = LEDC_TIMER_8_BIT;
    const int BLUE_MAX_DUTY_CYCLE = (int)(pow(2, bluePWMResolution) - 1);
};


#endif