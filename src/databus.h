#ifndef DATABUS_H
#define DATABUS_H
#include "shared.h"

class emulator;
class databus {
   public:
    emulator* emu;
    uint8_t read(uint16_t);
    bool write(uint8_t, uint16_t);

    databus(emulator* emu);

   private:
    // databus(*emu);
};
#endif