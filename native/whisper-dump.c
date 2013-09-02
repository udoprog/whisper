#include "wsp.h"

#include <string.h>
#include <stdlib.h>

int main(int argc, const char **argv) {
    wsp_ctx_t ctx = WSP_CTX_INIT;
    wsp_error_t error = WSP_ERROR_INIT;

    if (wsp_init(&ctx, &error) == WSP_ERROR) {
        printf("Error: %s\n", wsp_errorstr(&error));
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: wsp-dump <file>\n");
        return 1;
    }

    const char *filename = argv[1];
    wsp_file_t file = WSP_FILE_INIT;

    if (wsp_file_open(&ctx, &file, filename, WSP_MAPPING_MAP, &error) == WSP_ERROR) {
        fprintf(
            stderr, "%s: %s: %s\n",
            wsp_errorstr(&error),
            filename,
            strerror(error.sys_errno)
        );
        return 1;
    }

    wsp_metadata_t metadata = WSP_METADATA_INIT;

    if (wsp_file_read_metadata(&file, &metadata, &error) == WSP_ERROR) {
        fprintf(stderr, "%s: %s: %s\n", wsp_errorstr(&error), filename, strerror(error.sys_errno));
        return 1;
    }

    if (wsp_metadata_load_archives(&metadata, &error) == WSP_ERROR) {
        fprintf(stderr, "%s: %s: %s\n", wsp_errorstr(&error), filename, strerror(error.sys_errno));
        return 1;
    }

    uint32_t i, j;
    wsp_archive_info_t *ai;

    for (i = 0; i < metadata.archives_count; i++) {
        ai = metadata.archives + i;

        if (wsp_archive_info_load_points(ai, &error) == WSP_ERROR) {
            fprintf(stderr, "%s: %s: %s\n", wsp_errorstr(&error), filename, strerror(error.sys_errno));
            return 1;
        }
    }

    printf("aggregation_type = %ul\n", metadata.aggregation_type);
    printf("max_retention = %ul\n", metadata.max_retention);
    printf("xff = %f\n", metadata.x_files_factor);
    printf("archives_count = %ul\n", metadata.archives_count);

    wsp_point_t point;

    for (i = 0; i < metadata.archives_count; i++) {
        ai = metadata.archives + i;

        printf("archive #%ul\n", i);

        printf("  offset = %ul\n", ai->offset);
        printf("  seconds_per_point = %ul\n", ai->seconds_per_point);
        printf("  points = %ul\n", ai->points_count);
        printf("  points_size = %lu\n", ai->points_size);

        for (j = 0; j < ai->points_count; j++) {
            point = ai->points[j];
            printf("  %u = %u %f\n", j, point.timestamp, point.value);
        }
    }

    if (wsp_metadata_free(&metadata, &error) == WSP_ERROR) {
        fprintf(stderr, "%s: %s: %s\n", wsp_errorstr(&error), filename, strerror(error.sys_errno));
        return 1;
    }

    wsp_close(&ctx, &error);
    return 0;
}
