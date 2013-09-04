/* vim: set foldmethod=marker */

#ifndef _WSP_H_
#define _WSP_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

struct wsp_error_t;
struct wsp_t;
struct wsp_point_b;
struct wsp_point_t;
struct wsp_archive_info_b;
struct wsp_archive_info_t;
struct wsp_metadata_b;
struct wsp_metadata_t;

typedef enum {
    WSP_MAPPING_NONE = 0,
    WSP_FILE = 1,
    WSP_MMAP = 2
} wsp_mapping_t;

typedef enum {
    WSP_ERROR_NONE = 0,
    WSP_ERROR_NOT_INITIALIZED = 1,
    WSP_ERROR_ALREADY_INITIALIZED = 2,
    WSP_ERROR_IO = 3,
    WSP_ERROR_NOT_OPEN = 4,
    WSP_ERROR_ALREADY_OPEN = 5,
    WSP_ERROR_MALLOC = 6,
    WSP_ERROR_OFFSET = 7,
    WSP_ERROR_SIZE = 8
} wsp_errornum_t;

typedef enum {
    WSP_AVERAGE = 1,
    WSP_SUM = 2,
    WSP_LAST = 3,
    WSP_MAX = 4,
    WSP_MIN = 5
} wsp_aggregation_method_t;

typedef struct wsp_error_t wsp_error_t;
typedef struct wsp_t wsp_t;
typedef struct wsp_point_b wsp_point_b;
typedef struct wsp_point_t wsp_point_t;
typedef struct wsp_archive_info_b wsp_archive_info_b;
typedef struct wsp_archive_info_t wsp_archive_info_t;
typedef struct wsp_metadata_b wsp_metadata_b;
typedef struct wsp_metadata_t wsp_metadata_t;

/* public functions {{{ */
const char *wsp_strerror(wsp_error_t *);

struct wsp_error_t {
    wsp_errornum_t type;
    int syserr;
};
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
    uint32_t archives_count;
};

#define WSP_METADATA_INIT {\
    .aggregation_type = 0,\
    .max_retention = 0,\
    .x_files_factor = 0,\
    .archives_count = 0\
}
/* }}} */

/* wsp_t functions {{{ */
/* read function */
typedef int(*wsp_io_read_f)(
    wsp_t *w,
    long offset,
    size_t size,
    void **buf,
    wsp_error_t *error
);

/*
 * Mapping writer function definition.
 *
 *
 */
typedef int(*wsp_io_write_f)(
    wsp_t *w,
    long offset,
    size_t size,
    void *buf,
    wsp_error_t *error
);

typedef struct {
    wsp_io_read_f read;
    wsp_io_write_f write;
} wsp_io;

struct wsp_t {
    // metadata header
    wsp_metadata_t meta;
    // file descriptor (as returned by fopen)
    FILE *io_fd;
    // mapped memory of file.
    void *io_mmap;
    // size of the file, for later munmap call.
    off_t io_size;
    // specific type of mapping.
    wsp_mapping_t io_mapping;
    // indicates if I/O allocates an internal buffer that needs to be
    // de-allocated after it has been used.
    int io_manual_buf;
    // io functions.
    wsp_io *io;

    // archives
    // these are empty (NULL) until wsp_load_archives has been called.
    wsp_archive_info_t *archives;
    size_t archives_size;
    uint32_t archives_count;
};

#define WSP_INIT(w) do {\
    (w)->io_fd = NULL;\
    (w)->io_mmap = NULL;\
    (w)->io_size = 0;\
    (w)->io_mapping = 0;\
    (w)->io_manual_buf = 0;\
    (w)->io = NULL;\
    (w)->archives = NULL;\
    (w)->archives_size = 0;\
    (w)->archives_count = 0;\
} while(0)

int wsp_open(wsp_t *, const char *path, wsp_mapping_t, wsp_error_t *);
int wsp_close(wsp_t *, wsp_error_t *);

int wsp_read_archive_info(
    wsp_t *,
    int index,
    wsp_archive_info_t *,
    wsp_error_t *
);

int wsp_load_archives(
    wsp_t *,
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
};

/**
 * Read points into buffer for a specific archive.
 */
int wsp_load_points(
    wsp_t *w,
    wsp_archive_info_t *ai,
    wsp_point_t *points,
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
#define WSP_ERROR_INIT {.type = WSP_ERROR_NONE, .syserr = 0}

#define WSP_ERROR -1
#define WSP_OK 0
/* }}} */

#endif /* _WSP_H_ */
