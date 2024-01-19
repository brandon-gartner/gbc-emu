#ifndef RAM_H
#define RAM_H
#include "shared.h"

class ram {
   private:
    uint8_t wram[0x2000];
    uint8_t hram[0x7F];
    uint8_t interrupt;

   public:
    uint8_t wram_read(uint16_t address);
    void wram_write(uint16_t address, uint8_t value);
    uint8_t hram_read(uint16_t address);
    void hram_write(uint16_t address, uint8_t value);
    uint8_t get_interrupt();
    void set_interrupt(uint8_t value);
};
#endif