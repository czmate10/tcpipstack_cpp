#include <utility>

#include <utility>

#include "../include/tap.h"

#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <malloc.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>


Tap::Tap(const std::string& desiredDeviceName) {
    InitDevice(desiredDeviceName);
}

Tap::~Tap() {
    close(m_sock_fd);
}

void Tap::InitDevice(const std::string& desiredDeviceName) {
    int fd;

    if((fd = open("/dev/net/tun", O_RDWR)) == -1)
        throw std::runtime_error("cannot open /dev/net/tun");

    if(fd < 0)
        throw std::runtime_error("failed to open file descriptor of network device");

    m_sock_fd = fd;

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    struct ifreq ifr = {0};
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, desiredDeviceName.c_str(), IFNAMSIZ);

    if(ioctl(m_sock_fd, TUNSETIFF, (void *) &ifr) < 0){
        close(m_sock_fd);
        throw std::runtime_error("ioctl failed");
    }

    m_deviceName = std::string(ifr.ifr_name);

    // Get MAC address
    struct ifreq ifr_mac = {};
    ioctl(m_sock_fd, SIOCGIFHWADDR, &ifr_mac);
    memcpy(m_mac, ifr_mac.ifr_hwaddr.sa_data, IFHWADDRLEN);
}
