//
// Created by goforbroke on 8/9/26.
//

#ifndef SYMBIAN_EMULATOR_BINARY_READER_H
#define SYMBIAN_EMULATOR_BINARY_READER_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>
#include <iterator>

class BinaryReader {
public:
    explicit BinaryReader(const std::vector<uint8_t> &data);

    static BinaryReader from_file(const std::string &filename);

    void seek(size_t pos) {
        this->pos_ = pos;
    }

    [[nodiscard]] size_t size() const {
        return data_.size();
    }

    uint8_t read_u8();

    uint16_t read_u16_le();

    uint32_t read_u32_le();

    uint64_t read_u64_le();

    std::vector<uint8_t> read_bytes(size_t count);

    std::u16string read_string(uint64_t length);

    [[nodiscard]] size_t position() const {
        return pos_;
    }

    [[nodiscard]] size_t remaining() const;

    [[nodiscard]] bool empty() const { return remaining() == 0; }

    // BinaryReader to_new_reader(size_t from, size_t length) const;

private:
    void require(size_t count) const {
        if (count > remaining())
            throw std::runtime_error(
                "Unexpected end of file: want " + std::to_string(count) + ", have " + std::to_string(remaining()));
    }

    std::vector<uint8_t> data_;
    size_t pos_ = 0;
};

std::string bytes_to_string(const std::vector<uint8_t> &bytes);

#endif //SYMBIAN_EMULATOR_BINARY_READER_H
