#include "icmp.h"
#include "ipv4.h"

Icmp::Icmp(Tap& tap_device, Ipv4 &ipv4_state) : m_tap_device(tap_device), m_ipv4_state(ipv4_state) {

}

void Icmp::processIcmpPacket(const std::shared_ptr<Buffer> &buffer) {

}
