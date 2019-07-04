#include <cstdlib>

#include <buffer.h>
#include <stdexcept>
#include <stdlib.h>
#include <memory>
#include <cstring>
#include <netinet/in.h>


Buffer::Buffer(size_t size) : m_size(size) {
    m_data = (uint8_t *)std::malloc(size);
    if(m_data == nullptr) {
        throw std::runtime_error("could not allocate memory for buffer");
    }

    std::memset(m_data, 0, size);

    m_data_original = m_data;
}

Buffer::~Buffer() {
    std::free(m_data_original);
}

uint8_t Buffer::unpack8(size_t offset) {
    return m_data[offset];
}

uint16_t Buffer::unpack16(size_t offset) {
    return ntohs(*(uint16_t*)(m_data + offset));
}

uint32_t Buffer::unpack32(size_t offset) {
    return ntohs(*(uint32_t*)(m_data + offset));
}
