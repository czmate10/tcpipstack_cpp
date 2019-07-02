#pragma once

#include <string>
#include <cstdint>


class Tap {
private:
    int m_sock_fd;
    std::string m_deviceName;
    uint8_t m_mac[6];
    uint32_t m_ipv4;
    uint64_t m_ipv6[2];
    uint16_t m_mtu;

public:
    explicit Tap(const std::string& desiredDeviceName);
    ~Tap();

    void InitDevice(const std::string& deviceName);

    std::string deviceName() { return m_deviceName; }
    uint8_t *mac() { return m_mac; }
    uint32_t ipv4() { return m_ipv4; }
    uint64_t* ipv6() { return m_ipv6; }
    uint16_t mtu() { return m_mtu; }
};