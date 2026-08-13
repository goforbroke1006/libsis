//
// Created by goforbroke on 8/13/26.
//

#include <stdio.h>
#include <stdint.h>
#include <sis/c_api.h>

int main(const int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s /path/to/your_app.sis\n", argv[0]);
        return 1;
    }

    const char* input_sis_filepath = argv[1];

    sis_package_t *pkg = NULL;

    if (sis_open_file(input_sis_filepath, &pkg) != SIS_OK) {
        fprintf(stderr, "%s\n", sis_last_error());
        return 1;
    }

    // indexed API for the reading files

    const size_t files_count = sis_file_count(pkg);

    for (size_t file_idx = 0; file_idx < files_count; ++file_idx) {
        const char *path = sis_file_path(pkg, file_idx);
        const uint64_t size = sis_file_size(pkg, file_idx);

        printf("%s (%llu bytes)\n", path, (unsigned long long) size);
    }

    sis_close(pkg);
}
