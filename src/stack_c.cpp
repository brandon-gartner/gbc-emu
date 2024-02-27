#include "stack_c.h"

stack_c::stack_c(emulator* emu) : emu(emu) {}

void stack_c::stack_push8(uint8_t value) {
    emu->cpu->set_reg(REG_SP, true, emu->cpu->get_reg(REG_SP) - 1);
    emu->bus->write8(emu->cpu->get_reg(REG_SP), value);
}

uint8_t stack_c::stack_pop8() {
    uint8_t retrieved = emu->bus->read8(emu->cpu->get_reg(REG_SP));
    emu->cpu->set_reg(REG_SP, true, emu->cpu->get_reg(REG_SP) + 1);
    return retrieved;
}

void stack_c::stack_push16(uint16_t value) {
    stack_push8(value >> 8);
    stack_push8(value & 0xFF);
}

uint16_t stack_c::stack_pop16() {
    return append(stack_pop8(), stack_pop8());
}