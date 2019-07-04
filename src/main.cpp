#include <utility>

#include <memory>

#include <iostream>
#include <sstream>
#include <memory>

#include "utility.h"
#include "tap.h"


int main() {
    auto tap_device = new Tap("tap0", "192.168.100.6");

    std::cout << macToString(tap_device->m_mac) << std::endl;
    std::cout << "TAP device created, name: " << tap_device->m_device_name << ", MTU: " << tap_device->m_mtu << std::endl;

    tap_device->listen();

    delete tap_device;
    return 0;
}