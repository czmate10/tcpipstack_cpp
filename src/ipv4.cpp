#include <netinet/in.h>
#include "ipv4.h"
#include "tap.h"


Ipv4::Ipv4(Tap *tap_device) : m_tap_device(tap_device) {

}

Ipv4::Ipv4Packet Ipv4::parseIpv4Packet(const std::shared_ptr<Buffer> &buffer) {
    auto ipv4_packet = reinterpret_cast<Ipv4Packet *>(buffer->m_data);

    ipv4_packet->len = ntohs(ipv4_packet->len);
    ipv4_packet->id = ntohs(ipv4_packet->id);
    ipv4_packet->fragment_offset = ntohs(ipv4_packet->fragment_offset);
    ipv4_packet->checksum = ntohs(ipv4_packet->checksum);

    return *ipv4_packet;
}

void Ipv4::processIpv4Packet(const std::shared_ptr<Buffer> &buffer) {
    auto ipv4_packet = parseIpv4Packet(buffer);

    printf("Ipv4 packet len: %d, ttl: %d\n", ipv4_packet.len, ipv4_packet.ttl);
}
