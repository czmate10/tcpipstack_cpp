#pragma once

#include <cstdint>
#include <memory>

#include "buffer.h"

#define IPV4_ADDRESS_LEN 4
#define IPV4_FLAG_DF 0x4000  // don't fragment
#define IPV4_FLAG_MF 0x2000 // more fragments

class Tap;
class Icmp;

class Ipv4 {
private:

    struct Ipv4Packet
    {
        uint8_t header_len:4, version:4;
        uint8_t tos;
        uint16_t len;
        uint16_t id;
        uint16_t fragment_offset;
        uint8_t ttl;
        uint8_t protocol;
        uint16_t checksum;
        uint32_t source_ip;
        uint32_t dest_ip;
        uint8_t data[];
    } __attribute__((packed));

    Tap &m_tap_device;
    Icmp &m_icmp_state;

    Ipv4Packet parseIpv4Packet(const std::shared_ptr<Buffer>& buffer);

public:
    Ipv4(Tap &tap_device, Icmp &icmp_state);

    void processIpv4Packet(const std::shared_ptr<Buffer>& buffer);
};