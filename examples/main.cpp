#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "datalink/datalink.h"
#include "package/package.h"
#include "dispatcher/dispatcher.h"

#include "bme280/bme280.h"
#include "rgb_led/rgb_led.h"

//KEYS
const uint8_t HUMIDITY_KEY = 0xA4;
const uint8_t TEMPERATURE_KEY = 0xA6;

const uint8_t RED_LED_KEY = 0xB4;
const uint8_t GREEN_LED_KEY = 0xB5;
const uint8_t BLUE_LED_KEY = 0xB6;

//PINES
const uint8_t RED_LED_PIN = 13;
const uint8_t GREEN_LED_PIN = 12;
const uint8_t BLUE_LED_PIN = 14;

//OBJETOS GLOBALES
Datalink datalink = Datalink(0x7E, 90);
uint16_t send_freq = 3000;
uint16_t read_freq = 100;
Dispatcher dispatch = Dispatcher(255);
Rgb_Led led(RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN);
Package dataParser(Datalink link, Stream& uart);

//OBJETOS GLOBALES: MQTT Y WIFI
WiFiClient wifiClient;
PubSubClient mqtt("broker.hivemq.com", 1883, {}, wifiClient);
const char* TOPIC_SENSOR = "SDA/Equipo_DMS/Sensor";
const char* MSG_TEMPLATE = "{\n  \"Temperatura\": %.1f,\n  \"Humedad\": %.1f\n}\0";

//VARIABLES GLOBALES
long timer = millis();
bool parsed_data = false;
static Package pack = dataParser(datalink, Serial);
//VARIABLES GLOBALES: MQTT Y WIFI
const char ssid[] = "DANIEL";
const char pass[] = "1037653263D";
char msg[64];

// Funciones del dispatcher:

uint16_t setRedLed(uint16_t val) {
    uint8_t red_led_value = (uint8_t)val;
    led.setRed(red_led_value);
    return 0;
}

uint16_t setGreenLed(uint16_t val) {
    uint8_t green_led_value = (uint8_t)val;
    led.setGreen(green_led_value);
    return 0;
}

uint16_t setBlueLed(uint16_t val) {
    uint8_t blue_led_value = (uint8_t)val;
    led.setBlue(blue_led_value);
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

// Iniciar MQTT
void setupMQTT() {
    Serial.print("Conectando a SSID: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        // failed, retry
        Serial.print(".");
        delay(500);
    }
    if (!mqtt.connect("ESP32-Client")) {
        Serial.print("MQTT connection failed!");
    }
    else {
        mqtt.subscribe(TOPIC_SENSOR);
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
    BME280 sensor((const uint8_t)0x76);
    while (true) {
        Package output = Package(6);
        float temp = sensor.getTemperature();
        float hum = sensor.getHumidity();

        //Enviar por serial con Datalink
        output.addData(HUMIDITY_KEY, (uint16_t)round(hum));
        output.addData(TEMPERATURE_KEY, (uint16_t)round(temp));
        datalink.send(output.dump(), output.getSize(), Serial);

        //Enviar por MQTT
        snprintf(msg, 64, MSG_TEMPLATE, temp, hum);
        mqtt.publish(TOPIC_SENSOR, msg);

        vTaskDelayUntil(&xLastWakeTime, send_freq / portTICK_PERIOD_MS);
    }
}

//Inicio Arduino:

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(1000);
    
    led.setRed(0);
    led.setBlue(0);
    led.setGreen(0);

    dispatch.attachHandle(Handler(&setRedLed), RED_LED_KEY);
    dispatch.attachHandle(Handler(&setGreenLed), GREEN_LED_KEY);
    dispatch.attachHandle(Handler(&setBlueLed), BLUE_LED_KEY);

    setupMQTT();

    xTaskCreate(parseDataTask, "Parse data periodically", 4096, NULL, 1, NULL);
    xTaskCreate(sendDataTask, "Send data periodically", 4096, NULL, 1, NULL);
}

void loop() {
    vTaskDelete(NULL); // Las tasks fueron creadas en el setup(), no hay necesidad de usar loop()
}