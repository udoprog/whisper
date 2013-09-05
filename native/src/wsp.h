// vim: foldmethod=marker

#ifndef _WSP_H_
#define _WSP_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "wsp_time.h"

struct wsp_error_t;
struct wsp_t;
struct wsp_point_b;
struct wsp_point_t;
struct wsp_archive_b;
struct wsp_archive_t;
struct wsp_metadata_b;
struct wsp_metadata_t;

typedef enum {
    WSP_ERROR = -1,
    WSP_OK = 0
} wsp_return_t;

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
    WSP_ERROR_FUTURE_TIMESTAMP = 8,
    WSP_ERROR_RETENTION = 9,
    WSP_ERROR_ARCHIVE = 10,
    WSP_ERROR_POINT_OOB = 11,
    WSP_ERROR_UNKNOWN_AGGREGATION = 12,
    WSP_ERROR_ARCHIVE_MISALIGNED = 13,
    WSP_ERROR_TIME_INTERVAL = 14,
    WSP_ERROR_SIZE = 15
} wsp_errornum_t;

typedef enum {
    WSP_AVERAGE = 1,
    WSP_SUM = 2,
    WSP_LAST = 3,
    WSP_MAX = 4,
    WSP_MIN = 5
} wsp_aggregation_t;

typedef struct wsp_error_t wsp_error_t;
typedef struct wsp_t wsp_t;
typedef struct wsp_point_b wsp_point_b;
typedef struct wsp_point_t wsp_point_t;
typedef struct wsp_archive_b wsp_archive_b;
typedef struct wsp_archive_t wsp_archive_t;
typedef struct wsp_metadata_b wsp_metadata_b;
typedef struct wsp_metadata_t wsp_metadata_t;

/* public functions {{{ */
const char *wsp_strerror(wsp_error_t *);

struct wsp_error_t {
    wsp_errornum_t type;
    int syserr;
};
/* }}} */

/* wsp_t functions {{{ */
/* read function */
typedef wsp_return_t(*wsp_io_read_f)(
    wsp_t *w,
    long offset,
    size_t size,
    void **buf,
    wsp_error_t *error
);

/*
 * Mapping writer function definition.
 */
typedef wsp_return_t(*wsp_io_write_f)(
    wsp_t *w,
    long offset,
    size_t size,
    void *buf,
    wsp_error_t *error
);

typedef wsp_return_t(*wsp_io_open_f)(
    wsp_t *w,
    const char *path,
    wsp_error_t *error
);

typedef wsp_return_t(*wsp_io_close_f)(
    wsp_t *w,
    wsp_error_t *error
);

typedef wsp_return_t(*wsp_aggregate_f)(
    wsp_t *w,
    wsp_point_t *points,
    uint32_t count,
    double *value,
    int *skip,
    wsp_error_t *e
);

/* wsp_metadata_t functions {{{ */
struct wsp_metadata_b {
    char aggregation[sizeof(uint32_t)];
    char max_retention[sizeof(uint32_t)];
    char x_files_factor[sizeof(float)];
    char archives_count[sizeof(uint32_t)];
};

struct wsp_metadata_t {
    wsp_aggregation_t aggregation;
    uint32_t max_retention;
    float x_files_factor;
    uint32_t archives_count;
    // aggregate function.
    wsp_aggregate_f aggregate;
};

#define WSP_METADATA_INIT(m) do {\
    (m)->aggregation = 0;\
    (m)->max_retention = 0;\
    (m)->x_files_factor = 0;\
    (m)->archives_count = 0;\
    (m)->aggregate = NULL;\
} while(0)
/* }}} */

typedef struct {
    wsp_io_open_f open;
    wsp_io_close_f close;
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
    wsp_archive_t *archives;
    // Size in bytes of the archive block in the database.
    size_t archives_size;
    // Real archive count that has *actually* been loaded.
    // This might differ from metadata if laoding fails.
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

wsp_return_t wsp_open(
    wsp_t *,
    const char *path,
    wsp_mapping_t,
    wsp_error_t *
);

wsp_return_t wsp_close(
    wsp_t *,
    wsp_error_t *
);

wsp_return_t wsp_update(
    wsp_t *w,
    wsp_point_t *p,
    wsp_error_t *e
);

wsp_return_t wsp_update_point(
    wsp_t *w,
    wsp_archive_t *archive,
    wsp_time_t timestamp,
    double value,
    wsp_point_t *base,
    wsp_error_t *e
);
/* }}} */

/* wsp_archive_t functions {{{ */
struct wsp_archive_b {
    char offset[sizeof(uint32_t)];
    char spp[sizeof(uint32_t)];
    char count[sizeof(uint32_t)];
};

struct wsp_archive_t {
    // absolute offset of archive in database.
    uint32_t offset;
    // seconds per point.
    uint32_t spp;
    // the amount of points in database.
    uint32_t count;
    /* extra fields */
    size_t points_size;
    uint64_t retention;
};

/*
 * Load points between two timestamps.
 *
 * w: Whisper Database
 * archive: Whisper archive to load from.
 * time_from: Starting time interval.
 * time_until: Ending time interval.
 * result: Where to store the result, this should have a t least archive->count
 *         space allocated.
 * size: Where to store the length of the resulting points.
 * e: Where to store error information.
 */
wsp_return_t wsp_load_time_points(
    wsp_t *w,
    wsp_archive_t *archive,
    wsp_time_t time_from,
    wsp_time_t time_until,
    wsp_point_t *result,
    uint32_t *size,
    wsp_error_t *e
);

wsp_return_t wsp_load_all_points(
    wsp_t *w,
    wsp_archive_t *archive,
    wsp_point_t *points,
    wsp_error_t *error
);

wsp_return_t wsp_load_point(
    wsp_t *w,
    wsp_archive_t *archive,
    long index,
    wsp_point_t *point,
    wsp_error_t *e
);

/**
 * Read points into buffer for a specific archive.
 * This function takes care for any wrap around in the archive.
 */
wsp_return_t wsp_load_points(
    wsp_t *w,
    wsp_archive_t *archive,
    int offset,
    uint32_t count,
    wsp_point_t *points,
    wsp_error_t *error
);

/**
 * Free an archive info instance.
 */
wsp_return_t wsp_archive_free(
    wsp_archive_t *,
    wsp_error_t *
);
/* }}} */

/* wsp_point_t functions {{{ */
struct wsp_point_b {
    char timestamp[sizeof(uint32_t)];
    char value[sizeof(double)];
};

struct wsp_point_t {
    wsp_time_t timestamp;
    double value;
};

#define WSP_POINT_INIT(p) do { \
    (p)->timestamp = 0; \
    (p)->value = 0; \
} while(0)
/* }}} */

/* public macros {{{ */
#define WSP_ERROR_INIT(e) do {\
    (e)->type = WSP_ERROR_NONE;\
    (e)->syserr = 0;\
} while(0)

#define VALIDATE_ARCHIVE
/* }}} */

#endif /* _WSP_H_ */
