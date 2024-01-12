#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "processor.h"
#include "shared.h"

// represents a game boy instruction, from the game boy instruction set sheet.
// TODO: add a link to the sheet once i have internet, or just upload it into resources.

class processor;
class instruction {
   private:
   public:
    static std::map<uint8_t, instruction*> instruction_set;

    instruction(instruction_type type, addressing_mode mode)
        : type(type), mode(mode) {}
    instruction(instruction_type type, addressing_mode mode, register_type reg_1)
        : type(type), mode(mode), reg_1(reg_1) {}
    instruction(instruction_type type, addressing_mode mode, register_type reg_1, register_type reg_2)
        : type(type), mode(mode), reg_1(reg_1), reg_2(reg_2) {}
    instruction(instruction_type type, addressing_mode mode, register_type reg_1, register_type reg_2, uint8_t parameter)
        : type(type), mode(mode), reg_1(reg_1), reg_2(reg_2), parameter(parameter) {}

    int execute();
    bool check_conditions();
    void print();
    void simple_print();

    // codes
    void di();
    void jp();
    void run_xor();

    processor* cpu;
    instruction_type type;
    addressing_mode mode;
    register_type reg_1;
    register_type reg_2;
    condition_type cond;
    uint8_t parameter;
};

#endif