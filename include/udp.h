#pragma once

#include "packets.h"

class Ipv4;

class Udp {
public:
	explicit Udp(Ipv4 &ipv4_state);

	void processUdpPacket(Ipv4Packet *ipv4_packet);
private:
	Ipv4 &m_ipv4_state;
};