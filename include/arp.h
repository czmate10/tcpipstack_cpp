#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "packets.h"
#include "buffer.h"
#include "ethernet.h"


#define ARP_HWTYPE_ETHERNET 1
#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2


class Tap;
class Ipv4;

class Arp {
public:
    explicit Arp(Tap &tap_device, Ipv4 &ipv4_state);

    void processArpPacket(EthernetFrame *frame);
    uint8_t *translateProtocolAddr(uint32_t protocol_addr);

private:
    struct ArpCacheEntry
    {
        uint8_t hardware_addr[ETHERNET_ADDRESS_LEN];
        uint32_t protocol_addr;
    };

    Tap &m_tap_device;
    Ipv4 &m_ipv4_state;
    std::unordered_map<uint32_t, Arp::ArpCacheEntry> m_arp_cache;

    void processArpRequest(uint8_t *hardware_addr, uint32_t protocol_addr);
    void addToArpCache(uint8_t *hardware_addr, uint32_t protocol_addr);
    void sendArpRequest(uint32_t protocol_address);
};