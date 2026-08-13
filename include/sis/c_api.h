//
// Created by goforbroke on 8/10/26.
//

#ifndef LIBSIS_C_API_H
#define LIBSIS_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sis_package sis_package_t;

typedef enum sis_result {
    SIS_OK = 0,
    SIS_ERROR_INVALID_ARGUMENT,
    SIS_ERROR_IO,
    SIS_ERROR_INVALID_FORMAT,
    SIS_ERROR_OUT_OF_RANGE,
    SIS_ERROR_BUFFER_TOO_SMALL,
    SIS_ERROR_UNSUPPORTED,
    SIS_ERROR_INTERNAL
} sis_result_t;


/*
 * Parsing
 */

sis_result_t sis_open_file(
    const char* filepath,
    sis_package_t** out_package
);

sis_result_t sis_open_memory(
    const uint8_t* data,
    size_t size,
    sis_package_t** out_package
);

void sis_close(
    sis_package_t* package
);


/*
 * Package metadata
 */

uint32_t sis_package_uid(
    const sis_package_t* package
);

uint32_t sis_package_version_major(
    const sis_package_t* package
);

uint32_t sis_package_version_minor(
    const sis_package_t* package
);


/*
 * Files
 */

size_t sis_file_count(const sis_package_t* package);






/*
 * File metadata
 */

const char* sis_file_path(
    const sis_package_t* package,
    size_t index
);

uint64_t sis_file_size(
    const sis_package_t* package,
    size_t index
);


/*
 * File content
 */

sis_result_t sis_file_extract(
    const sis_package_t* package,
    size_t index,
    uint8_t* buffer,
    size_t buffer_size,
    size_t* out_size
);


/*
 * Errors
 */

const char* sis_last_error(void);

#ifdef __cplusplus
}
#endif

#endif //LIBSIS_C_API_H
