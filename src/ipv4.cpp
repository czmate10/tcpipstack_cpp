#include <netinet/in.h>
#include <iostream>
#include "ipv4.h"
#include "tap.h"


Ipv4::Ipv4(Tap &tap_device, Icmp& icmp_state) : m_tap_device(tap_device), m_icmp_state(icmp_state) {

}

Ipv4::Ipv4Packet Ipv4::parseIpv4Packet(const std::shared_ptr<Buffer> &buffer) {
    auto ip_packet = reinterpret_cast<Ipv4Packet *>(buffer->m_data);

    ip_packet->len = ntohs(ip_packet->len);
    ip_packet->id = ntohs(ip_packet->id);
    ip_packet->fragment_offset = ntohs(ip_packet->fragment_offset);
    ip_packet->checksum = ntohs(ip_packet->checksum);

    return *ip_packet;
}

void Ipv4::processIpv4Packet(const std::shared_ptr<Buffer> &buffer) {
    auto ip_packet = parseIpv4Packet(buffer);

    if(ip_packet.fragment_offset & (uint16_t)IPV4_FLAG_MF) {
        std::cerr << "Received fragmented IP packet, dropping it..." << std::endl;
        return;
    }

    switch(ip_packet.protocol) {
        case IPPROTO_ICMP:
            printf("icmp\n");
            break;

        case IPPROTO_UDP:
            printf("udp\n");
            break;

        case IPPROTO_TCP:
            printf("tcp\n");
            break;

        default:
            std::cerr << "Encountered unknown IPv4 protocol: " << ip_packet.protocol << " - dropping it..." << std::endl;
    }

    printf("Ipv4 packet len: %d, ttl: %d\n", ip_packet.len, ip_packet.ttl);
}
