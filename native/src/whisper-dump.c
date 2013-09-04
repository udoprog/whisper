#include "wsp.h"

#include <string.h>
#include <stdlib.h>

int main(int argc, const char **argv) {
    wsp_error_t e = WSP_ERROR_INIT;

    if (argc < 2) {
        printf("Usage: whisper-dump <file>\n");
        return 1;
    }

    const char *p = argv[1];
    wsp_t w;
    WSP_INIT(&w);

    if (wsp_open(&w, p, WSP_MMAP, &e) == WSP_ERROR) {
        printf("%s: %s: %s\n", wsp_strerror(&e), strerror(e.syserr), p);
        return 1;
    }

    uint32_t i, j;
    wsp_archive_info_t *ai;

    printf("Meta data:\n");
    printf("  aggregation_type = %u\n", w.meta.aggregation_type);
    printf("  max_retention = %u\n", w.meta.max_retention);
    printf("  xff = %f\n", w.meta.x_files_factor);
    printf("  archives_count = %u\n", w.meta.archives_count);
    printf("\n");

    wsp_point_t point;

    for (i = 0; i < w.archives_count; i++) {
        ai = w.archives + i;

        printf("Archive #%u info:\n", i);

        printf("  offset = %ul\n", ai->offset);
        printf("  seconds_per_point = %ul\n", ai->seconds_per_point);
        printf("  points = %ul\n", ai->points_count);
        printf("  points_size = %lu\n", ai->points_size);
        printf("\n");

        wsp_point_t points[ai->points_count];

        if (wsp_load_points(&w, ai, points, &e) == WSP_ERROR) {
            printf("%s: %s: %s\n", wsp_strerror(&e), strerror(e.syserr), p);
            return 1;
        }

        printf("Archive #%u data:\n", i);

        for (j = 0; j < ai->points_count; j++) {
            point = points[j];
            printf("%u: %u, %.4f\n", j, point.timestamp, point.value);
        }

        printf("\n");
    }

    wsp_close(&w, &e);

    return 0;
}
