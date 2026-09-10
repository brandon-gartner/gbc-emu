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
class interrupt;
class processor {
   public:
    enum {
        running,
        stopped,
        stepping,
    } processor_status;
    emulator* emu;

    uint16_t reg_pc;

    bool get_flag_z();
    bool get_flag_n();
    bool get_flag_h();
    bool get_flag_c();

    void set_flag_z(bool);
    void set_flag_n(bool);
    void set_flag_h(bool);
    void set_flag_c(bool);

    processor(emulator* emu);
    void init_registers(bool cgb);
    uint16_t get_reg(register_type);
    void set_reg(register_type, bool, int);

    int step();
    void fetch_decode();
    void fetch();
    void decode();
    void execute();

    void set_pc(uint16_t);
    void set_reg_8(register_type, uint8_t);
    void set_reg_16(register_type, uint16_t);

    // stack operations
    void push8(uint8_t value);
    uint8_t pop8();
    void push16(uint16_t value);
    uint16_t pop16();

    bool is_double_speed() const { return double_speed; }
    bool is_speed_switch_prepared() const { return speed_switch_prepared; }
    uint8_t read_key1() const;
    void write_key1(uint8_t value);
    void switch_speed();

    uint16_t fetch_result;
    uint16_t destination_address;
    bool destination_is_memory;
    int ei_delay = 0;
    bool interrupt_enabled = true;
    uint8_t interrupt_flags = 0;
    interrupt* intr = nullptr;
    uint8_t opcode;
    instruction* current;
    uint8_t reg_a;
    uint8_t reg_f;
    uint8_t reg_b;
    uint8_t reg_c;
    uint8_t reg_d;
    uint8_t reg_e;
    uint8_t reg_h;
    uint8_t reg_l;
    uint16_t reg_sp;

   private:
    uint8_t fetch_imm8();
    uint16_t fetch_imm16();
    bool double_speed;
    bool speed_switch_prepared;
};

#endif