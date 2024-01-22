#ifndef STACK_C_H
#define STACK_C_H

#include "emulator.h"
#include "shared.h"

class emulator;

class stack_c {
   private:
    emulator* emu;

   public:
    void stack_push8(uint8_t value);
    uint8_t stack_pop8();
    void stack_push16(uint16_t value);
    uint16_t stack_pop16();
};

#endif