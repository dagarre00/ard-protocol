#ifndef DISPATCHER
#define DISPATCHER

#include <stdint.h>
#include <stdlib.h>
#include "exception.h"

namespace dispatch_error {
    static const int FULL = 1;
    static const int EMPTY = 2;
    static const int NOT_FOUND = 3;
    static const int INVALID_PAYLOAD = 4;
    static const int INVALID_KEY = 5;
    static const int NO_HANDLERS = 6;
};

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
    uint8_t buffer_size;
    void handleKey(uint8_t key, uint16_t value);
    void addHandle(uint8_t key, uint16_t(*func)(uint16_t));
    uint8_t handler_index = 0;
    Handler* handle_list;
    uint8_t* added_keys;
};

#endif