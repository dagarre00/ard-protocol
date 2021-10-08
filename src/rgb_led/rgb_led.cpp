#include "rgb_led.h"

Rgb_Led::Rgb_Led(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin) {
    ledcSetup(this->redPWMChannel, this->redPWMFreq, this->redPWMResolution);
    ledcSetup(this->greenPWMChannel, this->greenPWMFreq, this->greenPWMResolution);
    ledcSetup(this->bluePWMChannel, this->bluePWMFreq, this->bluePWMResolution);

    ledcAttachPin(red_pin, this->redPWMChannel);
    ledcAttachPin(green_pin, this->greenPWMChannel);
    ledcAttachPin(blue_pin, this->bluePWMChannel);
}

void Rgb_Led::setRed(uint8_t value) {
    uint16_t redDutyCycle = map(value, 0, 255, 0, this->RED_MAX_DUTY_CYCLE);
    ledcWrite(redPWMChannel, redDutyCycle);
}

void Rgb_Led::setGreen(uint8_t value) {
    uint16_t greenDutyCycle = map(value, 0, 255, 0, this->GREEN_MAX_DUTY_CYCLE);
    ledcWrite(greenPWMChannel, greenDutyCycle);
}

void Rgb_Led::setBlue(uint8_t value) {
    uint16_t blueDutyCycle = map(value, 0, 255, 0, this->BLUE_MAX_DUTY_CYCLE);
    ledcWrite(bluePWMChannel, blueDutyCycle);
}

