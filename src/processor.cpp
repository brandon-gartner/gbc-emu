#include "processor.h"

#include "databus.h"
#include "emulator.h"
#include "instruction.h"
#include "interrupt.h"
#include "ram.h"

processor::processor(emulator *emu)
    : emu(emu),
      reg_pc(0),
      fetch_result(0),
      destination_address(0),
      destination_is_memory(false),
      opcode(0),
      current(nullptr),
      reg_a(0),
      reg_f(0),
      reg_b(0),
      reg_c(0),
      reg_d(0),
      reg_e(0),
      reg_h(0),
      reg_l(0),
      reg_sp(0),
      double_speed(false),
      speed_switch_prepared(false) {
    init_registers(false);
}

void processor::init_registers(bool cgb) {
    processor_status = running;
    reg_pc = 0x0100;
    reg_sp = 0xFFFE;
    fetch_result = 0;
    destination_address = 0;
    destination_is_memory = false;
    opcode = 0;
    current = nullptr;
    double_speed = false;
    speed_switch_prepared = false;

    if (cgb) {
        // cgb power-up register state
        reg_a = 0x11;
        reg_f = 0x80;
        reg_b = 0x00;
        reg_c = 0x00;
        reg_d = 0xFF;
        reg_e = 0x56;
        reg_h = 0x00;
        reg_l = 0x0D;
    } else {
        // dmg power-up register state
        reg_a = 0x01;
        reg_f = 0xB0;
        reg_b = 0x00;
        reg_c = 0x13;
        reg_d = 0x00;
        reg_e = 0xD8;
        reg_h = 0x01;
        reg_l = 0x4D;
    }
}

int processor::step() {
    if (intr) {
        intr->handle_interrupts();
    }

    int cycles = 0;
    if (processor_status == running || processor_status == stepping) {
        // fetch/decode
        fetch_decode();
        // execute
        execute();
        cycles = instruction::opcode_cycles[opcode];
        emu->add_cycles(cycles);
    } else {
        cycles = 1;
        emu->add_cycles(1);
        if (interrupt_flags & emu->mem->get_interrupt() & 0x1F) {
            processor_status = running;
        }
    }

    // ei delay: interrupts are enabled after the instruction following ei
    if (ei_delay > 0) {
        ei_delay--;
        if (ei_delay == 0) {
            interrupt_enabled = true;
        }
    }

    return cycles;
}

void processor::fetch_decode() {
    fetch();
    decode();
}

void processor::fetch() {
    opcode = emu->bus->read8(reg_pc);
    reg_pc++;
    current = instruction::instruction_set[opcode];
    if (current == nullptr) {
        NOT_IMPLEMENTED();
    }
    current->cpu = this;
}

uint8_t processor::fetch_imm8() {
    uint8_t val = emu->bus->read8(reg_pc);
    reg_pc++;
    return val;
}

uint16_t processor::fetch_imm16() {
    uint8_t lo = emu->bus->read8(reg_pc);
    reg_pc++;
    uint8_t hi = emu->bus->read8(reg_pc);
    reg_pc++;
    return append(lo, hi);
}

void processor::decode() {
    destination_address = 0;
    destination_is_memory = false;
    switch (current->mode) {
        case addressing_mode::REG:
            fetch_result = get_reg(current->reg_1);
            return;

        case addressing_mode::REG_REG:
            fetch_result = get_reg(current->reg_2);
            return;

        case addressing_mode::REG_D8:
            fetch_result = fetch_imm8();
            return;

        case addressing_mode::REG_D16:
            fetch_result = fetch_imm16();
            return;

        case addressing_mode::REG_ADDR8:
            fetch_result = fetch_imm8();
            return;

        case addressing_mode::REG_ADDR16:
            fetch_result = emu->bus->read8(fetch_imm16());
            return;

        case addressing_mode::REG_MEMREG: {
            uint16_t address = (current->reg_2 == REG_C) ? (0xFF00 | get_reg(current->reg_2)) : get_reg(current->reg_2);
            fetch_result = emu->bus->read8(address);
            return;
        }

        case addressing_mode::REG_HLPLUS:
            fetch_result = emu->bus->read8(get_reg(current->reg_2));
            set_reg(current->reg_2, true, get_reg(current->reg_2) + 1);
            return;

        case addressing_mode::REG_HLMINUS:
            fetch_result = emu->bus->read8(get_reg(current->reg_2));
            set_reg(current->reg_2, true, get_reg(current->reg_2) - 1);
            return;

        case addressing_mode::D8:
            fetch_result = fetch_imm8();
            return;

        case addressing_mode::D8_REG:

        case addressing_mode::D16:
            fetch_result = fetch_imm16();
            return;

        case addressing_mode::D16_REG:
            destination_is_memory = true;
            destination_address = fetch_imm16();
            fetch_result = get_reg(current->reg_2);
            return;

        case addressing_mode::MEMREG_D8:
            destination_is_memory = true;
            fetch_result = fetch_imm8();
            destination_address = get_reg(current->reg_1);
            return;

        case addressing_mode::ADDR8_REG:
            destination_is_memory = true;
            destination_address = (fetch_imm8() | 0xFF00);
            return;

        case addressing_mode::ADDR16_REG:
            destination_is_memory = true;
            destination_address = fetch_imm16();
            fetch_result = get_reg(current->reg_2);
            return;

        case addressing_mode::MEMREG:
            destination_is_memory = true;
            destination_address = get_reg(current->reg_1);
            fetch_result = emu->bus->read8(destination_address);
            return;

        case addressing_mode::MEMREG_REG:
            destination_is_memory = true;
            fetch_result = get_reg(current->reg_2);
            destination_address = (current->reg_1 == REG_C) ? (0xFF00 | get_reg(current->reg_1)) : get_reg(current->reg_1);
            return;

        case addressing_mode::HLPLUS_REG:
            destination_is_memory = true;
            fetch_result = get_reg(current->reg_2);
            destination_address = get_reg(current->reg_1);
            set_reg(REG_HL, true, get_reg(REG_HL) + 1);
            return;

        case addressing_mode::HLMINUS_REG:
            destination_is_memory = true;
            fetch_result = get_reg(current->reg_2);
            destination_address = get_reg(current->reg_1);
            set_reg(REG_HL, true, get_reg(REG_HL) - 1);
            return;

        case addressing_mode::HL_SPTR:
            fetch_result = emu->bus->read8(reg_pc);
            reg_pc++;
            return;

        case addressing_mode::IMPLIED:
            return;
        default:
            return;
    }
}

void processor::execute() {
    current->execute();
}

uint16_t processor::get_reg(register_type reg) {
    switch (reg) {
        case register_type::REG_A:
            return reg_a;
        case register_type::REG_B:
            return reg_b;
        case register_type::REG_C:
            return reg_c;
        case register_type::REG_D:
            return reg_d;
        case register_type::REG_E:
            return reg_e;
        case register_type::REG_F:
            return reg_f;
        case register_type::REG_H:
            return reg_h;
        case register_type::REG_L:
            return reg_l;
        case register_type::REG_AF:
            return (reg_a << 8) | reg_f;
        case register_type::REG_BC:
            return (reg_b << 8) | reg_c;
        case register_type::REG_DE:
            return (reg_d << 8) | reg_e;
        case register_type::REG_HL:
            return (reg_h << 8) | reg_l;
        case register_type::REG_SP:
            return reg_sp;
        case register_type::REG_PC:
            return reg_pc;
        default:
            return 0;
    }
}

void processor::set_pc(uint16_t value) {
    reg_pc = value;
}

bool processor::get_flag_z() {
    return (reg_f & 0x80) == 0x80;
}

bool processor::get_flag_n() {
    return (reg_f & 0x40) == 0x40;
}

bool processor::get_flag_h() {
    return (reg_f & 0x20) == 0x20;
}

bool processor::get_flag_c() {
    return (reg_f & 0x10) == 0x10;
}

void processor::set_flag_z(bool flag) {
    if (flag) {
        reg_f |= 0x80;
    } else {
        reg_f &= ~0x80;
    }
}

void processor::set_flag_n(bool flag) {
    if (flag) {
        reg_f |= 0x40;
    } else {
        reg_f &= ~0x40;
    }
}

void processor::set_flag_h(bool flag) {
    if (flag) {
        reg_f |= 0x20;
    } else {
        reg_f &= ~0x20;
    }
}

void processor::set_flag_c(bool flag) {
    if (flag) {
        reg_f |= 0x10;
    } else {
        reg_f &= ~0x10;
    }
}

void processor::set_reg_8(register_type type, uint8_t new_val) {
    switch (type) {
        case REG_A:
            reg_a = new_val;
            return;
        case REG_F:
            reg_f = new_val;
            return;
        case REG_B:
            reg_b = new_val;
            return;
        case REG_C:
            reg_c = new_val;
            return;
        case REG_D:
            reg_d = new_val;
            return;
        case REG_E:
            reg_e = new_val;
            return;
        case REG_H:
            reg_h = new_val;
            return;
        case REG_L:
            reg_l = new_val;
            return;

        default:
            return;
    }
}

void processor::set_reg_16(register_type type, uint16_t new_val) {
    switch (type) {
        case REG_AF:
            reg_a = new_val >> 8;
            reg_f = new_val & 0xFF;
            return;
        case REG_BC:
            reg_b = new_val >> 8;
            reg_c = new_val & 0xFF;
            return;
        case REG_DE:
            reg_d = new_val >> 8;
            reg_e = new_val & 0xFF;
            return;
        case REG_HL:
            reg_h = new_val >> 8;
            reg_l = new_val & 0xFF;
            return;
        case REG_SP:
            reg_sp = new_val;
            return;
        case REG_PC:
            reg_pc = new_val;
            return;
        default:
            return;
    }
}

void processor::push8(uint8_t value) {
    reg_sp--;
    emu->bus->write8(reg_sp, value);
}

uint8_t processor::pop8() {
    uint8_t val = emu->bus->read8(reg_sp);
    reg_sp++;
    return val;
}

void processor::push16(uint16_t value) {
    push8(static_cast<uint8_t>((value >> 8) & 0xFF));
    push8(static_cast<uint8_t>(value & 0xFF));
}

uint16_t processor::pop16() {
    uint8_t lo = pop8();
    uint8_t hi = pop8();
    return (static_cast<uint16_t>(hi) << 8) | lo;
}

void processor::set_reg(register_type type, bool sixteen_bit, int new_val) {
    if (sixteen_bit) {
        set_reg_16(type, new_val);
    } else {
        set_reg_8(type, new_val);
    }
}

uint8_t processor::read_key1() const {
    // bit 7: current speed (0=normal, 1=double)
    // bits 1-6: unused, always read 1 (0x7E)
    // bit 0: prepare speed switch flag
    return (double_speed ? 0x80 : 0x00) | 0x7E | (speed_switch_prepared ? 0x01 : 0x00);
}

void processor::write_key1(uint8_t value) {
    // only bit 0 is writable
    speed_switch_prepared = (value & 0x01) != 0;
}

void processor::switch_speed() {
    double_speed = !double_speed;
    speed_switch_prepared = false;
}