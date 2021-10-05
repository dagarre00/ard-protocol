#include "bme280.h"

BME280::BME280(const uint8_t i2c_addr) {
    this->bme280_i2c_addr = i2c_addr;
    this->has_inited = this->bme.begin(this->bme280_i2c_addr);

    if (!this->has_inited) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
    }
}

float BME280::getTemperature() {
    return this->bme.readTemperature();
}

float BME280::getHumidity() {
    return this->bme.readHumidity();
}

