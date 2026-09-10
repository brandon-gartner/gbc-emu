#include "interrupt.h"
#include "emulator.h"
#include "ram.h"

interrupt::interrupt(processor* cpu) : cpu(cpu) {}

uint16_t interrupt::get_interrupt_address(interrupt_type type) {
    switch (type) {
        case vblank:
            return 0x40;
        case lcd:
            return 0x48;
        case timer:
            return 0x50;
        case serial:
            return 0x58;
        case joypad:
            return 0x60;
    }
    return 0x00;
}

void interrupt::handle_interrupt(uint16_t address) {
    cpu->push16(cpu->get_reg(REG_PC));
    cpu->set_pc(address);
    cpu->emu->add_cycles(5);
}

bool interrupt::check_interrupt(interrupt_type type) {
    if (cpu->interrupt_enabled && (cpu->interrupt_flags & type) && (cpu->emu->mem->get_interrupt() & type)) {
        handle_interrupt(get_interrupt_address(type));
        cpu->interrupt_flags &= ~(type);
        cpu->emu->set_status(cpu->emu->main_mode);
        cpu->processor_status = processor::running;
        cpu->interrupt_enabled = false;
        return true;
    }

    return false;
}

void interrupt::handle_interrupts() {
    if (!cpu->interrupt_enabled) {
        return;
    }

    if (check_interrupt(vblank)) {
        return;
    } else if (check_interrupt(lcd)) {
        return;
    } else if (check_interrupt(timer)) {
        return;
    } else if (check_interrupt(serial)) {
        return;
    } else if (check_interrupt(joypad)) {
        return;
    }
}

void interrupt::request_interrupt(interrupt_type type) {
    cpu->interrupt_flags |= type;
}