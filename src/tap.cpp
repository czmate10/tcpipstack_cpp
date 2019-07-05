#include <utility>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <utility.h>

#include "tap.h"
#include "arp.h"
#include "ethernet.h"
#include "ipv4.h"


Tap::Tap(const std::string& desired_device_name, const std::string& ipv4)
        : m_sock_fd()
        , m_ipv4()
        , m_device_name()
        , m_mac()
        , m_ipv6()
        , m_mtu()
        , m_arp_state(*this, m_ipv4_state)
        , m_ipv4_state(*this, m_arp_state, m_icmp_state)
        , m_icmp_state(*this, m_ipv4_state) {
    initDevice(desired_device_name);

    // Parse IPv4 address
    inet_pton(AF_INET, ipv4.c_str(), &m_ipv4);
}

Tap::~Tap() {
    close(m_sock_fd);
}

void Tap::initDevice(const std::string &device_name) {
    if((m_sock_fd = open("/dev/net/tun", O_RDWR)) == -1)
        throw std::runtime_error("cannot open /dev/net/tun");

    if(m_sock_fd < 0)
        throw std::runtime_error("failed to open file descriptor of network device");

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    struct ifreq ifr = {0};
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    std::strncpy(ifr.ifr_name, device_name.c_str(), IFNAMSIZ);

    if(ioctl(m_sock_fd, TUNSETIFF, reinterpret_cast<void *>(&ifr)) < 0){
        close(m_sock_fd);
        throw std::runtime_error("ioctl(TUNSETIFF) failed: " + std::string(strerror(errno)));
    }

    m_device_name = ifr.ifr_name;

    // Get MAC address
//    ifr = {0};
//    std::strcpy(ifr.ifr_name, m_device_name.c_str());
//    if(ioctl(m_sock_fd, SIOCGIFHWADDR, reinterpret_cast<void *>(&ifr)) < 0) {
//        close(m_sock_fd);
//        throw std::runtime_error("ioctl(SIOCGIFHWADDR) failed: " + std::string(strerror(errno)));
//    }
//    std::memcpy(m_mac, ifr.ifr_hwaddr.sa_data, ETHERNET_ADDRESS_LEN);
    stringToMac("20:20:20:20:20:20", m_mac);

    // Get MTU
    m_mtu = 1500;  // TODO: fix invalid argument error for ioctl when reading MTU
}

std::shared_ptr<Buffer> Tap::read(size_t size) {
    auto buffer = std::make_shared<Buffer>(size);
    ssize_t bytes = ::read(m_sock_fd, buffer->m_data, size);

    if(bytes < 0)
        throw std::runtime_error("failed to read buffer");

    return buffer;
}

void Tap::listen() {
    m_running = true;

    while(m_running) {
        auto buffer = read(ETHERNET_MAX_PAYLOAD_SIZE);
        auto ethernet_frame = reinterpret_cast<EthernetFrame *>(buffer->m_data);
        auto ethernet_type = ntohs(ethernet_frame->ethernet_type);

        switch(ethernet_type) {
            case ETH_P_ARP:
                m_arp_state.processArpPacket(ethernet_frame);
                break;

            case ETH_P_IP:
                m_ipv4_state.processIpv4Packet(ethernet_frame);
                break;

            case ETH_P_IPV6:
                break;

            default:
                throw std::runtime_error("unknown ethernet type encountered: " + std::to_string(ethernet_type));
        }
    }
}

void Tap::send(uint8_t *dest_mac, uint16_t ethernet_type, const std::shared_ptr<Buffer>& buffer) {
    // Setup Ethernet frame
    auto ethernet_frame = reinterpret_cast<EthernetFrame *>(buffer->getDefaultDataOffset());
    ethernet_frame->ethernet_type = htons(ethernet_type);
    std::memcpy(ethernet_frame->dest_mac, dest_mac, 6);
    std::memcpy(ethernet_frame->source_mac, m_mac, 6);

    ssize_t bytes = ::write(m_sock_fd, buffer->getDefaultDataOffset(), buffer->m_size);

    if(bytes < 0)
        throw std::runtime_error("failed to send buffer");
}
