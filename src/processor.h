#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <algorithm>
#include <cctype>
#include <functional>
#include <unordered_map>

#include "emulator.h"
#include "instruction.h"
#include "shared.h"

class emulator;
class instruction;
class processor {
   public:
    enum {
        running,
        stopped,
        stepping,
    } processor_status;
    emulator* emu;

    bool flag_z();
    bool flag_n();
    bool flag_h();
    bool flag_c();

    processor(emulator* emu);
    // uint8_t get_reg_8(char);
    // uint16_t get_reg_16(std::string);

    // uint8_t get_reg_8(register_type);
    // uint16_t get_reg_16(register_type);
    uint16_t get_reg(register_type);

    int step();
    void fetch_decode();
    void fetch();
    void decode();
    void execute();

    void set_pc(uint16_t);

    uint16_t fetch_result;
    uint16_t destination_address;
    bool destination_is_memory;
    uint8_t opcode;
    instruction* current;

   private:
    uint8_t reg_a;
    uint8_t reg_f;
    uint8_t reg_b;
    uint8_t reg_c;
    uint8_t reg_d;
    uint8_t reg_e;
    uint8_t reg_h;
    uint8_t reg_l;
    uint16_t reg_sp;
    uint16_t reg_pc;
};

#endif