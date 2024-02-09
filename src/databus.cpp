#include "databus.h"

#include "emulator.h"
#include "timer.h"

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
        return emu->mem->wram_read(address);
    } else if (address < 0xFE00) {
        // reading from echo ram
        std::cout << "Reading from echo ram." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFEA0) {
        // reading from oam
        std::cout << "Reading from oam." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFF00) {
        // unused memory (0xFEA0 - 0xFEFF)
        return 0xFF;
    } else if (address < 0xFF80) {
        // reading from io registers
        return io_read(address);
    } else if (address == 0xFFFF) {
        // reading from interrupt enable register
        return emu->mem->get_interrupt();
    }
    return emu->mem->hram_read(address);
}

void databus::write8(uint16_t address, uint8_t value) {
    if (address <= 0x8000) {
        // writing to rom bank 1 or 2, can just write to cart
        std::cout << "writing value " << std::to_string(value) << std::endl;
        emu->cart->write_cart(value, address);
        return;
    } else if (address < 0xA000) {
        std::cout << "Do not currently support character/map data." << std::endl;
        NOT_IMPLEMENTED()
    } else if (address < 0xC000) {
        // writing to vram/cart ram
        // this is on the cartridge like before, can just write to cart
        emu->cart->write_cart(value, address);
        return;
    } else if (address < 0xE000) {
        // writing to ram banks/wram
        emu->mem->wram_write(address, value);
        return;
    } else if (address < 0xFE00) {
        // writing to echo ram
        std::cout << "Writing to echo ram." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFEA0) {
        // writing to oam
        std::cout << "Writing to oam." << std::endl;
        NOT_IMPLEMENTED();
    } else if (address < 0xFF00) {
        // unused memory (0xFEA0 - 0xFEFF)
        return;
    } else if (address < 0xFF80) {
        // writing to io registers
        io_write(address, value);
        return;
    } else if (address == 0xFFFF) {
        // writing to interrupt enable register
        emu->mem->set_interrupt(value);
        return;
    }
    emu->mem->hram_write(address, value);
}

uint8_t databus::io_read(uint16_t address) {
    // joypad (P1/JOYP) - return 0xFF (no buttons pressed)
    if (address == 0xFF00) {
        return 0xFF;
    }
    // timer registers (0xFF04 - 0xFF07)
    if (address >= 0xFF04 && address <= 0xFF07) {
        return emu->tmr->read(address);
    }
    // interrupt flag (IF)
    if (address == 0xFF0F) {
        return emu->cpu->interrupt_flags | 0xE0;
    }
    // lcd y-coordinate (LY)
    if (address == 0xFF44) {
        // cycle through scanlines 0..153 so vblank loops proceed
        static uint8_t scanline = 0;
        scanline = (scanline + 1) % 154;
        return scanline;
    }
    return io[address - 0xFF00];
}

void databus::io_write(uint16_t address, uint8_t value) {
    // timer registers (0xFF04 - 0xFF07)
    if (address >= 0xFF04 && address <= 0xFF07) {
        emu->tmr->write(address, value);
        return;
    }
    // interrupt flag (IF)
    if (address == 0xFF0F) {
        emu->cpu->interrupt_flags = value;
        return;
    }
    // serial transfer for test rom output
    if (address == 0xFF02 && value == 0x81) {
        std::cout << (char)io[0x01] << std::flush;
    }
    io[address - 0xFF00] = value;
}

uint16_t databus::read16(uint16_t address) {
    return append(read8(address), read8(address + 1));
}

void databus::write16(uint16_t address, uint16_t value) {
    // TODO: verify 0xFF?
    write8(address + 1, (value >> 8) & 0xFF);
    write8(address, value & 0xFF);
}