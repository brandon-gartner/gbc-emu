#include <cassert>
#include <iostream>

#include "databus.h"
#include "emulator.h"
#include "ppu.h"

void test_dmg_isolation() {
    emulator emu;
    emu.init_hardware_mode(false);

    // in dmg mode, hdma registers 0xFF51-0xFF55 read 0xFF and ignore writes
    for (uint16_t addr = 0xFF51; addr <= 0xFF55; addr++) {
        assert(emu.bus->read8(addr) == 0xFF);
        emu.bus->write8(addr, 0x42);
        assert(emu.bus->read8(addr) == 0xFF);
    }

    assert(!emu.vid->is_hdma_active());

    std::cout << "DMG HDMA isolation tests passed!" << std::endl;
}

void test_cgb_dma_address_alignment_and_registers() {
    emulator emu;
    emu.init_hardware_mode(true);

    // hdma1-4 are write-only and read back 0xFF
    for (uint16_t addr = 0xFF51; addr <= 0xFF54; addr++) {
        assert(emu.bus->read8(addr) == 0xFF);
    }
    // default hdma5 reads 0xFF (no active transfer)
    assert(emu.bus->read8(0xFF55) == 0xFF);

    // source address alignment: lower 4 bits are ignored (forced to 0)
    emu.bus->write8(0xFF51, 0xC1);
    emu.bus->write8(0xFF52, 0x2F); // lower 4 bits 0xF should be ignored
    assert(emu.vid->get_hdma_source() == 0xC120);

    // destination address alignment: upper 3 bits forced to 0x8000, lower 4 bits ignored
    emu.bus->write8(0xFF53, 0x13); // (0x13 & 0x1F) | 0x80 = 0x93
    emu.bus->write8(0xFF54, 0x47); // lower 4 bits 0x7 ignored -> 0x40
    assert(emu.vid->get_hdma_dest() == 0x9340);

    // registers still read back 0xFF
    assert(emu.bus->read8(0xFF51) == 0xFF);
    assert(emu.bus->read8(0xFF52) == 0xFF);
    assert(emu.bus->read8(0xFF53) == 0xFF);
    assert(emu.bus->read8(0xFF54) == 0xFF);

    std::cout << "CGB DMA address alignment and register tests passed!" << std::endl;
}

void test_gdma_transfer() {
    emulator emu;
    emu.init_hardware_mode(true);

    // prepare 64 bytes (4 blocks) in WRAM at 0xC000
    for (int i = 0; i < 64; i++) {
        emu.bus->write8(0xC000 + i, (uint8_t)(0x10 + i));
    }

    // configure GDMA: source = 0xC000, dest = 0x8000
    emu.bus->write8(0xFF51, 0xC0);
    emu.bus->write8(0xFF52, 0x00);
    emu.bus->write8(0xFF53, 0x80);
    emu.bus->write8(0xFF54, 0x00);

    // vram bank 0 selected
    emu.bus->write8(0xFF4F, 0x00);

    // trigger gdma: bit 7 = 0, length = 4 blocks - 1 = 3 -> 0x03
    emu.bus->write8(0xFF55, 0x03);

    // verify all 64 bytes transferred immediately to vram bank 0
    for (int i = 0; i < 64; i++) {
        assert(emu.vid->read_vram(i, 0) == (uint8_t)(0x10 + i));
    }
    // hdma5 reads 0xFF upon completion
    assert(emu.bus->read8(0xFF55) == 0xFF);
    assert(!emu.vid->is_hdma_active());

    // test gdma transfer targeting vram bank 1
    emu.bus->write8(0xFF4F, 0x01); // switch to bank 1

    for (int i = 0; i < 16; i++) {
        emu.bus->write8(0xC080 + i, (uint8_t)(0xAA + i));
    }

    emu.bus->write8(0xFF51, 0xC0);
    emu.bus->write8(0xFF52, 0x80);
    emu.bus->write8(0xFF53, 0x80);
    emu.bus->write8(0xFF54, 0x00);
    emu.bus->write8(0xFF55, 0x00); // 1 block = 16 bytes

    // verify bank 1 received the new bytes
    for (int i = 0; i < 16; i++) {
        assert(emu.vid->read_vram(i, 1) == (uint8_t)(0xAA + i));
    }
    // verify bank 0 retained its previous data
    assert(emu.vid->read_vram(0, 0) == 0x10);

    std::cout << "CGB General-Purpose DMA (GDMA) tests passed!" << std::endl;
}

void test_hdma_transfer() {
    emulator emu;
    emu.init_hardware_mode(true);

    // prepare 32 bytes (2 blocks) in WRAM at 0xC100
    for (int i = 0; i < 16; i++) {
        emu.bus->write8(0xC100 + i, (uint8_t)(0x20 + i));
        emu.bus->write8(0xC110 + i, (uint8_t)(0x30 + i));
    }

    emu.bus->write8(0xFF4F, 0x00); // bank 0
    emu.vid->set_lcdc(0x80);       // lcd enabled

    // configure hdma: source = 0xC100, dest = 0x8200
    emu.bus->write8(0xFF51, 0xC1);
    emu.bus->write8(0xFF52, 0x00);
    emu.bus->write8(0xFF53, 0x82);
    emu.bus->write8(0xFF54, 0x00);

    // trigger hdma: bit 7 = 1, length = 2 blocks - 1 = 1 -> 0x81
    emu.bus->write8(0xFF55, 0x81);

    assert(emu.vid->is_hdma_active());
    // hdma5 reads remaining blocks minus 1: 2 blocks remain -> reads 0x01
    assert(emu.bus->read8(0xFF55) == 0x01);

    // verify vram has not been modified before hblank
    assert(emu.vid->read_vram(0x0200, 0) == 0x00);

    // advance ppu to hblank on scanline 0 (114 M-cycles)
    emu.vid->set_ly(0);
    emu.add_cycles(114);

    // block 0 (first 16 bytes) transferred
    for (int i = 0; i < 16; i++) {
        assert(emu.vid->read_vram(0x0200 + i, 0) == (uint8_t)(0x20 + i));
    }
    // block 1 has not transferred yet
    assert(emu.vid->read_vram(0x0210, 0) == 0x00);

    // hdma active with 1 block remaining -> reads 0x00
    assert(emu.vid->is_hdma_active());
    assert(emu.bus->read8(0xFF55) == 0x00);

    // advance ppu to next hblank on scanline 1
    emu.add_cycles(114);

    // block 1 transferred
    for (int i = 0; i < 16; i++) {
        assert(emu.vid->read_vram(0x0210 + i, 0) == (uint8_t)(0x30 + i));
    }

    // hdma finished
    assert(!emu.vid->is_hdma_active());
    assert(emu.bus->read8(0xFF55) == 0xFF);

    std::cout << "CGB HBlank DMA (HDMA) tests passed!" << std::endl;
}

void test_hdma_cancellation() {
    emulator emu;
    emu.init_hardware_mode(true);

    for (int i = 0; i < 64; i++) {
        emu.bus->write8(0xC200 + i, (uint8_t)(0x50 + i));
    }

    emu.bus->write8(0xFF4F, 0x00);
    emu.vid->set_lcdc(0x80);

    // configure hdma: 4 blocks (64 bytes) to 0x8300
    emu.bus->write8(0xFF51, 0xC2);
    emu.bus->write8(0xFF52, 0x00);
    emu.bus->write8(0xFF53, 0x83);
    emu.bus->write8(0xFF54, 0x00);
    emu.bus->write8(0xFF55, 0x83);

    // advance 1 hblank (block 0 transferred, 3 blocks remain)
    emu.vid->set_ly(0);
    emu.add_cycles(114);
    assert(emu.bus->read8(0xFF55) == 0x02);

    // cancel hdma by writing 0 to bit 7 of hdma5
    emu.bus->write8(0xFF55, 0x00);
    assert(!emu.vid->is_hdma_active());

    // reading hdma5 returns 0x80 | (3 - 1) = 0x82
    assert(emu.bus->read8(0xFF55) == 0x82);

    // advance through subsequent hblanks: no further blocks transferred
    emu.add_cycles(114);
    emu.add_cycles(114);
    assert(emu.vid->read_vram(0x0310, 0) == 0x00);

    std::cout << "CGB HDMA cancellation tests passed!" << std::endl;
}

void test_hdma_lcd_disabled() {
    emulator emu;
    emu.init_hardware_mode(true);

    for (int i = 0; i < 16; i++) {
        emu.bus->write8(0xC300 + i, (uint8_t)(0x70 + i));
    }

    // lcd disabled: bit 7 of lcdc is 0
    emu.vid->set_lcdc(0x00);

    emu.bus->write8(0xFF51, 0xC3);
    emu.bus->write8(0xFF52, 0x00);
    emu.bus->write8(0xFF53, 0x84);
    emu.bus->write8(0xFF54, 0x00);

    // write hdma mode with lcd disabled -> executes immediately as gdma
    emu.bus->write8(0xFF55, 0x80); // 1 block

    for (int i = 0; i < 16; i++) {
        assert(emu.vid->read_vram(0x0400 + i, 0) == (uint8_t)(0x70 + i));
    }
    assert(!emu.vid->is_hdma_active());
    assert(emu.bus->read8(0xFF55) == 0xFF);

    std::cout << "CGB HDMA with LCD disabled tests passed!" << std::endl;
}

int main() {
    test_dmg_isolation();
    test_cgb_dma_address_alignment_and_registers();
    test_gdma_transfer();
    test_hdma_transfer();
    test_hdma_cancellation();
    test_hdma_lcd_disabled();
    std::cout << "All CGB DMA (GDMA & HDMA) unit tests passed 100%!" << std::endl;
    return 0;
}
