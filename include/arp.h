#pragma once

#include <cstdint>
#include <memory>

#include "buffer.h"
#include "ethernet.h"


class Tap;

class Arp {
private:
    Tap *m_tap_device;

public:
    explicit Arp(Tap *tap_device);

    void processArpPacket(const std::shared_ptr<Buffer>& buffer);
};