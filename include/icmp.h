#pragma once

#include <memory>

#include "buffer.h"


class Tap;
class Ipv4;


class Icmp {
private:
    Tap &m_tap_device;
    Ipv4 &m_ipv4_state;

public:
    Icmp(Tap& tap_device, Ipv4 &ipv4_state);
    void processIcmpPacket(const std::shared_ptr<Buffer> &buffer);
};