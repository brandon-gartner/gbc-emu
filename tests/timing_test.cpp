#include <cassert>
#include <iostream>
#include "emulator.h"
#include "instruction.h"
#include "ppu.h"
#include "timer.h"

void test_tables() {
    // Standard opcodes
    assert(instruction::opcode_cycles[0x00] == 1); // NOP
    assert(instruction::opcode_cycles[0x01] == 3); // LD BC, d16
    assert(instruction::opcode_cycles[0x08] == 5); // LD (a16), SP
    assert(instruction::opcode_cycles[0x20] == 2); // JR NZ, r8 (base)
    assert(instruction::opcode_cycles[0x36] == 3); // LD (HL), d8
    assert(instruction::opcode_cycles[0x76] == 1); // HALT
    assert(instruction::opcode_cycles[0xC0] == 2); // RET NZ (base)
    assert(instruction::opcode_cycles[0xC3] == 4); // JP a16
    assert(instruction::opcode_cycles[0xC4] == 3); // CALL NZ, a16 (base)
    assert(instruction::opcode_cycles[0xC5] == 4); // PUSH BC
    assert(instruction::opcode_cycles[0xC9] == 4); // RET
    assert(instruction::opcode_cycles[0xCB] == 0); // CB prefix (cycles handled by cb)
    assert(instruction::opcode_cycles[0xCD] == 6); // CALL a16
    assert(instruction::opcode_cycles[0xE0] == 3); // LDH (a8), A
    assert(instruction::opcode_cycles[0xEA] == 4); // LD (a16), A
    assert(instruction::opcode_cycles[0xF3] == 1); // DI
    assert(instruction::opcode_cycles[0xFB] == 1); // EI

    // CB opcodes
    assert(instruction::cb_opcode_cycles[0x00] == 2); // RLC B
    assert(instruction::cb_opcode_cycles[0x06] == 4); // RLC (HL)
    assert(instruction::cb_opcode_cycles[0x7C] == 2); // BIT 7, H
    assert(instruction::cb_opcode_cycles[0x7E] == 3); // BIT 7, (HL)
    assert(instruction::cb_opcode_cycles[0x86] == 4); // RES 0, (HL)
    assert(instruction::cb_opcode_cycles[0xC6] == 4); // SET 0, (HL)

    std::cout << "Cycle lookup tables verified successfully!" << std::endl;
}

void test_timer_timing() {
    emulator emu;
    // timer divider ticks 1 for every 64 M-cycles (256 T-cycles)
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
    // PPU advances 1 scanline (LY) every 114 M-cycles (456 T-cycles)
    assert(emu.vid->read_reg(0xFF44) == 0);

    emu.add_cycles(113);
    assert(emu.vid->read_reg(0xFF44) == 0);

    emu.add_cycles(1);
    assert(emu.vid->read_reg(0xFF44) == 1);

    // 154 lines total per frame: 154 * 114 = 17556 M-cycles
    // We already did 114 M-cycles, so do remaining 153 * 114 = 17442 M-cycles
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
