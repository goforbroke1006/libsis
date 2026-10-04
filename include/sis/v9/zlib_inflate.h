//
// Created by goforbroke on 8/12/26.
//

#ifndef LIBSIS_SIS_V9_ZLIB_INFLATE_H
#define LIBSIS_SIS_V9_ZLIB_INFLATE_H

#include <vector>
#include <cstdint>
#include <zlib.h>
#include <iostream>

inline
std::vector<uint8_t>
inflate_impl(const std::vector<uint8_t> &input, uint64_t uncompressedSize, int windowBits) {
    if (uncompressedSize > SIZE_MAX)
        throw std::runtime_error("Output too large");

    if (input.size() > UINT_MAX)
        throw std::runtime_error("Input too large");

    if (uncompressedSize > UINT_MAX)
        throw std::runtime_error(
            "Output too large for this one-shot inflater"
        );

    std::vector<uint8_t> output(
        static_cast<size_t>(uncompressedSize)
    );

    z_stream stream{};

    stream.next_in =
            const_cast<Bytef *>(
                reinterpret_cast<const Bytef *>(input.data())
            );

    stream.avail_in =
            static_cast<uInt>(input.size());

    stream.next_out =
            reinterpret_cast<Bytef *>(output.data());

    stream.avail_out =
            static_cast<uInt>(output.size());

    int rc = inflateInit2(&stream, windowBits);

    if (rc != Z_OK)
        throw std::runtime_error("inflateInit2 failed");

    rc = ::inflate(&stream, Z_FINISH);

    const auto totalIn = stream.total_in;
    const auto totalOut = stream.total_out;

    std::string msg =
            stream.msg ? stream.msg : "";

    inflateEnd(&stream);

    if (rc != Z_STREAM_END) {
        throw std::runtime_error(
            "inflate failed: rc=" + std::to_string(rc) +
            ", total_in=" + std::to_string(totalIn) +
            ", total_out=" + std::to_string(totalOut) +
            ", msg=" + msg
        );
    }

    if (totalOut != uncompressedSize) {
        throw std::runtime_error(
            "SISX decompressed size mismatch: expected=" +
            std::to_string(uncompressedSize) +
            ", actual=" +
            std::to_string(totalOut)
        );
    }

    return output;
}

inline
std::vector<uint8_t>
zlib_inflate(const std::vector<uint8_t> &input, uint64_t uncompressedSize) {
    try {
        // Raw RFC 1951 DEFLATE
        return inflate_impl(
            input,
            uncompressedSize,
            -MAX_WBITS
        );
    } catch (const std::exception &rawError) {
        std::cerr
                << "raw DEFLATE failed: "
                << rawError.what()
                << '\n';

        // Diagnostic fallback: RFC 1950 zlib wrapper
        return inflate_impl(
            input,
            uncompressedSize,
            MAX_WBITS
        );
    }
}


#endif //LIBSIS_SIS_V9_ZLIB_INFLATE_H
