#include "cartridge.h"

#include <cstring>
#include "static_mappings.h"

bool cartridge::load_cartridge(char** argv) {
    this->file_name = argv[1];

    std::ifstream file(this->file_name, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cout << "Cannot find requested file." << std::endl;
        return false;
    }

    this->file_size = file.tellg();  // gets current position in input sequence; in this case, the end of the file.

    this->file_data = new uint8_t[this->file_size]();

    file.seekg(0, std::ios::beg);  // sets the position of the 'next character' as the first character in the file.

    file.read((char*)this->file_data, this->file_size);
    // skip to byte 0x100, beginning of the header, and store it.
    // this is the entry point, tells us how to jump to the main program
    file.seekg(0x100);
    file.read((char*)this->entry_point, 4);

    file.seekg(0x104);
    file.read((char*)this->nintendo_logo, 48);

    file.seekg(0x134);
    char game_title_chars[16] = {0};
    file.read(game_title_chars, 16);
    this->game_title = std::string(game_title_chars, 16);

    file.seekg(0x144);
    file.read((char*)&this->new_publisher_code, 2);

    this->new_publisher = static_mappings::new_publisher_codes[this->new_publisher_code];

    file.seekg(0x146);
    file.read((char*)&this->sgb_enabled, 1);

    file.seekg(0x147);
    file.read((char*)&this->cartridge_type_code, 1);

    this->cartridge_type = static_mappings::cartridge_type[this->cartridge_type_code];

    file.seekg(0x148);
    file.read((char*)&this->rom_size_code, 1);

    int bank_number = static_mappings::rom_banks[this->rom_size_code];
    this->total_rom_banks = (bank_number > 0) ? bank_number : 2;
    // at 64 banks it becomes 1 MB in size, counted in KB until then
    if (bank_number > 32) {
        this->rom_size = std::to_string(bank_number / 64) + " MB, " + std::to_string(bank_number) + " banks";
    } else {
        this->rom_size = std::to_string(bank_number * 16) + " KB, " + std::to_string(bank_number) + " banks";
    }

    file.seekg(0x149);
    file.read((char*)&this->ram_size, 1);

    // allocate cartridge ram if present
    switch (this->ram_size) {
        case 0x02:
            this->ram_size_bytes = 8 * 1024;
            break;
        case 0x03:
            this->ram_size_bytes = 32 * 1024;
            break;
        case 0x04:
            this->ram_size_bytes = 128 * 1024;
            break;
        // 0x05 was defined first which is why it's smaller than 0x04, 0x04 came later
        case 0x05:
            this->ram_size_bytes = 64 * 1024;
            break;
        default:
            // if cartridge type has ram but header specifies 0, allocate default 8 kb
            if (this->cartridge_type_code == 0x02 || this->cartridge_type_code == 0x03 ||
                this->cartridge_type_code == 0x08 || this->cartridge_type_code == 0x09 ||
                this->cartridge_type_code == 0x0C || this->cartridge_type_code == 0x0D ||
                this->cartridge_type_code == 0x10 || this->cartridge_type_code == 0x12 ||
                this->cartridge_type_code == 0x13 || this->cartridge_type_code == 0x1A ||
                this->cartridge_type_code == 0x1B || this->cartridge_type_code == 0x1D ||
                this->cartridge_type_code == 0x1E) {
                this->ram_size_bytes = 8 * 1024;
            } else {
                this->ram_size_bytes = 0;
            }
            break;
    }
    if (this->ram_size_bytes > 0) {
        this->ram_data = new uint8_t[this->ram_size_bytes]();
    }

    // mbc3 cartridges with timer (0x0F and 0x10)
    this->has_rtc = (this->cartridge_type_code == 0x0F || this->cartridge_type_code == 0x10);

    file.seekg(0x14A);
    file.read((char*)&this->destination_code, 1);

    this->destination = static_mappings::destination_codes[this->destination_code];

    file.seekg(0x14B);
    file.read((char*)&this->old_publisher_code, 1);

    this->old_publisher = static_mappings::old_publisher_code[this->old_publisher_code];

    file.seekg(0x14C);
    file.read((char*)&this->rom_version_number, 1);

    file.seekg(0x14D);
    file.read((char*)&this->header_checksum, 1);

    file.close();

    this->valid = true;
    return true;
}

bool cartridge::verify_header_checksum(bool debug) {
    // verify header checksum, provided by Pan Docs
    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - this->file_data[address] - 1;
    }

    if (debug) {
        std::cout << "Expected: " << std::hex << (int)this->header_checksum << std::endl;
        std::cout << "Actual: " << std::hex << (int)checksum << std::endl;
    }

    if (checksum != this->header_checksum) {
        std::cout << "Header checksum failed." << std::endl;

        return false;
    }

    return true;
}

cartridge::cartridge() {
    this->file_data = nullptr;
    this->ram_data = nullptr;
    this->rom_bank = 1;
    this->ram_bank = 0;
    this->ram_enabled = false;
    this->banking_mode = 0;
    this->ram_size_bytes = 0;
    this->total_rom_banks = 2;
    std::memset(this->rtc_regs, 0, sizeof(this->rtc_regs));
    std::memset(this->rtc_latched, 0, sizeof(this->rtc_latched));
    this->rtc_latch_prep = 0xFF;
    this->has_rtc = false;
}

cartridge::~cartridge() {
    if (this->file_data) {
        delete[] this->file_data;
        this->file_data = nullptr;
    }
    if (this->ram_data) {
        delete[] this->ram_data;
        this->ram_data = nullptr;
    }
}

void cartridge::print_header_hex() {
    int count = 0;
    std::cout << std::endl;
    for (uint16_t address = 0x100; address <= 0x14F; address++) {
        if (count == 0) {
            std::cout << std::hex << address << ": ";
        }
        std::cout << ((int)this->file_data[address] > 15 ? "" : "0") << std::hex << (int)this->file_data[address] << " ";
        count++;
        if (count == 16) {
            std::cout << std::endl;
            count = 0;
        }
    }
    std::cout << std::endl;
}

void cartridge::print_beginning_hex() {
    int count = 0;
    std::cout << std::endl;
    for (uint16_t address = 0x0; address <= 0x14F; address++) {
        if (count == 0) {
            std::cout << std::hex << address << ": ";
        }
        std::cout << ((int)this->file_data[address] > 15 ? "" : "0") << std::hex << (int)this->file_data[address] << " ";
        count++;
        if (count == 16) {
            std::cout << std::endl;
            count = 0;
        }
    }
    std::cout << std::endl;
}

uint8_t cartridge::read_cart(uint16_t address) {
    // rom only cartridge
    if (this->cartridge_type_code == 0x00) {
        if (address < this->file_size) {
            return this->file_data[address];
        }
        return 0xFF;
    }

    // mbc1 cartridge (0x01: mbc1, 0x02: mbc1+ram, 0x03: mbc1+ram+battery)
    if (this->cartridge_type_code >= 0x01 && this->cartridge_type_code <= 0x03) {
        // rom bank 00 (0x0000 - 0x3FFF)
        if (address < 0x4000) {
            if (this->banking_mode == 1) {
                int bank = (this->ram_bank << 5) % this->total_rom_banks;
                uint32_t offset = (bank * 0x4000) + address;
                if (offset < this->file_size) {
                    return this->file_data[offset];
                }
            }
            if (address < this->file_size) {
                return this->file_data[address];
            }
            return 0xFF;
        }
        // switchable rom bank (0x4000 - 0x7FFF)
        if (address < 0x8000) {
            int bank = ((this->ram_bank << 5) | this->rom_bank) % this->total_rom_banks;
            uint32_t offset = (bank * 0x4000) + (address - 0x4000);
            if (offset < this->file_size) {
                return this->file_data[offset];
            }
            return 0xFF;
        }
        // external ram (0xA000 - 0xBFFF)
        if (address >= 0xA000 && address < 0xC000) {
            if (!this->ram_enabled || !this->ram_data || this->ram_size_bytes == 0) {
                return 0xFF;
            }
            uint32_t ram_offset = 0;
            if (this->banking_mode == 0) {
                ram_offset = (address - 0xA000) % this->ram_size_bytes;
            } else {
                ram_offset = ((this->ram_bank * 0x2000) + (address - 0xA000)) % this->ram_size_bytes;
            }
            return this->ram_data[ram_offset];
        }
    }

    // mbc3 cartridge (0x0F - 0x13)
    if (this->cartridge_type_code >= 0x0F && this->cartridge_type_code <= 0x13) {
        // rom bank 00 (0x0000 - 0x3FFF)
        if (address < 0x4000) {
            if (address < this->file_size) {
                return this->file_data[address];
            }
            return 0xFF;
        }
        // switchable rom bank 01-7F (0x4000 - 0x7FFF)
        if (address < 0x8000) {
            uint16_t bank = (this->rom_bank == 0) ? 1 : this->rom_bank;
            bank %= this->total_rom_banks;
            uint32_t offset = (bank * 0x4000) + (address - 0x4000);
            if (offset < this->file_size) {
                return this->file_data[offset];
            }
            return 0xFF;
        }
        // external ram / rtc registers (0xA000 - 0xBFFF)
        if (address >= 0xA000 && address < 0xC000) {
            if (!this->ram_enabled) {
                return 0xFF;
            }
            // ram bank 0x00 - 0x07
            if (this->ram_bank <= 0x07) {
                if (!this->ram_data || this->ram_size_bytes == 0) {
                    return 0xFF;
                }
                uint32_t ram_offset = ((this->ram_bank * 0x2000) + (address - 0xA000)) % this->ram_size_bytes;
                return this->ram_data[ram_offset];
            }
            // rtc registers 0x08 - 0x0C
            if (this->ram_bank >= 0x08 && this->ram_bank <= 0x0C) {
                if (this->has_rtc) {
                    return this->rtc_latched[this->ram_bank - 0x08];
                }
            }
            return 0xFF;
        }
    }

    // mbc5 cartridge (0x19 - 0x1E)
    if (this->cartridge_type_code >= 0x19 && this->cartridge_type_code <= 0x1E) {
        // rom bank 00 (0x0000 - 0x3FFF)
        if (address < 0x4000) {
            if (address < this->file_size) {
                return this->file_data[address];
            }
            return 0xFF;
        }
        // switchable rom bank 000-1FF (0x4000 - 0x7FFF, bank 0 is valid)
        if (address < 0x8000) {
            uint16_t bank = this->rom_bank % this->total_rom_banks;
            uint32_t offset = (bank * 0x4000) + (address - 0x4000);
            if (offset < this->file_size) {
                return this->file_data[offset];
            }
            return 0xFF;
        }
        // external ram (0xA000 - 0xBFFF, up to 16 banks)
        if (address >= 0xA000 && address < 0xC000) {
            if (!this->ram_enabled || !this->ram_data || this->ram_size_bytes == 0) {
                return 0xFF;
            }
            uint32_t ram_offset = ((this->ram_bank * 0x2000) + (address - 0xA000)) % this->ram_size_bytes;
            return this->ram_data[ram_offset];
        }
    }

    // fallback for other cartridges
    if (address < this->file_size) {
        return this->file_data[address];
    }
    return 0xFF;
}

bool cartridge::write_cart(uint8_t value, uint16_t address) {
    // rom only cartridges ignore writes to rom space
    if (this->cartridge_type_code == 0x00) {
        return true;
    }

    // mbc1 cartridge (0x01: mbc1, 0x02: mbc1+ram, 0x03: mbc1+ram+battery)
    if (this->cartridge_type_code >= 0x01 && this->cartridge_type_code <= 0x03) {
        // ram enable (0x0000 - 0x1FFF)
        if (address < 0x2000) {
            this->ram_enabled = ((value & 0x0F) == 0x0A);
            return true;
        }
        // rom bank number lower 5 bits (0x2000 - 0x3FFF)
        if (address < 0x4000) {
            uint8_t bank = value & 0x1F;
            if (bank == 0) {
                bank = 1;
            }
            this->rom_bank = bank;
            return true;
        }
        // ram bank number or upper 2 bits of rom bank (0x4000 - 0x5FFF)
        if (address < 0x6000) {
            this->ram_bank = value & 0x03;
            return true;
        }
        // banking mode select (0x6000 - 0x7FFF)
        if (address < 0x8000) {
            this->banking_mode = value & 0x01;
            return true;
        }
        // external ram write (0xA000 - 0xBFFF)
        if (address >= 0xA000 && address < 0xC000) {
            if (this->ram_enabled && this->ram_data && this->ram_size_bytes > 0) {
                uint32_t ram_offset = 0;
                if (this->banking_mode == 0) {
                    ram_offset = (address - 0xA000) % this->ram_size_bytes;
                } else {
                    ram_offset = ((this->ram_bank * 0x2000) + (address - 0xA000)) % this->ram_size_bytes;
                }
                this->ram_data[ram_offset] = value;
            }
            return true;
        }
    }

    // mbc3 cartridge (0x0F - 0x13)
    if (this->cartridge_type_code >= 0x0F && this->cartridge_type_code <= 0x13) {
        // ram and rtc enable (0x0000 - 0x1FFF)
        if (address < 0x2000) {
            this->ram_enabled = ((value & 0x0F) == 0x0A);
            return true;
        }
        // rom bank number 7 bits (0x2000 - 0x3FFF)
        if (address < 0x4000) {
            uint8_t bank = value & 0x7F;
            if (bank == 0) {
                bank = 1;
            }
            this->rom_bank = bank;
            return true;
        }
        // ram bank number or rtc register select (0x4000 - 0x5FFF)
        if (address < 0x6000) {
            this->ram_bank = value;
            return true;
        }
        // latch clock data (0x6000 - 0x7FFF)
        if (address < 0x8000) {
            if (this->has_rtc) {
                if (this->rtc_latch_prep == 0x00 && value == 0x01) {
                    std::memcpy(this->rtc_latched, this->rtc_regs, sizeof(this->rtc_regs));
                }
                this->rtc_latch_prep = value;
            }
            return true;
        }
        // external ram or rtc write (0xA000 - 0xBFFF)
        if (address >= 0xA000 && address < 0xC000) {
            if (this->ram_enabled) {
                if (this->ram_bank <= 0x07) {
                    if (this->ram_data && this->ram_size_bytes > 0) {
                        uint32_t ram_offset = ((this->ram_bank * 0x2000) + (address - 0xA000)) % this->ram_size_bytes;
                        this->ram_data[ram_offset] = value;
                    }
                } else if (this->ram_bank >= 0x08 && this->ram_bank <= 0x0C) {
                    if (this->has_rtc) {
                        this->rtc_regs[this->ram_bank - 0x08] = value;
                    }
                }
            }
            return true;
        }
    }

    // mbc5 cartridge (0x19 - 0x1E)
    if (this->cartridge_type_code >= 0x19 && this->cartridge_type_code <= 0x1E) {
        // ram enable (0x0000 - 0x1FFF)
        if (address < 0x2000) {
            this->ram_enabled = ((value & 0x0F) == 0x0A);
            return true;
        }
        // rom bank number lower 8 bits (0x2000 - 0x2FFF)
        if (address < 0x3000) {
            this->rom_bank = (this->rom_bank & 0x100) | value;
            return true;
        }
        // rom bank number 9th bit (0x3000 - 0x3FFF)
        if (address < 0x4000) {
            this->rom_bank = (this->rom_bank & 0x0FF) | (((uint16_t)(value & 0x01)) << 8);
            return true;
        }
        // ram bank number (0x4000 - 0x5FFF)
        if (address < 0x6000) {
            this->ram_bank = value & 0x0F;
            return true;
        }
        // unused register area for mbc5 (0x6000 - 0x7FFF)
        if (address < 0x8000) {
            return true;
        }
        // external ram write (0xA000 - 0xBFFF)
        if (address >= 0xA000 && address < 0xC000) {
            if (this->ram_enabled && this->ram_data && this->ram_size_bytes > 0) {
                uint32_t ram_offset = ((this->ram_bank * 0x2000) + (address - 0xA000)) % this->ram_size_bytes;
                this->ram_data[ram_offset] = value;
            }
            return true;
        }
    }

    return false;
}

void cartridge::print_cartridge_info() {
    std::cout << std::endl;
    std::cout << "======================================================================" << std::endl;
    std::cout << "File name: " << this->file_name << std::endl;
    std::cout << "File size: " << this->file_size << std::endl;
    std::cout << "Game title: " << this->game_title << std::endl;
    std::cout << "Old publisher code: " << this->old_publisher << std::endl;
    std::cout << "New publisher: " << this->new_publisher << std::endl;
    std::cout << "Cartridge type: " << this->cartridge_type << std::endl;
    std::cout << "ROM size: " << this->rom_size << std::endl;
    std::cout << "Destination: " << this->destination << std::endl;
    std::cout << "Valid: " << (this->valid ? "true" : "false") << std::endl;
    print_header_hex();
    std::cout << "======================================================================" << std::endl;
    std::cout << std::endl;
}