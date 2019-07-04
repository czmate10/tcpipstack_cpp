#pragma once

#include <cstdio>
#include <cstdint>
#include <netinet/in.h>
#include <sstream>


std::string macToString(uint8_t *mac);
void stringToMac(const std::string& macString, uint8_t *out);
