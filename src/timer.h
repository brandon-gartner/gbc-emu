#ifndef TIMER_H
#define TIMER_H

#include <cstdint>

class emulator;

class timer {
   public:
    timer(emulator* emu);

    void tick(int cycles);
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

   private:
    emulator* emu;
    uint16_t div_counter;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;
    int timer_counter;

    int get_clock_threshold();
};

#endif
