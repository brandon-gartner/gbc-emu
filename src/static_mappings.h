#ifndef STATIC_MAPPINGS_H
#define STATIC_MAPPINGS_H

#include <cstdint>
#include <map>
#include <string>

class static_mappings {
   public:
    static std::map<uint8_t, std::string> new_publisher_codes;
    static std::map<uint8_t, std::string> cartridge_type;
    static std::map<uint8_t, int> rom_banks;
    static std::map<uint8_t, std::string> old_publisher_code;
    static std::map<uint8_t, std::string> destination_codes;
};

#endif
