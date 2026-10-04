# libsis

Library for work with [Symbian Installation Source](https://en.wikipedia.org/wiki/Symbian#Software_Installation_Script)
files *.sis *.sisx

## Build status

| Platform       | Status                                                                                                 |
|----------------|--------------------------------------------------------------------------------------------------------|
| Ubuntu / GCC   | ![Ubuntu GCC](https://github.com/goforbroke1006/libsis/actions/workflows/ubuntu-gcc.yml/badge.svg)     |
| Ubuntu / Clang | ![Ubuntu Clang](https://github.com/goforbroke1006/libsis/actions/workflows/ubuntu-clang.yml/badge.svg) |
| Windows / MSVC | ![Windows MSVC](https://github.com/goforbroke1006/libsis/actions/workflows/windows-msvc.yml/badge.svg) |

## Installation

### As Cmake subdirectory

```shell
git modules init
git modules add https://github.com/goforbroke1006/libsis.git ./third_party/libsis
```

```cmake
add_subdirectory(./third_party/libsis)

# ...

target_link_libraries(${PROJECT_NAME} PRIVATE sis)
```

### To the system

```shell
cmake -S . -B build-release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local

cmake --build build-release -j$(nproc)

sudo cmake --install build-release

# Обновить кэш динамических библиотек
sudo ldconfig

# ...

# sudo cmake --build build-release --target uninstall
```

## Usage

```c++
#include <sis/BinaryReader.h>
#include <sis/legacy/Parser.h>
#include <sis/v9/all.h>

enum class FileFormat {
    Unknown,
    SISLegacy, // Symbian v6, v7, v8
    SISv9, // Symbian v9+
    ZIP,
};

FileFormat
detect_format(const std::string &filename) {
    // TODO: read 4 bytes
    // TODO: convert to UIN32 in little indian
    
    constexpr uint32_t ZIP_LOCAL_FILE_HEADER = 0x04034B50;
    constexpr uint32_t SIS_LEGACY_HEADER = 0x10000419;
    constexpr uint32_t SIS_V9_HEADER = 0x10201A7A;
    
    // TODO: compare with consts
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <archive_file>" << std::endl;
        return 1;
    }
    const std::string archiveFilepath = argv[1];
    
    const auto fileFormat = detect_format(archiveFilepath);
    
    if (fileFormat == FileFormat::ZIP) {
        std::cerr << "ZIP format is not supported" << std::endl;
        return 1;
    }
    if (fileFormat == FileFormat::Unknown) {
        std::cerr << "Unknown file format" << std::endl;
        return 1;
    }
    
    auto reader = sis::BinaryReader::from_file(archiveFilepath);
    std::cout << "  Read size: " << reader.size() << " bytes" << std::endl;
    
    if (fileFormat == FileFormat::SISLegacy) {
        std::cout << "  Format: SIS (legacy)" << std::endl;

        sis::legacy::Parser parser(reader);
        const auto sisFile = parser.parse();

        std::cout << "  Header" << std::endl;
        std::cout << std::hex;
        std::cout << "    UID1: " << "0x" << std::setw(8) << std::setfill('0') << sisFile.header.uid1 << std::endl;
        std::cout << "    UID2: " << "0x" << std::setw(8) << std::setfill('0') << sisFile.header.uid2 << std::endl;
        std::cout << "    UID3: " << "0x" << std::setw(8) << std::setfill('0') << sisFile.header.uid3 << std::endl;
        std::cout << "    UID4: " << "0x" << std::setw(8) << std::setfill('0') << sisFile.header.uid4 << std::endl;
        std::cout << std::dec;
        std::cout << "    Checksum:           " << sisFile.header.checksum << std::endl;
        std::cout << "    Num. of files:      " << sisFile.header.numberOfFiles << std::endl;
        std::cout << "    Num. of languages:  " << sisFile.header.numberOfLanguages << std::endl;
        std::cout << "    Installation files: " << sisFile.header.installationFiles << std::endl;
        std::cout << "    Files pointer:      "
                << "0x" << std::setw(8) << std::setfill('0') << sisFile.header.filesPointer << std::dec << std::endl;
        std::cout << "    Version:            " << sisFile.header.majorVersion << "." << sisFile.header.minorVersion <<
                std::endl;
        // TODO:
    }
    if (fileFormat == FileFormat::SISv9) {
        std::cout << "  Format: SIS (v9)" << std::endl;

        const auto fileObj = sis::v9::Parser::parse(reader);

        std::cout << "  Header" << std::endl;
        std::cout << std::hex;
        std::cout << "    UID1: " << fileObj.header.uid1 << std::endl;
        std::cout << "    UID2: " << fileObj.header.uid2 << std::endl;
        std::cout << "    UID3: " << fileObj.header.uid3 << std::endl;
        std::cout << "    UID4: " << fileObj.header.uid4 << std::endl;
        std::cout << std::dec;
        std::cout << "  Content:" << std::endl;
        std::cout << "    Ctrl Checksum:   " << fileObj.contents.controllerChecksum << std::endl;
        std::cout << "    Data Checksum:   " << fileObj.contents.dataChecksum << std::endl;
        std::cout << "    Vendor ID:       " << fileObj.contents.controller.info.uid << std::endl;
        std::cout << "    Vendor name:     " << fileObj.contents.controller.info.vendorUniqueName << std::endl;
        std::cout << "    Version:         " << fileObj.contents.controller.info.version << std::endl;
        std::cout << "    Created at:      " << fileObj.contents.controller.info.creationTime << std::endl;
        // std::cout << "    Cmp algorithm:   " << fileObj.contents.compressed.algorithm << std::endl;
        std::cout << "    Installations:   " << fileObj.contents.controller.installBlock.files.size() << std::endl;
        for (const auto &fileDescr: fileObj.contents.controller.installBlock.files) {
            std::cout << "      Installation:    "
                    << fileDescr.target
                    << " "
                    << fileDescr.mimeType
                    << std::endl;
        }
        const size_t embCtrlLen = fileObj.contents.controller.installBlock.embeddedControllers.size();
        std::cout << "    Emb controllers: " << embCtrlLen << std::endl;
        for (size_t ctrlIdx = 0; ctrlIdx < embCtrlLen; ++ctrlIdx) {
            const auto &embCtrl = fileObj.contents.controller.installBlock.embeddedControllers[ctrlIdx];

            std::cout << "      Emb controller:   # " << (ctrlIdx + 1) << std::endl;

            for (const auto &fileDescr: embCtrl.installBlock.files) {
                std::cout << "      Installation:   "
                        << fileDescr.target
                        << " "
                        << fileDescr.mimeType
                        << std::endl;
            }
        }

        const size_t ifBlocksLen = fileObj.contents.controller.installBlock.ifBlocks.size();
        std::cout << "    IF blocks:     " << ifBlocksLen << std::endl;
        for (size_t ifBlIdx = 0; ifBlIdx < ifBlocksLen; ++ifBlIdx) {
            const auto &ifBlock = fileObj.contents.controller.installBlock.ifBlocks[ifBlIdx];

            std::cout << "      IF block:   # " << (ifBlIdx + 1) << std::endl;
            std::cout << "        Exp:        " << ifBlock.expression << std::endl;

            if (nullptr != ifBlock.installBlock)
                for (const auto &fileDescr: ifBlock.installBlock->files) {
                    auto normalizedFilepath = normalize_filepath(fileDescr.target.str);
                    std::cout << "        File:     "
                            << fileDescr.target
                            << " (MIME: \"" << fileDescr.mimeType << "\")"
                            << std::endl;
                }
        }
    }
}
```

## Docs

https://www.cryer.co.uk/file-types/s/sis/softwareinstallsis.pdf
