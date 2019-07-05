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

void Arp::processArpPacket(const std::shared_ptr<Buffer>& buffer) {
    auto arp_packet = reinterpret_cast<ArpPacket *>(buffer->m_data);

    arp_packet->hw_type = ntohs(arp_packet->hw_type);
    arp_packet->protocol_type = ntohs(arp_packet->protocol_type);
    arp_packet->op_code = ntohs(arp_packet->op_code);

    if(arp_packet->hw_type != ARP_HWTYPE_ETHERNET)
        return; // not Ethernet

    if(arp_packet->hw_size != ETHERNET_ADDRESS_LEN)
        return;

    if(arp_packet->protocol_size != IPV4_ADDRESS_LEN)
        return;

    if(arp_packet->protocol_type != ETH_P_IP)
        return;

    // Is it for us or broadcast?
    for(uint8_t i : arp_packet->dest_hw_addr) {
        if (i != 0x00) {
            if (memcmp(arp_packet->dest_hw_addr, m_tap_device.m_mac, 6) != 0)
                return;
            else
                break;
        }
    }

    switch(arp_packet->op_code) {
        case ARP_OP_REQUEST:
            addToArpCache(arp_packet->source_hw_addr, arp_packet->source_protocol_addr);
            processArpRequest(arp_packet->source_hw_addr, arp_packet->source_protocol_addr);
            break;

        case ARP_OP_REPLY:
            break;

        default:
            std::cerr << "Unknown ARP opcode: " << arp_packet->op_code << std::endl;
    }
}

void Arp::processArpRequest(uint8_t *hardware_addr, uint32_t protocol_addr) {
    auto buffer = std::make_shared<Buffer>(ETHERNET_HEADER_SIZE + sizeof(ArpPacket));
    buffer->m_data += ETHERNET_HEADER_SIZE;

    auto arp_packet = reinterpret_cast<ArpPacket *>(buffer->m_data);

    arp_packet->hw_type = htons(ARP_HWTYPE_ETHERNET);
    arp_packet->protocol_type = htons(ETH_P_IP);
    arp_packet->hw_size = ETHERNET_ADDRESS_LEN;
    arp_packet->protocol_size = IPV4_ADDRESS_LEN;
    arp_packet->op_code = htons(ARP_OP_REPLY);
    arp_packet->source_protocol_addr = m_tap_device.m_ipv4;
    arp_packet->dest_protocol_addr = protocol_addr;

    std::memcpy(arp_packet->source_hw_addr, m_tap_device.m_mac, 6);
    std::memcpy(arp_packet->dest_hw_addr, hardware_addr, 6);

    m_tap_device.send(hardware_addr, ETH_P_ARP, buffer);
}

void Arp::addToArpCache(uint8_t *hardware_addr, uint32_t protocol_addr) {
    // Add to cache
    ArpCacheEntry cache_entry = {
            .hardware_addr = {hardware_addr[0], hardware_addr[1], hardware_addr[2], hardware_addr[3], hardware_addr[4], hardware_addr[5]},
            .protocol_addr = protocol_addr
    };
    m_arp_cache[protocol_addr] = cache_entry;

    std::cout << "Added IP to ARP cache: " << ipv4ToString(protocol_addr) << " : " << macToString(hardware_addr) << std::endl;
}


uint8_t *Arp::translateProtocolAddr(uint32_t protocol_addr) {
    auto result = m_arp_cache.find(protocol_addr);

    if(result == m_arp_cache.end())
        return nullptr;
    else
        return result->second.hardware_addr;
}