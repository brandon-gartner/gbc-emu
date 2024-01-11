#ifndef EMULATOR_H
#define EMULATOR_H

#include "cartridge.h"
#include "databus.h"
#include "processor.h"
#include "shared.h"

class processor;
class emulator {
   public:
    enum status {
        running,
        paused,
        stopped
    };
    cartridge *cart;
    processor *cpu;
    databus *bus;
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