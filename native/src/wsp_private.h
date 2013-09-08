// vim: foldmethod=marker
/**
 * Private functions not intended for external use.
 */
#ifndef _WSP_PRIVATE_H_
#define _WSP_PRIVATE_H_

// parse & dump functions {{{
void __wsp_parse_point(
    wsp_point_b *buf,
    wsp_point_t *p
);

void __wsp_dump_point(
    wsp_point_t *p,
    wsp_point_b *buf
);

void __wsp_parse_points(
    wsp_point_b *buf,
    uint32_t points_count,
    wsp_point_t *points
);

void __wsp_dump_points(
    wsp_point_t *points,
    uint32_t points_count,
    wsp_point_b *buf
);

void __wsp_parse_metadata(
    wsp_metadata_b *buf,
    wsp_metadata_t *m
);

void __wsp_dump_metadata(
    wsp_metadata_t *m,
    wsp_metadata_b *buf
);

void __wsp_parse_archive(
    wsp_archive_b *buf,
    wsp_archive_t *archive
);

void __wsp_dump_archive(
    wsp_archive_t *archive,
    wsp_archive_b *buf
);
// parse & dump functions }}}

/*
 * Setup memory mapping for the specified file.
 *
 * io_fd: The file descriptor to memory map.
 * io_mmap: Pointer to a pointer that will be written to the new memory region.
 * io_size: Pointer to write the file size to.
 * e: Error object.
 */
wsp_return_t __wsp_setup_mmap(
    FILE *io_fd,
    void **io_mmap,
    off_t *io_size,
    wsp_error_t *e
);

/*
 * Read metadata from file.
 *
 * w: Whisper file to read metadata from.
 * m: Pointer to data that will be updated with the read metadata.
 * e: Error object.
 */
wsp_return_t __wsp_read_metadata(
    wsp_t *w,
    wsp_metadata_t *m,
    wsp_error_t *e
);

wsp_return_t __wsp_read_archive(
    wsp_t *w,
    int index,
    wsp_archive_t *archive,
    wsp_error_t *e
);

wsp_return_t __wsp_valid_archive(
    wsp_archive_t *prev,
    wsp_archive_t *cur,
    wsp_error_t *e
);

wsp_return_t __wsp_load_archives(
    wsp_t *w,
    wsp_error_t *e
);

wsp_return_t __wsp_archive_free(
    wsp_archive_t *archive,
    wsp_error_t *e
);

wsp_return_t __wsp_find_highest_precision(
    wsp_time_t diff,
    wsp_t *w,
    wsp_archive_t **low,
    uint32_t *low_size,
    wsp_error_t *e
);

int __wsp_load_points(
    wsp_t *w,
    wsp_archive_t *archive,
    uint32_t offset,
    uint32_t size,
    wsp_point_t *result,
    wsp_error_t *e
);

#endif /* _WSP_PRIVATE_H_ */
