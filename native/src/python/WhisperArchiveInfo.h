#ifndef _PY_WHISPER_ARCHIVE_INFO_H_
#define _PY_WHISPER_ARCHIVE_INFO_H_

#include <Python.h>
#include <structmember.h>

#include <wsp.h>

#include "WhisperException.h"

#include "WhisperFile.h"

typedef struct {
    PyObject_HEAD;
    /* Type-specific fields go here. */
    WhisperFile *whisper_file;
    wsp_archive_info_t *archive_info;
    uint32_t seconds_per_point;
    uint32_t points_count;
} WhisperArchiveInfo;

extern PyTypeObject WhisperArchiveInfo_T;

void init_WhisperArchiveInfo_T(PyObject *m);

#endif /* _PY_WHISPER_ARCHIVE_INFO_H_ */
