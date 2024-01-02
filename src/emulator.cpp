#include "emulator.h"

#include <iostream>

#include "cartridge.h"

emulator::status emulator::get_status() {
    return this->current;
}

void emulator::set_status(emulator::status new_status) {
    this->current = new_status;
}

int emulator::run(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Please supply a ROM file to load." << std::endl;
        return 1;
    }
    std::cout << "Loading ROM file: " << argv[1] << std::endl;
    this->cart = cartridge(argv);
    this->cart.print_cartridge_info();

    if (!this->cart.valid) {
        std::cout << "ROM file failed to load." << std::endl;
        return 1;
    }
    std::cout << "ROM file loaded successfully." << std::endl;

    this->current = emulator::status::running;
    return this->start();
}

int emulator::start() {
    // while (this->current != emulator::status::stopped) {
    this->cpu.step();
    // }

    return 0;
}

// setup:
// main----|
//         emulator----|
//                     cartridge
//                     cpu
//                     buses
//                     memory
//                     ppu