#ifndef _PY_WHISPER_FILE_H_
#define _PY_WHISPER_FILE_H_

#include <Python.h>
#include <structmember.h>

#include <wsp.h>

#include "WhisperException.h"

typedef struct {
    PyObject_HEAD;
    /* Type-specific fields go here. */
    wsp_t *wsp;
    PyObject *archives;
} WhisperFile;

extern PyTypeObject WhisperFile_T;

void init_WhisperFile_T(PyObject *m);

#endif /* _PY_WHISPER_FILE_H_ */
