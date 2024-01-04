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

#endif