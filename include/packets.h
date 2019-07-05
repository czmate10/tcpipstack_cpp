#pragma once

#include <cstdint>


struct EthernetFrame
{
    uint8_t dest_mac[6];
    uint8_t source_mac[6];
    uint16_t ethernet_type;
    uint8_t payload[];
};

struct ArpPacket
{
    uint16_t hw_type;
    uint16_t protocol_type;
    uint8_t hw_size;
    uint8_t protocol_size;
    uint16_t op_code;
    uint8_t source_hw_addr[6];
    uint32_t source_protocol_addr;
    uint8_t dest_hw_addr[6];
    uint32_t dest_protocol_addr;
} __attribute__((packed));

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
    uint8_t payload[];
} __attribute__((packed));

struct IcmpPacket {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t header_data;
    uint8_t payload[];
};