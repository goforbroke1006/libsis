//
// Created by goforbroke on 8/9/26.
//

#include "../../include/sis/BinaryReader.h"

#include <iomanip>
#include <fstream>
#include <sstream>

BinaryReader::BinaryReader(const std::vector<uint8_t> &data) : data_(data), pos_(0) {
    if (data_.empty())
        throw std::runtime_error("Empty data vector");
}

BinaryReader BinaryReader::from_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file)
        throw std::runtime_error("Cannot open file: " + filename);

    std::vector<uint8_t> data{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    if (data.empty())
        throw std::runtime_error("Cannot read from file: " + filename);

    return BinaryReader(data);
}

uint8_t BinaryReader::read_u8() {
    require(1);

    return data_[pos_++];
}

uint16_t BinaryReader::read_u16_le() {
    require(2);

    uint16_t value =
            static_cast<uint16_t>(data_[pos_]) |
            (static_cast<uint16_t>(data_[pos_ + 1]) << 8);

    pos_ += 2;
    return value;
}

uint32_t BinaryReader::read_u32_le() {
    require(4);

    uint32_t value =
            static_cast<uint32_t>(data_[pos_]) |
            (static_cast<uint32_t>(data_[pos_ + 1]) << 8) |
            (static_cast<uint32_t>(data_[pos_ + 2]) << 16) |
            (static_cast<uint32_t>(data_[pos_ + 3]) << 24);

    pos_ += 4;
    return value;
}

uint64_t BinaryReader::read_u64_le() {
    require(8);

    uint64_t value =
            static_cast<uint64_t>(data_[pos_]) |
            (static_cast<uint64_t>(data_[pos_ + 1]) << 8) |
            (static_cast<uint64_t>(data_[pos_ + 2]) << 16) |
            (static_cast<uint64_t>(data_[pos_ + 3]) << 24) |
            (static_cast<uint64_t>(data_[pos_ + 4]) << 32) |
            (static_cast<uint64_t>(data_[pos_ + 5]) << 40) |
            (static_cast<uint64_t>(data_[pos_ + 6]) << 48) |
            (static_cast<uint64_t>(data_[pos_ + 7]) << 56);

    pos_ += 8;
    return value;
}

std::vector<uint8_t> BinaryReader::read_bytes(const size_t count) {
    require(count);

    std::vector<uint8_t> result(
        data_.begin() + pos_,
        data_.begin() + pos_ + count
    );

    pos_ += count;
    return result;
}

std::u16string BinaryReader::read_string(uint64_t length) {
    std::u16string result;

    // UTF-16 = 2 bytes per code unit.
    if ((length % 2) != 0) {
        throw std::runtime_error("Invalid SISString length");
    }

    const uint64_t charCount = length / 2;

    result.reserve(
        static_cast<std::size_t>(charCount)
    );

    for (uint64_t i = 0; i < charCount; ++i) {
        const uint16_t ch = this->read_u16_le();

        result.push_back(
            static_cast<char16_t>(ch)
        );
    }

    return result;
}

size_t BinaryReader::remaining() const {
    return data_.size() - pos_;
}

/*BinaryReader BinaryReader::to_new_reader(size_t from, size_t length) const {
    return BinaryReader(
        std::vector<uint8_t>(
            data_.begin() + from,
            data_.begin() + from + length
        )
    );
}*/

std::string bytes_to_string(const std::vector<uint8_t> &bytes) {
    std::stringstream ss;

    for (auto byte: bytes) {
        ss << std::hex
                << std::setw(2)
                << std::setfill('0')
                << static_cast<unsigned int>(byte)
                << ' ';
    }
    return ss.str();
}
