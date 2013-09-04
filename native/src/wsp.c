/* vim: set foldmethod=marker */

#include "wsp.h"

#include "wsp_io_file.h"
#include "wsp_io_mmap.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

/* private macros {{{ */
#if BYTE_ORDER == LITTLE_ENDIAN
#define READ4(t, l) do {\
    (t)[0] = (l)[3];\
    (t)[1] = (l)[2];\
    (t)[2] = (l)[1];\
    (t)[3] = (l)[0];\
} while (0);

#define READ8(t, l) do {\
    (t)[0] = (l)[7];\
    (t)[1] = (l)[6];\
    (t)[2] = (l)[5];\
    (t)[3] = (l)[4];\
    (t)[4] = (l)[3];\
    (t)[5] = (l)[2];\
    (t)[6] = (l)[1];\
    (t)[7] = (l)[0];\
} while (0);
#else
#define READ4(t, l) do {\
    (t)[0] = (l)[0];\
    (t)[1] = (l)[1];\
    (t)[2] = (l)[2];\
    (t)[3] = (l)[3];\
} while (0);

#define READ8(t, l) do {\
    (t)[0] = (l)[0];\
    (t)[1] = (l)[1];\
    (t)[2] = (l)[2];\
    (t)[3] = (l)[3];\
    (t)[4] = (l)[4];\
    (t)[5] = (l)[5];\
    (t)[6] = (l)[6];\
    (t)[7] = (l)[7];\
} while (0);
#endif
/* }}} */

/* static initialization {{{ */
const char *wsp_error_strings[WSP_ERROR_SIZE] = {
    /* WSP_ERROR_NONE */
    "No error",
    /* WSP_ERROR_NOT_INITIALIZED */
    "Context not initialized",
    /* WSP_ERROR_ALREADY_INITIALIZED */
    "Context already initialized",
    /* WSP_ERROR_IO */
    "I/O error",
    /* WSP_ERROR_NOT_OPEN */
    "Whisper file not open",
    /* WSP_ERROR_ALREADY_OPEN */
    "Whisper file already open",
    /* WSP_ERROR_MALLOC */
    "Allocation failure",
    /* WSP_ERROR_OFFSET */
    "Invalid offset"
};
/* }}} */

/* private functions {{{ */
static inline void __wsp_parse_point(
    wsp_point_b *buf,
    wsp_point_t *p
)
{
    READ4((char *)&p->timestamp, buf->timestamp);
    READ8((char *)&p->value, buf->value);
} // __wsp_parse_point

static inline void __wsp_dump_point(
    wsp_point_t *p,
    wsp_point_b *buf
)
{
    READ4(buf->timestamp, (char *)&p->timestamp);
    READ8(buf->value, (char *)&p->value);
} // __wsp_dump_point

static inline void __wsp_parse_points(
    wsp_point_b *buf,
    uint32_t points_count,
    wsp_point_t *points
)
{
    uint32_t i;

    for (i = 0; i < points_count; i++) {
        __wsp_parse_point(buf + i, points + i);
    }
} // __wsp_parse_points

static inline void __wsp_dump_points(
    wsp_point_t *points,
    uint32_t points_count,
    wsp_point_b *buf
)
{
    uint32_t i;

    for (i = 0; i < points_count; i++) {
        __wsp_dump_point(points + i, buf + i);
    }
} // __wsp_dump_points

static inline void __wsp_parse_metadata(
    wsp_metadata_b *buf,
    wsp_metadata_t *m
)
{
    READ4((char *)&m->aggregation_type, buf->aggregation_type);
    READ4((char *)&m->max_retention, buf->max_retention);
    READ4((char *)&m->x_files_factor, buf->x_files_factor);
    READ4((char *)&m->archives_count, buf->archives_count);
} // __wsp_parse_metadata

static inline void __wsp_dump_metadata(
    wsp_metadata_t *m,
    wsp_metadata_b *buf
)
{
    READ4(buf->aggregation_type, (char *)&m->aggregation_type);
    READ4(buf->max_retention, (char *)&m->max_retention);
    READ4(buf->x_files_factor, (char *)&m->x_files_factor);
    READ4(buf->archives_count, (char *)&m->archives_count);
} // __wsp_dump_metadata

static inline void __wsp_parse_archive_info(
    wsp_archive_info_b *buf,
    wsp_archive_info_t *ai
)
{
    READ4((char *)&ai->offset, buf->offset);
    READ4((char *)&ai->seconds_per_point, buf->seconds_per_point);
    READ4((char *)&ai->points_count, buf->points_count);
} // __wsp_parse_archive_info

static inline void __wsp_dump_archive_info(
    wsp_archive_info_t *ai,
    wsp_archive_info_b *buf
)
{
    READ4(buf->offset, (char *)&ai->offset);
    READ4(buf->seconds_per_point, (char *)&ai->seconds_per_point);
    READ4(buf->points_count, (char *)&ai->points_count);
} // __wsp_dump_archive_info

/*
 * Setup memory mapping for the specified file.
 *
 * io_fd: The file descriptor to memory map.
 * io_mmap: Pointer to a pointer that will be written to the new memory region.
 * io_size: Pointer to write the file size to.
 * e: Error object.
 */
static int __wsp_setup_mmap(
    FILE *io_fd,
    void **io_mmap,
    off_t *io_size,
    wsp_error_t *e
)
{
    int fn = fileno(io_fd);

    struct stat st;

    if (fstat(fn, &st) == -1) {
        e->type = WSP_ERROR_IO;
        e->syserr = errno;
        return WSP_ERROR;
    }

    void *tmp = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fn, 0);

    if (tmp == MAP_FAILED) {
        fclose(io_fd);
        e->type = WSP_ERROR_IO;
        e->syserr = errno;
        return WSP_ERROR;
    }

    *io_mmap = tmp;
    *io_size = st.st_size;
    return WSP_OK;
} // __wsp_setup_mmap

/*
 * Read metadata from file.
 *
 * w: Whisper file to read metadata from.
 * m: Pointer to data that will be updated with the read metadata.
 * e: Error object.
 */
static int __wsp_read_metadata(
    wsp_t *w,
    wsp_metadata_t *m,
    wsp_error_t *e
)
{
    wsp_metadata_b *buf;

    if (w->io->read(w, 0, sizeof(wsp_metadata_b), (void **)&buf, e) == WSP_ERROR) {
        return WSP_ERROR;
    }

    wsp_metadata_t tmp;

    __wsp_parse_metadata(buf, &tmp);

    if (w->io_manual_buf) {
        free(buf);
    }

    m->aggregation_type = tmp.aggregation_type;
    m->max_retention = tmp.max_retention;
    m->x_files_factor = tmp.x_files_factor;
    m->archives_count = tmp.archives_count;

    return WSP_OK;
} // __wsp_read_metadata

int __wsp_load_archives(
    wsp_t *w,
    wsp_error_t *e
)
{
    wsp_archive_info_t *archives = malloc(w->archives_size);

    if (!archives) {
        e->type = WSP_ERROR_MALLOC;
        return WSP_ERROR;
    }

    uint32_t i;

    for (i = 0; i < w->meta.archives_count; i++) {
        wsp_archive_info_t *ai = archives + i;

        if (wsp_read_archive_info(w, i, ai, e) == WSP_ERROR) {
            free(archives);
            return WSP_ERROR;
        }
    }

    // free the old archives.
    if (w->archives != NULL) {
        free(w->archives);
        w->archives = NULL;
        w->archives_count = 0;
    }

    w->archives = archives;
    w->archives_count = w->meta.archives_count;

    return WSP_OK;
} // __wsp_load_archives

/* }}} */

/* public functions {{{ */
const char *wsp_strerror(wsp_error_t *e)
{
    return wsp_error_strings[e->type];
}
/* }}} */

/* wsp_t functions {{{ */
int wsp_read_archive_info(
    wsp_t *w,
    int index,
    wsp_archive_info_t *ai,
    wsp_error_t *e
)
{
    wsp_archive_info_b *buf;

    size_t offset = sizeof(wsp_metadata_b) + sizeof(wsp_archive_info_b) * index;

    if (w->io->read(w, offset, sizeof(wsp_archive_info_b), (void **)&buf, e) == WSP_ERROR) {
        return WSP_ERROR;
    }

    __wsp_parse_archive_info(buf, ai);

    ai->points = NULL;
    ai->points_size = sizeof(wsp_point_t) * ai->points_count;

    if (w->io_manual_buf) {
        free(buf);
    }

    return WSP_OK;
}

int wsp_open(wsp_t *w, const char *path, wsp_mapping_t mapping, wsp_error_t *e)
{
    if (w->io_fd != NULL || w->io_mmap != NULL) {
        e->type = WSP_ERROR_ALREADY_OPEN;
        return WSP_ERROR;
    }

    FILE *io_fd = fopen(path, "r+");

    if (!io_fd) {
        e->type = WSP_ERROR_IO;
        e->syserr = errno;
        return WSP_ERROR;
    }

    if (mapping == WSP_MMAP) {
        void *io_mmap;
        off_t io_size;

        if (__wsp_setup_mmap(io_fd, &io_mmap, &io_size, e) == WSP_ERROR) {
            return WSP_ERROR;
        }

        w->io_fd = io_fd;
        w->io_mmap = io_mmap;
        w->io_size = io_size;
        w->io_mapping = WSP_MMAP;
        w->io_manual_buf = 0;
        w->io = &wsp_io_mmap;
    }
    else if (mapping == WSP_FILE) {
        w->io_fd = io_fd;
        w->io_mmap = NULL;
        w->io_size = 0;
        w->io_mapping = WSP_FILE;
        w->io_manual_buf = 1;
        w->io = &wsp_io_file;
    }
    else {
        e->type = WSP_ERROR_IO;
        return WSP_ERROR;
    }

    wsp_metadata_t meta = WSP_METADATA_INIT;

    if (__wsp_read_metadata(w, &meta, e) == WSP_ERROR) {
        return WSP_ERROR;
    }

    w->meta = meta;

    w->archives = NULL;
    w->archives_size = sizeof(wsp_archive_info_t) * meta.archives_count;
    w->archives_count = 0;

    if (__wsp_load_archives(w, e) == WSP_ERROR) {
        return WSP_ERROR;
    }

    return WSP_OK;
} // wsp_open

int wsp_close(wsp_t *w, wsp_error_t *e)
{
    if (w->archives != NULL) {
        uint32_t i;

        wsp_metadata_t m = w->meta;

        for (i = 0; i < m.archives_count; i++) {
            wsp_archive_info_t *ai = w->archives + i;

            if (wsp_archive_info_free(ai, e) == WSP_ERROR) {
                return WSP_ERROR;
            }
        }

        free(w->archives);
        w->archives = NULL;
    }

    if (w->io_fd != NULL) {
        fclose(w->io_fd);
        w->io_fd = NULL;
    }

    if (w->io_mmap != NULL) {
        munmap(w->io_mmap, w->io_size);
        w->io_mmap = NULL;
    }

    w->archives = NULL;
    w->archives_size = 0;
    w->meta.aggregation_type = 0l;
    w->meta.max_retention = 0l;
    w->meta.x_files_factor = 0.0f;
    w->meta.archives_count = 0l;

    return WSP_OK;
} // wsp_close
/* }}} */

/* wsp_archive_info_t functions {{{ */
int wsp_load_points(
    wsp_t *w,
    wsp_archive_info_t *ai,
    wsp_point_t *points,
    wsp_error_t *e
)
{
    if (ai->points_count == 0 || ai->points != NULL) {
        return WSP_OK;
    }

    wsp_point_b *buf = NULL;

    size_t read_size = sizeof(wsp_point_b) * ai->points_count;

    if (w->io->read(w, ai->offset, read_size, (void **)&buf, e) == WSP_ERROR) {
        return WSP_ERROR;
    }

    __wsp_parse_points(buf, ai->points_count, points);

    if (w->io_manual_buf) {
        free(buf);
    }

    return WSP_OK;
} // wsp_archive_info_load_points

int wsp_archive_info_free(
    wsp_archive_info_t *ai,
    wsp_error_t *e
)
{
    if (ai->points != NULL) {
        free(ai->points);
        ai->points = NULL;
    }

    ai->offset = 0;
    ai->seconds_per_point = 0;
    ai->points_count = 0;
    ai->points_size = 0;

    return WSP_OK;
} // wsp_archive_info_free
/* }}} */

/* wsp_point_t function {{{ */
/* }}} */
