#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include "shared.h"
class cartridge {
   public:
    std::string file_name;
    uint8_t *file_data = {0};  // we don't know how big it will be, yet.
    uint32_t file_size;
    std::string game_title;
    std::string new_publisher;
    std::string cartridge_type;
    std::string rom_size;
    std::string old_publisher;
    std::string destination;
    bool valid = false;

   private:
    // parts of cartridge header
    uint8_t entry_point[4];
    uint8_t nintendo_logo[48];
    uint8_t game_title_code[16];
    // uint8_t manufacturer_code;
    // uint8_t gbc_enabled;
    uint16_t new_publisher_code;
    uint8_t sgb_enabled;
    uint8_t cartridge_type_code;
    uint8_t rom_size_code;
    uint8_t ram_size;
    uint8_t destination_code;
    uint8_t old_publisher_code;
    uint8_t rom_version_number;
    uint8_t header_checksum = 0;
    uint8_t global_checksum;

    // mbc banking and ram state
    uint16_t rom_bank;
    uint8_t ram_bank;
    bool ram_enabled;
    uint8_t banking_mode;
    uint8_t *ram_data;
    uint32_t ram_size_bytes;
    int total_rom_banks;

    // mbc3 real-time clock (rtc) state
    uint8_t rtc_regs[5];
    uint8_t rtc_latched[5];
    uint8_t rtc_latch_prep;
    bool has_rtc;

   public:
    cartridge();
    ~cartridge();
    bool load_cartridge(char **argv);
    void print_cartridge_info();
    void print_header_hex();
    void print_beginning_hex();
    bool verify_header_checksum(bool debug);
    uint8_t read_cart(uint16_t);
    bool write_cart(uint8_t, uint16_t);
};

#endif