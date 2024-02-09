#include "timer.h"

#include "emulator.h"
#include "interrupt.h"

timer::timer(emulator* emu) : emu(emu), div_counter(0), tima(0), tma(0), tac(0), timer_counter(0) {}

int timer::get_clock_threshold() {
    switch (tac & 0x03) {
        case 0:
            return 256;  // 4096 hz (1024 t-cycles = 256 m-cycles)
        case 1:
            return 4;    // 262144 hz (16 t-cycles = 4 m-cycles)
        case 2:
            return 16;   // 65536 hz (64 t-cycles = 16 m-cycles)
        case 3:
            return 64;   // 16384 hz (256 t-cycles = 64 m-cycles)
    }
    return 256;
}

void timer::tick(int cycles) {
    // advance internal divider (4 t-cycles per m-cycle)
    div_counter += cycles * 4;

    // timer counter enabled (bit 2 of tac)
    if (tac & 0x04) {
        timer_counter += cycles;
        int threshold = get_clock_threshold();
        while (timer_counter >= threshold) {
            timer_counter -= threshold;
            if (tima == 0xFF) {
                tima = tma;
                if (emu->intr) {
                    emu->intr->request_interrupt(interrupt::timer);
                }
            } else {
                tima++;
            }
        }
    }
}

uint8_t timer::read(uint16_t address) {
    switch (address) {
        case 0xFF04:
            // top 8 bits of internal 16-bit divider
            return (div_counter >> 8) & 0xFF;
        case 0xFF05:
            return tima;
        case 0xFF06:
            return tma;
        case 0xFF07:
            // upper 5 unused bits return 1s on dmg
            return tac | 0xF8;
    }
    return 0xFF;
}

void timer::write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF04:
            // writing any value to div resets it to 0
            div_counter = 0;
            break;
        case 0xFF05:
            tima = value;
            break;
        case 0xFF06:
            tma = value;
            break;
        case 0xFF07:
            tac = value & 0x07;
            break;
    }
}
