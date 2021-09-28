#ifndef DISPATCHER
#define DISPATCHER

#include <stdint.h>
#include <stdlib.h>

class Handler
{
public:
    Handler(uint16_t(*func)(uint16_t) = {});
    uint16_t execute(uint16_t value_to_pass);
private:
    uint8_t key;
    uint16_t(*func)(uint16_t);
};

class Dispatcher
{
public:
    Dispatcher(uint8_t buffer_size);
    void attachHandle(Handler handle_user, uint8_t key);
    void readPackage(uint8_t* package, uint8_t size);
    uint8_t getHandleIndex(const uint8_t key);
private:
    uint8_t size;
    void handleKey(uint8_t key, uint16_t value);
    void addHandle(uint8_t key, uint16_t(*func)(uint16_t));
    uint8_t handler_index;
    Handler* handle_list;
    uint8_t* added_keys;
};

#endif