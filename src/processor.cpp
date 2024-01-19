#include "processor.h"

#include "instruction.h"

processor::processor(emulator *emu) : emu(emu), reg_pc(0), fetch_result(0), destination_address(0), destination_is_memory(false), opcode(0), current(nullptr), reg_a(0), reg_f(0), reg_b(0), reg_c(0), reg_d(0), reg_e(0), reg_h(0), reg_l(0), reg_sp(0) {
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
    opcode = emu->bus->read8(reg_pc);
    std::cout << "Opcode: " << int_to_hex(opcode) << std::endl;
    reg_pc++;
    // std::cout << "Getting next instruction" << std::endl;
    current = instruction::instruction_set[opcode];
    if (current == nullptr) {
        NOT_IMPLEMENTED();
    }
    // std::cout << "Updating cpu pointer in instruction" << std::endl;
    current->cpu = this;
    // std::cout << "Printing instruction" << std::endl;
    current->print();
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
            fetch_result = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            return;

        case addressing_mode::REG_D16: {
            uint8_t lo = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            uint8_t hi = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            fetch_result = append(lo, hi);
            return;
        }

        case addressing_mode::REG_ADDR8:
            fetch_result = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            return;

        case addressing_mode::REG_ADDR16: {
            uint8_t lo = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            uint8_t hi = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            fetch_result = emu->bus->read8(append(lo, hi));
            emu->add_cycles(1);
            return;
        }

        case addressing_mode::REG_MEMREG: {
            uint16_t address = get_reg(current->reg_2);
            fetch_result = emu->bus->read8(address);
            emu->add_cycles(1);
            return;
        }

        case addressing_mode::REG_HLPLUS:
            fetch_result = emu->bus->read8(get_reg(current->reg_2));
            emu->add_cycles(1);
            set_reg(current->reg_2, true, get_reg(current->reg_2) + 1);
            return;

        case addressing_mode::REG_HLMINUS:
            fetch_result = emu->bus->read8(get_reg(current->reg_2));
            emu->add_cycles(1);
            set_reg(current->reg_2, true, get_reg(current->reg_2) - 1);

        case addressing_mode::D8:
            return;

        case addressing_mode::D8_REG:

        case addressing_mode::D16: {
            uint8_t lo = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            uint8_t hi = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            fetch_result = append(lo, hi);
            return;
        }

        case addressing_mode::D16_REG: {
            destination_is_memory = true;
            uint8_t lo = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            uint8_t hi = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            destination_address = append(lo, hi);
            fetch_result = get_reg(current->reg_2);
            return;
        }

        case addressing_mode::MEMREG_D8:
            destination_is_memory = true;
            fetch_result = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            destination_address = get_reg(current->reg_1);
            return;

        case addressing_mode::ADDR8_REG:
            destination_is_memory = true;
            destination_address = (emu->bus->read8(reg_pc) | 0xFF00);
            emu->add_cycles(1);
            reg_pc++;
            return;

        case addressing_mode::ADDR16_REG: {
            destination_is_memory = true;
            uint8_t lo = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            uint8_t hi = emu->bus->read8(reg_pc);
            emu->add_cycles(1);
            reg_pc++;
            destination_address = append(lo, hi);
            fetch_result = get_reg(current->reg_2);
            return;
        }

        case addressing_mode::MEMREG:
            destination_is_memory = true;
            destination_address = get_reg(current->reg_1);
            fetch_result = emu->bus->read8(current->reg_1);
            emu->add_cycles(1);
            return;

        case addressing_mode::MEMREG_REG:
            destination_is_memory = true;
            fetch_result = get_reg(current->reg_2);
            destination_address = get_reg(current->reg_1);
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
            emu->add_cycles(1);
            reg_pc++;
            return;

        case addressing_mode::IMPLIED:
            return;
        default:
            std::cout << "Invalid addressing mode.  Program terminated." << std::endl;
            exit(1);
    }
}

void processor::execute() {
    printf("Executing instruction: %02X\n", opcode);
    // printf("Current PC: %04X\n", reg_pc);
    current->execute();
    // NOT_IMPLEMENTED();
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

// TODO: test these better
void processor::set_flag_z(bool flag) {
    reg_f |= flag * 0x80;
}

void processor::set_flag_n(bool flag) {
    reg_f |= flag * 0x40;
}

void processor::set_flag_h(bool flag) {
    reg_f |= flag * 0x20;
}

void processor::set_flag_c(bool flag) {
    reg_f |= flag * 0x10;
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

        case NONE:
        case REG_AF:
            std::cout << "Invalid register for 8-bit register modification: AF" << std::endl;
            exit(1);
        case REG_BC:
            std::cout << "Invalid register for 8-bit register modification: BC" << std::endl;
            exit(1);
        case REG_DE:
            std::cout << "Invalid register for 8-bit register modification: DE" << std::endl;
            exit(1);
        case REG_HL:
            std::cout << "Invalid register for 8-bit register modification: HL" << std::endl;
            exit(1);
        case REG_SP:
            std::cout << "Invalid register for 8-bit register modification: SP" << std::endl;
            exit(1);
        case REG_PC:
            std::cout << "Invalid register for 8-bit register modification: PC" << std::endl;
            exit(1);
    }
}

void processor::set_reg_16(register_type type, uint16_t new_val) {
    switch (type) {
        case NONE:
        case REG_A:
        case REG_F:
        case REG_B:
        case REG_C:
        case REG_D:
        case REG_E:
        case REG_H:
        case REG_L:
            std::cout << "Invalid register for 16-bit register modification." << std::endl;
            exit(1);
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
    }
}

void processor::set_reg(register_type type, bool sixteen_bit, int new_val) {
    if (sixteen_bit) {
        set_reg_16(type, new_val);
    } else {
        set_reg_8(type, new_val);
    }
}