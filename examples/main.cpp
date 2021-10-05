#include <Arduino.h>

#include "datalink/datalink.h"
#include "package/package.h"
#include "dispatcher/dispatcher.h"

#include "bme280/bme280.h"
#include "rgb_led/rgb_led.h"

const uint8_t HUMIDITY_KEY = 0xA4;
const uint8_t TEMPERATURE_KEY = 0xA6;

const uint8_t RED_LED_KEY = 0xB4;
const uint8_t GREEN_LED_KEY = 0xB5;
const uint8_t BLUE_LED_KEY = 0xB6;

const uint8_t RED_LED_PIN = 13;
const uint8_t GREEN_LED_PIN = 12;
const uint8_t BLUE_LED_PIN = 14;

Datalink datalink = Datalink(0x7E, 90);
uint16_t send_freq = 3000;
uint16_t read_freq = 100;
Dispatcher dispatch = Dispatcher(255);

BME280 sensor((const uint8_t)0x76);
Rgb_Led led(RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN);

long timer = millis();

bool parsed_data = false;

Package dataParser(Datalink link, Stream& uart);

static Package pack = dataParser(datalink, Serial);

// Funciones del dispatcher:

uint16_t setRedLed(uint16_t val) {
    uint8_t red_led_value = (uint8_t)val;
    led.setRed(red_led_value);
    return 0;
}

// Obtener paquete desde serial:

Package dataParser(Datalink link, Stream& uart) {
    link.read(uart);
    uint8_t size = link.available();
    if (size) {
        parsed_data = true;
        return Package(link.getPayload(), size);
    }
    else {
        if (parsed_data)
            parsed_data = false; // evitar realizar escrituras en cada ciclo
        return Package(0);
    }
}

//Funciones FreeRTOS (Multi task):

void parseDataTask(void* parameter) {
    TickType_t xLastWakeTime;            // Variable necesaria para vTaskDelayUntil()
    xLastWakeTime = xTaskGetTickCount(); // Funcion similar a millis()

    while (true) {
        vTaskDelayUntil(&xLastWakeTime, read_freq / portTICK_PERIOD_MS); // Similar a delay()
        pack = dataParser(datalink, Serial); // Crea el pack de data

        if (parsed_data) {
            // Buscar key unica
            if (pack.hasValue(0x0A)) {
                Package output = Package(3);
                output.addData(0x0B, pack.getValue(0x0A));
                datalink.send(output.dump(), output.getSize(), Serial);
            }

            // Ejecutar el dispatcher:
            try {
                dispatch.readPackage(pack.dump(), pack.getSize());
            }
            catch (Exception& err) {
                Serial.print("Codigo de error en funcion parseDataTask()");
                Serial.println(err.getCode());
            }

        }
    }
}

void sendDataTask(void* parameter) {
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();    
    while (true) {
        Package output = Package(12);
        output.addData(HUMIDITY_KEY, sensor.getHumidity());
        output.addData(TEMPERATURE_KEY, sensor.getHumidity());
        datalink.send(output.dump(), output.getSize(), Serial);
        vTaskDelayUntil(&xLastWakeTime, send_freq / portTICK_PERIOD_MS);
    }
}

//Inicio Arduino:

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(1000);
    
    led.setRed(200);
    vTaskDelay(5000);
    led.setRed(0);
    led.setBlue(200);
    vTaskDelay(5000);
    led.setBlue(0);
    led.setGreen(200);
    vTaskDelay(5000);
    led.setGreen(0);

    dispatch.attachHandle(Handler(&setRedLed), RED_LED_KEY);   

    xTaskCreate(parseDataTask, "Parse data periodically", 4096, NULL, 1, NULL);
    xTaskCreate(sendDataTask, "Send data periodically", 4096, NULL, 1, NULL);
}

void loop() {
    vTaskDelete(NULL); // Las tasks fueron creadas en el setup(), no hay necesidad de usar loop()
}