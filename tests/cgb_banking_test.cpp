#include <cassert>
#include <iostream>

#include "databus.h"
#include "emulator.h"
#include "ppu.h"
#include "ram.h"

void test_vram_banking() {
    emulator emu;

    // test dmg mode: 0xFF4F unmapped (returns 0xFF, write ignored)
    emu.init_hardware_mode(false);
    assert(emu.bus->read8(0xFF4F) == 0xFF);
    emu.bus->write8(0xFF4F, 0x01);
    assert(emu.vid->get_vram_bank() == 0);
    assert(emu.bus->read8(0xFF4F) == 0xFF);

    // test cgb mode: default bank 0 (reads 0xFE with unused bits)
    emu.init_hardware_mode(true);
    assert(emu.bus->read8(0xFF4F) == 0xFE);
    assert(emu.vid->get_vram_bank() == 0);

    // write to vram bank 0 at 0x8000
    emu.bus->write8(0x8000, 0x55);
    assert(emu.bus->read8(0x8000) == 0x55);
    assert(emu.vid->read_vram(0x0000, 0) == 0x55);

    // switch to vram bank 1
    emu.bus->write8(0xFF4F, 0x01);
    assert(emu.vid->get_vram_bank() == 1);
    assert(emu.bus->read8(0xFF4F) == 0xFF);

    // bank 1 at 0x8000 should be 0, write 0xAA
    assert(emu.bus->read8(0x8000) == 0x00);
    emu.bus->write8(0x8000, 0xAA);
    assert(emu.bus->read8(0x8000) == 0xAA);
    assert(emu.vid->read_vram(0x0000, 1) == 0xAA);

    // switch back to bank 0 and verify data separation
    emu.bus->write8(0xFF4F, 0x00);
    assert(emu.vid->get_vram_bank() == 0);
    assert(emu.bus->read8(0x8000) == 0x55);

    // verify bitmasking: only bit 0 matters
    emu.bus->write8(0xFF4F, 0xFE);
    assert(emu.vid->get_vram_bank() == 0);
    emu.bus->write8(0xFF4F, 0xFF);
    assert(emu.vid->get_vram_bank() == 1);

    std::cout << "VRAM banking unit tests passed!" << std::endl;
}

void test_wram_banking() {
    emulator emu;

    // test dmg mode: 0xFF70 unmapped (returns 0xFF, write ignored)
    emu.init_hardware_mode(false);
    assert(emu.bus->read8(0xFF70) == 0xFF);
    emu.bus->write8(0xFF70, 0x02);
    assert(emu.mem->get_wram_bank() == 1);
    assert(emu.bus->read8(0xFF70) == 0xFF);

    // test cgb mode: default bank 1 (reads 0xF9 with unused bits 3-7 set)
    emu.init_hardware_mode(true);
    assert(emu.bus->read8(0xFF70) == 0xF9);
    assert(emu.mem->get_wram_bank() == 1);

    // writing 0 selects bank 1, reads back 0xF8
    emu.bus->write8(0xFF70, 0x00);
    assert(emu.mem->get_wram_bank() == 1);
    assert(emu.bus->read8(0xFF70) == 0xF8);

    // write fixed value to bank 0 at 0xC000
    emu.bus->write8(0xC000, 0x77);
    assert(emu.bus->read8(0xC000) == 0x77);

    // write distinct values across banks 1-7 at 0xD000
    for (uint8_t b = 1; b <= 7; b++) {
        emu.bus->write8(0xFF70, b);
        assert(emu.mem->get_wram_bank() == b);
        assert(emu.bus->read8(0xFF70) == (0xF8 | b));
        emu.bus->write8(0xD000, (uint8_t)(0x10 + b));
    }

    // verify fixed bank 0 remains intact regardless of active bank
    for (uint8_t b = 1; b <= 7; b++) {
        emu.bus->write8(0xFF70, b);
        assert(emu.bus->read8(0xC000) == 0x77);
    }

    // verify each bank preserved its unique data
    for (uint8_t b = 1; b <= 7; b++) {
        emu.bus->write8(0xFF70, b);
        assert(emu.bus->read8(0xD000) == (uint8_t)(0x10 + b));
    }

    // verify echo ram mirrors bank 0 and active banked wram
    assert(emu.bus->read8(0xE000) == 0x77);
    emu.bus->write8(0xFF70, 3);
    assert(emu.bus->read8(0xF000) == 0x13);
    emu.bus->write8(0xFF70, 5);
    assert(emu.bus->read8(0xF000) == 0x15);

    std::cout << "WRAM banking unit tests passed!" << std::endl;
}

int main() {
    test_vram_banking();
    test_wram_banking();
    std::cout << "All CGB VRAM and WRAM banking unit tests passed 100%!" << std::endl;
    return 0;
}
