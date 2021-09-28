#include <Arduino.h>
#include "datalink/datalink.h"
#include "package/package.h"
#include "dispatcher/dispatcher.h"

Datalink datalink = Datalink(0x7E, 90);
uint16_t send_freq = 3000;
uint16_t read_freq = 100;
Dispatcher dispatch = Dispatcher(255);

long timer = millis();

const uint8_t key1 = 0xA4;
const uint8_t key2 = 0xA6;

bool parsed_data = false;

Package dataParser(Datalink link, Stream &uart);

static Package pack = dataParser(datalink, Serial);

// Funciones del dispatcher:

uint16_t funcion_prueba_1(uint16_t val)
{
    Serial.print("Funcion prueba 1 con valor: ");
    Serial.println(val);
    return 0;
}

uint16_t funcion_prueba_2(uint16_t val)
{
    Serial.print("Funcion prueba 2 con valor: ");
    Serial.println(val);
    return 0;
}

// Obtener paquete desde serial:

Package dataParser(Datalink link, Stream &uart)
{
    link.read(uart);
    uint8_t size = link.available();
    if (size)
    {
        parsed_data = true;
        return Package(link.getPayload(), size);
    }
    else
    {
        if (parsed_data) parsed_data = false; // evitar realizar escrituras en cada ciclo
        return Package(0);
    }
}

//Funciones FreeRTOS (Multi task):

void parseDataTask(void *parameter)
{
    TickType_t xLastWakeTime; // Variable necesaria para vTaskDelayUntil()
    xLastWakeTime = xTaskGetTickCount(); // Funcion similar a millis()
    while (true)
    {
        vTaskDelayUntil(&xLastWakeTime, read_freq / portTICK_PERIOD_MS); // Similar a delay()
        pack = dataParser(datalink, Serial);
    }
}

void checkForKeyTask(void *parameter)
{
    while (true)
    {
        if (parsed_data) {
            if (pack.hasValue(0x0A))
            {
                Package output = Package(3);
                output.addData(0x0B, pack.getValue(0x0A));
                datalink.send(output.dump(), output.getSize(), Serial);
            }
        }
        vTaskDelay(read_freq / portTICK_PERIOD_MS);
    }
}

void dispatchTask(void *parameter)
{
    while (true)
    {
        if (parsed_data)
        {
            dispatch.readPackage(pack.dump(), pack.getSize());
        }
        vTaskDelay(read_freq / portTICK_PERIOD_MS);
    }
}

void sendDataTask(void *parameter)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount ();
    while (true)
    {
        Package output = Package(3);
        output.addData(0xA4, 0);
        datalink.send(output.dump(), output.getSize(), Serial);
        vTaskDelayUntil(&xLastWakeTime, send_freq / portTICK_PERIOD_MS);
    }
}

//Inicio Arduino:

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(1000);

    dispatch.attachHandle(Handler(*funcion_prueba_1), key1);
    dispatch.attachHandle(Handler(*funcion_prueba_2), key2);

    Serial.println("Attach handle OK");        

    xTaskCreate(parseDataTask, "Parse data periodically", 4096, (void *)&pack, 1, NULL);
    xTaskCreate(checkForKeyTask, "Check for specific key", 4096, (void *)&pack, 1, NULL);
    xTaskCreate(dispatchTask, "Process the Package through dispatcher", 4096, (void *)&pack, 1, NULL);
    xTaskCreate(sendDataTask, "Send data periodically", 4096, NULL, 1, NULL);

}

void loop()
{
    vTaskDelete(NULL);
}