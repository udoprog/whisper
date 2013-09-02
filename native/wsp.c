/* vim: set foldmethod=marker */

#include "wsp.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
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
}

static inline void __wsp_dump_point(
    wsp_point_t *p,
    wsp_point_b *buf
)
{
    READ4(buf->timestamp, (char *)&p->timestamp);
    READ8(buf->value, (char *)&p->value);
}

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
}

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
}

static inline void __wsp_parse_metadata(
    wsp_metadata_b *buf,
    wsp_metadata_t *m
)
{
    READ4((char *)&m->aggregation_type, buf->aggregation_type);
    READ4((char *)&m->max_retention, buf->max_retention);
    READ4((char *)&m->x_files_factor, buf->x_files_factor);
    READ4((char *)&m->archives_count, buf->archives_count);
}

static inline void __wsp_dump_metadata(
    wsp_metadata_t *m,
    wsp_metadata_b *buf
)
{
    READ4(buf->aggregation_type, (char *)&m->aggregation_type);
    READ4(buf->max_retention, (char *)&m->max_retention);
    READ4(buf->x_files_factor, (char *)&m->x_files_factor);
    READ4(buf->archives_count, (char *)&m->archives_count);
}

static inline void __wsp_parse_archive_info(
    wsp_archive_info_b *buf,
    wsp_archive_info_t *ai
)
{
    READ4((char *)&ai->offset, buf->offset);
    READ4((char *)&ai->seconds_per_point, buf->seconds_per_point);
    READ4((char *)&ai->points_count, buf->points_count);
}

static inline void __wsp_dump_archive_info(
    wsp_archive_info_t *ai,
    wsp_archive_info_b *buf
)
{
    READ4(buf->offset, (char *)&ai->offset);
    READ4(buf->seconds_per_point, (char *)&ai->seconds_per_point);
    READ4(buf->points_count, (char *)&ai->points_count);
}

static inline int __wsp_file_setup_mmap(
    FILE *fd,
    void **map,
    wsp_error_t *error
)
{
    int fn = fileno(fd);

    struct stat st;

    if (fstat(fn, &st) == -1) {
        error->type = WSP_ERROR_IO;
        error->err = errno;
        return WSP_ERROR;
    }

    void *tmp = mmap(NULL, st.st_size, PROT_READ | PROT_READ, MAP_SHARED, fn, 0);

    if (tmp == MAP_FAILED) {
        fclose(fd);
        error->type = WSP_ERROR_IO;
        error->err = errno;
        return WSP_ERROR;
    }

    *map = tmp;
    return WSP_OK;
}
/* }}} */

/* public functions {{{ */
const char *wsp_strerror(wsp_error_t *error)
{
    return wsp_error_strings[error->type];
}
/* }}} */

/* wsp_file_t functions {{{ */
int wsp_file_read_metadata(
    wsp_file_t *file,
    wsp_metadata_t *m,
    wsp_error_t *error
)
{
    wsp_metadata_b *buf;

    if (wsp_file_read(file, 0, sizeof(wsp_metadata_b), (void **)&buf, error) == WSP_ERROR) {
        return WSP_ERROR;
    }

    wsp_metadata_t tmp;

    __wsp_parse_metadata(buf, &tmp);

    if (file->manual_buffer) {
        free(buf);
    }

    m->aggregation_type = tmp.aggregation_type;
    m->max_retention = tmp.max_retention;
    m->x_files_factor = tmp.x_files_factor;
    m->archives = NULL;
    m->archives_count = tmp.archives_count;
    m->archives_size = sizeof(wsp_archive_info_t) * tmp.archives_count;
    m->file = file;

    return WSP_OK;
}

int wsp_file_read_archive_info(
    wsp_file_t *file,
    int index,
    wsp_archive_info_t *ai,
    wsp_error_t *error
)
{
    wsp_archive_info_b *buf;

    size_t offset = sizeof(wsp_metadata_b) + sizeof(wsp_archive_info_b) * index;

    if (wsp_file_read(file, offset, sizeof(wsp_archive_info_b), (void **)&buf, error) == WSP_ERROR) {
        return WSP_ERROR;
    }

    __wsp_parse_archive_info(buf, ai);

    if (file->manual_buffer) {
        free(buf);
    }

    ai->points = NULL;
    ai->file = file;
    ai->points_size = sizeof(wsp_point_t) * ai->points_count;

    return WSP_OK;
}

int wsp_file_read(
    wsp_file_t *file,
    long offset,
    size_t size,
    void **buf,
    wsp_error_t *error
)
{
    if (file->mapping_type == WSP_MAPPING_FILE) {
        void *tmp = malloc(size);

        if (tmp == NULL) {
            error->type = WSP_ERROR_MALLOC;
            error->err = errno;
            return WSP_ERROR;
        }

        if (ftell(file->fd) != offset) {
            free(tmp);
            error->type = WSP_ERROR_OFFSET;
            error->err = errno;
            return WSP_ERROR;
        }

        if (fread(tmp, size, 1, file->fd) != 1) {
            free(tmp);
            error->type = WSP_ERROR_IO;
            error->err = errno;
            return WSP_ERROR;
        }

        *buf = tmp;
    }
    else {
        /* the beauty of mmap:ed files */
        *buf = file->map + offset;
    }

    return WSP_OK;
}

int wsp_file_open(
    wsp_file_t *file,
    const char *path,
    int mapping_type,
    wsp_error_t *error
)
{
    if (file->fd != NULL || file->map != NULL) {
        error->type = WSP_ERROR_ALREADY_OPEN;
        return WSP_ERROR;
    }

    FILE *fd = fopen(path, "r+");

    if (!fd) {
        error->type = WSP_ERROR_IO;
        error->err = errno;
        return WSP_ERROR;
    }

    if (mapping_type == WSP_MAPPING_MMAP) {
        void *map;

        if (__wsp_file_setup_mmap(fd, &map, error) == WSP_ERROR) {
            return WSP_ERROR;
        }

        file->fd = fd;
        file->map = map;
        file->mapping_type = WSP_MAPPING_MMAP;
        file->manual_buffer = 0;
    }
    else {
        file->fd = fd;
        file->mapping_type = WSP_MAPPING_FILE;
        file->manual_buffer = 1;
    }

    return WSP_OK;
}
/* }}} */

/* wsp_metadata_t functions {{{ */
int wsp_metadata_load_archives(
    wsp_metadata_t *m,
    wsp_error_t *error
)
{
    if (m->archives != NULL) {
        return WSP_OK;
    }

    wsp_archive_info_t *archives = malloc(m->archives_size);

    if (!archives) {
        error->type = WSP_ERROR_MALLOC;
        return WSP_ERROR;
    }

    uint32_t i;

    for (i = 0; i < m->archives_count; i++) {
        wsp_archive_info_t *ai = archives + i;

        if (wsp_file_read_archive_info(m->file, i, ai, error) == WSP_ERROR) {
            free(archives);
            return WSP_ERROR;
        }
    }

    m->archives = archives;

    return WSP_OK;
}

int wsp_metadata_free(
    wsp_metadata_t *m,
    wsp_error_t *error
)
{
    if (m->archives != NULL) {
        uint32_t i;

        for (i = 0; i < m->archives_count; i++) {
            wsp_archive_info_t *ai = m->archives + i;

            if (wsp_archive_info_free(ai, error) == WSP_ERROR) {
                return WSP_ERROR;
            }
        }

        free(m->archives);
        m->archives = NULL;
    }

    m->aggregation_type = 0l;
    m->max_retention = 0l;
    m->x_files_factor = 0.0f;
    m->archives = NULL;
    m->archives_count = 0l;
    m->archives_size = 0;
    m->file = NULL;

    return WSP_OK;
}
/* }}} */

/* wsp_archive_info_t functions {{{ */
int wsp_archive_info_load_points(
    wsp_archive_info_t *ai,
    wsp_error_t *error
)
{
    if (ai->points_count == 0 || ai->points != NULL) {
        return WSP_OK;
    }

    wsp_point_t *points = malloc(ai->points_size);

    if (points == NULL) {
        error->type = WSP_ERROR_MALLOC;
        error->err = errno;
        return WSP_ERROR;
    }

    wsp_point_b *buf = NULL;

    size_t read_size = sizeof(wsp_point_b) * ai->points_count;

    if (wsp_file_read(ai->file, ai->offset, read_size, (void **)&buf, error) == WSP_ERROR) {
        return WSP_ERROR;
    }

    __wsp_parse_points(buf, ai->points_count, points);

    if (ai->file->manual_buffer) {
        free(buf);
    }

    ai->points = points;

    return WSP_OK;
}

int wsp_archive_info_free(
    wsp_archive_info_t *ai,
    wsp_error_t *error
)
{
    if (ai->points != NULL) {
        free(ai->points);
    }

    ai->offset = 0;
    ai->seconds_per_point = 0;
    ai->points_count = 0;
    ai->points = NULL;
    ai->points_size = 0;
    ai->file = NULL;

    return WSP_OK;
}
/* }}} */

/* wsp_point_t function {{{ */
/* }}} */
