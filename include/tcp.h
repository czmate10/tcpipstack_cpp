#pragma once

#include "packets.h"

class Ipv4;

class Tcp {
public:
	explicit Tcp(Ipv4 &ipv4_state);

	void processTcpPacket(Ipv4Packet *ipv4_packet);
private:
	Ipv4 &m_ipv4_state;
};