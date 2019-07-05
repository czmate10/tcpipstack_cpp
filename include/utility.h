#pragma once

#include <cstdio>
#include <cstdint>
#include <sstream>


std::string macToString(uint8_t *mac);
void stringToMac(const std::string& macString, uint8_t *out);

std::string ipv4ToString(uint32_t ipv4);
uint32_t stringToIpv4(const std::string& ipv4_str);

uint16_t checksum(uint16_t *ptr, uint32_t len, uint32_t sum);
uint16_t tcp_checksum(void *tcp_segment, uint16_t tcp_segment_len, uint32_t source_ip, uint32_t dest_ip);
