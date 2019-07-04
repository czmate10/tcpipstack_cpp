#include "utility.h"
#include "arp.h"

struct ArpPacket
{
    uint16_t hw_type;
    uint16_t protocol_type;
    uint8_t hw_size;
    uint8_t protocol_size;
    uint16_t op_code;
    uint8_t *source_mac;
    uint32_t source_address;
    uint8_t *dest_mac;
    uint32_t dest_address;
} __attribute__((packed));


Arp::Arp(Tap *tap_device) : m_tap_device(tap_device) {

}

void Arp::processArpPacket(const std::shared_ptr<Buffer>& buffer) {
    ArpPacket arp_packet;

    arp_packet.hw_type = buffer->unpack16(0);
    arp_packet.protocol_type = buffer->unpack16(2);
    arp_packet.hw_size = buffer->unpack8(4);
    arp_packet.protocol_size = buffer->unpack8(5);
    arp_packet.op_code = buffer->unpack16(6);
    arp_packet.source_mac = &buffer->m_data[8];
    arp_packet.source_address = buffer->m_data[14];
    arp_packet.dest_mac = &buffer->m_data[18];
    arp_packet.source_address = buffer->m_data[24];


    printf("ARP request, from: %s - to: %s\n", macToString(arp_packet.source_mac).c_str(), macToString(arp_packet.dest_mac).c_str());
}
