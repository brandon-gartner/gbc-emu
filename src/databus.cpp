#include "databus.h"

#include "emulator.h"

databus::databus(emulator* emu) : emu(emu) {}

uint8_t databus::read8(uint16_t address) {
    if (address <= 0x8000) {
        // reading from rom bank 1 or 2, can just read from cart
        uint8_t value = emu->cart->read_cart(address);
        return value;
    } else {
        std::cout << "Address is too large for current accepted cartridges." << std::endl;
        NOT_IMPLEMENTED()
    }
    return -5;
}

void databus::write8(uint16_t address, uint8_t value) {
    if (address <= 0x8000) {
        // writing to rom bank 1 or 2, can just write to cart
        std::cout << "writing value " << std::to_string(value) << std::endl;
        emu->cart->write_cart(value, address);
    } else {
        std::cout << "Address is too large for current accepted cartridges." << std::endl;
        NOT_IMPLEMENTED()
    }
}

uint16_t databus::read16(uint16_t address) {
    return append(read8(address), read8(address + 1));
}

void databus::write16(uint16_t address, uint16_t value) {
    // TODO: verify 0xFF?
    write8(address + 1, (value >> 8) & 0xFF);
    write8(address, value & 0xFF);
}