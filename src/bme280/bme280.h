#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

class BME280 {
    public:
        BME280(const uint8_t i2c_addr);
        float getTemperature();
        float getHumidity();
    private:
        float temperature = 0;
        float humidity = 0;
        Adafruit_BME280 bme;
        uint8_t bme280_i2c_addr = 0x76;
        bool has_inited = false;
};

#endif