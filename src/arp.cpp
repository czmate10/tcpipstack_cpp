#include <linux/if_ether.h>
#include <cstring>

#include "tap.h"
#include "ipv4.h"
#include "utility.h"
#include "arp.h"

Arp::Arp(Tap *tap_device) : m_tap_device(tap_device), m_arp_cache() {

}

Arp::ArpPacket Arp::parseArpPacket(const std::shared_ptr<Buffer> &buffer) {
    ArpPacket arp_packet{};

    arp_packet.hw_type = buffer->unpack16(0);
    arp_packet.protocol_type = buffer->unpack16(2);
    arp_packet.hw_size = buffer->unpack8(4);
    arp_packet.protocol_size = buffer->unpack8(5);
    arp_packet.op_code = buffer->unpack16(6);
    arp_packet.source_mac = &buffer->m_data[8];
    arp_packet.source_addr = buffer->m_data[14];
    arp_packet.dest_mac = &buffer->m_data[18];
    arp_packet.source_addr = buffer->m_data[24];

    return arp_packet;
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

    processArpPacketIPv4(arp_packet.op_code, arp_packet.source_mac, arp_packet.dest_mac, arp_packet.source_addr, arp_packet.dest_addr);

    printf("ARP request, from: %s - to: %s\n", macToString(arp_packet.source_mac).c_str(), macToString(arp_packet.dest_mac).c_str());
}

void Arp::processArpPacketIPv4(uint16_t opcode, uint8_t *source_mac, uint8_t *dest_mac, uint32_t source_addr, uint32_t dest_addr) {
    if(opcode == ARP_OP_REQUEST) {
        auto buffer = std::make_shared<Buffer>(ETHERNET_HEADER_SIZE + sizeof(ArpPacket));
        buffer->m_data += ETHERNET_HEADER_SIZE;

        buffer->pack16(ARP_HWTYPE_ETHERNET, 0);
        buffer->pack16(ETH_P_IP, 2);
        buffer->pack8(ETHERNET_ADDRESS_LEN, 4);
        buffer->pack8(IPV4_ADDRESS_LEN, 5);
        buffer->pack16(ARP_OP_REPLY, 6);
        std::memcpy(buffer->m_data + 8, m_tap_device->m_mac, 6);
        std::memcpy(buffer->m_data + 14, &m_tap_device->m_ipv4, 4);
        std::memcpy(buffer->m_data + 18, source_mac, 6);
        std::memcpy(buffer->m_data + 24, &source_addr, 4);

        m_tap_device->send(source_mac, ETH_P_ARP, buffer);
    }

    // Add to cache
    ArpCacheEntry cache_entry = {
            .hw_address = {source_mac[0], source_mac[1], source_mac[2], source_mac[3], source_mac[4], source_mac[5]},
            .protocol_address = source_addr
    };
    m_arp_cache[source_addr] = cache_entry;
}
