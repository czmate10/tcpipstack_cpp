#pragma once

#include <cstdint>
#include "ethernet.h"

class Buffer {
private:
    uint8_t *m_data_original;

public:
    size_t m_size;
    uint8_t *m_data;

    explicit Buffer(size_t size);
    ~Buffer();

    void pack8(size_t offset);
    void pack16(size_t offset);
    void pack32(size_t offset);

    uint8_t unpack8(size_t offset);
    uint16_t unpack16(size_t offset);
    uint32_t unpack32(size_t offset);
};