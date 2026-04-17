#pragma once
// ============================================================================
//  NibbleArray  --  compact array of 4-bit values (2 entries per byte).
//  The corner database's max value is 11, so 4 bits are enough: 88M entries
//  fit in ~44 MB instead of 88 MB.
// ============================================================================
#include <cstdint>
#include <vector>

class NibbleArray {
public:
    NibbleArray(size_t size, uint8_t initial = 0xF)
        : size_(size), data_((size + 1) / 2, static_cast<uint8_t>((initial & 0xF) * 0x11)) {}

    uint8_t get(size_t pos) const {
        const uint8_t byte = data_[pos >> 1];
        return (pos & 1) ? (byte >> 4) : (byte & 0x0F);
    }

    void set(size_t pos, uint8_t val) {
        uint8_t& byte = data_[pos >> 1];
        if (pos & 1) byte = static_cast<uint8_t>((byte & 0x0F) | (val << 4));
        else         byte = static_cast<uint8_t>((byte & 0xF0) | (val & 0x0F));
    }

    size_t size() const { return size_; }
    size_t storageBytes() const { return data_.size(); }
    uint8_t* data() { return data_.data(); }
    const uint8_t* data() const { return data_.data(); }

private:
    size_t size_;
    std::vector<uint8_t> data_;
};
