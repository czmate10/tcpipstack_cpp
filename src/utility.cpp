#include "utility.h"

std::string macToString(uint8_t *mac) {
    std::stringstream buff;
    buff << std::hex;

    for(int i = 0; i < 6; i++) {
        buff << static_cast<int>(mac[i]);
        if(i != 5)
            buff << ":";
    }
    return buff.str();
}