#include <linux/if_ether.h>
#include <cstring>
#include <netinet/in.h>
#include <iostream>

#include "tap.h"
#include "ipv4.h"
#include "utility.h"
#include "arp.h"

Arp::Arp(Tap &tap_device, Ipv4 &ipv4_state) : m_tap_device(tap_device), m_arp_cache(), m_ipv4_state(ipv4_state) {

}

void Arp::processArpPacket(EthernetFrame *frame) {
    auto arp_packet = reinterpret_cast<ArpPacket *>(frame->payload);

    // Is it for us or broadcast?
    for(uint8_t i : arp_packet->dest_hw_addr) {
        if (i != 0x00) {
            if (memcmp(arp_packet->dest_hw_addr, m_tap_device.m_mac, 6) != 0)
                return;
            else
                break;
        }
    }

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


    switch(arp_packet->op_code) {
        case ARP_OP_REQUEST:
            addToArpCache(arp_packet->source_hw_addr, arp_packet->source_protocol_addr);
            processArpRequest(arp_packet->source_hw_addr, arp_packet->source_protocol_addr);
            break;

        case ARP_OP_REPLY:
            addToArpCache(arp_packet->source_hw_addr, arp_packet->source_protocol_addr);
            break;

        default:
            std::cerr << "Unknown ARP opcode: " << arp_packet->op_code << std::endl;
    }
}

void Arp::processArpRequest(uint8_t *hardware_addr, uint32_t protocol_addr) {
    auto buffer = std::make_shared<Buffer>(sizeof(EthernetFrame) + sizeof(ArpPacket));

    // Set Ethernet type
    auto ethernet_frame = reinterpret_cast<EthernetFrame *>(buffer->m_data);
    ethernet_frame->ethernet_type = htons(ETH_P_ARP);

    auto arp_packet = reinterpret_cast<ArpPacket *>(ethernet_frame->payload);

    arp_packet->hw_type = htons(ARP_HWTYPE_ETHERNET);
    arp_packet->protocol_type = htons(ETH_P_IP);
    arp_packet->hw_size = ETHERNET_ADDRESS_LEN;
    arp_packet->protocol_size = IPV4_ADDRESS_LEN;
    arp_packet->op_code = htons(ARP_OP_REPLY);
    arp_packet->source_protocol_addr = m_tap_device.m_ipv4;
    arp_packet->dest_protocol_addr = protocol_addr;

    std::memcpy(arp_packet->source_hw_addr, m_tap_device.m_mac, ETHERNET_ADDRESS_LEN);
    std::memcpy(arp_packet->dest_hw_addr, hardware_addr, ETHERNET_ADDRESS_LEN);

    m_tap_device.send(hardware_addr, buffer);
}

void Arp::addToArpCache(uint8_t *hardware_addr, uint32_t protocol_addr) {
    // Add to cache
    ArpCacheEntry cache_entry = {
            .hardware_addr = {hardware_addr[0], hardware_addr[1], hardware_addr[2], hardware_addr[3], hardware_addr[4], hardware_addr[5]},
            .protocol_addr = protocol_addr
    };
    m_arp_cache[protocol_addr] = cache_entry;

    // Signal to ipv4 that we can send the outbound packets now
    m_ipv4_state.retryPendingPackets(protocol_addr);

    std::cout << "Added IP to ARP cache: " << ipv4ToString(protocol_addr) << " : " << macToString(hardware_addr) << std::endl;
}


void Arp::sendArpRequest(uint32_t protocol_address) {
    auto buffer = std::make_shared<Buffer>(sizeof(EthernetFrame) + sizeof(ArpPacket));
    auto ethernet_frame = reinterpret_cast<EthernetFrame *>(buffer->m_data);
    auto arp_packet = reinterpret_cast<ArpPacket *>(ethernet_frame->payload);

    ethernet_frame->ethernet_type = htons(ETH_P_ARP);

    arp_packet->hw_type = htons(ARP_HWTYPE_ETHERNET);
    arp_packet->protocol_type = htons(ETH_P_IP);
    arp_packet->hw_size = ETHERNET_ADDRESS_LEN;
    arp_packet->protocol_size = IPV4_ADDRESS_LEN;
    arp_packet->op_code = htons(ARP_OP_REQUEST);
    arp_packet->source_protocol_addr = m_tap_device.m_ipv4;
    arp_packet->dest_protocol_addr = protocol_address;

    std::memset(arp_packet->dest_hw_addr, 0xFF, ETHERNET_ADDRESS_LEN);
    std::memcpy(arp_packet->source_hw_addr, m_tap_device.m_mac, ETHERNET_ADDRESS_LEN);

    m_tap_device.send(arp_packet->dest_hw_addr, buffer);
}


uint8_t *Arp::translateProtocolAddr(uint32_t protocol_addr) {
    auto result = m_arp_cache.find(protocol_addr);

    if(result == m_arp_cache.end()) {
        sendArpRequest(protocol_addr);
        return nullptr;
    }

    return result->second.hardware_addr;
}