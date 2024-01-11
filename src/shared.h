#ifndef SHARED_H
#define SHARED_H
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#define NOT_IMPLEMENTED()                                                         \
    std::cout << "Not implemented: " << __FILE__ << ":" << __LINE__ << std::endl; \
    exit(1);

enum instruction_type {
    NOP,
    LD,
    INC,
    DEC,
    RLCA,
    ADD,
    RRCA,
    STOP,
    RLA,
    JR,
    RRA,
    DAA,
    CPL,
    SCF,
    CCF,
    HALT,
    ADC,
    SUB,
    SBC,
    AND,
    XOR,
    OR,
    CP,
    RET,
    POP,
    JP,
    CALL,
    PUSH,
    RST,
    RETI,
    CB,
};

enum addressing_mode {
    REG_D16,
    REG_REG,
    MEMREG_REG,
    REG,
    REG_D8,
    REG_MEMREG,
    D8,
    D16,
    MEMREG,
    IMPLIED,
};

enum register_type {
    NONE,
    REG_A,
    REG_F,
    REG_B,
    REG_C,
    REG_D,
    REG_E,
    REG_H,
    REG_L,
    REG_AF,
    REG_BC,
    REG_DE,
    REG_HL,
    REG_SP,
    REG_PC,
};

enum condition_type {
    NZ,      // not zero
    Z,       // zero
    NC,      // not carry
    C,       // carry
    NH,      // not half carry
    H,       // half carry
    NN,      // not negative
    N,       // negative
    ALWAYS,  // always
};

std::string print_condition(condition_type cond);
std::string print_register(register_type reg);
std::string print_addressing_mode(addressing_mode mode);
std::string print_instruction_type(instruction_type type);
void print_instruction(instruction_type type, addressing_mode mode, register_type reg_1, register_type reg_2, condition_type cond, uint8_t parameter);

#endif