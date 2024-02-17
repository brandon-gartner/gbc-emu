#include "emulator.h"

#include "cartridge.h"
#include "interrupt.h"
#include "joypad.h"
#include "ppu.h"
#include "timer.h"

emulator::emulator() {
    this->cart = new cartridge();
    this->bus = new databus(this);
    this->cpu = new processor(this);
    this->mem = new ram();
    this->stack = new stack_c(this);
    this->intr = new interrupt(this->cpu);
    this->cpu->intr = this->intr;
    this->tmr = new timer(this);
    this->vid = new ppu(this);
    this->pad = new joypad(this);
    this->current = emulator::status::stopped;
}

int emulator::run(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Please supply a ROM file to load." << std::endl;
        return 1;
    }
    std::cout << "Loading ROM file: " << argv[1] << std::endl;
    cart->load_cartridge(argv);
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

emulator::status emulator::get_status() {
    return this->current;
}

void emulator::set_status(emulator::status new_status) {
    this->current = new_status;
}

int emulator::add_cycles(int cycles) {
    if (this->tmr) {
        this->tmr->tick(cycles);
    }
    if (this->vid) {
        this->vid->tick(cycles);
    }
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