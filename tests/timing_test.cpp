#include <cassert>
#include <iostream>
#include "emulator.h"
#include "instruction.h"
#include "ppu.h"
#include "timer.h"

void test_tables() {
    // standard opcodes
    assert(instruction::opcode_cycles[0x00] == 1); // nop
    assert(instruction::opcode_cycles[0x01] == 3); // ld bc, d16
    assert(instruction::opcode_cycles[0x08] == 5); // ld (a16), sp
    assert(instruction::opcode_cycles[0x20] == 2); // jr nz, r8 (base)
    assert(instruction::opcode_cycles[0x36] == 3); // ld (hl), d8
    assert(instruction::opcode_cycles[0x76] == 1); // halt
    assert(instruction::opcode_cycles[0xC0] == 2); // ret nz (base)
    assert(instruction::opcode_cycles[0xC3] == 4); // jp a16
    assert(instruction::opcode_cycles[0xC4] == 3); // call nz, a16 (base)
    assert(instruction::opcode_cycles[0xC5] == 4); // push bc
    assert(instruction::opcode_cycles[0xC9] == 4); // ret
    assert(instruction::opcode_cycles[0xCB] == 0); // cb prefix (cycles handled by cb)
    assert(instruction::opcode_cycles[0xCD] == 6); // call a16
    assert(instruction::opcode_cycles[0xE0] == 3); // ldh (a8), a
    assert(instruction::opcode_cycles[0xEA] == 4); // ld (a16), a
    assert(instruction::opcode_cycles[0xF3] == 1); // di
    assert(instruction::opcode_cycles[0xFB] == 1); // ei

    // cb opcodes
    assert(instruction::cb_opcode_cycles[0x00] == 2); // rlc b
    assert(instruction::cb_opcode_cycles[0x06] == 4); // rlc (hl)
    assert(instruction::cb_opcode_cycles[0x7C] == 2); // bit 7, h
    assert(instruction::cb_opcode_cycles[0x7E] == 3); // bit 7, (hl)
    assert(instruction::cb_opcode_cycles[0x86] == 4); // res 0, (hl)
    assert(instruction::cb_opcode_cycles[0xC6] == 4); // set 0, (hl)

    std::cout << "Cycle lookup tables verified successfully!" << std::endl;
}

void test_timer_timing() {
    emulator emu;
    // timer divider ticks 1 for every 64 m-cycles (256 t-cycles)
    assert(emu.tmr->read(0xFF04) == 0);

    emu.add_cycles(63);
    assert(emu.tmr->read(0xFF04) == 0);

    emu.add_cycles(1);
    assert(emu.tmr->read(0xFF04) == 1);

    emu.add_cycles(64 * 5);
    assert(emu.tmr->read(0xFF04) == 6);

    std::cout << "Timer divider cycle timing verified successfully!" << std::endl;
}

void test_ppu_timing() {
    emulator emu;
    // ppu advances 1 scanline (ly) every 114 m-cycles (456 t-cycles)
    assert(emu.vid->read_reg(0xFF44) == 0);

    emu.add_cycles(113);
    assert(emu.vid->read_reg(0xFF44) == 0);

    emu.add_cycles(1);
    assert(emu.vid->read_reg(0xFF44) == 1);

    // 154 lines total per frame: 154 * 114 = 17556 m-cycles
    // already did 114 m-cycles, so do remaining 153 * 114 = 17442 m-cycles
    emu.add_cycles(17442);
    assert(emu.vid->is_frame_ready());
    assert(emu.vid->read_reg(0xFF44) == 0); // rolled over back to line 0

    std::cout << "PPU scanline and frame cycle timing verified successfully!" << std::endl;
}

int main() {
    test_tables();
    test_timer_timing();
    test_ppu_timing();
    std::cout << "All cycle timing integration tests passed 100%!" << std::endl;
    return 0;
}
