#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "buffer.h"
#include "ethernet.h"


#define ARP_HWTYPE_ETHERNET 1
#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2


class Tap;

class Arp {
private:
    struct ArpPacket
    {
        uint16_t hw_type;
        uint16_t protocol_type;
        uint8_t hw_size;
        uint8_t protocol_size;
        uint16_t op_code;
        uint8_t *source_mac;
        uint32_t source_addr;
        uint8_t *dest_mac;
        uint32_t dest_addr;
    } __attribute__((packed));

    struct ArpCacheEntry
    {
        uint8_t hw_address[ETHERNET_ADDRESS_LEN];
        uint32_t protocol_address;
    };

    Tap *m_tap_device;
    std::unordered_map<uint32_t, Arp::ArpCacheEntry> m_arp_cache;

    ArpPacket parseArpPacket(const std::shared_ptr<Buffer>& buffer);
    void processArpPacketIPv4(uint16_t opcode, uint8_t *source_mac, uint8_t *dest_mac, uint32_t source_addr, uint32_t dest_addr);


public:
    explicit Arp(Tap *tap_device);

    void processArpPacket(const std::shared_ptr<Buffer>& buffer);
};