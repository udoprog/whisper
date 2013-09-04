#include "WhisperFile.h"
#include "WhisperArchiveInfo.h"

typedef WhisperFile C;

static PyObject* _open(C *self, PyObject *args) {
    char *path;
    int path_l;
    wsp_mapping_t mapping = WSP_MAPPING_NONE;

    if (!PyArg_ParseTuple(args, "s#|i", &path, &path_l, &mapping)) {
        return NULL;
    }

    if (mapping == WSP_MAPPING_NONE) {
        mapping = WSP_MMAP;
    }

    self->wsp = PyMem_Malloc(sizeof(wsp_t));
    WSP_INIT(self->wsp);

    wsp_error_t e;

    if (wsp_open(self->wsp, path, mapping, &e) == WSP_ERROR) {
        PyErr_Whisper(&e);
        return NULL;
    }

    uint32_t i = 0;

    for (i = 0; i < self->wsp->archives_count; i++) {
        PyObject *ai_args = Py_BuildValue("Oi", self, i);

        if (ai_args == NULL) {
            return NULL;
        }

        Py_INCREF(ai_args);

        PyObject *ai_object = PyObject_CallObject((PyObject *)&WhisperArchiveInfo_T, ai_args);

        if (ai_object == NULL) {
            return NULL;
        }

        PyList_Append(self->archives, ai_object);
    }

    Py_RETURN_NONE;
}

static PyMethodDef _methods[] = {
    {"open", (PyCFunction)_open, METH_VARARGS, "Open the specified path"},
    {NULL}
};

static PyMemberDef _members[] = {
    {"archives", T_OBJECT_EX, offsetof(WhisperFile, archives), 0, "archives"},
    {NULL}
};

static int
_init(C *self, PyObject *args, PyObject *kwds) {
    self->archives = PyList_New(0);
    return 0;
}

PyTypeObject WhisperFile_T = {
    PyObject_HEAD_INIT(NULL)
    0,                  /*ob_size*/
    "WhisperFile",      /*tp_name*/
    sizeof(C),          /*tp_basicsize*/
    0,                  /*tp_itemsize*/
    0,                  /*tp_dealloc*/
    0,                  /*tp_print*/
    0,                  /*tp_getattr*/
    0,                  /*tp_setattr*/
    0,                  /*tp_compare*/
    0,                  /*tp_repr*/
    0,                  /*tp_as_number*/
    0,                  /*tp_as_sequence*/
    0,                  /*tp_as_mapping*/
    0,                  /*tp_hash */
    0,                  /*tp_call*/
    0,                  /*tp_str*/
    0,                  /*tp_getattro*/
    0,                  /*tp_setattro*/
    0,                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
    "Whisper File",     /*tp_doc*/
    0,                  /*tp_traverse*/
    0,                  /*tp_clear*/
    0,                  /*tp_richcompare*/
    0,                  /*tp_weaklistoffset*/
    0,                  /*tp_iter*/
    0,                  /*tp_iternext*/
    _methods,           /*tp_methods*/
    _members,           /*tp_members*/
    0,                  /*tp_getset*/
    0,                  /*tp_base*/
    0,                  /*tp_dict*/
    0,                  /*tp_descr_get*/
    0,                  /*tp_descr_set*/
    0,                  /*tp_dictoffset*/
    (initproc)_init,    /*tp_init*/
    0,                  /*tp_alloc*/
    0,                  /*tp_new*/
};

void init_WhisperFile_T(PyObject *m) {
    WhisperFile_T.tp_new = PyType_GenericNew;

    if (PyType_Ready(&WhisperFile_T) == 0) {
        Py_INCREF(&WhisperFile_T);
        PyModule_AddObject(m, "WhisperFile", (PyObject *)&WhisperFile_T);
    }
}
