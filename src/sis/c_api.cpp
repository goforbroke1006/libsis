//
// Created by goforbroke on 8/13/26.
//

#include "../../include/sis/c_api.h"
#include "../../include/sis/v9/all.h"
#include <string>

//

struct sis_package {
    std::unique_ptr<sis::v9::File> file;

    std::vector<sis::v9::FileDescription> files;
};

//

static thread_local std::string g_last_error;

static void set_last_error(std::string message) {
    g_last_error = std::move(message);
}

extern "C"
const char *sis_last_error(void) {
    return g_last_error.c_str();
}

//

extern "C"
sis_result_t sis_open_file(
    const char *filepath,
    sis_package_t **out_package
) {
    try {
        auto reader = sis::BinaryReader::from_file(std::string(filepath));

        auto package = std::make_unique<sis_package>();
        package->file = std::make_unique<sis::v9::File>(
            sis::v9::Parser::parse(reader) // create SISFile
        );


        const auto cnt = std::move(package->file->contents);

        for (const auto &fileDescr: cnt.controller.installBlock.files) {
            package->files.push_back(fileDescr);
        }

        const size_t ifBlocksLen = cnt.controller.installBlock.ifBlocks.size();
        for (size_t ifBlIdx = 0; ifBlIdx < ifBlocksLen; ++ifBlIdx) {
            const auto &ifBlock = cnt.controller.installBlock.ifBlocks[ifBlIdx];

            if (nullptr != ifBlock.installBlock)
                for (const auto &fileDescr: ifBlock.installBlock->files) {
                    package->files.push_back(fileDescr);
                }
        }

        *out_package = package.release();

        return SIS_OK;
    } catch (const std::exception &e) {
        set_last_error(e.what());
        return SIS_ERROR_INTERNAL;
    } catch (...) {
        set_last_error("Unknown C++ exception");
        return SIS_ERROR_INTERNAL;
    }
}

size_t sis_file_count(const sis_package_t *package) {
    return package->files.size();
}

const char *sis_file_path(const sis_package_t *package, size_t index) {
    return package->files[index].target.str.c_str();
}

uint64_t sis_file_size(const sis_package_t *package, size_t index) {
    return package->files[index].uncompressedLength;
}

sis_result_t sis_file_extract(
    const sis_package_t *package,
    size_t index,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *out_size
) {
    // TODO: ???

    return SIS_OK;
}

void sis_close(sis_package_t *package) {
    // TODO: ???
}
