#include <iostream>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include "ipv4.h"
#include "utility.h"
#include "tap.h"
#include "arp.h"


Ipv4::Ipv4(Tap &tap_device, Arp& arp_state, Icmp& icmp_state) : m_tap_device(tap_device), m_arp_state(arp_state), m_icmp_state(icmp_state) {

}

void Ipv4::processIpv4Packet(const std::shared_ptr<Buffer> &buffer) {
    auto ip_packet = reinterpret_cast<Ipv4Packet *>(buffer->m_data);

    // Is it for us?
    if(ip_packet->dest_ip != m_tap_device.m_ipv4)
        return;

    // Validate checksum
    uint16_t checksum_original = ip_packet->checksum;
    ip_packet->checksum = 0;
    ip_packet->checksum = checksum(reinterpret_cast<uint16_t *>(ip_packet), ip_packet->header_len * 4, 0);

    if(checksum_original != ip_packet->checksum) {
        std::cerr << "Dropping IPv4 packet with wrong checksum" << std::endl;
        return;
    }

    // Set to host endianness
    ip_packet->len = ntohs(ip_packet->len);
    ip_packet->id = ntohs(ip_packet->id);
    ip_packet->fragment_offset = ntohs(ip_packet->fragment_offset);

    if(ip_packet->fragment_offset & (uint16_t)IPV4_FLAG_MF) {
        std::cerr << "Received fragmented IP packet, dropping it..." << std::endl;
        return;
    }

    buffer->m_data += ip_packet->header_len * 4;

    switch(ip_packet->protocol) {
        case IPPROTO_ICMP:
            m_icmp_state.processIcmpPacket(buffer, ip_packet);
            break;

        case IPPROTO_UDP:
            printf("udp\n");
            break;

        case IPPROTO_TCP:
            printf("tcp\n");
            break;

        default:
            std::cerr << "Encountered unknown IPv4 protocol: " << ip_packet->protocol << " - dropping it..." << std::endl;
    }
}

std::shared_ptr<Buffer> Ipv4::createBuffer(size_t size) {
    auto buffer = std::make_shared<Buffer>(ETHERNET_HEADER_SIZE + sizeof(Ipv4Packet) + size);
    buffer->m_data += ETHERNET_HEADER_SIZE + sizeof(Ipv4Packet);

    return buffer;
}

void Ipv4::transmitBuffer(uint32_t ip_destination, uint8_t ip_protocol, std::shared_ptr<Buffer> buffer) {
    buffer->resetDataOffset(ETHERNET_HEADER_SIZE);

    // Header stuff
    uint8_t header_len = 20;

    auto ip_packet = reinterpret_cast<Ipv4Packet *>(buffer->m_data);
    ip_packet->header_len = header_len >> 2u;
    ip_packet->version = 4;
    ip_packet->len = htons(buffer->m_size - ETHERNET_HEADER_SIZE);
    ip_packet->id = static_cast<uint16_t>(lrand48());
    ip_packet->fragment_offset = htons(IPV4_FLAG_DF);
    ip_packet->ttl = IPV4_DEFAULT_TTL;
    ip_packet->protocol = ip_protocol;
    ip_packet->source_ip = m_tap_device.m_ipv4;
    ip_packet->dest_ip = ip_destination;
    ip_packet->checksum = 0;
    ip_packet->checksum = checksum(reinterpret_cast<uint16_t *>(ip_packet), 20, 0);

    // Get hardware address of destination
    auto dest_mac = m_arp_state.translate_protocol_addr(ip_destination);
    if(dest_mac == nullptr) {
        std::cerr << "destination IP not found in ARP cache: " << ipv4ToString(ip_destination) << std::endl;
        return;
    }

    std::cout << "sending to " << macToString(dest_mac) << std::endl;
    m_tap_device.send(dest_mac, ETH_P_IP, buffer);
}
