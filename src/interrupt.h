#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "processor.h"
#include "shared.h"
#include "stack_c.h"

class interrupt {
   public:
    enum interrupt_type {
        joypad = 0x10,
        serial = 0x08,
        timer = 0x04,
        lcd = 0x02,
        vblank = 0x01,
    };

    interrupt(processor* cpu);
    void request_interrupt(interrupt_type type);
    void handle_interrupts();

   private:
    processor* cpu;
    bool check_interrupt(interrupt_type);
    void handle_interrupt(uint16_t address);
    uint16_t get_interrupt_address(interrupt_type);
};

#endif