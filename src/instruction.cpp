#include "instruction.h"

#include "shared.h"

int instruction::execute() {
    switch (this->type) {
        case DI:
            di();
        case NOP:
            break;
        // case LD:
        // case INC:
        // case DEC:
        // case RLCA:
        // case ADD:
        // case RRCA:
        // case STOP:
        // case RLA:
        // case JR:
        // case RRA:
        // case DAA:
        // case CPL:
        // case SCF:
        // case CCF:
        // case HALT:
        // case ADC:
        // case SUB:
        // case SBC:
        // case AND:
        // case XOR:
        // case OR:
        // case CP:
        // case RET:
        // case POP:
        case JP:
            jp();
            break;

            // case CALL:
            // case PUSH:
            // case RST:
            // case RETI:
            // case CB:

        default:
            std::cout << "Instruction type not implemented.  Program terminated." << std::endl;
            NOT_IMPLEMENTED();
    }
    return 0;  // wrong, TODO: decide if return int or void
}

void instruction::di() {
    cpu->interrupt_enabled = false;
}

void instruction::jp() {
    // std::cout << "JP" << std::endl;
    if (check_conditions()) {
        // std::cout << "Checked conditions" << std::endl;
        cpu->set_pc(cpu->fetch_result);
        // std::cout << "PC updated" << std::endl;
        cpu->emu->add_cycles(1);
        // std::cout << "Cycles added" << std::endl;
    }
}

void instruction::run_xor() {
    cpu->reg_a ^= (cpu->fetch_result & 0xFF);
    this->cpu->set_flag_z(cpu->reg_a == 0);
    this->cpu->set_flag_n(0);
    this->cpu->set_flag_h(0);
    this->cpu->set_flag_c(0);
}

// whenever using this, set the cpu immediately.
std::map<uint8_t, instruction *> instruction::instruction_set = {
    {0x00, new instruction{instruction_type::NOP, addressing_mode::IMPLIED}},
    {0x05, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_B}},
    {0x0E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_C}},
    {0xAF, new instruction{instruction_type::XOR, addressing_mode::REG, register_type::REG_A}},
    {0xC3, new instruction{instruction_type::JP, addressing_mode::D16}},
    {0xF3, new instruction{instruction_type::DI, addressing_mode::IMPLIED}},
};

bool instruction::check_conditions() {
    bool z = cpu->get_flag_z();
    bool n = cpu->get_flag_n();
    bool h = cpu->get_flag_h();
    bool c = cpu->get_flag_c();

    switch (this->cond) {
        case condition_type::ALWAYS:
            return true;
        case condition_type::NZ:
            return !z;
        case condition_type::Z:
            return z;
        case condition_type::NC:
            return !c;
        case condition_type::C:
            return c;
        case condition_type::NH:
            return !h;
        case condition_type::H:
            return h;
        case condition_type::NN:
            return !n;
        case condition_type::N:
            return n;
        default:
            std::cout << "Invalid condition type.  Program terminated." << std::endl;
            exit(1);
    }
}

void instruction::print() {
    simple_print();
}

void instruction::simple_print() {
    std::cout << this->cpu->reg_pc << ": ";
    std::cout << print_instruction_type(this->type) << " ";
    std::cout << int_to_hex(cpu->opcode) << " " << int_to_hex(cpu->emu->bus->read(cpu->reg_pc + 1)) << " " << int_to_hex(cpu->emu->bus->read(cpu->reg_pc + 2)) << std::endl;
    // print_instructions(type, mode, reg_1, reg_2, cond, parameter);
}