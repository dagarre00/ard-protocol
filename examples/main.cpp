#include <Arduino.h>
#include "datalink/datalink.h"
#include "package/package.h"
#include "dispatcher/dispatcher.h"

Datalink datalink = Datalink(0x7E, 90);
uint16_t send_freq = 3000;
Dispatcher dispatch = Dispatcher(255);

long timer = millis();

const uint8_t key1 = 0xA4;
const uint8_t key2 = 0xA6;

uint16_t funcion_prueba_1 (uint16_t val) {
    Serial.println("Funcion prueba 1");
    return 0;
}

uint16_t funcion_prueba_2 (uint16_t val) {
    Serial.println("Funcion prueba 2");
    return 0;
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(1000);

    dispatch.attachHandle(Handler(*funcion_prueba_1),key1);
    dispatch.attachHandle(Handler(*funcion_prueba_2),key2);

    Serial.println("Attach handle OK");
}

Package dataParser(Datalink link, Stream &uart) {
    link.read(uart);
    uint8_t size = link.available();
    if (size){
        return Package(link.getPayload(), size);
    } else {
        return Package(0);
    }
}


void loop() {
    Package pack = dataParser(datalink, Serial);

    if (pack.hasValue(0x0A)){
        Package output = Package(3);
        output.addData(0x0B, pack.getValue(0x0A));
        datalink.send(output.dump(), output.getSize(), Serial);
    } 

    if (pack.getIndex() != 0){
        dispatch.readPackage(pack.dump(), pack.getSize());
    }


    if (millis() - timer >= send_freq) {
        timer = millis();
        Package output = Package(3);
        output.addData(0x0A, 10);
        datalink.send(output.dump(), output.getSize(), Serial);
    }
    
    delay(100);
}