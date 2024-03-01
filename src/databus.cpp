#include "databus.h"

#include "emulator.h"
#include "joypad.h"
#include "ppu.h"
#include "timer.h"

databus::databus(emulator* emu) : emu(emu) {}

uint8_t databus::read8(uint16_t address) {
    if (address < 0x8000) {
        // reading from rom bank 1 or 2, can just read from cart
        uint8_t value = emu->cart->read_cart(address);
        return value;
    } else if (address < 0xA000) {
        // reading from vram (0x8000 - 0x9FFF)
        return emu->vid->read_vram(address - 0x8000);
    } else if (address < 0xC000) {
        // reading from cart ram (0xA000 - 0xBFFF)
        return emu->cart->read_cart(address);
    } else if (address < 0xE000) {
        // reading from ram banks/wram (0xC000 - 0xDFFF)
        return emu->mem->wram_read(address);
    } else if (address < 0xFE00) {
        // reading from echo ram (0xE000 - 0xFDFF, mirrors 0xC000 - 0xDDFF)
        return emu->mem->wram_read(address - 0x2000);
    } else if (address < 0xFEA0) {
        // reading from oam (0xFE00 - 0xFE9F)
        return emu->vid->read_oam(address - 0xFE00);
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
    if (address < 0x8000) {
        // writing to rom bank 1 or 2, can just write to cart
        emu->cart->write_cart(value, address);
        return;
    } else if (address < 0xA000) {
        // writing to vram (0x8000 - 0x9FFF)
        emu->vid->write_vram(address - 0x8000, value);
        return;
    } else if (address < 0xC000) {
        // writing to cart ram (0xA000 - 0xBFFF)
        emu->cart->write_cart(value, address);
        return;
    } else if (address < 0xE000) {
        // writing to ram banks/wram (0xC000 - 0xDFFF)
        emu->mem->wram_write(address, value);
        return;
    } else if (address < 0xFE00) {
        // writing to echo ram (0xE000 - 0xFDFF, mirrors 0xC000 - 0xDDFF)
        emu->mem->wram_write(address - 0x2000, value);
        return;
    } else if (address < 0xFEA0) {
        // writing to oam (0xFE00 - 0xFE9F)
        emu->vid->write_oam(address - 0xFE00, value);
        return;
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
    // joypad (P1/JOYP)
    if (address == 0xFF00) {
        return emu->pad->read();
    }
    // timer registers (0xFF04 - 0xFF07)
    if (address >= 0xFF04 && address <= 0xFF07) {
        return emu->tmr->read(address);
    }
    // interrupt flag (IF)
    if (address == 0xFF0F) {
        return emu->cpu->interrupt_flags | 0xE0;
    }
    // ppu registers (0xff40 - 0xff4b, 0xff4f)
    if ((address >= 0xFF40 && address <= 0xFF4B) || address == 0xFF4F) {
        return emu->vid->read_reg(address);
    }
    // key1 speed switch register (0xFF4D)
    if (address == 0xFF4D) {
        return emu->is_cgb_mode() ? emu->cpu->read_key1() : 0xFF;
    }
    // cgb palette registers (0xff68 - 0xff6b)
    if (address >= 0xFF68 && address <= 0xFF6B) {
        return emu->vid->read_reg(address);
    }
    // wram bank register (svbk, 0xff70)
    if (address == 0xFF70) {
        return emu->is_cgb_mode() ? emu->mem->get_svbk() : 0xFF;
    }
    return io[address - 0xFF00];
}

void databus::io_write(uint16_t address, uint8_t value) {
    // joypad (P1/JOYP)
    if (address == 0xFF00) {
        emu->pad->write(value);
        return;
    }
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
    // ppu registers (0xff40 - 0xff4b, 0xff4f)
    if ((address >= 0xFF40 && address <= 0xFF4B) || address == 0xFF4F) {
        emu->vid->write_reg(address, value);
        return;
    }
    // key1 speed switch register (0xFF4D)
    if (address == 0xFF4D) {
        if (emu->is_cgb_mode()) {
            emu->cpu->write_key1(value);
        }
        return;
    }
    // cgb palette registers (0xff68 - 0xff6b)
    if (address >= 0xFF68 && address <= 0xFF6B) {
        emu->vid->write_reg(address, value);
        return;
    }
    // wram bank register (svbk, 0xff70)
    if (address == 0xFF70) {
        if (emu->is_cgb_mode()) {
            emu->mem->set_svbk(value);
        }
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
    write8(address + 1, (value >> 8) & 0xFF);
    write8(address, value & 0xFF);
}