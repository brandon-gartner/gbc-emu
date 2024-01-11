#include "databus.h"

#include "emulator.h"

databus::databus(emulator* emu) : emu(emu) {}

uint8_t databus::read(uint16_t address) {
    if (address <= 0x8000) {
        // reading from rom bank 1 or 2, can just read from cart
        uint8_t value = emu->cart->read_cart(address);
        return value;
    } else {
        NOT_IMPLEMENTED()
    }
    return -5;
}

bool databus::write(uint8_t value, uint16_t address) {
    if (address <= 0x8000) {
        // writing to rom bank 1 or 2, can just write to cart
        std::cout << "writing value " << std::to_string(value) << std::endl;
        return emu->cart->write_cart(value, address);
    } else {
        NOT_IMPLEMENTED()
    }
    return false;
}