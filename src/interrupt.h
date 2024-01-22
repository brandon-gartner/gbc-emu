#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "processor.h"
#include "shared.h"
#include "stack_c.h"

class interrupt {
   public:
    enum interrupt_type {
        joypad = 0x10000,
        serial = 0x1000,
        timer = 0x100,
        lcd = 0x10,
        vblank = 0x1,
    };

    void request_interrupt();
    void handle_interrupts();

   private:
    processor* cpu;
    bool check_interrupt(interrupt_type);
    void handle_interrupt(uint16_t address);
    uint16_t get_interrupt_address(interrupt_type);
};

#endif