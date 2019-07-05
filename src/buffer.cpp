#include <cstdlib>

#include <buffer.h>
#include <stdexcept>
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

void Buffer::resetDataOffset(size_t offset) {
    m_data = m_data_original + offset;
}

uint8_t *Buffer::getDefaultDataOffset() {
    return m_data_original;
}

uint8_t Buffer::unpack8(size_t offset) {
    return m_data[offset];
}

uint16_t Buffer::unpack16(size_t offset) {
    return ntohs(*(uint16_t*)(m_data + offset));
}

uint32_t Buffer::unpack32(size_t offset) {
    return ntohl(*(uint32_t*)(m_data + offset));
}

void Buffer::pack8(uint8_t data, size_t offset) {
    m_data[offset] = data;
}

void Buffer::pack16(uint16_t data, size_t offset) {
    uint16_t data_network = htons(data);
    std::memcpy(m_data + offset, &data_network, 2);
}

void Buffer::pack32(uint32_t data, size_t offset) {
    uint32_t data_network = htonl(data);
    std::memcpy(m_data + offset, &data_network, 4);
}
