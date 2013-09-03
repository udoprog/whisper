#include "wsp_io_mmap.h"

#include <string.h>

/*
 * Reader function for WSP_MMAP mappings.
 *
 * See wsp_read_f for documentation on arguments.
 */
static int __wsp_io_read__mmap(
    wsp_t *file,
    long offset,
    size_t size,
    void **buf,
    wsp_error_t *e
)
{
    /* the beauty of mmaped files */
    *buf = (char *)file->io_mmap + offset;
    return WSP_OK;
} // __wsp_read__mmap

/*
 * Writer function for WSP_MMAP mappings.
 *
 * See wsp_write_f for documentation on arguments.
 */
static int __wsp_io_write__mmap(
    wsp_t *w,
    long offset,
    size_t size,
    void *buf,
    wsp_error_t *e
)
{
    memcpy((char *)w->io_mmap + offset, buf, size);
    return WSP_OK;
} // __wsp_io_write__mmap

wsp_io wsp_io_mmap = {
    .read = __wsp_io_read__mmap,
    .write = __wsp_io_write__mmap,
};
