#ifndef EMULATOR_H
#define EMULATOR_H

#include "RAM.h"
#include "cartridge.h"
#include "databus.h"
#include "processor.h"
#include "shared.h"
#include "stack_c.h"

class stack_c;
class processor;
class emulator {
   public:
    enum status {
        running,
        paused,
        stopped
    };
    status main_mode;
    cartridge *cart;
    processor *cpu;
    databus *bus;
    ram *mem;
    stack_c *stack;
    emulator();
    int run(int argc, char **argv);
    int start();
    int add_cycles(int cycles);
    emulator::status get_status();
    void set_status(emulator::status new_status);

   private:
    status current;
};

#endif