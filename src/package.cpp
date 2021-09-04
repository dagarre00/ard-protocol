#include "package.h"
#include "exception.h"

Package::Package(uint8_t size) {
    this->size = size;
    this->buffer = new uint8_t[this->size];
    this->index = 0;
}

void Package::loads(uint8_t* payload, uint8_t size){
    for (int i=0; i<size; i+=3) {
        try {
            this->addData(payload[i], payload[i+1], payload[i+2]);
        } 
        catch(...) {
            throw Exception(package_error::INVALID_PAYLOAD);
        }
    }
}

void Package::addData(uint8_t key, uint8_t msb, uint8_t lsb) {
    if(this->index > (this->size - 3)){
        throw Exception(package_error::FULL);
    }
    this->buffer[this->index] = key; this->index++;
    this->buffer[this->index] = msb; this->index++;
    this->buffer[this->index] = lsb; this->index++;    
}

void Package::addData(uint8_t key, uint16_t data) {
    uint8_t data_msb = uint8_t(data >> 8);
    uint8_t data_lsb = uint8_t(data);
    this->addData(key, data_msb, data_lsb);
}

int Package::findValueIndex(uint8_t key) {
    for(int i=0; i<this->index; i+=3){
        if (uint8_t(key) == this->buffer[i]) {
            return i;
        }
    }
    return -1;
}

bool Package::hasValue(uint8_t key) {
    return (this->findValueIndex(key) != -1);
}

uint16_t Package::getValue(uint8_t key) {
    int index = this->findValueIndex(key);
    if (index == -1) {
        throw Exception(package_error::NOT_FOUND);
    }
    return (this->buffer[index+1] << 8) | (this->buffer[index + 2]);
}

uint8_t Package::getSize(){
    return this->size;
}

uint8_t* Package::dump() {
    uint8_t* payload = new uint8_t[this->index];
    for(int i=0; i<this->index; i++){
        payload[i] = this->buffer[i];
    }
    return payload;
}

uint8_t Package::getIndex(){
    return this->index;
}