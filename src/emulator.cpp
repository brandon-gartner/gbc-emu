#include "emulator.h"

#include "cartridge.h"
#include "display.h"
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
    this->disp = new display();
    this->main_mode = emulator::status::running;
    this->current = emulator::status::stopped;
}

emulator::~emulator() {
    delete this->disp;
    delete this->pad;
    delete this->vid;
    delete this->tmr;
    delete this->intr;
    delete this->stack;
    delete this->mem;
    delete this->cpu;
    delete this->bus;
    delete this->cart;
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
    if (this->disp) {
        if (!this->disp->init("gbc-emu", 3)) {
            std::cout << "Warning: Display initialization failed. Running without display." << std::endl;
        }
    }

    while (this->current != emulator::status::stopped) {
        this->cpu->step();

        if (this->vid && this->vid->is_frame_ready()) {
            if (this->disp && this->disp->is_initialized()) {
                this->disp->update(this->vid->get_framebuffer());
                this->disp->poll_events(this);
            }
            this->vid->clear_frame_ready();
        }
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