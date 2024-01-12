#include "emulator.h"

#include "cartridge.h"

emulator::emulator() {
    this->cpu = new processor(this);
    this->bus = new databus(this);
    this->current = emulator::status::stopped;
}

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
    this->cart = new cartridge(argv);
    this->cart->print_cartridge_info();

    if (!this->cart->valid) {
        std::cout << "ROM file failed to load." << std::endl;
        return 1;
    }
    std::cout << "ROM file loaded successfully." << std::endl;

    this->current = emulator::status::running;
    return this->start();
}

int emulator::start() {
    while (this->current != emulator::status::stopped) {
        this->cpu->step();
    }

    return 0;
}

int emulator::add_cycles(int cycles) {
    // NOT_IMPLEMENTED();
    int temp_var = cycles;
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