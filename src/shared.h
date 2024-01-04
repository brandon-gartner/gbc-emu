#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#define NOT_IMPLEMENTED()                                                         \
    std::cout << "Not implemented: " << __FILE__ << ":" << __LINE__ << std::endl; \
    exit(1);