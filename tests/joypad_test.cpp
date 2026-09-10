#include "joypad.h"

#include <cassert>
#include <iostream>

#include "emulator.h"
#include "interrupt.h"
#include "processor.h"

int main() {
    emulator emu;
    joypad* pad = emu.pad.get();

    // clear IF register
    emu.cpu->interrupt_flags = 0;

    // 1. initial state: no buttons pressed, select lines default to 11 (neither selected)
    // bits 6-7 should be 1, bits 4-5 should be 1, bits 0-3 should be 1
    assert((pad->read() & 0xFF) == 0xFF);

    // 2. select direction buttons (write bit 4 = 0, bit 5 = 1 => 0x20)
    pad->write(0x20);
    // no directions pressed yet, so lower nibble is 0x0F, bits 6-7 are 1, bits 4-5 are 0x20
    // 0xC0 | 0x20 | 0x0F = 0xEF
    assert(pad->read() == 0xEF);

    // 3. press RIGHT (bit 0 of directions)
    pad->key_down(joypad::RIGHT);
    // bit 0 should now be 0 (pressed): 0xEF & ~0x01 = 0xEE
    assert(pad->read() == 0xEE);

    // 4. select action buttons (write bit 4 = 1, bit 5 = 0 => 0x10)
    pad->write(0x10);
    // RIGHT is still pressed, but action buttons are selected, so lower nibble reflects actions (all 1s)
    // 0xC0 | 0x10 | 0x0F = 0xDF
    assert(pad->read() == 0xDF);

    // 5. press A button (bit 0 of actions)
    emu.cpu->interrupt_flags = 0;
    pad->key_down(joypad::A);
    // bit 0 should now be 0: 0xDF & ~0x01 = 0xDE
    assert(pad->read() == 0xDE);
    // interrupt should have fired (bit 4 of IF / 0x10)!
    assert((emu.cpu->interrupt_flags & 0x10) == 0x10);

    // 6. select BOTH directions and actions (write 0x00)
    pad->write(0x00);
    // both RIGHT and A are pressed (both are bit 0)
    assert((pad->read() & 0x0F) == 0x0E);

    // press UP (bit 2 of directions)
    pad->key_down(joypad::UP);
    // bit 0 and bit 2 should be 0: 0x0E & ~0x04 = 0x0A
    assert((pad->read() & 0x0F) == 0x0A);

    // 7. release buttons
    pad->key_up(joypad::RIGHT);
    pad->key_up(joypad::A);
    pad->key_up(joypad::UP);
    assert((pad->read() & 0x0F) == 0x0F);

    std::cout << "All joypad unit tests passed successfully!" << std::endl;
    return 0;
}