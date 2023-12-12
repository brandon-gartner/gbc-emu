#include <iostream>

#include "emulator.h"

emulator::status emulator::get_status() {
    return this->current;
}

void emulator::set_status(emulator::status new_status) {
    this->current = new_status;
}

void emulator::start() {
    std::cout << "TEMP" << std::endl;
}