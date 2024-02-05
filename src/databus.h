#ifndef DATABUS_H
#define DATABUS_H
#include "shared.h"

class emulator;
class databus {
   public:
    emulator* emu;
    uint8_t read8(uint16_t);
    void write8(uint16_t, uint8_t);
    uint16_t read16(uint16_t);
    void write16(uint16_t, uint16_t);

    uint8_t io[0x80] = {0};
    uint8_t io_read(uint16_t address);
    void io_write(uint16_t address, uint8_t value);

    databus(emulator* emu);

   private:
    // databus(*emu);
};
#endif