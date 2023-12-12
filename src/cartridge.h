#ifndef CARTRIDGE_H
#define CARTRIDGE_H

class cartridge {
    private:
        // entry_point; //
        // nintendo_logo; //
        // game_title:
        // manufacturer_code:
        // gbc_enabled:
        // new_publisher_code:
        // sgb_enabled:
        // cartridge_type:
        // rom_size:
        // ram_size:
        // old_publisher_code:
        // rom_version_number:
        // header_checksum:
        // global_checksum:
    public:
        bool load_cartridge(char **argv);
};

#endif