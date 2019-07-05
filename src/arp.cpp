#include <linux/if_ether.h>
#include <cstring>
#include <netinet/in.h>
#include <iostream>

#include "tap.h"
#include "ipv4.h"
#include "utility.h"
#include "arp.h"

Arp::Arp(Tap &tap_device) : m_tap_device(tap_device), m_arp_cache() {

}

ArpPacket Arp::parseArpPacket(const std::shared_ptr<Buffer> &buffer) {
    auto arp_packet = reinterpret_cast<ArpPacket *>(buffer->m_data);

    arp_packet->hw_type = ntohs(arp_packet->hw_type);
    arp_packet->protocol_type = ntohs(arp_packet->protocol_type);
    arp_packet->op_code = ntohs(arp_packet->op_code);

    return *arp_packet;
}

void Arp::processArpPacket(const std::shared_ptr<Buffer>& buffer) {
    auto arp_packet = parseArpPacket(buffer);

    if(arp_packet.hw_type != ARP_HWTYPE_ETHERNET)
        return; // not Ethernet

    if(arp_packet.hw_size != ETHERNET_ADDRESS_LEN)
        return;

    if(arp_packet.protocol_size != IPV4_ADDRESS_LEN)
        return;

    if(arp_packet.protocol_type != ETH_P_IP)
        return;

    processArpPacket(arp_packet.op_code, arp_packet.source_mac, arp_packet.dest_mac, arp_packet.source_addr,
                     arp_packet.dest_addr);
}

void Arp::processArpPacket(uint16_t opcode, uint8_t *source_mac, uint8_t *dest_mac, uint32_t source_addr, uint32_t dest_addr) {
    // Is it for us or broadcast?
    for(int i = 0; i < ETHERNET_ADDRESS_LEN; i++) {
        if (dest_mac[i] != 0x00) {
            if (memcmp(dest_mac, m_tap_device.m_mac, 6) != 0)
                return;
            else
                break;
        }
    }

    if(opcode == ARP_OP_REQUEST) {
        auto buffer = std::make_shared<Buffer>(ETHERNET_HEADER_SIZE + sizeof(ArpPacket));
        buffer->m_data += ETHERNET_HEADER_SIZE;

        ArpPacket arp_packet{};
        arp_packet.hw_type = htons(ARP_HWTYPE_ETHERNET);
        arp_packet.protocol_type = htons(ETH_P_IP);
        arp_packet.hw_size = ETHERNET_ADDRESS_LEN;
        arp_packet.protocol_size = IPV4_ADDRESS_LEN;
        arp_packet.op_code = htons(ARP_OP_REPLY);
        arp_packet.source_addr = m_tap_device.m_ipv4;
        arp_packet.dest_addr = source_addr;

        std::memcpy(arp_packet.source_mac, m_tap_device.m_mac, 6);
        std::memcpy(arp_packet.dest_mac, source_mac, 6);

        std::memcpy(buffer->m_data, &arp_packet, sizeof(arp_packet));

        m_tap_device.send(source_mac, ETH_P_ARP, buffer);
    }

    // Add to cache
    ArpCacheEntry cache_entry = {
            .hw_address = {source_mac[0], source_mac[1], source_mac[2], source_mac[3], source_mac[4], source_mac[5]},
            .protocol_address = source_addr
    };
    m_arp_cache[source_addr] = cache_entry;

    std::cout << "Added IP to ARP cache: " << ipv4ToString(source_addr) << " : " << macToString(source_mac) << std::endl;
}

uint8_t *Arp::translate_protocol_addr(uint32_t protocol_addr) {
    auto result = m_arp_cache.find(protocol_addr);

    if(result == m_arp_cache.end())
        return nullptr;
    else
        return result->second.hw_address;
}
