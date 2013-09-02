/* vim: set foldmethod=marker */

#ifndef _WSP_H_
#define _WSP_H_

#include <stdio.h>
#include <stdint.h>

struct wsp_ctx_t;
struct wsp_error_t;
struct wsp_file_t;
struct wsp_point_b;
struct wsp_point_t;
struct wsp_archive_info_b;
struct wsp_archive_info_t;
struct wsp_metadata_b;
struct wsp_metadata_t;

typedef struct wsp_ctx_t wsp_ctx_t;
typedef struct wsp_error_t wsp_error_t;
typedef struct wsp_file_t wsp_file_t;
typedef struct wsp_point_b wsp_point_b;
typedef struct wsp_point_t wsp_point_t;
typedef struct wsp_archive_info_b wsp_archive_info_b;
typedef struct wsp_archive_info_t wsp_archive_info_t;
typedef struct wsp_metadata_b wsp_metadata_b;
typedef struct wsp_metadata_t wsp_metadata_t;

/* public functions {{{ */
const char *wsp_errorstr(wsp_error_t *);

struct wsp_ctx_t {
    int _init;
};

struct wsp_error_t {
    int type;
    int sys_errno;
};

int wsp_init(wsp_ctx_t *, wsp_error_t *);
int wsp_close(wsp_ctx_t *, wsp_error_t *);
/* }}} */

/* wsp_file_t functions {{{ */
struct wsp_file_t {
    /* file descriptor pointing to wsp file */
    FILE *fd;
    void *map;
    int mapping_type;
    /* will be set to 1 if this perticular mapping allocates buffers that
     * need to be freed after use */
    int manual_buffer;
};

int wsp_file_open(
    wsp_ctx_t *,
    wsp_file_t *,
    const char *path,
    int mapping_type,
    wsp_error_t *
);

int wsp_file_read_metadata(
    wsp_file_t *,
    wsp_metadata_t *,
    wsp_error_t *
);

int wsp_file_read(
    wsp_file_t *file,
    long offset,
    size_t size,
    void **buf,
    wsp_error_t *error
);

int wsp_file_read_archive_info(
    wsp_file_t *,
    int index,
    wsp_archive_info_t *,
    wsp_error_t *
);
/* }}} */

/* wsp_metadata_t functions {{{ */
struct wsp_metadata_b {
    char aggregation_type[sizeof(uint32_t)];
    char max_retention[sizeof(uint32_t)];
    char x_files_factor[sizeof(float)];
    char archives_count[sizeof(uint32_t)];
};

struct wsp_metadata_t {
    uint32_t aggregation_type;
    uint32_t max_retention;
    float x_files_factor;
    /* extra fields */
    wsp_archive_info_t *archives;
    uint32_t archives_count;
    size_t archives_size;
    wsp_file_t *file;
};

int wsp_metadata_load_archives(
    wsp_metadata_t *,
    wsp_error_t *
);

int wsp_metadata_free(
    wsp_metadata_t *,
    wsp_error_t *
);
/* }}} */

/* wsp_archive_info_t functions {{{ */
struct wsp_archive_info_b {
    char offset[sizeof(uint32_t)];
    char seconds_per_point[sizeof(uint32_t)];
    char points_count[sizeof(uint32_t)];
};

struct wsp_archive_info_t {
    uint32_t offset;
    uint32_t seconds_per_point;
    uint32_t points_count;
    /* extra fields */
    wsp_point_t *points;
    size_t points_size;
    wsp_file_t *file;
};

/**
 * Read points into buffer for a specific archive.
 */
int wsp_archive_info_load_points(
    wsp_archive_info_t *ai,
    wsp_error_t *error
);

/**
 * Free an archive info instance.
 */
int wsp_archive_info_free(
    wsp_archive_info_t *,
    wsp_error_t *
);
/* }}} */

/* wsp_point_t functions {{{ */
struct wsp_point_b {
    char timestamp[sizeof(uint32_t)];
    char value[sizeof(double)];
};

struct wsp_point_t {
    uint32_t timestamp;
    double value;
};
/* }}} */

/* public macros {{{ */
#define WSP_FILE_INIT {\
    .fd = NULL,\
    .map = NULL,\
    .mapping_type = 0,\
    .manual_buffer = 0\
}
#define WSP_CTX_INIT {\
    ._init = 0\
}
#define WSP_METADATA_INIT {\
    .aggregation_type = 0,\
    .max_retention = 0,\
    .x_files_factor = 0,\
    .archives = NULL,\
    .archives_count = 0,\
    .archives_size = 0,\
    .file = NULL\
}
#define WSP_ARCHIVE_INFO_INIT {\
    .offset = 0,\
    .seconds_per_point = 0,\
    .points_count = 0,\
    .points = NULL,\
    .points_size = 0,\
    .file = NULL\
}
#define WSP_ERROR_INIT {.type = WSP_ERROR_NONE, .sys_errno = 0}

#define WSP_ERROR -1
#define WSP_OK 0

#define WSP_ERROR_NONE 0
#define WSP_ERROR_NOT_INITIALIZED 1
#define WSP_ERROR_ALREADY_INITIALIZED 2
#define WSP_ERROR_IO 3
#define WSP_ERROR_NOT_OPEN 4
#define WSP_ERROR_ALREADY_OPEN 5
#define WSP_ERROR_MALLOC 6
#define WSP_ERROR_OFFSET 7
#define WSP_ERROR_SIZE 8

#define WSP_AVERAGE (1l)
#define WSP_SUM (2l)
#define WSP_LAST (3l)
#define WSP_MAX (4l)
#define WSP_MIN (5l)

#define WSP_MAPPING_FILE 1
#define WSP_MAPPING_MAP 2
/* }}} */

#endif /* _WSP_H_ */
