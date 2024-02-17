#ifndef JOYPAD_H
#define JOYPAD_H

#include <cstdint>

class emulator;

class joypad {
   public:
    enum button {
        RIGHT = 0,
        LEFT = 1,
        UP = 2,
        DOWN = 3,
        A = 4,
        B = 5,
        SELECT = 6,
        START = 7
    };

    joypad(emulator* emu);

    uint8_t read();
    void write(uint8_t value);

    void key_down(button btn);
    void key_up(button btn);

   private:
    emulator* emu;
    uint8_t p1;          // select lines (bits 4 and 5)
    uint8_t directions;  // bits 0-3: right, left, up, down (0 = pressed, 1 = released)
    uint8_t actions;     // bits 0-3: a, b, select, start (0 = pressed, 1 = released)
};

#endif
