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

Package dataParser(Datalink link, Stream &uart);

static Package pack = dataParser(datalink, Serial);

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

Package dataParser(Datalink link, Stream &uart)
{
    link.read(uart);
    uint8_t size = link.available();
    if (size)
    {
        return Package(link.getPayload(), size);
    }
    else
    {
        return Package(0);
    }
}

void parseDataTask(void *parameter)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount ();
    Package *pack_ptr = (Package *)parameter;
    while (true)
    {
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
        *pack_ptr = dataParser(datalink, Serial);
    }
}

void checkForKeyTask(void *parameter)
{
    Package *pack_ptr = (Package *)parameter;
    while (true)
    {
        if (pack_ptr->hasValue(0x0A))
        {
            Package output = Package(3);
            output.addData(0x0B, pack_ptr->getValue(0x0A));
            datalink.send(output.dump(), output.getSize(), Serial);
        }
        vTaskDelay(send_freq / portTICK_PERIOD_MS);
    }
}

void dispatchTask(void *parameter)
{
    Package *pack_ptr = (Package *)parameter;
    while (true)
    {
        if (pack_ptr->getIndex() != 0)
        {
            dispatch.readPackage(pack_ptr->dump(), pack_ptr->getSize());
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void sendDataTask(void *parameter)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount ();
    while (true)
    {
        if (millis() - timer >= send_freq)
        {
            timer = millis();
            Package output = Package(3);
            output.addData(0xA4, 0);
            datalink.send(output.dump(), output.getSize(), Serial);
        }
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
    }
}

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