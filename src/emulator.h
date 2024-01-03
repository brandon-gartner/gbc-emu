#ifndef EMULATOR_H
#define EMULATOR_H
#include "cartridge.h"
#include "processor.h"
#include "shared.h"

class emulator {
   public:
    enum status {
        running,
        paused,
        stopped
    };
    cartridge cart;
    processor cpu;

    int run(int argc, char** argv);
    int start();
    emulator::status get_status();
    void set_status(emulator::status new_status);

   private:
    status current;
};

#endif