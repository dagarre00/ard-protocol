#include "dispatcher.h"

Handler::Handler(uint16_t(*func)(uint16_t)) {
    this->func = func;
}

uint16_t Handler::execute(uint16_t value_to_pass) {
    return this->func(value_to_pass);
}

Dispatcher::Dispatcher(uint8_t buffer_size) {
    this->handler_index = 0;
    this->added_keys = new uint8_t[buffer_size];
    this->handle_list = new Handler[buffer_size];
}

void Dispatcher::attachHandle(Handler handle_user, uint8_t key) {
    handle_list[handler_index] = handle_user;
    added_keys[handler_index] = key;
    this->handler_index++;
}

void Dispatcher::readPackage(uint8_t* package, uint8_t size) {
    uint8_t local_key = 0;
    uint16_t local_val = 0;

    if (size % 3) {
        return;
    }

    for (int i = 0; i < size; i += 3) {
        local_key = package[i];
        local_val = ((uint16_t)package[i + 2] << 8) | package[i + 1];
        this->handleKey(local_key, local_val);
    }
}

uint8_t Dispatcher::getHandleIndex(const uint8_t key) {
    for (int i = 0; i < this->handler_index; i++) {
        if (key == this->added_keys[i]) {
            return i;
            break;
        }
    }
    return -1;
}

void Dispatcher::handleKey(uint8_t key, uint16_t value) {
    uint8_t handler_index = getHandleIndex(key);
    Handler local_handler = handle_list[handler_index];
    local_handler.execute(value);
}
