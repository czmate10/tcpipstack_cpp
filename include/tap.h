#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "buffer.h"
#include "arp.h"
#include "ipv4.h"

class Tap {
private:
    bool m_running = true;
    int m_sock_fd;

    Arp m_arp_state;
    Ipv4 m_ipv4_state;

    std::shared_ptr<Buffer> read(size_t size);

public:
    std::string m_device_name;
    uint8_t m_mac[6];
    uint32_t m_ipv4;
    uint64_t m_ipv6[2];
    uint16_t m_mtu;

    void send(uint8_t *dest_mac, uint16_t eth_type, const std::shared_ptr<Buffer>& buffer);
    explicit Tap(const std::string& desired_device_name, const std::string& ipv4);
    ~Tap();

    void initDevice(const std::string &device_name);
    void listen();
};