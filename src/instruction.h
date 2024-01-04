#include "shared.h"

// represents a game boy instruction, from the game boy instruction set sheet.
// TODO: add a link to the sheet once i have internet, or just upload it into resources.

class instruction {
   private:
   public:
    enum instruction_type {
        NONE,
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
        REG_16D,
        REG_REG,
        MEMREG_REG,
        REG,
        REG_8D,
        REG_MEMREG,
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

    instruction_type type;
    addressing_mode mode;
    register_type reg_1;
    register_type reg_2;
    uint8_t parameter;
};