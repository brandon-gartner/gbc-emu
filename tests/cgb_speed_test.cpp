#include <cassert>
#include <iostream>

#include "databus.h"
#include "emulator.h"
#include "ppu.h"
#include "processor.h"
#include "timer.h"

void test_dmg_isolation() {
    emulator emu;
    emu.init_hardware_mode(false);

    // in DMG mode, 0xFF4D is unmapped (reads 0xFF, writes ignored)
    assert(emu.bus->read8(0xFF4D) == 0xFF);
    emu.bus->write8(0xFF4D, 0x01);
    assert(emu.bus->read8(0xFF4D) == 0xFF);
    emu.bus->write8(0xFF4D, 0x80);
    assert(emu.bus->read8(0xFF4D) == 0xFF);

    assert(!emu.cpu->is_double_speed());
    assert(!emu.cpu->is_speed_switch_prepared());

    std::cout << "DMG KEY1 isolation tests passed!" << std::endl;
}

void test_cgb_key1_register_mechanics() {
    emulator emu;
    emu.init_hardware_mode(true);

    // default KEY1 value on CGB: bit 7 = 0, bits 1-6 = 1 (0x7E), bit 0 = 0 -> 0x7E
    assert(emu.bus->read8(0xFF4D) == 0x7E);
    assert(!emu.cpu->is_double_speed());
    assert(!emu.cpu->is_speed_switch_prepared());

    // write 0x01 to arm speed switch: bit 0 becomes 1 -> reads 0x7F
    emu.bus->write8(0xFF4D, 0x01);
    assert(emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0x7F);

    // write 0x00 to cancel speed switch preparation: reads 0x7E
    emu.bus->write8(0xFF4D, 0x00);
    assert(!emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0x7E);

    // bit 7 is read-only; writing 0x80 or 0x81 cannot alter bit 7
    emu.bus->write8(0xFF4D, 0x80);
    assert(!emu.cpu->is_double_speed());
    assert(!emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0x7E);

    emu.bus->write8(0xFF4D, 0x81);
    assert(!emu.cpu->is_double_speed());
    assert(emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0x7F);

    std::cout << "CGB KEY1 register mechanics tests passed!" << std::endl;
}

void test_cgb_speed_switch_execution() {
    emulator emu;
    emu.init_hardware_mode(true);

    // setup code at 0xC000 in WRAM: STOP opcode (0x10 0x00)
    emu.bus->write8(0xC000, 0x10); // STOP
    emu.bus->write8(0xC001, 0x00); // STOP operand byte
    emu.cpu->set_pc(0xC000);

    // arm speed switch
    emu.bus->write8(0xFF4D, 0x01);
    assert(emu.cpu->is_speed_switch_prepared());

    // step CPU to execute STOP instruction
    emu.cpu->step();

    // speed should now be double: bit 7 = 1, bit 0 = 0 -> 0xFE
    assert(emu.cpu->is_double_speed());
    assert(!emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0xFE);
    assert(emu.cpu->reg_pc == 0xC002);

    // test switching back to normal speed:
    // write next STOP at 0xC002
    emu.bus->write8(0xC002, 0x10);
    emu.bus->write8(0xC003, 0x00);

    // arm speed switch while in double speed: bit 7 = 1, bit 0 = 1 -> 0xFF
    emu.bus->write8(0xFF4D, 0x01);
    assert(emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0xFF);

    // execute second STOP
    emu.cpu->step();

    // speed should now be normal: bit 7 = 0, bit 0 = 0 -> 0x7E
    assert(!emu.cpu->is_double_speed());
    assert(!emu.cpu->is_speed_switch_prepared());
    assert(emu.bus->read8(0xFF4D) == 0x7E);
    assert(emu.cpu->reg_pc == 0xC004);

    // unarmed STOP should not toggle speed
    emu.bus->write8(0xC004, 0x10);
    emu.bus->write8(0xC005, 0x00);
    emu.cpu->step();
    assert(!emu.cpu->is_double_speed());
    assert(emu.bus->read8(0xFF4D) == 0x7E);
    assert(emu.cpu->reg_pc == 0xC006);

    std::cout << "CGB STOP speed switch execution tests passed!" << std::endl;
}

void test_cgb_cycle_scaling() {
    emulator emu;
    emu.init_hardware_mode(true);

    // in normal speed: 114 CPU M-cycles = 456 PPU T-cycles (1 scanline)
    emu.vid->set_lcdc(0x80); // enable LCD
    emu.vid->set_ly(0);
    emu.add_cycles(114);
    assert(emu.vid->get_ly() == 1);

    // switch to double speed mode
    emu.cpu->switch_speed();
    assert(emu.cpu->is_double_speed());

    // in double speed: 114 CPU M-cycles = 228 PPU T-cycles (half a scanline, LY stays 0)
    emu.vid->set_ly(0);
    emu.add_cycles(114);
    assert(emu.vid->get_ly() == 0);

    // another 114 CPU M-cycles brings total to 228 M-cycles = 456 PPU T-cycles (LY advances to 1)
    emu.add_cycles(114);
    assert(emu.vid->get_ly() == 1);

    // verify timer DIV advances with CPU cycles
    uint8_t initial_div = emu.bus->read8(0xFF04);
    emu.add_cycles(64); // 64 M-cycles = 256 T-cycles -> DIV increments by 1
    uint8_t new_div = emu.bus->read8(0xFF04);
    assert((uint8_t)(new_div - initial_div) == 1);

    std::cout << "CGB double speed cycle scaling tests passed!" << std::endl;
}

int main() {
    test_dmg_isolation();
    test_cgb_key1_register_mechanics();
    test_cgb_speed_switch_execution();
    test_cgb_cycle_scaling();
    std::cout << "All CGB double speed mode unit tests passed 100%!" << std::endl;
    return 0;
}
