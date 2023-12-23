#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <cstdint>
#include <string>

class cartridge {
    public:
        std::string file_name;
        uint8_t *file_data; // we don't know how big it will be, yet.
        uint8_t file_size;

    private:
        // parts of cartridge header
        uint8_t entry_point;
        uint8_t nintendo_logo;
        uint8_t game_title;
        uint8_t manufacturer_code;
        uint8_t gbc_enabled;
        uint8_t new_publisher_code;
        uint8_t sgb_enabled;
        uint8_t cartridge_type;
        uint8_t rom_size;
        uint8_t ram_size;
        uint8_t old_publisher_code;
        uint8_t rom_version_number;
        uint8_t header_checksum;
        uint8_t global_checksum;
    public:
        bool load_cartridge(char **argv);
};

#endif