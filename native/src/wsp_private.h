#ifndef _WSP_PRIVATE_H_
#define _WSP_PRIVATE_H_

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
    wsp_archive_t *ai
);

void __wsp_dump_archive(
    wsp_archive_t *ai,
    wsp_archive_b *buf
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
    wsp_archive_t *ai,
    wsp_error_t *e
);

wsp_return_t __wsp_load_archives(
    wsp_t *w,
    wsp_error_t *e
);

wsp_return_t __wsp_archive_free(
    wsp_archive_t *ai,
    wsp_error_t *e
);

/*
 * Calculate the absolute offset for a specific point in the database.
 */
inline size_t __wsp_point_offset(wsp_archive_t *ai, long index) {
    return ai->offset + sizeof(wsp_point_b) * index;
} // __wsp_point_offset

#endif /* _WSP_PRIVATE_H_ */
