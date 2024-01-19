#include "shared.h"

std::string print_condition(condition_type cond) {
    switch (cond) {
        case NZ:
            return "NZ";
        case Z:
            return "Z";
        case NC:
            return "NC";
        case C:
            return "C";
        case NH:
            return "NH";
        case H:
            return "H";
        case NN:
            return "NN";
        case N:
            return "N";
        case ALWAYS:
            return "ALWAYS";
        default:
            return "UNKNOWN";
    }
}

std::string print_register(register_type reg) {
    switch (reg) {
        case NONE:
            return "NONE";
        case REG_A:
            return "A";
        case REG_F:
            return "F";
        case REG_B:
            return "B";
        case REG_C:
            return "C";
        case REG_D:
            return "D";
        case REG_E:
            return "E";
        case REG_H:
            return "H";
        case REG_L:
            return "L";
        case REG_AF:
            return "AF";
        case REG_BC:
            return "BC";
        case REG_DE:
            return "DE";
        case REG_HL:
            return "HL";
        case REG_SP:
            return "SP";
        case REG_PC:
            return "PC";
        default:
            return "UNKNOWN";
    }
}

std::string print_addressing_mode(addressing_mode mode) {
    switch (mode) {
        case REG_D16:
            return "REG_D16";
        case REG_REG:
            return "REG_REG";
        case MEMREG_REG:
            return "MEMREG_REG";
        case REG:
            return "REG";
        case REG_D8:
            return "REG_D8";
        case REG_MEMREG:
            return "REG_MEMREG";
        case D8:
            return "D8";
        case D16:
            return "D16";
        case REG_HLPLUS:
            return "REG_HLPLUS";
        case REG_HLMINUS:
            return "REG_HLMINUS";
        case MEMREG:
            return "MEMREG";
        case IMPLIED:
            return "IMPLIED";
        default:
            return "UNKNOWN";
    }
}

std::string print_instruction_type(instruction_type type) {
    switch (type) {
        case NOP:
            return "NOP";
        case LD:
            return "LD";
        case DI:
            return "DI";
        case INC:
            return "INC";
        case DEC:
            return "DEC";
        case RLCA:
            return "RLCA";
        case ADD:
            return "ADD";
        case RRCA:
            return "RRCA";
        case STOP:
            return "STOP";
        case RLA:
            return "RLA";
        case JR:
            return "JR";
        case RRA:
            return "RRA";
        case DAA:
            return "DAA";
        case CPL:
            return "CPL";
        case SCF:
            return "SCF";
        case CCF:
            return "CCF";
        case HALT:
            return "HALT";
        case ADC:
            return "ADC";
        case SUB:
            return "SUB";
        case SBC:
            return "SBC";
        case AND:
            return "AND";
        case XOR:
            return "XOR";
        case OR:
            return "OR";
        case CP:
            return "CP";
        case RET:
            return "RET";
        case POP:
            return "POP";
        case JP:
            return "JP";
        case CALL:
            return "CALL";
        case PUSH:
            return "PUSH";
        case RST:
            return "RST";
        case RETI:
            return "RETI";
        case CB:
            return "CB";
        default:
            return "UNKNOWN";
    }
}

void print_instructions(instruction_type type, addressing_mode mode, register_type reg_1, register_type reg_2, condition_type cond, uint8_t parameter) {
    std::cout << "Instruction: " << std::endl;
    std::cout << "    Type: " << print_instruction_type(type) << std::endl;
    std::cout << "    Mode: " << print_addressing_mode(mode) << std::endl;
    std::cout << "    Reg 1: " << print_register(reg_1) << std::endl;
    std::cout << "    Reg 2: " << print_register(reg_2) << std::endl;
    std::cout << "    Cond: " << print_condition(cond) << std::endl;
    std::cout << "    Parameter: " << std::to_string(parameter) << std::endl;
}

std::string int_to_hex(uint8_t value) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2) << std::hex << (int)value;
    return stream.str();
}

std::string int_to_hex(uint16_t value) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(4) << std::hex << (int)value;
    return stream.str();
}

bool reg_is_8bit(register_type reg) {
    switch (reg) {
        case REG_A:
        case REG_F:
        case REG_B:
        case REG_C:
        case REG_D:
        case REG_E:
        case REG_H:
        case REG_L:
            return true;
        case REG_AF:
        case REG_BC:
        case REG_DE:
        case REG_HL:
        case REG_SP:
        case REG_PC:
            return false;
        case NONE:
            std::cout << "Invalid register." << std::endl;
            exit(1);
    }
    return false;
}

uint16_t append(uint8_t lo, uint8_t hi) {
    int returned = 0;
    returned |= hi;
    returned <<= 8;
    returned |= lo;
    return returned;
}