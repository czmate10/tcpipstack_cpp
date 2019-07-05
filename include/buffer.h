#pragma once

#include <cstdint>
#include "ethernet.h"

class Buffer {
public:
    size_t m_size;
    uint8_t *m_data;

    explicit Buffer(size_t size);
    ~Buffer();

    void resetDataOffset(size_t offset = 0);

    void pack8(uint8_t data, size_t offset = 0);
    void pack16(uint16_t data, size_t offset = 0);
    void pack32(uint32_t data, size_t offset = 0);

    uint8_t unpack8(size_t offset = 0);
    uint16_t unpack16(size_t offset = 0);
    uint32_t unpack32(size_t offset = 0);

private:
    uint8_t *m_data_original;
};