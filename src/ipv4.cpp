#include <iostream>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include "ipv4.h"
#include "utility.h"
#include "tap.h"
#include "arp.h"


Ipv4::Ipv4(Tap &tap_device, Arp& arp_state, Icmp& icmp_state, Udp& udp_state, Tcp& tcp_state)
	: m_tap_device(tap_device)
	, m_arp_state(arp_state)
	, m_icmp_state(icmp_state)
	, m_udp_state(udp_state)
	, m_tcp_state(tcp_state) {
}

void Ipv4::processIpv4Packet(EthernetFrame *frame) {
    auto ip_packet = reinterpret_cast<Ipv4Packet *>(frame->payload);

    // Is it for us?
    if(ip_packet->dest_ip != m_tap_device.m_ipv4)
        return;

    // Validate checksum
    uint16_t checksum_original = ip_packet->checksum;
    ip_packet->checksum = 0;
    ip_packet->checksum = checksum(reinterpret_cast<uint16_t *>(ip_packet), ip_packet->header_len * 4u, 0);

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

    switch(ip_packet->protocol) {
        case IPPROTO_ICMP:
            m_icmp_state.processIcmpPacket(ip_packet);
            break;

        case IPPROTO_UDP:
            m_udp_state.processUdpPacket(ip_packet);
            break;

        case IPPROTO_TCP:
            m_tcp_state.processTcpPacket(ip_packet);
            break;

        default:
            std::cerr << "Encountered unknown IPv4 protocol: " << ip_packet->protocol << " - dropping it..." << std::endl;
    }
}

std::shared_ptr<Buffer> Ipv4::createPacket(uint32_t ip_destination, uint8_t ip_protocol, size_t size) {
    auto buffer = std::make_shared<Buffer>(sizeof(EthernetFrame) + sizeof(Ipv4Packet) + size);

    // Header stuff
    uint8_t header_len = 20;

    auto ip_packet = reinterpret_cast<Ipv4Packet *>(buffer->m_data + sizeof(EthernetFrame));
    ip_packet->header_len = header_len >> 2u;
    ip_packet->version = 4;
    ip_packet->len = htons(buffer->m_size - sizeof(EthernetFrame));
    ip_packet->id = static_cast<uint16_t>(lrand48());
    ip_packet->fragment_offset = htons(IPV4_FLAG_DF);
    ip_packet->ttl = IPV4_DEFAULT_TTL;
    ip_packet->protocol = ip_protocol;
    ip_packet->source_ip = m_tap_device.m_ipv4;
    ip_packet->dest_ip = ip_destination;
    ip_packet->checksum = 0;
    ip_packet->checksum = checksum(reinterpret_cast<uint16_t *>(ip_packet), 20, 0);

    // Set pointer to the payload
    buffer->m_data += sizeof(EthernetFrame) + header_len;

    return buffer;
}

void Ipv4::transmitPacket(const std::shared_ptr<Buffer>& buffer) {
    auto ip_packet = reinterpret_cast<Ipv4Packet *>(buffer->getDefaultDataOffset() + sizeof(EthernetFrame));

    // Get hardware address of destination
    auto dest_mac = m_arp_state.translateProtocolAddr(ip_packet->dest_ip);
    if(dest_mac == nullptr) {
        m_pending_packets[ip_packet->dest_ip].push_back(buffer);
        return;
    }

    m_tap_device.send(dest_mac, ETH_P_IP, buffer);
}

void Ipv4::retryPendingPackets(uint32_t ip_destination) {
    auto pending_buffers = m_pending_packets.find(ip_destination);

    if(pending_buffers != m_pending_packets.end()) {
        for(const std::shared_ptr<Buffer>& buffer : pending_buffers->second) {
            transmitPacket(buffer);
        }
    }
}