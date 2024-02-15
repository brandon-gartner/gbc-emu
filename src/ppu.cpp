#include "ppu.h"

#include <cstring>
#include "databus.h"
#include "emulator.h"
#include "interrupt.h"

ppu::ppu(emulator* emu) : emu(emu) {
    std::memset(this->vram, 0, sizeof(this->vram));
    std::memset(this->oam, 0, sizeof(this->oam));

    this->lcdc = 0x91;
    this->stat = 0x82;
    this->scy = 0;
    this->scx = 0;
    this->ly = 0;
    this->lyc = 0;
    this->dma = 0xFF;
    this->bgp = 0xFC;
    this->obp0 = 0xFF;
    this->obp1 = 0xFF;
    this->wy = 0;
    this->wx = 0;

    this->scanline_counter = 0;
}

ppu::~ppu() {}

void ppu::set_mode(uint8_t mode) {
    this->stat = (this->stat & 0xFC) | (mode & 0x03);

    // check stat interrupt conditions
    bool request_intr = false;
    if (mode == 0 && (this->stat & 0x08)) {
        // mode 0 hblank interrupt (bit 3)
        request_intr = true;
    } else if (mode == 1 && (this->stat & 0x10)) {
        // mode 1 vblank interrupt (bit 4)
        request_intr = true;
    } else if (mode == 2 && (this->stat & 0x20)) {
        // mode 2 oam interrupt (bit 5)
        request_intr = true;
    }

    if (request_intr && this->emu && this->emu->intr) {
        this->emu->intr->request_interrupt(interrupt::lcd);
    }
}

void ppu::check_lyc() {
    if (this->ly == this->lyc) {
        // set coincidence flag (bit 2)
        this->stat |= 0x04;
        if ((this->stat & 0x40) && this->emu && this->emu->intr) {
            // lyc=ly stat interrupt enable (bit 6)
            this->emu->intr->request_interrupt(interrupt::lcd);
        }
    } else {
        // clear coincidence flag
        this->stat &= ~0x04;
    }
}

void ppu::start_dma(uint8_t value) {
    this->dma = value;
    uint16_t source = ((uint16_t)value) << 8;
    for (int i = 0; i < 0xA0; i++) {
        this->oam[i] = this->emu->bus->read8(source + i);
    }
}

void ppu::tick(int cycles) {
    // if lcd is disabled (bit 7 of lcdc is 0), do not advance scanlines
    if (!(this->lcdc & 0x80)) {
        return;
    }

    // 4 t-cycles per m-cycle
    this->scanline_counter += cycles * 4;

    if (this->scanline_counter >= 456) {
        this->scanline_counter -= 456;
        this->ly++;

        if (this->ly == 144) {
            // enter vblank
            this->set_mode(1);
            if (this->emu && this->emu->intr) {
                this->emu->intr->request_interrupt(interrupt::vblank);
            }
        } else if (this->ly > 153) {
            // frame complete, return to scanline 0
            this->ly = 0;
            this->set_mode(2);
        }

        this->check_lyc();
    }

    // update mode within visible scanlines (0 - 143)
    if (this->ly < 144) {
        if (this->scanline_counter < 80) {
            if ((this->stat & 0x03) != 2) {
                this->set_mode(2);  // mode 2: oam search (80 cycles)
            }
        } else if (this->scanline_counter < 252) {
            if ((this->stat & 0x03) != 3) {
                this->set_mode(3);  // mode 3: pixel transfer (172 cycles)
            }
        } else {
            if ((this->stat & 0x03) != 0) {
                this->set_mode(0);  // mode 0: hblank (204 cycles)
            }
        }
    }
}

uint8_t ppu::read_vram(uint16_t offset) {
    if (offset < 0x2000) {
        return this->vram[offset];
    }
    return 0xFF;
}

void ppu::write_vram(uint16_t offset, uint8_t value) {
    if (offset < 0x2000) {
        this->vram[offset] = value;
    }
}

uint8_t ppu::read_oam(uint16_t offset) {
    if (offset < 0xA0) {
        return this->oam[offset];
    }
    return 0xFF;
}

void ppu::write_oam(uint16_t offset, uint8_t value) {
    if (offset < 0xA0) {
        this->oam[offset] = value;
    }
}

uint8_t ppu::read_reg(uint16_t address) {
    switch (address) {
        case 0xFF40:
            return this->lcdc;
        case 0xFF41:
            // bit 7 is always 1 on dmg
            return this->stat | 0x80;
        case 0xFF42:
            return this->scy;
        case 0xFF43:
            return this->scx;
        case 0xFF44:
            return this->ly;
        case 0xFF45:
            return this->lyc;
        case 0xFF46:
            return this->dma;
        case 0xFF47:
            return this->bgp;
        case 0xFF48:
            return this->obp0;
        case 0xFF49:
            return this->obp1;
        case 0xFF4A:
            return this->wy;
        case 0xFF4B:
            return this->wx;
        default:
            return 0xFF;
    }
}

void ppu::write_reg(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF40: {
            bool was_enabled = (this->lcdc & 0x80);
            this->lcdc = value;
            bool is_enabled = (this->lcdc & 0x80);
            if (was_enabled && !is_enabled) {
                // lcd disabled: reset ly and scanline counter
                this->ly = 0;
                this->scanline_counter = 0;
                this->set_mode(0);
            } else if (!was_enabled && is_enabled) {
                // lcd enabled
                this->scanline_counter = 0;
                this->check_lyc();
            }
            break;
        }
        case 0xFF41:
            // bits 6-3 are writable; bits 2-0 are read-only; bit 7 is always 1
            this->stat = (this->stat & 0x07) | (value & 0x78) | 0x80;
            break;
        case 0xFF42:
            this->scy = value;
            break;
        case 0xFF43:
            this->scx = value;
            break;
        case 0xFF44:
            // ly is read-only on dmg
            break;
        case 0xFF45:
            this->lyc = value;
            this->check_lyc();
            break;
        case 0xFF46:
            this->start_dma(value);
            break;
        case 0xFF47:
            this->bgp = value;
            break;
        case 0xFF48:
            this->obp0 = value;
            break;
        case 0xFF49:
            this->obp1 = value;
            break;
        case 0xFF4A:
            this->wy = value;
            break;
        case 0xFF4B:
            this->wx = value;
            break;
    }
}
