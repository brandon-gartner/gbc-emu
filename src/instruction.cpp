#include "instruction.h"

#include "shared.h"

int instruction::execute() {
    switch (this->type) {
        case DI:
            di();
        case NOP:
            break;
        case LD:
            ld();
            break;
        case LDH:
            ldh();
            break;
        case INC:
            inc();
            break;
        case DEC:
            dec();
            break;
        case RLCA:
            rlca();
            return 0;
        case ADD:
            add();
            break;
        case RRCA:
            rrca();
            break;
        // case STOP:
        case RLA:
            rla();
            return 0;
        case JR:
            jr();
            break;
        case RRA:
            rra();
            return 0;
        case DAA:
            daa();
            return 0;
        case CPL:
            cpl();
            return 0;
        case SCF:
            scf();
            return 0;
        case CCF:
            ccf();
            return 0;
        case HALT:
            halt();
            return 0;
        case ADC:
            adc();
            break;
        case SUB:
            sub();
            break;
        case SBC:
            sbc();
            break;
        case AND:
            run_and();  // and is a c++ keyword
            break;
        case XOR:
            run_xor();  // xor is a c++ keyword
            break;
        case OR:
            run_or();  // or is a c++ keyword
            break;
        case CP:
            cp();
            break;
        case RET:
            ret();
            break;
        case RETI:
            reti();
            break;
        case PUSH:
            push();
            break;
        case POP:
            pop();
            break;
        case JP:
            jp();
            break;
        case CALL:
            call();
            break;
        case RST:
            rst();
            break;
        case CB:
            cb();
            break;

        default:
            std::cout << "Instruction type not implemented.  Program terminated." << std::endl;
            NOT_IMPLEMENTED();
    }
    return 0;  // wrong, TODO: decide if return int or void
}

// TODO: make sure that double setting the reg wont matter, it shouldnt, but...
void instruction::inc() {
    if (reg_is_8bit(reg_1)) {
        cpu->set_reg(this->reg_1, false, cpu->get_reg(this->reg_1) + 1);
    } else {
        cpu->set_reg(this->reg_1, true, cpu->get_reg(this->reg_1) + 1);
        cpu->emu->add_cycles(1);
    }

    uint16_t result = 0;
    // special case
    if (mode == MEMREG && reg_1 == REG_HL) {
        result = ((cpu->emu->bus->read8(cpu->get_reg(REG_HL)) + 1) & 0xFF);
        cpu->emu->bus->write8(REG_HL, result);
    } else {
        // TODO: this will break.  absolutely.  figure out a way to set_reg where you don't need to know at compile-time.
        result = cpu->get_reg(this->reg_1) + 1;  // this line might be kind of funny.  test it.
        // cpu->set_reg(reg_1, false, cpu->get_reg(this->reg_1) + 1);
        cpu->set_reg(reg_1, !reg_is_8bit(reg_1), cpu->get_reg(this->reg_1) + 1);
    }
    // cpu->set_reg(this->reg_1, cpu->get_reg(this->reg_1) + 1);

    // don't want to override flags for certain cases of inc
    if ((opcode & 0x03) == 0x03) {
        return;
    }

    cpu->set_flag_z(result == 0);
    cpu->set_flag_n(0);
    cpu->set_flag_h((result & 0x0F) == 0x00);
    cpu->set_flag_c(0);  // TODO: test if this is 0 or -1
}

// TODO: make sure that double setting the reg wont matter, it shouldnt, but...
void instruction::dec() {
    if (reg_is_8bit(reg_1)) {
        cpu->set_reg(this->reg_1, false, cpu->get_reg(this->reg_1) - 1);
    } else {
        cpu->set_reg(this->reg_1, true, cpu->get_reg(this->reg_1) - 1);
        cpu->emu->add_cycles(1);
    }

    uint16_t result = 0;
    // special case
    if (mode == MEMREG && reg_1 == REG_HL) {
        result = ((cpu->emu->bus->read8(cpu->get_reg(REG_HL)) - 1) & 0xFF);
        cpu->emu->bus->write8(REG_HL, result);
    } else {
        // TODO: this will break.  absolutely.  figure out a way to set_reg where you don't need to know at compile-time.
        result = cpu->get_reg(this->reg_1) - 1;  // this line might be kind of funny.  test it.
        // cpu->set_reg(reg_1, false, cpu->get_reg(this->reg_1) + 1);
        cpu->set_reg(reg_1, !reg_is_8bit(reg_1), cpu->get_reg(this->reg_1) - 1);
    }
    // cpu->set_reg(this->reg_1, cpu->get_reg(this->reg_1) + 1);

    // don't want to override flags for certain cases of inc
    if ((opcode & 0x0B) == 0x0B) {
        return;
    }

    cpu->set_flag_z(result == 0);
    cpu->set_flag_n(1);
    cpu->set_flag_h((result & 0x0F) == 0x0F);
    cpu->set_flag_c(0);  // TODO: test if this is 0 or -1
}

void instruction::add() {
    uint32_t result = 0;  // uint32 because 16bit + 16bit can carry over into 32bit.
    int z_flag = 0;
    int h_flag = 0;
    int c_flag = 0;
    result = cpu->get_reg(this->reg_1) + cpu->fetch_result;

    if (!reg_is_8bit(reg_1)) {
        cpu->emu->add_cycles(1);
    }

    if (reg_1 == REG_SP) {
        result = cpu->reg_sp + (char)cpu->fetch_result;  // allows for subtraction if fetch_result is negative
    }

    // TODO: make a function for each flag check
    z_flag = (result & 0xFF) == 0;
    h_flag = ((cpu->get_reg(this->reg_1) & 0x0F) + (cpu->fetch_result & 0x0F)) > 0x0F;  // 0x0F or 0x10
    c_flag = (result & 0x100) == 0x100;                                                 // TODO: verify this line

    if (!reg_is_8bit(reg_1)) {
        z_flag = -1;
        // allow addition to surpass 16-bit
        h_flag = ((int)cpu->get_reg(this->reg_1) & 0x0FFF) + (cpu->fetch_result & 0x0FFF) > 0x0FFF;  // 0xFFF or 0x1000
        c_flag = ((int)result & 0x10000) == 0x10000;                                                 // TODO: figure out if result should be right here
    }

    if (reg_1 == REG_SP) {
        // code reuse from 2 lines above
        z_flag = 0;
        h_flag = ((cpu->get_reg(REG_SP) & 0x0F) + (cpu->fetch_result & 0x0F)) > 0x0F;  // 0x0F or 0x10
        c_flag = (result & 0x100) == 0x100;                                            // TODO: verify this line
    }

    cpu->set_reg(this->reg_1, !reg_is_8bit(reg_1), result);
    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(0);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::adc() {
    uint32_t result = cpu->fetch_result;
    uint8_t acc = cpu->get_reg(REG_A);
    uint8_t carry = cpu->get_flag_c();

    cpu->set_reg(REG_A, reg_is_8bit(reg_1), acc + result + carry);

    int z_flag = (result & 0xFF) == 0;
    int h_flag = carry + ((cpu->get_reg(this->reg_1) & 0x0F) + (cpu->fetch_result & 0x0F)) > 0x0F;  // 0x0F or 0x10
    int c_flag = (result + acc + carry > 0xFF);                                                     // TODO: verify this line

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(0);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::sub() {
    // need not be 32, no overflow
    uint16_t result = cpu->get_reg(reg_1) - cpu->fetch_result;

    int z_flag = (result == 0);
    int h_flag = ((int)reg_1 & 0xF) - ((int)cpu->fetch_result & 0xF) < 0;
    int c_flag = (((int)reg_1 < 0) - ((int)cpu->fetch_result)) < 0;

    cpu->set_reg(reg_1, !reg_is_8bit(reg_1), result);
    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(1);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::sbc() {
    int carry = cpu->get_flag_c();
    uint8_t sub_amount = cpu->fetch_result + cpu->get_flag_c();

    int z_flag = cpu->get_reg(reg_1) - sub_amount == 0;
    int h_flag = ((int)reg_1 & 0xF) - ((int)sub_amount & 0xF) - carry < 0;
    int c_flag = ((int)cpu->get_reg(reg_1) - (int)sub_amount) < 0;  // may need carry here brain is tired

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(0);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}
void instruction::ei() {
    cpu->enable_interrupts_next = true;
}

void instruction::di() {
    cpu->interrupt_enabled = false;
}

void instruction::ld() {
    if (cpu->current->mode == addressing_mode::HL_SPTR) {
        NOT_IMPLEMENTED();
        return;
    }
    if (cpu->destination_is_memory) {
        if (reg_is_8bit(cpu->current->reg_2)) {
            cpu->emu->bus->write8(cpu->destination_address, cpu->fetch_result);
        } else {
            cpu->emu->add_cycles(1);
            cpu->emu->bus->write16(cpu->destination_address, cpu->fetch_result);
        }
        return;
    }

    // TODO: verify that this is correct, should be?
    cpu->set_reg(cpu->current->reg_1, false, cpu->fetch_result);
}

void instruction::ldh() {
    if (cpu->current->reg_1 == register_type::REG_A) {
        cpu->set_reg(cpu->current->reg_1, false, cpu->fetch_result | 0xFF00);
    } else {
        cpu->emu->bus->write8(cpu->fetch_result | 0xFF00, cpu->get_reg(cpu->current->reg_2));
    }

    cpu->emu->add_cycles(1);
}

// push and pop are always 16bit values

// make sure no & 0xFF is needed here
void instruction::push() {
    uint16_t hi = cpu->get_reg(cpu->current->reg_1) >> 8;
    cpu->emu->add_cycles(1);
    cpu->emu->stack->stack_push8(hi);

    uint16_t lo = cpu->get_reg(cpu->current->reg_1) & 0xFF;
    cpu->emu->add_cycles(1);
    cpu->emu->stack->stack_push8(lo);

    cpu->emu->add_cycles(1);
}

void instruction::pop() {
    uint16_t lo = cpu->emu->stack->stack_pop8();
    cpu->emu->add_cycles(1);
    uint16_t hi = cpu->emu->stack->stack_pop8();
    cpu->emu->add_cycles(1);

    uint16_t result = append(lo, hi);

    if (cpu->current->reg_1 != register_type::REG_AF) {
        cpu->set_reg(cpu->current->reg_1, true, result);
    } else {
        // this is the expected behaviour for AF, this way it also
        // does not mess up the flags on original hardware
        cpu->set_reg(cpu->current->reg_1, true, result & 0xFFF0);
    }

    // slightly different if AF:
}

void instruction::jp() {
    // std::cout << "JP" << std::endl;
    if (check_conditions()) {
        // std::cout << "Checked conditions" << std::endl;
        cpu->set_pc(cpu->fetch_result);
        // std::cout << "PC updated" << std::endl;
        cpu->emu->add_cycles(1);
        // std::cout << "Cycles added" << std::endl;
    }
}

// jump relative
void instruction::jr() {
    if (check_conditions()) {
        cpu->set_pc(cpu->reg_pc + ((char)(cpu->fetch_result) % 0xFF));
        // cpu->set_pc(cpu->reg_pc + ((uint8_t)(cpu->fetch_result) % 0xFF));
        cpu->emu->add_cycles(1);
    }
}

void instruction::call() {
    if (check_conditions()) {
        cpu->emu->stack->stack_push16(cpu->reg_pc);
        cpu->emu->add_cycles(2);
        // std::cout << "Checked conditions" << std::endl;

        cpu->set_pc(cpu->fetch_result);
        // std::cout << "PC updated" << std::endl;
        cpu->emu->add_cycles(1);
        // std::cout << "Cycles added" << std::endl;
    }
}

void instruction::ret() {
    if (cond != ALWAYS) {
        cpu->emu->add_cycles(1);
    }

    if (check_conditions()) {
        uint16_t lo = cpu->emu->stack->stack_pop8();
        cpu->emu->add_cycles(1);

        uint16_t hi = cpu->emu->stack->stack_pop8();
        cpu->emu->add_cycles(1);

        cpu->set_pc(append(lo, hi));
        cpu->emu->add_cycles(1);
    }
}

void instruction::rst() {
    if (check_conditions()) {
        cpu->emu->stack->stack_push16(cpu->reg_pc);
        cpu->emu->add_cycles(2);

        cpu->set_pc(cpu->current->parameter);
        cpu->emu->add_cycles(1);
    }
}

// return from interrupt
void instruction::reti() {
    ret();
    cpu->interrupt_enabled = true;
}

void instruction::rlca() {
    uint8_t result = cpu->get_reg(REG_A);
    bool c_flag = (result >> 7) & 1;
    result = (result << 1) | c_flag;
    cpu->set_reg(REG_A, false, result);

    cpu->set_flag_z(0);
    cpu->set_flag_n(0);
    cpu->set_flag_h(0);
    cpu->set_flag_c(c_flag);
}

void instruction::rrca() {
    uint8_t reg_a_bit_1 = cpu->get_reg(REG_A) & 1;
    uint8_t new_a_reg = (cpu->get_reg(REG_A) >> 1);
    new_a_reg |= (reg_a_bit_1 << 7);
    cpu->set_reg(REG_A, false, new_a_reg);

    cpu->set_flag_z(0);
    cpu->set_flag_n(0);
    cpu->set_flag_h(0);
    cpu->set_flag_c(reg_a_bit_1);
}

void instruction::rla() {
    uint8_t reg_a_val = cpu->get_reg(REG_A);
    uint8_t old_c_flag = cpu->get_flag_c();
    uint8_t new_c_flag = (reg_a_val >> 7) & 1;

    cpu->set_reg(REG_A, false, ((reg_a_val << 1)) | old_c_flag);

    cpu->set_flag_z(0);
    cpu->set_flag_n(0);
    cpu->set_flag_h(0);
    cpu->set_flag_c(new_c_flag);
}

void instruction::rra() {
    uint8_t old_c_flag = cpu->get_flag_c();
    uint8_t new_c_flag = cpu->get_reg(REG_A) & 1;
    uint8_t new_a = cpu->get_reg(REG_A) >> 1;
    new_a |= (old_c_flag << 7);

    cpu->set_flag_z(0);
    cpu->set_flag_n(0);
    cpu->set_flag_h(0);
    cpu->set_flag_c(new_c_flag);
}

// pretty much how gambatte does it.  i can't understand this one.
void instruction::daa() {
    uint8_t num = 0;
    bool carry = false;

    bool n_flag = cpu->get_flag_n();
    bool h_flag = cpu->get_flag_h();
    bool c_flag = cpu->get_flag_c();

    if (h_flag || (!n_flag && (cpu->get_reg(REG_A) & 0xF) > 9)) {
        num = 6;
    }

    if (c_flag || !(n_flag && cpu->get_reg(REG_A) > 0x99)) {
        num |= 0x60;
        carry = true;
    }

    uint8_t a_value = cpu->get_reg(REG_A);
    if (n_flag) {
        a_value -= num;
    } else {
        a_value += num;
    }

    bool z_flag = (a_value == 0);
    cpu->set_reg(REG_A, false, a_value);
    n_flag = -1;
    h_flag = 0;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(0);
    cpu->set_flag_c(carry);
}

void instruction::cpl() {
    cpu->set_reg(REG_A, false, -cpu->get_reg(REG_A));
    cpu->set_flag_z(-1);
    cpu->set_flag_n(1);
    cpu->set_flag_h(1);
    cpu->set_flag_c(-1);
}

void instruction::scf() {
    cpu->set_flag_z(-1);
    cpu->set_flag_n(0);
    cpu->set_flag_h(0);
    cpu->set_flag_c(1);
}

void instruction::ccf() {
    cpu->set_flag_z(-1);
    cpu->set_flag_n(0);
    cpu->set_flag_h(0);
    cpu->set_flag_c(cpu->get_flag_c() ^ 1);
}

void instruction::run_and() {
    cpu->reg_a &= cpu->fetch_result;
    this->cpu->set_flag_z(cpu->reg_a == 0);
    this->cpu->set_flag_n(0);
    this->cpu->set_flag_h(1);
    this->cpu->set_flag_c(0);
}

void instruction::run_xor() {
    cpu->reg_a ^= (cpu->fetch_result & 0xFF);
    this->cpu->set_flag_z(cpu->reg_a == 0);
    this->cpu->set_flag_n(0);
    this->cpu->set_flag_h(0);
    this->cpu->set_flag_c(0);
}

void instruction::run_or() {
    cpu->reg_a |= (cpu->fetch_result & 0xFF);
    this->cpu->set_flag_z(cpu->reg_a == 0);
    this->cpu->set_flag_n(0);
    this->cpu->set_flag_h(0);
    this->cpu->set_flag_c(0);
}

// cp = compare
// subtraction except the result isn't stored.  flags are updated though.
void instruction::cp() {
    uint16_t result = ((int)cpu->get_reg(REG_A) - cpu->fetch_result);

    int z_flag = (result == 0);
    int h_flag = ((int)cpu->get_reg(REG_A) & 0xF) - ((int)cpu->fetch_result & 0xF) < 0;
    int c_flag = ((int)result < 0);

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(1);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::halt() {
    cpu->emu->set_status(emulator::status::paused);
}

// whenever using this, set the cpu immediately.
// might need to come back to here for a lot of them, check todo
std::map<uint8_t, instruction*> instruction::instruction_set = {

    // 0x0 ✓
    {0x00, new instruction{instruction_type::NOP, addressing_mode::IMPLIED}},
    {0x01, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_BC}},
    {0x02, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_BC, register_type::REG_A}},
    {0x03, new instruction{INC, REG, REG_BC}},
    {0x04, new instruction{INC, REG, REG_B}},
    {0x05, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_B}},
    {0x06, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_B}},
    {0x07, new instruction{RLCA}},
    {0x08, new instruction{instruction_type::LD, addressing_mode::ADDR16_REG, register_type::NONE, register_type::REG_SP}},
    {0x09, new instruction{instruction_type::ADD, REG_REG, REG_HL, REG_BC}},
    {0x0A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_BC}},
    {0x0B, new instruction{DEC, REG, REG_BC}},
    {0x0C, new instruction{INC, REG, REG_C}},
    {0x0D, new instruction{DEC, REG, REG_C}},
    {0x0E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_C}},
    {0x0F, new instruction{RRCA}},

    // 0x1 ✓
    // load instructions are mostly clones of 0x loads, but BC -> DE, B -> D, C -> E
    {0x10, new instruction{STOP}},
    {0x11, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_DE}},
    {0x12, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_DE, register_type::REG_A}},
    {0x13, new instruction{INC, REG, REG_DE}},
    {0x14, new instruction{INC, REG, REG_D}},
    {0x15, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_D}},
    {0x16, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_D}},
    {0x17, new instruction{RLA}},
    {0x18, new instruction{JR, D8}},
    {0x19, new instruction{instruction_type::ADD, REG_REG, REG_HL, REG_DE}},
    {0x1A, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_DE}},
    {0x1B, new instruction{DEC, REG, REG_DE}},
    {0x1C, new instruction{INC, REG, REG_E}},
    {0x1D, new instruction{DEC, REG, REG_E}},
    {0x1E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_E}},
    {0x1F, new instruction{RRA}},

    // 0x2
    // load instructions are mostly clones of 0x loads, but BC -> HL, B -> H, C -> L0x20, new instruction{JR, D8, NONE, NONE, NZ}
    {0x20, new instruction{JR, D8, NONE, NONE, NZ}},
    {0x21, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_HL}},
    {0x22, new instruction{instruction_type::LD, addressing_mode::HLMINUS_REG, register_type::REG_HL, register_type::REG_A}},
    {0x23, new instruction{INC, REG, REG_HL}},
    {0x24, new instruction{INC, REG, REG_H}},
    {0x25, new instruction{instruction_type::DEC, addressing_mode::REG, register_type::REG_H}},
    {0x26, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_H}},
    {0x28, new instruction{JR, D8, NONE, NONE, Z}},
    {0x29, new instruction{instruction_type::ADD, REG_REG, REG_HL, REG_HL}},
    {0x2A, new instruction{instruction_type::LD, addressing_mode::REG_HLPLUS, register_type::REG_A, register_type::REG_HL}},
    {0x2B, new instruction{DEC, REG, REG_HL}},
    {0x2E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_L}},
    {0x2C, new instruction{INC, REG, REG_L}},
    {0x2D, new instruction{DEC, REG, REG_L}},
    {0x2E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_L}},

    // 0x3
    // load instructions are mostly clones of 0x loads,
    // but this one gets a bit more complex.HLMINUS is used,
    // and there are less direct swaps.in most cases,
    // B->SP,C->A
    {0x30, new instruction{JR, D8, NONE, NONE, NC}},
    {0x31, new instruction{instruction_type::LD, addressing_mode::REG_D16, register_type::REG_SP}},
    {0x32, new instruction{instruction_type::LD, addressing_mode::HLMINUS_REG, register_type::REG_HL, register_type::REG_A}},
    {0x33, new instruction{INC, REG, REG_SP}},
    {0x34, new instruction{INC, MEMREG, REG_HL}},
    {0x35, new instruction{instruction_type::DEC, addressing_mode::MEMREG, register_type::REG_HL}},
    {0x36, new instruction{instruction_type::LD, addressing_mode::MEMREG_D8, register_type::REG_HL}},
    {0x38, new instruction{JR, D8, NONE, NONE, C}},
    {0x39, new instruction{instruction_type::ADD, REG_REG, REG_HL, REG_SP}},
    {0x3A, new instruction{instruction_type::LD, addressing_mode::REG_HLMINUS, register_type::REG_A, register_type::REG_HL}},
    {0x3B, new instruction{DEC, REG, REG_SP}},
    {0x3C, new instruction{INC, REG, REG_A}},
    {0x3D, new instruction{DEC, REG, REG_A}},
    {0x3E, new instruction{instruction_type::LD, addressing_mode::REG_D8, register_type::REG_A}},

    // 0x4 ✓
    // this, 0x5, 0x6, and 0x7 are all just full of loads, nothing else at all.
    {0x40, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_B}},
    {0x41, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_C}},
    {0x42, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_D}},
    {0x43, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_E}},
    {0x44, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_H}},
    {0x45, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_L}},
    {0x46, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_B, register_type::REG_HL}},
    {0x47, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_B, register_type::REG_A}},
    {0x48, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_B}},
    {0x49, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_C}},
    {0x4A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_D}},
    {0x4B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_E}},
    {0x4C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_H}},
    {0x4D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_L}},
    {0x4E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_C, register_type::REG_HL}},
    {0x4F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_C, register_type::REG_A}},

    // 0x5 ✓
    // follows a similar pattern to 0x4, except with D instead of B and E instead of C
    {0x50, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_B}},
    {0x51, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_C}},
    {0x52, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_D}},
    {0x53, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_E}},
    {0x54, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_H}},
    {0x55, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_L}},
    {0x56, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_D, register_type::REG_HL}},
    {0x57, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_D, register_type::REG_A}},
    {0x58, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_B}},
    {0x59, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_C}},
    {0x5A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_D}},
    {0x5B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_E}},
    {0x5C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_H}},
    {0x5D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_L}},
    {0x5E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_E, register_type::REG_HL}},
    {0x5F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_E, register_type::REG_A}},

    // 0x6 ✓
    // follows a similar pattern to 0x4, except with H instead of B and L instead of C
    {0x60, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_B}},
    {0x61, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_C}},
    {0x62, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_D}},
    {0x63, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_E}},
    {0x64, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_H}},
    {0x65, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_L}},
    {0x66, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_H, register_type::REG_HL}},
    {0x67, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_H, register_type::REG_A}},
    {0x68, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_B}},
    {0x69, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_C}},
    {0x6A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_D}},
    {0x6B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_E}},
    {0x6C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_H}},
    {0x6D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_L}},
    {0x6E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_L, register_type::REG_HL}},
    {0x6F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_L, register_type::REG_A}},

    // 0x7 ✓
    // follows a similar pattern to 0x4, except with A instead of B and HL instead of C
    {0x70, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_B}},
    {0x71, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_C}},
    {0x72, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_D}},
    {0x73, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_E}},
    {0x74, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_H}},
    {0x75, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_L}},
    {0x76, new instruction{instruction_type::HALT}},
    {0x77, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_HL, register_type::REG_A}},
    {0x78, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_B}},
    {0x79, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_C}},
    {0x7A, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_D}},
    {0x7B, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_E}},
    {0x7C, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_H}},
    {0x7D, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_L}},
    {0x7E, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_HL}},
    {0x7F, new instruction{instruction_type::LD, addressing_mode::REG_REG, register_type::REG_A, register_type::REG_A}},

    // 0x8 ✓
    {0x80, new instruction{instruction_type::ADD, REG_REG, REG_A, REG_B}},
    {0x81, new instruction{instruction_type::ADD, REG_REG, REG_A, REG_C}},
    {0x82, new instruction{instruction_type::ADD, REG_REG, REG_A, REG_D}},
    {0x83, new instruction{instruction_type::ADD, REG_REG, REG_A, REG_E}},
    {0x84, new instruction{instruction_type::ADD, REG_REG, REG_A, REG_H}},
    {0x85, new instruction{instruction_type::ADD, REG_REG, REG_A, REG_L}},
    {0x86, new instruction{instruction_type::ADD, REG_MEMREG, REG_A, REG_HL}},
    {0x87, new instruction{instruction_type::ADD, REG_REG, REG_A, REG_A}},
    {0x88, new instruction{instruction_type::ADC, REG_REG, REG_A, REG_B}},
    {0x89, new instruction{instruction_type::ADC, REG_REG, REG_A, REG_C}},
    {0x8A, new instruction{instruction_type::ADC, REG_REG, REG_A, REG_D}},
    {0x8B, new instruction{instruction_type::ADC, REG_REG, REG_A, REG_E}},
    {0x8C, new instruction{instruction_type::ADC, REG_REG, REG_A, REG_H}},
    {0x8D, new instruction{instruction_type::ADC, REG_REG, REG_A, REG_L}},
    {0x8E, new instruction{instruction_type::ADC, REG_MEMREG, REG_A, REG_HL}},
    {0x8F, new instruction{instruction_type::ADC, REG_REG, REG_A, REG_A}},

    // 0x9 ✓
    {0x90, new instruction{instruction_type::SUB, REG_REG, REG_A, REG_B}},
    {0x91, new instruction{instruction_type::SUB, REG_REG, REG_A, REG_C}},
    {0x92, new instruction{instruction_type::SUB, REG_REG, REG_A, REG_D}},
    {0x93, new instruction{instruction_type::SUB, REG_REG, REG_A, REG_E}},
    {0x94, new instruction{instruction_type::SUB, REG_REG, REG_A, REG_H}},
    {0x95, new instruction{instruction_type::SUB, REG_REG, REG_A, REG_L}},
    {0x96, new instruction{instruction_type::SUB, REG_MEMREG, REG_A, REG_HL}},
    {0x97, new instruction{instruction_type::SUB, REG_REG, REG_A, REG_A}},
    {0x98, new instruction{instruction_type::SBC, REG_REG, REG_A, REG_B}},
    {0x99, new instruction{instruction_type::SBC, REG_REG, REG_A, REG_C}},
    {0x9A, new instruction{instruction_type::SBC, REG_REG, REG_A, REG_D}},
    {0x9B, new instruction{instruction_type::SBC, REG_REG, REG_A, REG_E}},
    {0x9C, new instruction{instruction_type::SBC, REG_REG, REG_A, REG_H}},
    {0x9D, new instruction{instruction_type::SBC, REG_REG, REG_A, REG_L}},
    {0x9E, new instruction{instruction_type::SBC, REG_MEMREG, REG_A, REG_HL}},
    {0x9F, new instruction{instruction_type::SBC, REG_REG, REG_A, REG_A}},

    // 0xA ✓
    {0xA0, new instruction{AND, REG_REG, REG_A, REG_B}},
    {0xA1, new instruction{AND, REG_REG, REG_A, REG_C}},
    {0xA2, new instruction{AND, REG_REG, REG_A, REG_D}},
    {0xA3, new instruction{AND, REG_REG, REG_A, REG_E}},
    {0xA4, new instruction{AND, REG_REG, REG_A, REG_H}},
    {0xA5, new instruction{AND, REG_REG, REG_A, REG_L}},
    {0xA6, new instruction{AND, REG_MEMREG, REG_A, REG_HL}},
    {0xA7, new instruction{AND, REG_REG, REG_A, REG_A}},
    {0xA8, new instruction{XOR, REG_REG, REG_A, REG_B}},
    {0xA9, new instruction{XOR, REG_REG, REG_A, REG_C}},
    {0xAA, new instruction{XOR, REG_REG, REG_A, REG_D}},
    {0xAB, new instruction{XOR, REG_REG, REG_A, REG_E}},
    {0xAC, new instruction{XOR, REG_REG, REG_A, REG_H}},
    {0xAD, new instruction{XOR, REG_REG, REG_A, REG_L}},
    {0xAE, new instruction{XOR, REG_MEMREG, REG_A, REG_HL}},
    {0xAF, new instruction{XOR, REG_REG, REG_A, REG_A}},

    // 0xB ✓
    {0xB0, new instruction{OR, REG_REG, REG_A, REG_B}},
    {0xB1, new instruction{OR, REG_REG, REG_A, REG_C}},
    {0xB2, new instruction{OR, REG_REG, REG_A, REG_D}},
    {0xB3, new instruction{OR, REG_REG, REG_A, REG_E}},
    {0xB4, new instruction{OR, REG_REG, REG_A, REG_H}},
    {0xB5, new instruction{OR, REG_REG, REG_A, REG_L}},
    {0xB6, new instruction{OR, REG_MEMREG, REG_A, REG_HL}},
    {0xB7, new instruction{OR, REG_REG, REG_A, REG_A}},
    {0xB8, new instruction{CP, REG_REG, REG_A, REG_B}},
    {0xB9, new instruction{CP, REG_REG, REG_A, REG_C}},
    {0xBA, new instruction{CP, REG_REG, REG_A, REG_D}},
    {0xBB, new instruction{CP, REG_REG, REG_A, REG_E}},
    {0xBC, new instruction{CP, REG_REG, REG_A, REG_H}},
    {0xBD, new instruction{CP, REG_REG, REG_A, REG_L}},
    {0xBE, new instruction{CP, REG_MEMREG, REG_A, REG_HL}},
    {0xBF, new instruction{CP, REG_REG, REG_A, REG_A}},

    // 0xC ✓
    // most opcodes that use NZ here use NC in 0xD
    {0xC0, new instruction{RET, IMPLIED, NONE, NONE, NZ}},
    {0xC1, new instruction{instruction_type::POP, addressing_mode::REG, register_type::REG_BC}},
    {0xC2, new instruction{JP, D16, NONE, NONE, NZ}},
    {0xC3, new instruction{instruction_type::JP, addressing_mode::D16}},
    {0xC4, new instruction{instruction_type::CALL, addressing_mode::D16, NONE, NONE, NZ}},
    {0xC5, new instruction{instruction_type::PUSH, addressing_mode::REG, register_type::REG_BC}},
    {0xC6, new instruction{ADD, REG_ADDR8, REG_A}},
    {0xC7, new instruction(RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x00)},
    {0xC8, new instruction{RET, IMPLIED, NONE, NONE, Z}},
    {0xC9, new instruction{RET, IMPLIED}},
    {0xCA, new instruction{JP, D16, NONE, NONE, Z}},
    {0xCB, new instruction{CB, D8}},
    {0xCC, new instruction{CALL, D16, NONE, NONE, Z}},
    {0xCD, new instruction{CALL, D16}},
    {0xCE, new instruction{ADC, REG_D8, REG_A}},
    {0xCF, new instruction(RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x08)},

    // 0xD ✓
    // most opcodes that use NC here use NZ in 0xC
    {0xD0, new instruction{RET, IMPLIED, NONE, NONE, NC}},
    {0xD1, new instruction{instruction_type::POP, addressing_mode::REG, register_type::REG_DE}},
    {0xD2, new instruction{JP, D16, NONE, NONE, NC}},
    // there is no D3
    {0xD4, new instruction{instruction_type::CALL, addressing_mode::D16, NONE, NONE, NC}},
    {0xD5, new instruction{instruction_type::PUSH, addressing_mode::REG, register_type::REG_DE}},
    {0xD6, new instruction{SUB, D8}},
    {0xD7, new instruction{RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x10}},
    {0xD8, new instruction{RET, IMPLIED, NONE, NONE, C}},
    {0xD9, new instruction{RETI, IMPLIED}},
    {0xDA, new instruction{JP, D16, NONE, NONE, C}},
    // there is no DB
    {0xDC, new instruction{CALL, D16, NONE, NONE, C}},
    // there is no DD
    {0xDE, new instruction{SBC, REG_D8, REG_A}},
    {0xDF, new instruction{RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x18}},

    // 0xE ✓
    {0xE0, new instruction{instruction_type::LDH, addressing_mode::ADDR8_REG, register_type::NONE, register_type::REG_A}},
    {0xE1, new instruction{instruction_type::POP, addressing_mode::REG, register_type::REG_HL}},
    {0xE2, new instruction{instruction_type::LD, addressing_mode::MEMREG_REG, register_type::REG_C, register_type::REG_A}},
    // there is no E3
    // there is no E4
    {0xE5, new instruction{instruction_type::PUSH, addressing_mode::REG, register_type::REG_HL}},
    {0xE6, new instruction{AND, REG_D8}},
    {0xE7, new instruction{RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x20}},
    {0xE8, new instruction{ADD, REG_D8, REG_SP}},
    {0xE9, new instruction{JP, MEMREG, NONE, REG_HL}},
    {0xEA, new instruction{instruction_type::LD, addressing_mode::ADDR16_REG, register_type::NONE, register_type::REG_A}},
    // there is no EB
    // there is no EC
    // there is no ED
    {0xEE, new instruction{XOR, REG_D8}},
    {0xEF, new instruction{RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x28}},

    // 0xF ✓
    {0xF0, new instruction{instruction_type::LDH, addressing_mode::REG_ADDR8, register_type::REG_A}},
    {0xF1, new instruction{instruction_type::POP, addressing_mode::REG, register_type::REG_AF}},
    {0xF2, new instruction{instruction_type::LD, addressing_mode::REG_MEMREG, register_type::REG_A, register_type::REG_C}},
    {0xF3, new instruction{instruction_type::DI, addressing_mode::IMPLIED}},
    // there is no F4
    {0xF5, new instruction{instruction_type::PUSH, addressing_mode::REG, register_type::REG_AF}},
    {0xF6, new instruction{OR, REG_D8}},
    {0xF7, new instruction{RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x30}},
    {0xF8, new instruction{LD, HL_SPTR, REG_HL, REG_SP}},
    {0xF9, new instruction{LD, REG_REG, REG_SP, REG_HL}},
    {0xFA, new instruction{instruction_type::LD, addressing_mode::REG_ADDR16, register_type::REG_A}},
    {0xFB, new instruction{EI}},
    // there is no FC
    // there is no FD
    {0xFE, new instruction{CP, REG_D8}},
    {0xFF, new instruction{RST, IMPLIED, NONE, NONE, ALWAYS, (uint8_t)0x38}},
};

std::map<int, register_type> instruction::cb_registers = {
    {0, REG_B},
    {1, REG_C},
    {2, REG_D},
    {3, REG_E},
    {4, REG_H},
    {5, REG_L},
    {6, REG_HL},
    {7, REG_A},
};

void instruction::cb() {
    // register order is: B C D E H L HL A
    // instruction order:
    // RLC: 0-7
    // RRC: 8-15
    // RL: 16-23
    // RR: 24-31
    // SLA: 32-39
    // SRA: 40-47
    // SWAP: 48-55
    // SRL: 56-63
    // BIT_0: 64-71
    // BIT_1: 72-79
    // BIT_2: 80-87
    // BIT_3: 88-95
    // BIT_4: 96-103
    // BIT_5: 104-111
    // BIT_6: 112-119
    // BIT_7: 120-127
    // RES_0: 128-135
    // RES_1: 136-143
    // RES_2: 144-151
    // RES_3: 152-159
    // RES_4: 160-167
    // RES_5: 168-175
    // RES_6: 176-183
    // RES_7: 184-191
    // SET_0: 192-199
    // SET_1: 200-207
    // SET_2: 208-215
    // SET_3: 216-223
    // SET_4: 224-231
    // SET_5: 232-239
    // SET_6: 240-247
    // SET_7: 248-255
    int byte = (cpu->fetch_result & 0xFF);  // TODO: test more
    int div_result = byte / 8;
    int mod = byte % 8;

    cpu->emu->add_cycles(1);

    if (mod == 6 /*HL*/) {
        cpu->emu->add_cycles(2);
    }
    switch (div_result) {
        case 0:
            rlc(mod);
            return;
        case 1:
            rrc(mod);
            return;
        case 2:
            rl(mod);
            return;
        case 3:
            rr(mod);
            return;
        case 4:
            sla(mod);
            return;
        case 5:
            sra(mod);
            return;
        case 6:
            swap(mod);
            return;
        case 7:
            srl(mod);
            return;
        case 8:
            bit(0, mod);
            return;
        case 9:
            bit(1, mod);
            return;
        case 10:
            bit(2, mod);
            return;
        case 11:
            bit(3, mod);
            return;
        case 12:
            bit(4, mod);
            return;
        case 13:
            bit(5, mod);
            return;
        case 14:
            bit(6, mod);
            return;
        case 15:
            bit(7, mod);
            return;
        case 16:
            res(0, mod);
            return;
        case 17:
            res(1, mod);
            return;
        case 18:
            res(2, mod);
            return;
        case 19:
            res(3, mod);
            return;
        case 20:
            res(4, mod);
            return;
        case 21:
            res(5, mod);
            return;
        case 22:
            res(6, mod);
            return;
        case 23:
            res(7, mod);
            return;
        case 24:
            set(0, mod);
            return;
        case 25:
            set(1, mod);
            return;
        case 26:
            set(2, mod);
            return;
        case 27:
            set(3, mod);
            return;
        case 28:
            set(4, mod);
            return;
        case 29:
            set(5, mod);
            return;
        case 30:
            set(6, mod);
            return;
        case 31:
            set(7, mod);
            return;
    }
    std::cout << "Invalid value for CB: " << (int)mod << std::endl;
    exit(1);
}

// if im miscalculating 1 << num, its supposed to be (whatever the better form is) everywhere
void instruction::bit(int num, int mod) {
    uint8_t result = cpu->get_reg(cb_registers.at(mod));
    int z_flag = !(result & (1 << num));  // TODO: test
    int n_flag = 0;
    int h_flag = 1;
    int c_flag = -1;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::res(int num, int mod) {
    uint8_t result = cpu->get_reg(cb_registers.at(mod));
    result &= ~(1 << num);
    cpu->set_reg(cb_registers.at(mod), false, result);
}

void instruction::set(int num, int mod) {
    uint8_t result = cpu->get_reg(cb_registers.at(mod));
    result |= (1 << num);
    cpu->set_reg(cb_registers.at(mod), false, result);
}

void instruction::rlc(int mod) {
    uint8_t result = cpu->get_reg(cb_registers.at(mod));
    uint8_t shift_res = (result << 1) & 0xFF;
    bool c_flag = false;
    if ((result & (1 << 7)) != 0) {
        c_flag = true;
        shift_res |= 1;
    }

    cpu->set_reg(cb_registers.at(mod), false, shift_res);

    bool z_flag = shift_res == 0;
    bool n_flag = 0;
    bool h_flag = 0;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::rrc(int mod) {
    uint8_t original = cpu->get_reg(cb_registers.at(mod));
    uint8_t result = original;
    result >>= 1;
    result |= (original << 7);

    cpu->set_reg(cb_registers.at(mod), false, result);

    bool z_flag = !result;
    bool n_flag = 0;
    bool h_flag = 0;
    bool c_flag = original & 0x1;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::rl(int mod) {
    uint8_t original = cpu->get_reg(cb_registers.at(mod));
    uint8_t result = original;
    result <<= 1;
    result |= cpu->get_flag_c();

    cpu->set_reg(cb_registers.at(mod), false, result);

    bool z_flag = !result;
    bool n_flag = 0;
    bool h_flag = 0;
    bool c_flag = (original & 0x80);  // may need explicit conversion to c++ bool

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::rr(int mod) {
    uint8_t original = cpu->get_reg(cb_registers.at(mod));
    uint8_t result = original;
    result >>= 1;
    result |= (cpu->get_flag_c() << 7);

    bool z_flag = !result;
    bool n_flag = 0;
    bool h_flag = 0;
    bool c_flag = original & 0x1;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::sla(int mod) {
    uint8_t original = cpu->get_reg(cb_registers.at(mod));
    uint8_t result = (uint8_t)original << 1;
    cpu->set_reg(cb_registers.at(mod), false, result);

    bool z_flag = !result;
    bool n_flag = 0;
    bool h_flag = 0;
    bool c_flag = original & 0x1;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::sra(int mod) {
    uint8_t original = cpu->get_reg(cb_registers.at(mod));
    uint8_t result = (uint8_t)original >> 1;
    cpu->set_reg(cb_registers.at(mod), false, result);

    bool z_flag = !result;
    bool n_flag = 0;
    bool h_flag = 0;
    bool c_flag = original & 0x80;  // may need explicit conversion to c++ bool

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::swap(int mod) {
    uint8_t value = cpu->get_reg(cb_registers.at(mod));
    value = ((value & 0xF0) >> 4) | ((value & 0x0F) << 4);
    cpu->set_reg(cb_registers.at(mod), false, value);

    bool z_flag = (value == 0);
    bool n_flag = 0;
    bool h_flag = 0;
    bool c_flag = 0;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

void instruction::srl(int mod) {
    uint8_t original = cpu->get_reg(cb_registers.at(mod));
    uint8_t result = original >> 1;
    cpu->set_reg(cb_registers.at(mod), false, result);

    bool z_flag = !result;
    bool n_flag = 0;
    bool h_flag = 0;
    bool c_flag = original & 0x1;

    cpu->set_flag_z(z_flag);
    cpu->set_flag_n(n_flag);
    cpu->set_flag_h(h_flag);
    cpu->set_flag_c(c_flag);
}

bool instruction::check_conditions() {
    bool z = cpu->get_flag_z();
    bool n = cpu->get_flag_n();
    bool h = cpu->get_flag_h();
    bool c = cpu->get_flag_c();

    switch (this->cond) {
        case condition_type::ALWAYS:
            return true;
        case condition_type::NZ:
            return !z;
        case condition_type::Z:
            return z;
        case condition_type::NC:
            return !c;
        case condition_type::C:
            return c;
        case condition_type::NH:
            return !h;
        case condition_type::H:
            return h;
        case condition_type::NN:
            return !n;
        case condition_type::N:
            return n;
        default:
            std::cout << "Invalid condition type.  Program terminated." << std::endl;
            exit(1);
    }
}

void instruction::print() {
    simple_print();
}

void instruction::simple_print() {
    std::cout << this->cpu->reg_pc << ": ";
    std::cout << print_instruction_type(this->type) << " ";
    std::cout << int_to_hex(cpu->opcode) << " " << int_to_hex(cpu->emu->bus->read8(cpu->reg_pc + 1)) << " " << int_to_hex(cpu->emu->bus->read8(cpu->reg_pc + 2)) << std::endl;
    // print_instructions(type, mode, reg_1, reg_2, cond, parameter);
}