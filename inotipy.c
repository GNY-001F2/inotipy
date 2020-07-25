// inotipy, a transparent wrapper for the Linux inotify system call
// Copyright (C) 2020  Aayush Agarwal
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, If not, see <https://www.gnu.org/licenses/>


#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include <Python.h>
#include <errno.h>
#include <sys/inotify.h>


static PyObject * inotipy_inotify_init(PyObject *self)
{
    int fd = inotify_init();
    return PyLong_FromLong(fd);
}

// static PyObject * inotipy_inotify_init1(PyObject *self, PyObject *args,
//                                         PyObject *kwargs)
// {
//     int flags;
//     char *kwlist[] = {"flags", NULL}
//     if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, flags))
//         return NULL;
//     // Upon successful assignment of flags, invoke inotify_init1
//     int fd = inotify_init1(flags)
//     return PyInt_FromLong(fd)
// }
// 
// static PyObject * inotipy_inotify_add_watch(PyObject *self, PyObject *args,
//                                             PyObject *kwargs)
// {
//     int fd;
//     const char *pathname;
//     // NOTE: PyArg_ParseTuple and its variants described in the API do not use
//     // uint32_t types. So we'll use an unsigned long integer and type cast it
//     // to uint32_t.
//     // Unsigned long is preferable to unsigned int because a long integer is
//     // guaranteed to be at least 32 bits, which is the guaranteed size of
//     // uint32_t and if it is longer than 32 bits, the leading zeros can be
//     // safely truncated after the bitwise mask operations.
//     unsigned long _mask;
//     uint32_t mask;
//     char *kwlist[] = {"fd", "pathname", "mask", NULL}
//     if (!PyArg_ParseTupleAndKeywords(args, kwargs, "isk", kwlist, fd, &path,
//         _mask))
//         return NULL;
//     // If the variables are safely assigned, we cast _mask as uint32_t mask.
//     mask = (uint32_t) _mask;
//     int wd = inotify_add_watch(fd, pathname, mask)
//     return PyInt_FromLong(fd)
// }
// 
// static PyObject * inotipy_inotify_rm_watch(PyObject *self, PyObject *args,
//                                            PyObject *kwargs)
// {
//     int fd, wd;
//     char *kwlist[] = {"fd", "wd", NULL}
//     if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii", kwlist, fd, wd))
//         return NULL;
//     int status = inotify_rm_watch(fd, wd)
//     return PyInt_FromLong(status)
// }
// 
// static PyObject * inotipy_expose_flags(PyObject *self)
// {
//     // Send the flags used by inotify_init1() to the python program so that
//     // the user can use it.
// }
// 
// static PyObject * inotpy_expose_masks(PyObject *self)
// {
//     // Send the flags used by inotify_add_watch() to the python program so
//     // that the user can use it.
// }

static PyMethodDef inotipy_methods[] = {
    {
        "inotipy_init", inotipy_inotify_init, METH_NOARGS,
        "Call inotify_init() system call and return the file descriptor."
    },
//     {
//         "inotipy_init1", inotipy_inotify_init1, METH_VARARGS | METH_KEYWORDS,
//         "Calll inotify_init1() system call and return the file descriptor."
//     }
//     {
//         "inotify_add_watch", inotipy_inotify_add_watch,
//         METH_VARARGS | METH_KEYWORDS,
//         "Call inotify_add_watch() system call and return the watch descriptor."
//     }
//     {"inotify_rm_watch", inotipy_inotify_rm_watch,
//         METH_VARARGS | METH_KEYWORDS,
//         "Call inotify_rm_watch() system call and return the status."
//     }
    {
        NULL, NULL, 0, NULL
    }
};

static PyModuleDef inotipy = {
    PyModuleDef_HEAD_INIT,
    "inotipy",
    NULL, //inotipy_doc,
    -1,
    inotipy_methods
};



PyMODINIT_FUNC PyInit_inotipy(void)
{
    PyObject *module = PyModule_Create(&inotipy);
    return module;
}
