#include "WhisperArchiveInfo.h"

#include <wsp.h>

typedef WhisperArchiveInfo C;

static PyObject* _points(C *self) {
    printf("Points\n");
    PyObject *result = PyList_New(0);

    if (result == NULL) {
        return NULL;
    }

    wsp_error_t e;
    wsp_point_t points[self->archive_info->points_count];

    if (wsp_load_points(self->whisper_file->wsp, self->archive_info, points, &e) == WSP_ERROR) {
        PyErr_Whisper(&e);
        return NULL;
    }

    uint32_t i;
    wsp_point_t point;

    for (i = 0; i < self->archive_info->points_count; i++) {
        point = points[i];

        PyObject *tuple = PyTuple_New(2);

        if (tuple == NULL) {
            return NULL;
        }

        PyObject *datetime = PyInt_FromLong(point.timestamp);
        PyObject *value = PyFloat_FromDouble(point.value);

        PyTuple_SetItem(tuple, 0, datetime);
        PyTuple_SetItem(tuple, 1, value);

        PyList_Append(result, tuple);
    }

    return result;
}

static PyMethodDef _methods[] = {
    {"points", (PyCFunction)_points, METH_NOARGS, "Load points"},
    {NULL}
};

static PyMemberDef _members[] = {
    {"whisper_file", T_OBJECT_EX, offsetof(WhisperArchiveInfo, whisper_file), 0, "Whisper File"},
    {"seconds_per_point", T_INT, offsetof(WhisperArchiveInfo, seconds_per_point), 0, "Seconds Per Point"},
    {"points_count", T_INT, offsetof(WhisperArchiveInfo, points_count), 0, "Points Count"},
    {NULL}
};

static int
_init(C *self, PyObject *args, PyObject *kwds) {
    PyObject *p_whisper_file = NULL;
    int index = 0;

    if (!PyArg_ParseTuple(args, "Oi", &p_whisper_file, &index)) {
        return -1; 
    }

    WhisperFile *whisper_file = (WhisperFile *)p_whisper_file;

    wsp_archive_info_t *archive_info = &whisper_file->wsp->archives[index];

    self->whisper_file = whisper_file;
    self->archive_info = archive_info;
    self->points_count = archive_info->points_count;
    self->seconds_per_point = archive_info->seconds_per_point;

    Py_INCREF(whisper_file);

    return 0;
}

static void
_dealloc(C *self) {
    Py_XDECREF(self->whisper_file);
}

PyTypeObject WhisperArchiveInfo_T = {
    PyObject_HEAD_INIT(NULL)
    0,                      /*ob_size*/
    "WhisperArchiveInfo",   /*tp_name*/
    sizeof(C),              /*tp_basicsize*/
    0,                      /*tp_itemsize*/
    (destructor)_dealloc,   /*tp_dealloc*/
    0,                      /*tp_print*/
    0,                      /*tp_getattr*/
    0,                      /*tp_setattr*/
    0,                      /*tp_compare*/
    0,                      /*tp_repr*/
    0,                      /*tp_as_number*/
    0,                      /*tp_as_sequence*/
    0,                      /*tp_as_mapping*/
    0,                      /*tp_hash */
    0,                      /*tp_call*/
    0,                      /*tp_str*/
    0,                      /*tp_getattro*/
    0,                      /*tp_setattro*/
    0,                      /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,     /*tp_flags*/
    "Whisper Archive Info", /*tp_doc*/
    0,                      /*tp_traverse*/
    0,                      /*tp_clear*/
    0,                      /*tp_richcompare*/
    0,                      /*tp_weaklistoffset*/
    0,                      /*tp_iter*/
    0,                      /*tp_iternext*/
    _methods,               /*tp_methods*/
    _members,               /*tp_members*/
    0,                      /*tp_getset*/
    0,                      /*tp_base*/
    0,                      /*tp_dict*/
    0,                      /*tp_descr_get*/
    0,                      /*tp_descr_set*/
    0,                      /*tp_dictoffset*/
    (initproc)_init,        /*tp_init*/
    0,                      /*tp_alloc*/
    0,                      /*tp_new*/
};

void init_WhisperArchiveInfo_T(PyObject *m) {
    WhisperArchiveInfo_T.tp_new = PyType_GenericNew;

    if (PyType_Ready(&WhisperArchiveInfo_T) == 0) {
        Py_INCREF(&WhisperArchiveInfo_T);
        PyModule_AddObject(m, "WhisperArchiveInfo", (PyObject *)&WhisperArchiveInfo_T);
    }
}
