#include "cartridge.h"

#include "static_mappings.h"

bool cartridge::load_cartridge(char** argv) {
    this->file_name = argv[1];

    std::ifstream file(this->file_name, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cout << "Cannot find requested file." << std::endl;
        return false;
    }

    this->file_size = file.tellg();  // gets current position in input sequence; in this case, the end of the file.

    std::cout << "File size: " << this->file_size << std::endl;

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
    // at 64 banks it becomes 1 MB in size, counted in KB until then
    if (bank_number > 32) {
        this->rom_size = std::to_string(bank_number / 64) + " MB, " + std::to_string(bank_number) + " banks";
    } else {
        this->rom_size = std::to_string(bank_number * 16) + " KB, " + std::to_string(bank_number) + " banks";
    }

    file.seekg(0x149);
    file.read((char*)&this->ram_size, 1);

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

    // if (!verify_header_checksum(true)) {
    //     return false;
    // }

    // not verified so I'll leave it until I'm looking for something to do
    // file.seekg(0x14E);
    // file.read((char*)&this->global_checksum, 1);

    file.close();

    this->valid = true;
    return true;
}

bool cartridge::verify_header_checksum(bool debug) {
    // verify header checksum, provided by Pan Docs
    uint8_t checksum = 0;
    // std::cout << "Initial checksum: " << checksum << std::endl;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        // std::cout << "Pre-math checksum: " << std::hex << (int)checksum << std::endl;
        // std::cout << "Data at current location 1: " << std::hex << (int)this->file_data[address] << std::endl;
        checksum = checksum - this->file_data[address] - 1;
        // std::cout << "Post-math checksum: " << std::hex << (int)checksum << std::endl;
        // std::cout << "===============================================" << std::endl;
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

cartridge::cartridge(char** argv) {
    this->load_cartridge(argv);
    std::cout << this->file_size << std::endl;
}

cartridge::cartridge() {
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
    return this->file_data[address];
}

bool cartridge::write_cart(uint8_t value, uint16_t address) {
    NOT_IMPLEMENTED();
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
    if (!this->valid) {
        // verify_header_checksum(true);
    }
    print_header_hex();
    std::cout << "======================================================================" << std::endl;
    std::cout << std::endl;
}