#include <cassert>
#include <iostream>

#include "databus.h"
#include "emulator.h"
#include "ppu.h"

void test_dmg_palette_registers() {
    emulator emu;

    // test dmg mode: palette registers unmapped (reads 0xFF, writes ignored)
    emu.init_hardware_mode(false);
    assert(emu.bus->read8(0xFF68) == 0xFF);
    assert(emu.bus->read8(0xFF69) == 0xFF);
    assert(emu.bus->read8(0xFF6A) == 0xFF);
    assert(emu.bus->read8(0xFF6B) == 0xFF);

    // writes should have no effect
    emu.bus->write8(0xFF68, 0x80);
    emu.bus->write8(0xFF69, 0x42);
    emu.bus->write8(0xFF6A, 0x80);
    emu.bus->write8(0xFF6B, 0x24);

    assert(emu.bus->read8(0xFF68) == 0xFF);
    assert(emu.bus->read8(0xFF69) == 0xFF);
    assert(emu.bus->read8(0xFF6A) == 0xFF);
    assert(emu.bus->read8(0xFF6B) == 0xFF);

    std::cout << "DMG palette register isolation tests passed!" << std::endl;
}

void test_cgb_palette_indexing_and_autoincrement() {
    emulator emu;

    // test cgb mode: default state
    emu.init_hardware_mode(true);

    // bit 6 always reads 1
    assert(emu.bus->read8(0xFF68) == 0x40);
    assert(emu.bus->read8(0xFF6A) == 0x40);

    // default palette ram is initialized to 0xFF
    assert(emu.bus->read8(0xFF69) == 0xFF);
    assert(emu.bus->read8(0xFF6B) == 0xFF);

    // test manual indexing without auto-increment (bit 7 = 0)
    emu.bus->write8(0xFF68, 0x05);
    assert(emu.bus->read8(0xFF68) == 0x45);
    emu.bus->write8(0xFF69, 0x12);
    assert(emu.bus->read8(0xFF68) == 0x45);
    assert(emu.bus->read8(0xFF69) == 0x12);
    assert(emu.vid->read_cram(false, 5) == 0x12);

    // test auto-increment on write (bit 7 = 1)
    emu.bus->write8(0xFF68, 0x80 | 0x0A);
    assert(emu.bus->read8(0xFF68) == 0xCA);

    // reading data register should not increment index
    assert(emu.bus->read8(0xFF69) == 0xFF);
    assert(emu.bus->read8(0xFF68) == 0xCA);

    // writing data register should increment index
    emu.bus->write8(0xFF69, 0x34);
    assert(emu.bus->read8(0xFF68) == 0xCB);
    assert(emu.vid->read_cram(false, 10) == 0x34);

    emu.bus->write8(0xFF69, 0x56);
    assert(emu.bus->read8(0xFF68) == 0xCC);
    assert(emu.vid->read_cram(false, 11) == 0x56);

    // test index wrapping from 63 to 0
    emu.bus->write8(0xFF68, 0x80 | 0x3F);
    assert(emu.bus->read8(0xFF68) == 0xFF);
    emu.bus->write8(0xFF69, 0x78);
    assert(emu.bus->read8(0xFF68) == 0xC0);
    assert(emu.vid->read_cram(false, 63) == 0x78);

    // test sprite palette registers (0xFF6A / 0xFF6B)
    emu.bus->write8(0xFF6A, 0x80 | 0x02);
    assert(emu.bus->read8(0xFF6A) == 0xC2);
    emu.bus->write8(0xFF6B, 0x99);
    assert(emu.bus->read8(0xFF6A) == 0xC3);
    assert(emu.vid->read_cram(true, 2) == 0x99);

    std::cout << "CGB palette indexing and auto-increment tests passed!" << std::endl;
}

void test_cgb_color_conversion() {
    emulator emu;
    emu.init_hardware_mode(true);

    // helper to write color (2 bytes) to a specific palette and color slot
    auto write_palette_color = [&](bool is_obj, uint8_t palette, uint8_t color, uint16_t bgr15) {
        uint8_t index = (palette * 8) + (color * 2);
        uint16_t spec_reg = is_obj ? 0xFF6A : 0xFF68;
        uint16_t data_reg = is_obj ? 0xFF6B : 0xFF69;
        emu.bus->write8(spec_reg, 0x80 | index);
        emu.bus->write8(data_reg, bgr15 & 0xFF);
        emu.bus->write8(data_reg, (bgr15 >> 8) & 0xFF);
    };

    // black: 0x0000 -> (0, 0, 0)
    write_palette_color(false, 0, 0, 0x0000);
    assert(emu.vid->get_cgb_color(false, 0, 0) == 0xFF000000);

    // white: 0x7FFF -> (255, 255, 255)
    write_palette_color(false, 0, 1, 0x7FFF);
    assert(emu.vid->get_cgb_color(false, 0, 1) == 0xFFFFFFFF);

    // red: 0x001F -> (255, 0, 0)
    write_palette_color(false, 0, 2, 0x001F);
    assert(emu.vid->get_cgb_color(false, 0, 2) == 0xFFFF0000);

    // green: 0x03E0 -> (0, 255, 0)
    write_palette_color(false, 0, 3, 0x03E0);
    assert(emu.vid->get_cgb_color(false, 0, 3) == 0xFF00FF00);

    // blue: 0x7C00 -> (0, 0, 255)
    write_palette_color(true, 1, 0, 0x7C00);
    assert(emu.vid->get_cgb_color(true, 1, 0) == 0xFF0000FF);

    // custom mixed color: r=15, g=10, b=20 -> 15 | (10 << 5) | (20 << 10) = 0x514F
    // r8 = (15 * 255) / 31 = 123
    // g8 = (10 * 255) / 31 = 82
    // b8 = (20 * 255) / 31 = 164
    uint16_t mixed = 15 | (10 << 5) | (20 << 10);
    write_palette_color(true, 3, 2, mixed);
    uint32_t expected = 0xFF000000 | (123 << 16) | (82 << 8) | 164;
    assert(emu.vid->get_cgb_color(true, 3, 2) == expected);

    std::cout << "CGB 15-bit to 32-bit ARGB color conversion tests passed!" << std::endl;
}

int main() {
    test_dmg_palette_registers();
    test_cgb_palette_indexing_and_autoincrement();
    test_cgb_color_conversion();
    std::cout << "All CGB color palette unit tests passed 100%!" << std::endl;
    return 0;
}
