#include "processor.h"

#include "instruction.h"

uint16_t append(uint8_t lo, uint8_t hi) {
    int returned = 0;
    returned |= hi;
    returned <<= 8;
    returned |= lo;
    return returned;
}

processor::processor(emulator *emu) : emu(emu), fetch_result(0), destination_address(0), destination_is_memory(false), opcode(0), current(nullptr), reg_a(0), reg_f(0), reg_b(0), reg_c(0), reg_d(0), reg_e(0), reg_h(0), reg_l(0), reg_sp(0), reg_pc(0) {
    processor_status = running;
    reg_pc = 0x100;
    fetch_result = 0;
    reg_a = 0x01;
}

int processor::step() {
    if (processor_status == running || processor_status == stepping) {
        // fetch/decode
        fetch_decode();
        // execute
        execute();
        return 0;
    }
    return 1;
}

void processor::fetch_decode() {
    fetch();
    decode();
}

void processor::fetch() {
    opcode = emu->bus->read(reg_pc);
    std::cout << "opcode: " << std::to_string(opcode) << std::endl;
    reg_pc++;
    current = instruction::instruction_set[opcode];
    current->print();
}

void processor::decode() {
    destination_address = 0;
    destination_is_memory = false;
    switch (current->mode) {
        case addressing_mode::REG_D16:
        case addressing_mode::REG_REG:
        case addressing_mode::MEMREG_REG:
        case addressing_mode::REG:
            fetch_result = get_reg(current->reg_1);
            return;
        case addressing_mode::REG_D8:
            fetch_result = emu->bus->read(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            return;

        case addressing_mode::REG_MEMREG:
        case addressing_mode::D8:
        case addressing_mode::D16: {
            uint8_t lo = emu->bus->read(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            uint8_t hi = emu->bus->read(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            fetch_result = append(lo, hi);
            return;
        }
        case addressing_mode::MEMREG:
        case addressing_mode::IMPLIED:
            return;
        default:
            std::cout << "Invalid addressing mode.  Program terminated." << std::endl;
            exit(1);
    }
}

void processor::execute() {
    printf("Executing instruction: %02X\n", opcode);
    printf("Current PC: %04X\n", reg_pc);
    current->execute();
    NOT_IMPLEMENTED();
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
            return append(reg_a, reg_f);
        case register_type::REG_BC:
            return append(reg_b, reg_c);
        case register_type::REG_DE:
            return append(reg_d, reg_e);
        case register_type::REG_HL:
            return append(reg_h, reg_l);
        case register_type::REG_SP:
            return reg_sp;
        case register_type::REG_PC:
            return reg_pc;
        default:
            std::cout << "Invalid register accessed.  Program terminated." << std::endl;
            exit(1);
    }
}

void processor::set_pc(uint16_t value) {
    reg_pc = value;
}

// TODO: test these better
bool processor::flag_z() {
    return (reg_f & 0x80) == 0x80;
}

bool processor::flag_n() {
    return (reg_f & 0x40) == 0x40;
}

bool processor::flag_h() {
    return (reg_f & 0x20) == 0x20;
}

bool processor::flag_c() {
    return (reg_f & 0x10) == 0x10;
}