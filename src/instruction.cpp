#include "instruction.h"

#include "shared.h"

int instruction::execute() {
    switch (this->type) {
        case DI:
            di();
        case NOP:
            break;
        case LD:
            ld();
            break;
        case LDH:
            ldh();
            break;
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

void instruction::ld() {
    if (cpu->current->mode == addressing_mode::HL_SPTR) {
        NOT_IMPLEMENTED();
        return;
    }
    if (cpu->destination_is_memory) {
        if (reg_is_8bit(cpu->current->reg_2)) {
            cpu->emu->bus->write8(cpu->destination_address, cpu->fetch_result);
        } else {
            cpu->emu->add_cycles(1);
            cpu->emu->bus->write16(cpu->destination_address, cpu->fetch_result);
        }
        return;
    }

    // TODO: verify that this is correct, should be?
    cpu->set_reg(cpu->current->reg_1, false, cpu->fetch_result);
}

void instruction::ldh() {
    if (cpu->current->reg_1 == register_type::REG_A) {
        cpu->set_reg(cpu->current->reg_1, false, cpu->fetch_result | 0xFF00);
    } else {
        cpu->emu->bus->write8(cpu->fetch_result | 0xFF00, cpu->get_reg(cpu->current->reg_2));
    }

    cpu->emu->add_cycles(1);
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
// might need to come back to here for a lot of them, check todo
std::map<uint8_t, instruction*> instruction::instruction_set = {
    // 0x0
    {0x00, new instruction{instruction_type::NOP, addressing_mode::IMPLIED}},
    {0x01, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_BC}},
    {0x02, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_BC, register_type::REG_A}},
    {0x05, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_B}},
    {0x06, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_B}},
    {0x08, new instruction{instruction_type::LD, addressing_mode::ADDR16_REG, register_type::NONE, register_type::REG_SP}},
    {0x0A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_BC}},
    {0x0E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_C}},

    // 0x1
    // load instructions are mostly clones of 0x loads, but BC -> DE, B -> D, C -> E
    {0x11, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_DE}},
    {0x12, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_DE, register_type::REG_A}},
    {0x15, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_D}},
    {0x16, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_D}},
    {0x1A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_DE}},
    {0x1E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_E}},

    // 0x2
    // load instructions are mostly clones of 0x loads, but BC -> HL, B -> H, C -> L
    {0x21, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_HL}},
    {0x22, new instruction{instruction_type::LD, addressing_mode::HLMINUS_REG, register_type::REG_HL, register_type::REG_A}},
    {0x25, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_H}},
    {0x26, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_H}},
    {0x2A, new instruction{instruction_type::LD, addressing_mode::REG_HLPLUS, register_type::REG_A, register_type::REG_HL}},
    {0x2E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_L}},

    // 0x3 load instructions are mostly clones of 0x loads,
    // but this one gets a bit more complex.HLMINUS is used,
    // and there are less direct swaps.in most cases,
    // B->SP,C->A
    {0x31, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_SP}},
    {0x32, new instruction{instruction_type::LD, addressing_mode::HLMINUS_REG, register_type::REG_HL, register_type::REG_A}},
    {0x35, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_HL}},
    {0x36, new instruction{instruction_type::LD, addressing_mode::MEMREG_D8, register_type::REG_HL}},
    {0x3A, new instruction{instruction_type::LD, addressing_mode::REG_HLMINUS, register_type::REG_A, register_type::REG_HL}},
    {0x3E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_A}},

    // 0x4
    // this, 0x5, 0x6, and 0x7 are all just full of loads, nothing else at all.
    {0x40, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_B}},
    {0x41, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_C}},
    {0x42, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_D}},
    {0x43, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_E}},
    {0x44, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_H}},
    {0x45, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_L}},
    {0x46, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_B, register_type::REG_HL}},
    {0x47, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_A}},
    {0x48, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_B}},
    {0x49, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_C}},
    {0x4A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_D}},
    {0x4B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_E}},
    {0x4C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_H}},
    {0x4D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_L}},
    {0x4E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_C, register_type::REG_HL}},
    {0x4F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_A}},

    // 0x5
    // follows a similar pattern to 0x4, except with D instead of B and E instead of C
    {0x50, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_B}},
    {0x51, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_C}},
    {0x52, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_D}},
    {0x53, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_E}},
    {0x54, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_H}},
    {0x55, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_L}},
    {0x56, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_D, register_type::REG_HL}},
    {0x57, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_A}},
    {0x58, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_B}},
    {0x59, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_C}},
    {0x5A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_D}},
    {0x5B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_E}},
    {0x5C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_H}},
    {0x5D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_L}},
    {0x5E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_E, register_type::REG_HL}},
    {0x5F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_A}},

    // 0x6
    // follows a similar pattern to 0x4, except with H instead of B and L instead of C
    {0x60, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_B}},
    {0x61, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_C}},
    {0x62, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_D}},
    {0x63, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_E}},
    {0x64, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_H}},
    {0x65, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_L}},
    {0x66, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_H, register_type::REG_HL}},
    {0x67, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_A}},
    {0x68, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_B}},
    {0x69, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_C}},
    {0x6A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_D}},
    {0x6B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_E}},
    {0x6C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_H}},
    {0x6D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_L}},
    {0x6E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_L, register_type::REG_HL}},
    {0x6F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_A}},

    // 0x7
    // follows a similar pattern to 0x4, except with A instead of B and HL instead of C
    {0x70, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_B}},
    {0x71, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_C}},
    {0x72, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_D}},
    {0x73, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_E}},
    {0x74, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_H}},
    {0x75, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_L}},
    {0x76, new instruction{instruction_type::HALT}},
    {0x77, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_A}},
    {0x78, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_B}},
    {0x79, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_C}},
    {0x7A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_D}},
    {0x7B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_E}},
    {0x7C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_H}},
    {0x7D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_L}},
    {0x7E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_HL}},
    {0x7F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_A}},

    {0xAF, new instruction{instruction_type::XOR, addressing_mode::REG, register_type::REG_A}},
    {0xC3, new instruction{instruction_type::JP, addressing_mode::D16}},

    // 0xE
    {0xE0, new instruction{instruction_type::LDH, addressing_mode::ADDR8_REG, register_type::NONE, register_type::REG_A}},
    {0xE2, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_C, register_type::REG_A}},
    {0xEA, new instruction{instruction_type::LD, addressing_mode::ADDR16_REG, register_type::NONE, register_type::REG_A}},

    // 0xF
    {0xF0, new instruction{instruction_type::LDH, addressing_mode::REG_ADDR8, register_type::REG_A}},
    {0xF2, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_C}},
    {0xF3, new instruction{instruction_type::DI, addressing_mode::IMPLIED}},
    {0xFA, new instruction{instruction_type::LD, addressing_mode::REG_ADDR16, register_type::REG_A}},
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
    std::cout << int_to_hex(cpu->opcode) << " " << int_to_hex(cpu->emu->bus->read8(cpu->reg_pc + 1)) << " " << int_to_hex(cpu->emu->bus->read8(cpu->reg_pc + 2)) << std::endl;
    // print_instructions(type, mode, reg_1, reg_2, cond, parameter);
}