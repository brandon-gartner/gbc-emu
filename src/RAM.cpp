#include "RAM.h"

uint8_t ram::wram_read(uint16_t address) {
    if (address < 0xC000 || address >= 0xE000) {
        std::cout << "Invalid wram address received, " << int_to_hex(address) << std::endl;
        exit(1);
    }
    return wram[address - 0xE000];
}

void ram::wram_write(uint16_t address, uint8_t value) {
    if (address < 0xC000 || address >= 0xE000) {
        std::cout << "Invalid wram address received, " << int_to_hex(address) << std::endl;
        exit(1);
    }
    wram[address - 0xE000] = value;
}

uint8_t ram::hram_read(uint16_t address) {
    if (address < 0xFF80 || address == 0xFFFF) {
        std::cout << "Invalid hram address received, " << int_to_hex(address) << std::endl;
        exit(1);
    }
    return hram[address - 0xFF80];
}

void ram::hram_write(uint16_t address, uint8_t value) {
    if (address < 0xFF80 || address == 0xFFFF) {
        std::cout << "Invalid hram address received, " << int_to_hex(address) << std::endl;
        exit(1);
    }
    hram[address - 0xFF80] = value;
}

uint8_t ram::get_interrupt() {
    return interrupt;
}

void ram::set_interrupt(uint8_t value) {
    interrupt = value;
}