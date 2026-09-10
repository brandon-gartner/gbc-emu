#ifndef EMULATOR_H
#define EMULATOR_H

#include <memory>
#include "shared.h"

class cartridge;
class processor;
class databus;
class ram;
class interrupt;
class timer;
class ppu;
class joypad;
class display;

class emulator {
   public:
    enum status {
        running,
        paused,
        stopped
    };
    status main_mode;
    std::unique_ptr<cartridge> cart;
    std::unique_ptr<processor> cpu;
    std::unique_ptr<databus> bus;
    std::unique_ptr<ram> mem;
    std::unique_ptr<interrupt> intr;
    std::unique_ptr<timer> tmr;
    std::unique_ptr<ppu> vid;
    std::unique_ptr<joypad> pad;
    std::unique_ptr<display> disp;

    emulator();
    ~emulator();
    emulator(const emulator&) = delete;
    emulator& operator=(const emulator&) = delete;
    emulator(emulator&&) = default;
    emulator& operator=(emulator&&) = default;

    int run(int argc, char **argv);
    int start();
    int add_cycles(int cycles);
    emulator::status get_status();
    void set_status(emulator::status new_status);
    bool is_cgb;
    bool is_cgb_mode() const { return is_cgb; }
    void init_hardware_mode(bool cgb);

   private:
    status current;
};

#endif