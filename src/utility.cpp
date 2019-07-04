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

void stringToMac(const std::string& macString, uint8_t *out) {
    unsigned int bytes[6];
    if (std::sscanf(macString.c_str(),
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    &bytes[0], &bytes[1], &bytes[2],
                    &bytes[3], &bytes[4], &bytes[5]) != 6)
    {
        throw std::runtime_error("invalid MAC address: " + macString);
    }
    for(int i = 0; i < 6; i++) {
        out[i] = bytes[i];
    }
}