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
    static std::map<int, register_type> cb_registers;

    instruction(instruction_type type)
        : type(type) {}
    instruction(instruction_type type, addressing_mode mode)
        : type(type), mode(mode) {}
    instruction(instruction_type type, addressing_mode mode, register_type reg_1)
        : type(type), mode(mode), reg_1(reg_1) {}
    instruction(instruction_type type, addressing_mode mode, register_type reg_1, register_type reg_2)
        : type(type), mode(mode), reg_1(reg_1), reg_2(reg_2) {}
    instruction(instruction_type type, addressing_mode mode, register_type reg_1, register_type reg_2, condition_type cond)
        : type(type), mode(mode), reg_1(reg_1), reg_2(reg_2), cond(cond) {}
    instruction(instruction_type type, addressing_mode mode, register_type reg_1, register_type reg_2, condition_type cond, uint8_t parameter)
        : type(type), mode(mode), reg_1(reg_1), reg_2(reg_2), cond(cond), parameter(parameter) {}

    int execute();
    bool check_conditions();
    void print();
    void simple_print();

    // codes
    void inc();
    void dec();
    void add();
    void adc();
    void sub();
    void sbc();
    void ei();
    void di();
    void jp();
    void jr();
    void call();
    void ret();
    void rst();
    void reti();
    void rlca();
    void rrca();
    void rra();
    void rla();
    void daa();
    void cpl();
    void scf();
    void ccf();
    void push();
    void pop();
    void ld();
    void ldh();
    void run_and();
    void run_xor();
    void run_or();
    void cp();
    void halt();
    void cb();
    void bit(int, int);
    void res(int, int);
    void set(int, int);
    void rlc(int);
    void rrc(int);
    void rl(int);
    void rr(int);
    void sla(int);
    void sra(int);
    void swap(int);
    void srl(int);

    processor* cpu;
    uint8_t opcode;
    instruction_type type;
    addressing_mode mode;
    register_type reg_1;
    register_type reg_2;
    condition_type cond;
    uint8_t parameter;
};

#endif