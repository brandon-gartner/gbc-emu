#include <iostream>
#include "cartridge.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please supply a ROM file to load." << std::endl;
        return 1;
    }
    cartridge cart;
    std::cout << "Loading ROM file: " << argv[1] << std::endl;
    if (cart.load_cartridge(argv)) {
        std::cout << "ROM file loaded successfully." << std::endl;
    } else {
        std::cout << "ROM file failed to load." << std::endl;
    }
}