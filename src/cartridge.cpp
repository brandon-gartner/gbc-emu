#include "cartridge.h"

bool cartridge::load_cartridge(char** argv) {
    this->file_name = argv[1];
    return false;
}
