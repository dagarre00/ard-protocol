#include "dispatcher.h"

Handler::Handler(uint16_t(*func)(uint16_t)) {
    this->func = func;
}

uint16_t Handler::execute(uint16_t value_to_pass) {
    return this->func(value_to_pass);
}

Dispatcher::Dispatcher(uint8_t buffer_size) {
    this->added_keys = new uint8_t[buffer_size];
    this->handle_list = new Handler[buffer_size];
    this->buffer_size = buffer_size;
}

void Dispatcher::attachHandle(Handler handle_user, uint8_t key) {
    if (this->handler_index == this->buffer_size) {
        throw Exception(dispatch_error::FULL); //Stack de handlers full
    }

    if (key == 0x00) {
        throw Exception(dispatch_error::INVALID_KEY);  // No permitir usar la key 0x00, para evitar lecturas en espacios vacios
    }
    
    this->handle_list[handler_index] = handle_user;
    this->added_keys[handler_index] = key;
    this->handler_index++;
}

void Dispatcher::readPackage(uint8_t* package, uint8_t size) {
    if (size % 3) {
        throw Exception(dispatch_error::INVALID_PAYLOAD); // Si el package tiene un tamaño incorrecto sacar una excepcion
    }

    if (handler_index == 0) {
        throw Exception(dispatch_error::NO_HANDLERS); // Si no hay handlers todavia, sacar una excepcion
    }

    uint8_t local_key = 0;
    uint16_t local_val = 0;

    for (int i = 0; i < size; i += 3) {
        local_key = package[i];
        local_val = (package[i + 1] << 8) | (package[i + 2]);
        this->handleKey(local_key, local_val);
    }
}

uint8_t Dispatcher::getHandleIndex(const uint8_t key) {
    for (int i = 0; i < this->handler_index; i++) {
        if (key == this->added_keys[i]) {
            return i;
        }
    }
    throw Exception(dispatch_error::NOT_FOUND);
}

void Dispatcher::handleKey(uint8_t key, uint16_t value) {
    this->handle_list[getHandleIndex(key)].execute(value);
}
