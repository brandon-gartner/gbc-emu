#include <iostream>

#include "cartridge.h"
#include "emulator.h"

int main(int argc, char** argv) {
    emulator emu;
    return emu.run(argc, argv);
}