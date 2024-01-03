#include "cartridge.h"
#include "emulator.h"
#include "shared.h"

int main(int argc, char** argv) {
    emulator emu;
    return emu.run(argc, argv);
}