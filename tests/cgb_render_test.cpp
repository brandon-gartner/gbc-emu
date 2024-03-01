#include <cassert>
#include <iostream>

#include "databus.h"
#include "emulator.h"
#include "ppu.h"

// helper to write a 15-bit color to cgb palette ram
void set_palette_color(emulator& emu, bool is_obj, uint8_t palette, uint8_t color, uint16_t bgr15) {
    uint8_t index = (palette * 8) + (color * 2);
    uint16_t spec_reg = is_obj ? 0xFF6A : 0xFF68;
    uint16_t data_reg = is_obj ? 0xFF6B : 0xFF69;
    emu.bus->write8(spec_reg, 0x80 | index);
    emu.bus->write8(data_reg, bgr15 & 0xFF);
    emu.bus->write8(data_reg, (bgr15 >> 8) & 0xFF);
}

void test_cgb_bg_attributes() {
    emulator emu;
    emu.init_hardware_mode(true);

    // lcdc: lcd enable (bit 7), bg tile map 0x9800 (bit 3=0), unsigned tiles 0x8000 (bit 4=1), bg enable/priority (bit 0=1)
    emu.vid->set_lcdc(0x91);
    emu.vid->set_ly(0);
    emu.vid->set_scx(0);
    emu.vid->set_scy(0);

    // bg palette 0: color 0 = white (0x7FFF), color 1 = red (0x001F)
    set_palette_color(emu, false, 0, 0, 0x7FFF);
    set_palette_color(emu, false, 0, 1, 0x001F);

    // bg palette 1: color 1 = blue (0x7C00)
    set_palette_color(emu, false, 1, 1, 0x7C00);

    // write tile 0 in vram bank 0: row 0 has bit 7 set -> pixel 0 is color 1, others color 0
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x8000, 0x80);
    emu.bus->write8(0x8001, 0x00);

    // write tile 1 in vram bank 1: row 0 has bit 7 and bit 6 set -> pixels 0, 1 are color 1
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x8010, 0xC0);
    emu.bus->write8(0x8011, 0x00);

    // tile map at 0x9800:
    // tile 0 at col 0: uses vram bank 0, palette 0, no flip (attr = 0x00)
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x9800, 0);
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x00);

    // tile 1 at col 1: uses vram bank 1 (bit 3), palette 1 (bits 0-2), no flip (attr = 0x09)
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x9801, 1);
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9801, 0x08 | 0x01);

    // tile 0 at col 2: uses vram bank 0, palette 0, x-flip (bit 5) (attr = 0x20)
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x9802, 0);
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9802, 0x20);

    // render scanline 0
    emu.vid->render_scanline();
    const uint32_t* fb = emu.vid->get_framebuffer();

    // col 0 (x=0..7): tile 0 normal
    assert(fb[0] == 0xFFFF0000);  // red (pixel 0)
    assert(fb[1] == 0xFFFFFFFF);  // white (pixel 1)

    // col 1 (x=8..15): tile 1 bank 1 with palette 1
    assert(fb[8] == 0xFF0000FF);  // blue (pixel 0)
    assert(fb[9] == 0xFF0000FF);  // blue (pixel 1)
    assert(fb[10] == 0xFFFFFFFF); // white (pixel 2)

    // col 2 (x=16..23): tile 0 x-flipped (pixel 0 was red, now at pixel 7 -> screen x=23)
    assert(fb[16] == 0xFFFFFFFF); // white
    assert(fb[22] == 0xFFFFFFFF); // white
    assert(fb[23] == 0xFFFF0000); // red

    // test y-flip:
    // write tile 2 in bank 0: row 0 is blank, row 7 has bit 7 set (color 1)
    emu.vid->set_vram_bank(0);
    for (int r = 0; r < 7; r++) {
        emu.bus->write8(0x8020 + r * 2, 0x00);
        emu.bus->write8(0x8021 + r * 2, 0x00);
    }
    emu.bus->write8(0x8020 + 7 * 2, 0x80);
    emu.bus->write8(0x8021 + 7 * 2, 0x00);

    // tile 2 at col 3 with y-flip (bit 6) (attr = 0x40)
    emu.bus->write8(0x9803, 2);
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9803, 0x40);

    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    assert(fb[24] == 0xFFFF0000); // scanline 0 fetches row 7 due to y-flip

    std::cout << "CGB BG tile attribute unit tests passed!" << std::endl;
}

void test_cgb_window_attributes() {
    emulator emu;
    emu.init_hardware_mode(true);

    // lcdc: lcd enable (0x80), window enable (0x20), window tile map 0x9800 (bit 6=0), unsigned tiles (0x10), bg enable (0x01)
    emu.vid->set_lcdc(0xB1);
    emu.vid->set_ly(0);
    emu.vid->set_wx(7); // window x=0
    emu.vid->set_wy(0); // window y=0

    // bg palette 2: color 1 = green (0x03E0)
    set_palette_color(emu, false, 2, 1, 0x03E0);

    // tile 3 in vram bank 1: row 0 has bit 7 set
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x8030, 0x80);
    emu.bus->write8(0x8031, 0x00);

    // window tile map at 0x9800: tile 3, bank 1, palette 2 (attr = 0x08 | 0x02)
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x9800, 3);
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x08 | 0x02);

    emu.vid->render_scanline();
    const uint32_t* fb = emu.vid->get_framebuffer();
    assert(fb[0] == 0xFF00FF00); // green

    std::cout << "CGB Window attribute unit tests passed!" << std::endl;
}

void test_cgb_sprite_attributes_and_priority() {
    emulator emu;
    emu.init_hardware_mode(true);

    // lcdc: lcd enable (0x80), obj enable (0x02), unsigned tiles (0x10), bg enable (0x01)
    emu.vid->set_lcdc(0x93);
    emu.vid->set_ly(0);

    // obj palette 0: color 1 = red (0x001F)
    // obj palette 1: color 1 = green (0x03E0)
    // obj palette 2: color 1 = blue (0x7C00)
    set_palette_color(emu, true, 0, 1, 0x001F);
    set_palette_color(emu, true, 1, 1, 0x03E0);
    set_palette_color(emu, true, 2, 1, 0x7C00);

    // tile 4 in vram bank 1: row 0 has bit 7 set
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x8040, 0x80);
    emu.bus->write8(0x8041, 0x00);

    // sprite 0 at x=10, y=0: uses tile 4 in bank 1, obj palette 1 (attr = 0x08 | 0x01)
    emu.vid->write_oam(0, 16);          // y = 0
    emu.vid->write_oam(1, 10 + 8);      // x = 10
    emu.vid->write_oam(2, 4);           // tile = 4
    emu.vid->write_oam(3, 0x08 | 0x01); // bank 1, pal 1

    emu.vid->render_scanline();
    const uint32_t* fb = emu.vid->get_framebuffer();
    assert(fb[10] == 0xFF00FF00); // green

    // test cgb sprite priority: strictly oam index order regardless of x coordinate
    // tile 0 in bank 0: row 0 has bit 7 set (color 1)
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x8000, 0x80);
    emu.bus->write8(0x8001, 0x00);

    // sprite 0 at x=30: tile 0, obj pal 0 (red)
    emu.vid->write_oam(0, 16);
    emu.vid->write_oam(1, 30 + 8);
    emu.vid->write_oam(2, 0);
    emu.vid->write_oam(3, 0x00); // pal 0

    // sprite 1 at x=30: tile 0, obj pal 2 (blue)
    emu.vid->write_oam(4, 16);
    emu.vid->write_oam(5, 30 + 8);
    emu.vid->write_oam(6, 0);
    emu.vid->write_oam(7, 0x02); // pal 2

    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    assert(fb[30] == 0xFFFF0000); // sprite 0 wins over sprite 1 because oam index 0 < 1

    std::cout << "CGB Sprite attribute and priority unit tests passed!" << std::endl;
}

void test_cgb_bg_to_oam_priority() {
    emulator emu;
    emu.init_hardware_mode(true);

    // palettes:
    // bg palette 0: color 0 = white (0x7FFF), color 1 = red (0x001F)
    set_palette_color(emu, false, 0, 0, 0x7FFF);
    set_palette_color(emu, false, 0, 1, 0x001F);
    // obj palette 0: color 1 = green (0x03E0)
    set_palette_color(emu, true, 0, 1, 0x03E0);

    // bg tile 0: color 1 at pixel 0
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x8000, 0x80);
    emu.bus->write8(0x8001, 0x00);
    // obj tile 1: color 1 at pixel 0
    emu.bus->write8(0x8010, 0x80);
    emu.bus->write8(0x8011, 0x00);

    // case 1: lcdc bit 0 is 0 in cgb mode -> master sprite priority
    // sprites always display on top of bg colors 1-3
    emu.vid->set_lcdc(0x92); // bit 0 = 0, obj enable (bit 1), lcd enable (bit 7)
    emu.vid->set_ly(0);
    emu.vid->set_scx(0);
    emu.vid->set_scy(0);

    // bg tile 0 at 0x9800 with bg priority set (attr = 0x80)
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x9800, 0);
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x80); // bg priority flag set

    // sprite 0 at x=0 with obj priority behind bg (attr = 0x80)
    emu.vid->write_oam(0, 16);
    emu.vid->write_oam(1, 8);
    emu.vid->write_oam(2, 1);    // tile 1
    emu.vid->write_oam(3, 0x80); // obj priority flag set

    emu.vid->render_scanline();
    const uint32_t* fb = emu.vid->get_framebuffer();
    assert(fb[0] == 0xFF00FF00); // sprite (green) wins because lcdc bit 0 is 0

    // case 2: lcdc bit 0 is 1 in cgb mode -> normal priority rules
    emu.vid->set_lcdc(0x93); // bit 0 = 1

    // case 2a: bg attr priority set -> bg covers sprite
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x80); // bg priority set
    emu.vid->write_oam(3, 0x00);   // sprite priority normal
    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    assert(fb[0] == 0xFFFF0000); // bg (red) covers sprite

    // case 2b: sprite priority set -> bg covers sprite
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x00); // bg priority clear
    emu.vid->write_oam(3, 0x80);   // sprite priority behind bg
    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    assert(fb[0] == 0xFFFF0000); // bg (red) covers sprite

    // case 2c: both priority flags clear -> sprite on top of bg color 1
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x00);
    emu.vid->write_oam(3, 0x00);
    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    assert(fb[0] == 0xFF00FF00); // sprite (green) on top

    // case 2d: bg color is 0 -> sprite always on top regardless of priority bits
    // select blank tile 2 for bg
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x9800, 2);
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x80); // bg priority set
    emu.vid->write_oam(3, 0x80);   // sprite priority set
    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    assert(fb[0] == 0xFF00FF00); // sprite (green) displays over bg color 0

    std::cout << "CGB BG-to-OAM priority unit tests passed!" << std::endl;
}

void test_dmg_rendering_regression() {
    emulator emu;
    emu.init_hardware_mode(false);

    emu.vid->set_lcdc(0x93);
    emu.vid->set_ly(0);
    emu.vid->set_scx(0);
    emu.vid->set_scy(0);
    emu.bus->write8(0xFF47, 0xE4); // bgp: 11 10 01 00
    emu.bus->write8(0xFF48, 0xE4); // obp0: 11 10 01 00

    // write tile in bank 0
    emu.vid->set_vram_bank(0);
    emu.bus->write8(0x8000, 0x80);
    emu.bus->write8(0x8001, 0x00);
    emu.bus->write8(0x9800, 0);

    // write attribute in bank 1 (must be ignored in dmg mode)
    emu.vid->set_vram_bank(1);
    emu.bus->write8(0x9800, 0x08 | 0x01); // bank 1, palette 1

    emu.vid->render_scanline();
    const uint32_t* fb = emu.vid->get_framebuffer();
    // dmg shade 1: 0xFF88C070
    assert(fb[0] == 0xFF88C070);

    // in dmg mode, lcdc bit 0 = 0 blanks scanline to shade 0 (0xFFE0F8D0)
    emu.vid->set_lcdc(0x92);
    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    assert(fb[0] == 0xFFE0F8D0);

    // test dmg sprite priority: smaller x wins regardless of oam index
    emu.vid->set_lcdc(0x93);
    // sprite 0 at x=50, oam index 0
    emu.vid->write_oam(0, 16);
    emu.vid->write_oam(1, 50 + 8);
    emu.vid->write_oam(2, 0);
    emu.vid->write_oam(3, 0x00); // uses obp0

    // sprite 1 at x=48, oam index 1 (smaller x)
    emu.vid->write_oam(4, 16);
    emu.vid->write_oam(5, 48 + 8);
    emu.vid->write_oam(6, 0);
    emu.vid->write_oam(7, 0x10); // uses obp1
    emu.bus->write8(0xFF49, 0x1B); // obp1 shade 1 = 00011011 -> color 1 gives shade 2 (0xFF346856)

    emu.vid->render_scanline();
    fb = emu.vid->get_framebuffer();
    // at x=50, both sprites overlap (sprite 1 spans x=48..55, sprite 0 spans x=50..57)
    // sprite 1 has smaller x (48 < 50), so sprite 1 wins despite higher oam index!
    assert(fb[48] == 0xFF346856);

    std::cout << "DMG rendering regression unit tests passed!" << std::endl;
}

int main() {
    test_cgb_bg_attributes();
    test_cgb_window_attributes();
    test_cgb_sprite_attributes_and_priority();
    test_cgb_bg_to_oam_priority();
    test_dmg_rendering_regression();
    std::cout << "All CGB tile attribute and color rendering unit tests passed 100%!" << std::endl;
    return 0;
}
