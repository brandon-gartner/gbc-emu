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

    databus(emulator* emu);

   private:
    // databus(*emu);
};
#endif