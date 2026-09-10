#include <cassert>
#include <iostream>

#include "cartridge.h"
#include "databus.h"
#include "emulator.h"
#include "instruction.h"
#include "ppu.h"
#include "processor.h"
#include "ram.h"

void test_gb_backwards_compatibility() {
    emulator emu;
    emu.init_hardware_mode(false);

    // verify CPU registers initialize to GB (DMG) power-up values
    assert(emu.cpu->reg_a == 0x01);
    assert(emu.cpu->reg_f == 0xB0);
    assert(emu.cpu->reg_b == 0x00);
    assert(emu.cpu->reg_c == 0x13);
    assert(emu.cpu->reg_d == 0x00);
    assert(emu.cpu->reg_e == 0xD8);
    assert(emu.cpu->reg_h == 0x01);
    assert(emu.cpu->reg_l == 0x4D);
    assert(!emu.cpu->is_double_speed());

    // verify VRAM bank selection (VBK 0xFF4F) is locked in GB mode
    assert(emu.bus->read8(0xFF4F) == 0xFF);
    emu.bus->write8(0xFF4F, 0x01);
    assert(emu.bus->read8(0xFF4F) == 0xFF);
    assert(emu.vid->get_vram_bank() == 0);

    // verify WRAM bank selection (SVBK 0xFF70) is locked in GB mode
    assert(emu.bus->read8(0xFF70) == 0xFF);
    emu.bus->write8(0xFF70, 0x04);
    assert(emu.bus->read8(0xFF70) == 0xFF);
    assert(emu.mem->get_wram_bank() == 1);

    // verify GBC color palette registers are isolated in GB mode
    assert(emu.bus->read8(0xFF68) == 0xFF);
    assert(emu.bus->read8(0xFF69) == 0xFF);
    assert(emu.bus->read8(0xFF6A) == 0xFF);
    assert(emu.bus->read8(0xFF6B) == 0xFF);
    emu.bus->write8(0xFF68, 0x80);
    emu.bus->write8(0xFF69, 0x1F);
    assert(emu.bus->read8(0xFF68) == 0xFF);

    // verify KEY1 speed switch register is isolated in GB mode
    assert(emu.bus->read8(0xFF4D) == 0xFF);
    emu.bus->write8(0xFF4D, 0x01);
    assert(emu.bus->read8(0xFF4D) == 0xFF);
    assert(!emu.cpu->is_speed_switch_prepared());

    // verify HDMA registers are isolated in GB mode
    for (uint16_t addr = 0xFF51; addr <= 0xFF55; addr++) {
        assert(emu.bus->read8(addr) == 0xFF);
        emu.bus->write8(addr, 0x10);
        assert(emu.bus->read8(addr) == 0xFF);
    }
    assert(!emu.vid->is_hdma_active());

    std::cout << "GB backwards compatibility isolation verified successfully!" << std::endl;
}

void test_gbc_full_subsystems_flow() {
    emulator emu;
    emu.init_hardware_mode(true);

    // 1. verify CPU registers initialize to GBC power-up values
    assert(emu.cpu->reg_a == 0x11);
    assert(emu.cpu->reg_f == 0x80);
    assert(emu.cpu->reg_b == 0x00);
    assert(emu.cpu->reg_c == 0x00);
    assert(emu.cpu->reg_d == 0xFF);
    assert(emu.cpu->reg_e == 0x56);
    assert(emu.cpu->reg_h == 0x00);
    assert(emu.cpu->reg_l == 0x0D);
    assert(!emu.cpu->is_double_speed());

    // 2. test VRAM banking and bank data isolation
    emu.bus->write8(0xFF4F, 0x00); // select VRAM bank 0
    emu.bus->write8(0x8000, 0xAA);
    emu.bus->write8(0xFF4F, 0x01); // select VRAM bank 1
    emu.bus->write8(0x8000, 0x55);

    assert(emu.vid->read_vram(0x0000, 0) == 0xAA);
    assert(emu.vid->read_vram(0x0000, 1) == 0x55);

    // 3. test WRAM banking across banks 1 to 7
    for (uint8_t bank = 1; bank <= 7; bank++) {
        emu.bus->write8(0xFF70, bank);
        assert((emu.bus->read8(0xFF70) & 0x07) == bank);
        emu.bus->write8(0xD000, (uint8_t)(0x10 * bank));
    }
    for (uint8_t bank = 1; bank <= 7; bank++) {
        emu.bus->write8(0xFF70, bank);
        assert(emu.bus->read8(0xD000) == (uint8_t)(0x10 * bank));
    }

    // 4. test GBC color palette configuration (auto-increment and color conversion)
    // set BG palette 0, color 0 to full red (0x001F)
    emu.bus->write8(0xFF68, 0x80); // index 0, auto-increment on
    emu.bus->write8(0xFF69, 0x1F); // low byte: 5 bits red (31)
    emu.bus->write8(0xFF69, 0x00); // high byte: 0

    // index should auto-increment to 2
    assert((emu.bus->read8(0xFF68) & 0x3F) == 2);
    assert(emu.vid->get_cgb_color(false, 0, 0) == 0xFFFF0000);

    // 5. test double speed switching
    assert(!emu.cpu->is_double_speed());
    emu.bus->write8(0xFF4D, 0x01); // arm speed switch
    assert(emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0x7F);

    // load STOP instruction (0x10 0x00) into WRAM at 0xC000
    emu.bus->write8(0xC000, 0x10);
    emu.bus->write8(0xC001, 0x00);
    emu.cpu->reg_pc = 0xC000;
    emu.cpu->step();

    // confirm CPU switched to double speed
    assert(emu.cpu->is_double_speed());
    assert(emu.bus->read8(0xFF4D) == 0xFE);
    assert(emu.cpu->reg_pc == 0xC002);

    // 6. test GDMA memory transfer into active VRAM bank 1
    // prepare 32 bytes (2 blocks) in WRAM at 0xC200
    for (int i = 0; i < 32; i++) {
        emu.bus->write8(0xC200 + i, (uint8_t)(0x40 + i));
    }
    emu.bus->write8(0xFF4F, 0x01); // target VRAM bank 1
    emu.bus->write8(0xFF51, 0xC2); // source 0xC200
    emu.bus->write8(0xFF52, 0x00);
    emu.bus->write8(0xFF53, 0x85); // dest 0x8500
    emu.bus->write8(0xFF54, 0x00);
    emu.bus->write8(0xFF55, 0x01); // trigger GDMA: 2 blocks (32 bytes)

    for (int i = 0; i < 32; i++) {
        assert(emu.vid->read_vram(0x0500 + i, 1) == (uint8_t)(0x40 + i));
    }
    assert(!emu.vid->is_hdma_active());
    assert(emu.bus->read8(0xFF55) == 0xFF);

    // 7. test HDMA memory transfer across HBlanks
    for (int i = 0; i < 32; i++) {
        emu.bus->write8(0xC300 + i, (uint8_t)(0x80 + i));
    }
    emu.bus->write8(0xFF4F, 0x00); // target VRAM bank 0
    emu.vid->set_lcdc(0x80);       // LCD enabled
    emu.bus->write8(0xFF51, 0xC3);
    emu.bus->write8(0xFF52, 0x00);
    emu.bus->write8(0xFF53, 0x86);
    emu.bus->write8(0xFF54, 0x00);
    emu.bus->write8(0xFF55, 0x81); // trigger HDMA: 2 blocks (32 bytes)

    assert(emu.vid->is_hdma_active());
    assert(emu.bus->read8(0xFF55) == 0x01);

    // advance 1 HBlank (block 0 copied)
    emu.vid->set_ly(0);
    emu.add_cycles(228); // 228 M-cycles in double speed = 456 T-cycles (1 scanline)

    for (int i = 0; i < 16; i++) {
        assert(emu.vid->read_vram(0x0600 + i, 0) == (uint8_t)(0x80 + i));
    }
    assert(emu.bus->read8(0xFF55) == 0x00);

    // advance 2nd HBlank (block 1 copied, finished)
    emu.add_cycles(228);

    for (int i = 0; i < 16; i++) {
        assert(emu.vid->read_vram(0x0610 + i, 0) == (uint8_t)(0x90 + i));
    }
    assert(!emu.vid->is_hdma_active());
    assert(emu.bus->read8(0xFF55) == 0xFF);

    // 8. test switching back to normal speed
    emu.bus->write8(0xFF4D, 0x01);
    emu.bus->write8(0xC002, 0x10);
    emu.bus->write8(0xC003, 0x00);
    emu.cpu->reg_pc = 0xC002;
    emu.cpu->step();

    assert(!emu.cpu->is_double_speed());
    assert(emu.bus->read8(0xFF4D) == 0x7E);

    std::cout << "GBC full subsystems integration flow verified successfully!" << std::endl;
}

void test_real_rom_headers_integration() {
    // verify Tetris DMG header
    {
        emulator emu;
        char* args[] = {(char*)"emu", (char*)"roms/tetris.gb", nullptr};
        assert(emu.cart->load_cartridge(args));
        assert(!emu.cart->is_cgb());
        assert(emu.cart->game_title.find("TETRIS") != std::string::npos);
    }

    // verify Link's Awakening DX GBC header
    {
        emulator emu;
        char* args[] = {(char*)"emu", (char*)"roms/loz.gbc", nullptr};
        assert(emu.cart->load_cartridge(args));
        assert(emu.cart->is_cgb());
        assert(emu.cart->game_title.find("ZELDA") != std::string::npos);
    }

    // verify Pokémon Gold GBC header
    {
        emulator emu;
        char* args[] = {(char*)"emu", (char*)"roms/pokemon_gold.gbc", nullptr};
        assert(emu.cart->load_cartridge(args));
        assert(emu.cart->is_cgb());
        assert(emu.cart->game_title.find("POKEMON") != std::string::npos);
    }

    std::cout << "Real ROM header integration verified successfully!" << std::endl;
}

int main() {
    test_gb_backwards_compatibility();
    test_gbc_full_subsystems_flow();
    test_real_rom_headers_integration();
    std::cout << "All GBC end-to-end integration tests passed 100%!" << std::endl;
    return 0;
}
