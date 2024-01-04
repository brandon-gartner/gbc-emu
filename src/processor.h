#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <algorithm>
#include <cctype>
#include <functional>
#include <unordered_map>

#include "instruction.h"
#include "shared.h"

class processor {
   public:
    enum {
        running,
        stopped,
        stepping,
    } processor_status;

    uint8_t get_reg_8(char);
    uint16_t get_reg_16(std::string);

    int step();
    uint8_t fetch();

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

    uint16_t fetch_result;
    uint16_t memory_address;
    uint8_t opcode;
    instruction *current;
};

#endif