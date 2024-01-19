#include "stack_c.h"

void stack_c::stack_push8(uint8_t value) {
    emu->bus->write8(emu->cpu->get_sp(), value);
    emu->cpu->set_sp(emu->cpu->get_sp() - 1);
}

uint8_t stack_c::stack_pop8() {
    uint8_t retrieved = emu->bus->read8(emu->cpu->get_sp());
    emu->cpu->set_sp(emu->cpu->get_sp() + 1);
    return retrieved;
}

// TODO: check if 0xFF for first one?
void stack_c::stack_push16(uint16_t value) {
    stack_push8(value >> 8);
    stack_push8(value & 0xFF);
}

// TODO: make sure this good?
uint16_t stack_c::stack_pop16() {
    // uint16_t value = stack_pop8();
    // value |= stack_pop8() << 8;
    return append(stack_pop8(), stack_pop8());
}