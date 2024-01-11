#include "instruction.h"

#include "shared.h"

int instruction::execute() {
    switch (this->type) {
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
            NOT_IMPLEMENTED();
    }
    return 0;  // wrong, TODO: decide if return int or void
}

void instruction::jp() {
    if (check_conditions()) {
        cpu->set_pc(cpu->fetch_result);
        cpu->emu->add_cycles(1);
    }
}

std::map<uint8_t, instruction *> instruction::instruction_set = {
    {0x00, new instruction{instruction_type::NOP, addressing_mode::IMPLIED}},
    {0x05, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_B}},
    {0x0E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_C}},
    {0xAF, new instruction{instruction_type::XOR, addressing_mode::REG, register_type::REG_A}},
    {0xC3, new instruction{instruction_type::JP, addressing_mode::D16}},

    // {0x00, new instruction{instruction_type::NOP, addressing_mode::IMPLIED}},
    // {0x01, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_BC}},
    // {0x02, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_A, register_type::REG_BC}},
    // {0x03, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_BC}},
    // {0x04, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_B}},
    // {0x05, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_B}},
    // {0x06, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_B}},
    // {0x07, new instruction{instruction_type::RLCA, addressing_mode::IMPLIED}},
    // {0x08, new instruction{instruction_type::LD, addressing_mode::MEMREG_D16, register_type::REG_SP}},
    // {0x09, new instruction{instruction_type::ADD, addressing_mode::REG_REG, register_type::REG_HL, register_type::REG_BC}},
    // {0x0A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_BC, register_type::REG_A}},
    // {0x0B, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_BC}},
    // {0x0C, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_C}},
    // {0x0D, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_C}},
    // {0x0E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_C}},
    // {0x0F, new instruction{instruction_type::RRCA, addressing_mode::IMPLIED}},
    // {0x10, new instruction{instruction_type::STOP, addressing_mode::IMPLIED}},
    // {0x11, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_DE}},
    // {0x12, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_A, register_type::REG_DE}},
    // {0x13, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_DE}},
    // {0x14, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_D}},
    // {0x15, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_D}},
    // {0x16, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_D}},
    // {0x17, new instruction{instruction_type::RLA, addressing_mode::IMPLIED}},
    // {0x18, new instruction{instruction_type::JR, addressing_mode::D8}},
    // {0x19, new instruction{instruction_type::ADD, addressing_mode::REG_REG, register_type::REG_HL, register_type::REG_DE}},
    // {0x1A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_DE, register_type::REG_A}},
    // {0x1B, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_DE}},
    // {0x1C, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_E}},
    // {0x1D, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_E}},
    // {0x1E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_E}},
    // {0x1F, new instruction{instruction_type::RRA, addressing_mode::IMPLIED}},
    // {0x20, new instruction{instruction_type::JR, addressing_mode::D8, register_type::REG_NZ}},
    // {0x21, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_HL}},
    // {0x22, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_A, register_type::REG_HL}},
    // {0x23, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_HL}},
    // {0x24, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_H}},
    // {0x25, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_H}},
    // {0x26, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_H}},
    // {0x27, new instruction{instruction_type::DAA, addressing_mode::IMPLIED}},
    // {0x28, new instruction{instruction_type::JR, addressing_mode::D8, register_type::REG_Z}},
    // {0x29, new instruction{instruction_type::ADD, addressing_mode::REG_REG, register_type::REG_HL, register_type::REG_HL}},
    // {0x2A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_A}},
    // {0x2B, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_HL}},
    // {0x2C, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_L}},
    // {0x2D, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_L}},
    // {0x2E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_L}},
    // {0x2F, new instruction{instruction_type::CPL, addressing_mode::IMPLIED}},
    // {0x30, new instruction{instruction_type::JR, addressing_mode::D8, register_type::REG_NC}},
    // {0x31, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_SP}},
    // {0x32, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_A, register_type::REG_HL}},
    // {0x33, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_SP}},
    // {0x34, new instruction{instruction_type::INC, addressing_mode::REG_MEMREG, register_type::REG_HL}},
    // {0x35, new instruction{instruction_type::DEC, addressing_mode::REG_MEMREG, register_type::REG_HL}},
    // {0x36, new instruction{instruction_type::LD, addressing_mode::MEMREG_D8, register_type::REG_HL}},
    // {0x37, new instruction{instruction_type::SCF, addressing_mode::IMPLIED}},
    // {0x38, new instruction{instruction_type::JR, addressing_mode::D8, register_type::REG_C}},
    // {0x39, new instruction{instruction_type::ADD, addressing_mode::REG_REG, register_type::REG_HL, register_type::REG_SP}},
    // {0x3A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_HL}},
    // {0x3B, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_SP}},
    // {0x3C, new instruction{instruction_type::INC, addressing_mode::REG, register_type::REG_A}},
    // {0x3D, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_A}},
    // {0x3E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_A}},
    // {0x3F, new instruction{instruction_type::CCF, addressing_mode::IMPLIED}},
    // {0x40, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_B}},
    // {0x41, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_C}},
    // {0x42, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_D}},
    // {0x43, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_E}},
    // {0x44, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_H}},
    // {0x45, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_L}},
    // {0x46, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_B, register_type::REG_HL}},
    // {0x47, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_A}},
    // {0x48, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_B}},
    // {0x49, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_C}},
    // {0x4A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_D}},
    // {0x4B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_E}},
    // {0x4C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_H}},
    // {0x4D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_L}},
    // {0x4E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_C, register_type::REG_HL}},
    // {0x4F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_A}},
    // {0x50, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_B}},
    // {0x51, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_C}},
    // {0x52, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_D}},
    // {0x53, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_E}},
    // {0x54, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_H}},
    // {0x55, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_L}},
    // {0x56, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_D, register_type::REG_H}},
    // {0x57, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_A}},
    // {0x58, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_B}},
    // {0x59, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_C}},
    // {0x5A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_D}},
    // {0x5B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_E}},
    // {0x5C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_H}},
    // {0x5D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_L}},
    // {0x5E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_E, register_type::REG_HL}},
    // {0x5F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_A}},
    // {0x60, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_B}},
    // {0x61, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_C}},
    // {0x62, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_D}},
    // {0x63, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_E}},
    // {0x64, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_H}},
    // {0x65, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_L}},
    // {0x66, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_H, register_type::REG_HL}},
    // {0x67, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_A}},
    // {0x68, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_B}},
    // {0x69, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_C}},
    // {0x6A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_D}},
    // {0x6B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_E}},
    // {0x6C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_H}},
    // {0x6D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_L}},
    // {0x6E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_L, register_type::REG_HL}},
    // {0x6F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_A}},
    // {0x70, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_HL, register_type::REG_B}},
    // {0x71, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_HL, register_type::REG_C}},
    // {0x72, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_HL, register_type::REG_D}},
    // {0x73, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_HL, register_type::REG_E}},
    // {0x74, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_HL, register_type::REG_H}},
    // {0x75, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_HL, register_type::REG_L}},
    // {0x76, new instruction{instruction_type::HALT, addressing_mode::IMPLIED}},
    // {0x77, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_HL}},
    // {0x78, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_B}},
    // {0x79, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_C}},
    // {0x7A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_D}},
    // {0xAF, new instruction{instruction_type::XOR, addressing_mode::REG, register_type::REG_A}},
    // {0xC3, new instruction{instruction_type::JP, addressing_mode::D16}},
    // {0xC5, new instruction{instruction_type::PUSH, addressing_mode::REG, register_type::REG_BC}},
    // {0xC9, new instruction{instruction_type::RET, addressing_mode::IMPLIED}},
    // {0xCD, new instruction{instruction_type::CALL, addressing_mode::D16}},
    // {0xE0, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_A}},
    // {0xE2, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_C}},
    // {0xEA, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_A, register_type::REG_D16}},
    // {0xF0, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_A}},
    // {0xFE, new instruction{instruction_type::CP, addressing_mode::D8}},
    // {0xFF, new instruction{instruction_type::RST, addressing_mode::D8}},
};

bool instruction::check_conditions() {
    bool z = cpu->flag_z();
    bool n = cpu->flag_n();
    bool h = cpu->flag_h();
    bool c = cpu->flag_c();

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
    print_instruction(type, mode, reg_1, reg_2, cond, parameter);
}