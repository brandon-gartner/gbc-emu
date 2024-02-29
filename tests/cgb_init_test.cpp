#include <cassert>
#include <iostream>

#include "cartridge.h"
#include "emulator.h"
#include "processor.h"

void test_cgb_flags() {
    // test pure dmg cartridge
    cartridge dmg_cart;
    dmg_cart.set_cgb_flag(0x00);
    assert(!dmg_cart.is_cgb());
    assert(!dmg_cart.is_cgb_only());
    assert(dmg_cart.get_cgb_flag() == 0x00);

    // test dual-mode cartridge (0x80)
    cartridge dual_cart;
    dual_cart.set_cgb_flag(0x80);
    assert(dual_cart.is_cgb());
    assert(!dual_cart.is_cgb_only());
    assert(dual_cart.get_cgb_flag() == 0x80);

    // test cgb-only cartridge (0xC0)
    cartridge cgb_only_cart;
    cgb_only_cart.set_cgb_flag(0xC0);
    assert(cgb_only_cart.is_cgb());
    assert(cgb_only_cart.is_cgb_only());
    assert(cgb_only_cart.get_cgb_flag() == 0xC0);

    std::cout << "Cartridge CGB header flag tests passed!" << std::endl;
}

void test_cpu_initial_registers() {
    emulator emu;

    // test dmg cpu boot state
    emu.cpu->init_registers(false);
    assert(emu.cpu->get_reg(REG_A) == 0x01);
    assert(emu.cpu->get_reg(REG_F) == 0xB0);
    assert(emu.cpu->get_reg(REG_B) == 0x00);
    assert(emu.cpu->get_reg(REG_C) == 0x13);
    assert(emu.cpu->get_reg(REG_D) == 0x00);
    assert(emu.cpu->get_reg(REG_E) == 0xD8);
    assert(emu.cpu->get_reg(REG_H) == 0x01);
    assert(emu.cpu->get_reg(REG_L) == 0x4D);
    assert(emu.cpu->get_reg(REG_SP) == 0xFFFE);
    assert(emu.cpu->get_reg(REG_PC) == 0x0100);

    // test cgb cpu boot state
    emu.cpu->init_registers(true);
    assert(emu.cpu->get_reg(REG_A) == 0x11);
    assert(emu.cpu->get_reg(REG_F) == 0x80);
    assert(emu.cpu->get_reg(REG_B) == 0x00);
    assert(emu.cpu->get_reg(REG_C) == 0x00);
    assert(emu.cpu->get_reg(REG_D) == 0xFF);
    assert(emu.cpu->get_reg(REG_E) == 0x56);
    assert(emu.cpu->get_reg(REG_H) == 0x00);
    assert(emu.cpu->get_reg(REG_L) == 0x0D);
    assert(emu.cpu->get_reg(REG_SP) == 0xFFFE);
    assert(emu.cpu->get_reg(REG_PC) == 0x0100);

    std::cout << "CPU register boot state tests passed!" << std::endl;
}

void test_emulator_mode_integration() {
    emulator emu;
    assert(!emu.is_cgb_mode());

    // switch to cgb mode
    emu.init_hardware_mode(true);
    assert(emu.is_cgb_mode());
    assert(emu.cpu->get_reg(REG_A) == 0x11);

    // switch back to dmg mode
    emu.init_hardware_mode(false);
    assert(!emu.is_cgb_mode());
    assert(emu.cpu->get_reg(REG_A) == 0x01);

    std::cout << "Emulator hardware mode integration tests passed!" << std::endl;
}

void test_real_rom_headers() {
    // test tetris (pure dmg)
    cartridge tetris_cart;
    char* tetris_argv[] = {(char*)"emu", (char*)"roms/tetris.gb", nullptr};
    if (tetris_cart.load_cartridge(tetris_argv)) {
        assert(!tetris_cart.is_cgb());
        assert(!tetris_cart.is_cgb_only());
        std::cout << "Tetris DMG ROM header verified successfully!" << std::endl;
    }

    // test pokemon gold (dual-mode cgb)
    cartridge pg_cart;
    char* pg_argv[] = {(char*)"emu", (char*)"roms/pokemon_gold.gbc", nullptr};
    if (pg_cart.load_cartridge(pg_argv)) {
        assert(pg_cart.is_cgb());
        assert(!pg_cart.is_cgb_only());
        assert(pg_cart.get_cgb_flag() == 0x80);
        std::cout << "Pokemon Gold dual-mode CGB ROM header verified successfully!" << std::endl;
    }
}

int main() {
    test_cgb_flags();
    test_cpu_initial_registers();
    test_emulator_mode_integration();
    test_real_rom_headers();
    std::cout << "All CGB initialization unit tests passed 100%!" << std::endl;
    return 0;
}
