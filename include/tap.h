#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "buffer.h"
#include "arp.h"

class Tap {
private:
    bool m_running = true;
    int m_sock_fd;
    Arp m_arp_state;

    std::shared_ptr<Buffer> read(size_t size);
    void write(std::shared_ptr<Buffer> buffer);

public:
    std::string m_device_name;
    uint8_t m_mac[6];
    uint32_t m_ipv4;
    uint64_t m_ipv6[2];
    uint16_t m_mtu;

    explicit Tap(const std::string& desired_device_name, const std::string& ipv4);
    ~Tap();

    void initDevice(const std::string &device_name);
    void listen();
};