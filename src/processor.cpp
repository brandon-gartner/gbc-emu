#include "processor.h"

int processor::step() {
    return 0;
}

uint8_t processor::get_reg_8(char c) {
    switch (c) {
        case 'a':
            return reg_a;
        case 'b':
            return reg_b;
        case 'c':
            return reg_c;
        case 'd':
            return reg_d;
        case 'e':
            return reg_e;
        case 'f':
            return reg_f;
        case 'h':
            return reg_h;
        case 'l':
            return reg_l;
        default:
            std::cout << "Invalid 8-bit register accessed.  Program terminated." << std::endl;
            exit(1);
    }
}

uint16_t processor::get_reg_16(std::string s) {
    // TODO: lowercase s
    switch (s) {
        case std::to_string("af"):
            return append(reg_a, reg_f);
        case std::to_string("bc"):
            return append(reg_b, reg_c);
        case std::to_string("de"):
            return append(reg_d, reg_e);
        case std::to_string("hl"):
            return append(reg_h, reg_l);
        case std::to_string("sp"):
            return reg_sp;
        case std::to_string("pc"):
            return reg_pc;
        default:
            std::cout << "Invalid 16-bit register accessed.  Program terminated." << std::endl;
            exit(1);
    }
}

uint16_t append(uint8_t first, uint8_t second) {
    int returned = 0;
    returned |= first;
    returned <<= 4;
    returned |= second;
    return returned;
}