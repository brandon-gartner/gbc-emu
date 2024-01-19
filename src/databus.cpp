#include "databus.h"

#include "emulator.h"

databus::databus(emulator* emu) : emu(emu) {}

uint8_t databus::read8(uint16_t address) {
    if (address <= 0x8000) {
        // reading from rom bank 1 or 2, can just read from cart
        uint8_t value = emu->cart->read_cart(address);
        return value;
    } else if (address < 0xA000) {
        std::cout << "Address is too large for current accepted cartridges." << std::endl;
        std::cout << "Do not support map or character data loading yet." << std::endl;
        NOT_IMPLEMENTED()
    } else if (address < 0xC000) {
        // reading from vram/cart ram
        // this is on the cartridge like before, can just read from cart
        return emu->cart->read_cart(address);
    } else if (address < 0xE000) {
        // reading from ram banks/wram
        std::cout << "Reading from ram banks/wram." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFE00) {
        // reading from echo ram
        std::cout << "Reading from echo ram." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFEA0) {
        // reading from oam
        std::cout << "Reading from oam." << std::endl;
        NOT_IMPLEMENTED();
    }
    return emu->mem->hram_read(address);
}

void databus::write8(uint16_t address, uint8_t value) {
    if (address <= 0x8000) {
        // writing to rom bank 1 or 2, can just write to cart
        std::cout << "writing value " << std::to_string(value) << std::endl;
        emu->cart->write_cart(value, address);
    } else if (address < 0xA000) {
        std::cout << "Do not currently support character/map data." << std::endl;
        NOT_IMPLEMENTED()
    } else if (address < 0xC000) {
        // writing to vram/cart ram
        // this is on the cartridge like before, can just write to cart
        emu->cart->write_cart(value, address);
    } else if (address < 0xE000) {
        // writing to ram banks/wram
        std::cout << "Writing to ram banks/wram." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFE00) {
        // writing to echo ram
        std::cout << "Writing to echo ram." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFEA0) {
        // writing to oam
        std::cout << "Writing to oam." << std::endl;
        NOT_IMPLEMENTED();
    }
    emu->mem->hram_write(address, value);
}

uint16_t databus::read16(uint16_t address) {
    return append(read8(address), read8(address + 1));
}

void databus::write16(uint16_t address, uint16_t value) {
    // TODO: verify 0xFF?
    write8(address + 1, (value >> 8) & 0xFF);
    write8(address, value & 0xFF);
}