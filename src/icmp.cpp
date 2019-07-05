#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include "utility.h"
#include "icmp.h"
#include "ipv4.h"

Icmp::Icmp(Tap& tap_device, Ipv4 &ipv4_state) : m_tap_device(tap_device), m_ipv4_state(ipv4_state) {

}

void Icmp::processIcmpPacket(Ipv4Packet *ipv4_packet) {
    auto icmp_packet = reinterpret_cast<IcmpPacket *>(ipv4_packet->payload);

    uint32_t icmp_packet_size = ipv4_packet->len - (ipv4_packet->header_len * 4);

    // Calculate checksum, need to set to 0 for it
    uint16_t checksum_original = icmp_packet->checksum;
    icmp_packet->checksum = 0;
    icmp_packet->checksum = checksum(reinterpret_cast<uint16_t *>(icmp_packet), icmp_packet_size, 0);

    if(checksum_original != icmp_packet->checksum) {
        std::cerr << "Dropping ICMP packet with wrong checksum" << std::endl;
        return;
    }

    switch(icmp_packet->type) {
        case ICMP_ECHO: {
            auto buffer_reply = m_ipv4_state.createPacket(ipv4_packet->source_ip, IPPROTO_ICMP, icmp_packet_size);
            auto icmp_packet_reply = reinterpret_cast<IcmpPacket *>(buffer_reply->m_data);

            icmp_packet_reply->type = ICMP_ECHOREPLY;
            icmp_packet_reply->code = 0;
            icmp_packet_reply->header_data = icmp_packet->header_data;
            std::memcpy(icmp_packet_reply->payload, icmp_packet->payload, icmp_packet_size - sizeof(IcmpPacket));
            icmp_packet_reply->checksum = checksum(reinterpret_cast<uint16_t *>(icmp_packet_reply), icmp_packet_size, 0);

            m_ipv4_state.transmitPacket(buffer_reply);

            std::cout << "sent reply" <<std::endl;
            break;
        }

        case ICMP_DEST_UNREACH:
            std::cerr << "ICMP - destination unreachable (" << icmp_packet->code << ")" << std::endl;
            break;

        case ICMP_TIME_EXCEEDED:
            std::cerr << "ICMP - time exceeded (" << icmp_packet->code << ")" << std::endl;
            break;

        default:
            std::cerr << "ICMP - unknown type: " << icmp_packet->type << ", code: " << icmp_packet->code << std::endl;
    }
}
