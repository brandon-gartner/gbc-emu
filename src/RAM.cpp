#include "RAM.h"
#include <cstring>

ram::ram() {
    std::memset(wram, 0, sizeof(wram));
    std::memset(hram, 0, sizeof(hram));
    interrupt = 0;
    svbk = 1;
    wram_bank = 1;
}

uint8_t ram::wram_read(uint16_t address) {
    if (address < 0xC000 || address >= 0xE000) {
        std::cout << "Invalid wram address received, " << int_to_hex(address) << std::endl;
        exit(1);
    }
    if (address < 0xD000) {
        return wram[0][address - 0xC000];
    }
    return wram[wram_bank][address - 0xD000];
}

void ram::wram_write(uint16_t address, uint8_t value) {
    if (address < 0xC000 || address >= 0xE000) {
        std::cout << "Invalid wram address received, " << int_to_hex(address) << std::endl;
        exit(1);
    }
    if (address < 0xD000) {
        wram[0][address - 0xC000] = value;
    } else {
        wram[wram_bank][address - 0xD000] = value;
    }
}

void ram::set_svbk(uint8_t value) {
    svbk = value & 0x07;
    wram_bank = (svbk == 0) ? 1 : svbk;
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