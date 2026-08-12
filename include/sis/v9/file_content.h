//
// Created by goforbroke on 8/12/26.
//

#ifndef LIBSIS_SIS_V9_FILE_CONTENT_H
#define LIBSIS_SIS_V9_FILE_CONTENT_H

#include <cstdint>
#include <vector>
#include "types.h"
#include "zlib_inflate.h"

namespace sis::v9 {
    inline
    std::vector<uint8_t> get_file_content(
        const sis::v9::Contents &contents,
        const sis::v9::Controller &controller,
        const sis::v9::FileDescription &fd
    ) {
        const auto dataUnitIndex = controller.dataIndex.index;

        const auto fileIndex = fd.dataIndex;

        if (dataUnitIndex >= contents.data.units.size())
            throw std::runtime_error("Invalid SISDataIndex");

        const auto &unit = contents.data.units[dataUnitIndex];

        if (fileIndex >= unit.files.size())
            throw std::runtime_error("Invalid FileDescription.fileIndex");

        const auto &compressed =
                unit.files[fileIndex].compressed;

        switch (compressed.algorithm) {
            case sis::v9::COMP_ALG_NONE:
                return compressed.compressedData;

            case sis::v9::COMP_ALG_DEFLATE:
                return zlib_inflate(
                    compressed.compressedData,
                    compressed.uncompressedSize
                );

            default:
                throw std::runtime_error(
                    "Unsupported compression algorithm"
                );
        }
    }
} //sis::v9

#endif //LIBSIS_SIS_V9_FILE_CONTENT_H
