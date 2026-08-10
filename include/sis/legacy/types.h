//
// Created by goforbroke on 8/9/26.
//

#ifndef LIBSIS_SIS_LEGACY_TYPES_H
#define LIBSIS_SIS_LEGACY_TYPES_H

// https://www.reviversoft.com/file-extensions/sisx
// https://www.thouky.co.uk/software/psifs/sis.html

namespace sis::legacy {
    struct SisField {
        uint32_t type;
        uint32_t length;
        std::vector<uint8_t> data;
    };

    constexpr uint32_t SIS_UID1 = 0x10201A7A;

    struct SisHeader {
        uint32_t uid1;
        uint32_t uid2;
        uint32_t uid3;
        uint32_t uid4;
        uint16_t checksum;
        uint16_t numberOfLanguages;
        uint16_t numberOfFiles;
        uint16_t numberOfRequisites; // ???
        uint16_t installationLanguage;
        uint16_t installationFiles;
        uint16_t installationDrive;
        uint16_t numberOfCapabilities;
        uint32_t installerVersion;
        uint16_t options;
        uint16_t type;
        uint16_t majorVersion;
        uint16_t minorVersion;
        uint32_t variant;
        uint32_t languagesPointer;
        uint32_t filesPointer;
        uint32_t requisitesPointer;
        uint32_t certificatesPointer;
        uint32_t componentNamePointer;

        uint32_t signaturePointer;
        uint32_t capabilitiesPointer;
        uint32_t installedSpace;
        uint32_t maximumInstalledSpace;
        std::vector<uint8_t> reserved; // 16 bytes
    };

    enum class SisType : uint32_t {
        String = 1,
        Array = 2,
        Compressed = 3,

        Version = 4,
        Date = 6,
        Time = 7,
        DateTime = 8,

        Contents = 12,
        Controller = 13,
        Info = 14,

        Prerequisites = 17,
        Dependency = 18,

        InstallBlock = 28,
        Data = 30,
        DataUnit = 31,
        FileData = 32,

        DataIndex = 40,
    };

    /*std::vector<uint8_t> decompress(
        uint32_t algorithm,
        std::span<const uint8_t> input,
        size_t expectedSize)
    {
        if (algorithm == 0) {
            return std::vector<uint8_t>(
                input.begin(),
                input.end()
            );
        }

        if (algorithm == 1) {
            return inflate_zlib(input, expectedSize);
        }

        throw std::runtime_error("Unknown SIS compression");
    }*/

    struct SISFileDescription {
        std::string filename;
        std::vector<uint8_t> content;
    };

    struct SisFile {
        SisHeader header;
        std::vector<SISFileDescription> files;
    };
} //sis::legacy

#endif //LIBSIS_SIS_LEGACY_TYPES_H
