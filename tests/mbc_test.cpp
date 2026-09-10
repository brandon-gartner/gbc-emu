#include <cassert>
#include <cstring>
#include <iostream>

#include "shared.h"
#define private public
#include "cartridge.h"
#undef private

// helper mock class inheriting cartridge for direct testing
class test_cartridge : public cartridge {
   public:
    test_cartridge(uint8_t type_code, uint32_t rom_banks_count, uint32_t ram_bytes) {
        this->cartridge_type_code = type_code;
        this->total_rom_banks = rom_banks_count;
        this->file_size = rom_banks_count * 0x4000;
        this->file_data.assign(this->file_size, 0);

        // fill each rom bank with its bank number so we can easily verify reads
        for (uint32_t b = 0; b < rom_banks_count; b++) {
            for (uint32_t i = 0; i < 0x4000; i++) {
                this->file_data[(b * 0x4000) + i] = (uint8_t)(b & 0xFF);
            }
        }

        this->ram_size_bytes = ram_bytes;
        if (ram_bytes > 0) {
            this->ram_data.assign(ram_bytes, 0);
        }

        if (type_code == 0x0F || type_code == 0x10) {
            this->has_rtc = true;
        }
    }
};

void test_mbc3() {
    // MBC3 with timer, ram, battery: 0x10, 64 banks (1 MB), 32 KB RAM
    test_cartridge cart(0x10, 64, 32 * 1024);

    // 1. rom bank 0 should always read bank 0 at 0x0000-0x3FFF
    assert(cart.read_cart(0x0000) == 0);
    assert(cart.read_cart(0x3FFF) == 0);

    // 2. default switchable bank at 0x4000-0x7FFF should be bank 1
    assert(cart.read_cart(0x4000) == 1);

    // 3. write to 0x2000 to select bank 5
    cart.write_cart(5, 0x2000);
    assert(cart.read_cart(0x4000) == 5);

    // 4. write 0 to 0x2000: should map to bank 1 on MBC3
    cart.write_cart(0, 0x2000);
    assert(cart.read_cart(0x4000) == 1);

    // 5. select bank 31
    cart.write_cart(31, 0x2000);
    assert(cart.read_cart(0x4000) == 31);

    // 6. ram disabled initially
    assert(cart.read_cart(0xA000) == 0xFF);
    cart.write_cart(0x42, 0xA000);
    assert(cart.read_cart(0xA000) == 0xFF);

    // 7. enable ram and rtc (write 0x0A to 0x0000)
    cart.write_cart(0x0A, 0x0000);

    // 8. test RAM bank 0 and 1
    cart.write_cart(0x00, 0x4000); // select ram bank 0
    cart.write_cart(0xAA, 0xA000);
    assert(cart.read_cart(0xA000) == 0xAA);

    cart.write_cart(0x01, 0x4000); // select ram bank 1
    assert(cart.read_cart(0xA000) == 0x00);
    cart.write_cart(0xBB, 0xA000);
    assert(cart.read_cart(0xA000) == 0xBB);

    cart.write_cart(0x00, 0x4000); // back to ram bank 0
    assert(cart.read_cart(0xA000) == 0xAA);

    // 9. test RTC registers (0x08 - 0x0C) and latching
    cart.write_cart(0x08, 0x4000); // select RTC seconds
    cart.write_cart(45, 0xA000);   // write 45 seconds to live register

    cart.write_cart(0x09, 0x4000); // select RTC minutes
    cart.write_cart(30, 0xA000);   // write 30 minutes to live register

    // latch data: write 0x00 then 0x01 to 0x6000
    cart.write_cart(0x00, 0x6000);
    cart.write_cart(0x01, 0x6000);

    // read latched seconds
    cart.write_cart(0x08, 0x4000);
    assert(cart.read_cart(0xA000) == 45);

    // read latched minutes
    cart.write_cart(0x09, 0x4000);
    assert(cart.read_cart(0xA000) == 30);

    std::cout << "MBC3 tests passed!" << std::endl;
}

void test_mbc5() {
    // MBC5+RAM+BATTERY: 0x1B, 512 banks (8 MB), 128 KB RAM (16 banks)
    test_cartridge cart(0x1B, 512, 128 * 1024);

    // 1. default bank
    assert(cart.read_cart(0x0000) == 0);
    assert(cart.read_cart(0x4000) == 1);

    // 2. bank 0 CAN be mapped to 0x4000-0x7FFF on MBC5!
    cart.write_cart(0x00, 0x2000);
    assert(cart.read_cart(0x4000) == 0);

    // 3. select bank 255 (lower 8 bits)
    cart.write_cart(0xFF, 0x2000);
    assert(cart.read_cart(0x4000) == 0xFF);

    // 4. select 9th bit (bit 8 = 1 => bank 256 + 0 = 256)
    cart.write_cart(0x00, 0x2000);
    cart.write_cart(0x01, 0x3000);
    assert(cart.read_cart(0x4000) == (256 & 0xFF)); // 256 modulo 256 is 0

    // select bank 257 (256 + 1)
    cart.write_cart(0x01, 0x2000);
    cart.write_cart(0x01, 0x3000);
    assert(cart.read_cart(0x4000) == 1);

    // 5. enable RAM
    cart.write_cart(0x0A, 0x0000);

    // 6. test RAM banking up to 16 banks (0x00 - 0x0F)
    for (uint8_t b = 0; b < 16; b++) {
        cart.write_cart(b, 0x4000);
        cart.write_cart(0x50 + b, 0xA000);
    }

    for (uint8_t b = 0; b < 16; b++) {
        cart.write_cart(b, 0x4000);
        assert(cart.read_cart(0xA000) == (0x50 + b));
    }

    std::cout << "MBC5 tests passed!" << std::endl;
}

int main() {
    test_mbc3();
    test_mbc5();
    std::cout << "All MBC3 and MBC5 unit tests passed successfully!" << std::endl;
    return 0;
}
