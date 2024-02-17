#include "joypad.h"

#include "emulator.h"
#include "interrupt.h"

joypad::joypad(emulator* emu) : emu(emu), p1(0x30), directions(0x0F), actions(0x0F) {}

uint8_t joypad::read() {
    // bits 6 and 7 are always 1 on dmg; bits 4 and 5 reflect select lines
    uint8_t res = 0xCF | (p1 & 0x30);

    // bit 4 = 0: direction buttons selected
    if (!(p1 & 0x10)) {
        res &= (0xF0 | (directions & 0x0F));
    }

    // bit 5 = 0: action buttons selected
    if (!(p1 & 0x20)) {
        res &= (0xF0 | (actions & 0x0F));
    }

    return res;
}

void joypad::write(uint8_t value) {
    // only bits 4 and 5 are writable
    p1 = value & 0x30;
}

void joypad::key_down(button btn) {
    bool request_intr = false;

    if (btn <= DOWN) {
        // direction button
        uint8_t bit = 1 << btn;
        if (directions & bit) {
            directions &= ~bit;
            // if direction buttons are currently selected (bit 4 is 0)
            if (!(p1 & 0x10)) {
                request_intr = true;
            }
        }
    } else {
        // action button
        uint8_t bit = 1 << (btn - A);
        if (actions & bit) {
            actions &= ~bit;
            // if action buttons are currently selected (bit 5 is 0)
            if (!(p1 & 0x20)) {
                request_intr = true;
            }
        }
    }

    if (request_intr && emu && emu->intr) {
        emu->intr->request_interrupt(interrupt::joypad);
    }
}

void joypad::key_up(button btn) {
    if (btn <= DOWN) {
        directions |= (1 << btn);
    } else {
        actions |= (1 << (btn - A));
    }
}
