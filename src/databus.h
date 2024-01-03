#include "emulator.h"
#include "shared.h"

class databus {
   public:
    emulator* emu;

   private:
    // databus(*emu);
    uint8_t read(uint16_t);
    bool write(uint8_t, uint16_t);
};