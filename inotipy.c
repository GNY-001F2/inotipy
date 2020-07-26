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
    return PyLong_FromLong((long)fd);
}

static PyObject * inotipy_inotify_init1(PyObject *self, PyObject *args,
                                        PyObject *kwargs)
{
    long _flags;
    char *kwlist[] = {"flags", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l", kwlist, &_flags))
        return NULL;
    // Upon successful assignment of flags, invoke inotify_init1
    int flags = (int) _flags;
    int fd = inotify_init1(flags);
    return PyLong_FromLong((long) fd);
}
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

// static int _inotipy_expose_flags(PyObject *module, char* flagname,
//                                  int flag)
// {
//     // Send the flags used by inotify_init1() to the python program so that
//     // the user can use it.
//     PyObject *flag_py = PyLong_FromLong((long) flag);
//     return PyModule_AddObject(module, flagname, flag_py);
// }

// static PyObject * inotpy_expose_masks(PyObject *self)
// {
//     // Send the flags used by inotify_add_watch() to the python program so
//     // that the user can use it.
// }

static PyMethodDef inotipy_methods[] = {
    {
        "inotify_init", inotipy_inotify_init, METH_NOARGS,
        "Call inotify_init() system call and return the file descriptor."
    },
    {
        "inotify_init1", inotipy_inotify_init1, METH_VARARGS | METH_KEYWORDS,
        "Call inotify_init1() system call and return the file descriptor."
    },
    {
        "inotify_add_watch", inotipy_inotify_add_watch,
        METH_VARARGS | METH_KEYWORDS,
        "Call inotify_add_watch() system call and return the watch descriptor."
    }
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
    int add_flag_status = 0;
    // NOTE: Always ensure that the number
    // the number of char *flags!
    // Also ensure that the names of the flags are declared in the same order!
    // NOTE: Check inotify_init1(2) for details on the masks.
    int flags[] = {IN_NONBLOCK, IN_CLOEXEC};
    int flags_len = sizeof(flags)/sizeof(int);
    char *flagnames[] = {"IN_NONBLOCK", "IN_CLOEXEC"};
    for(int i = 0; i < flags_len && add_flag_status == 0; i++)
    {
// NOTE: EF = Expose Flags
#define _EF_(module, name, value) PyModule_AddIntConstant(module, name, value)
        add_flag_status = _EF_(module, flagnames[i], flags[i]);
#undef _EF_
    }
    if(add_flag_status == -1) {
        return NULL;
    }
    int add_mask_status = 0;
    uint32_t masks[] = {
        // NOTE: Check inotify(7) for details
        // Works on files inside directories only
        IN_ACCESS, IN_CLOSE_WRITE, IN_CREATE, IN_DELETE, IN_OPEN,
        // Works on watched directories and files inside them:
        IN_ATTRIB, IN_CLOSE_NOWRITE, IN_MODIFY, IN_MOVED_FROM, IN_MOVED_TO,
        // Not given + or - symbol in man page
        IN_DELETE_SELF, IN_MOVE_SELF,
        // Convenience Macros
        IN_MOVE, IN_CLOSE,
        // inotify-specific masks
        IN_DONT_FOLLOW, IN_EXCL_UNLINK, IN_MASK_ADD, IN_ONESHOT, IN_ONLYDIR,
        IN_MASK_CREATE,
        // masks returned by read
        IN_IGNORED, IN_ISDIR, IN_Q_OVERFLOW, IN_UNMOUNT
    };
    char *masknames[] = {
        // Works on files inside directories only
        "IN_ACCESS", "IN_CLOSE_WRITE", "IN_CREATE", "IN_DELETE", "IN_OPEN",
        // Works on watched directories and files inside them:
        "IN_ATTRIB", "IN_CLOSE_NOWRITE", "IN_MODIFY", "IN_MOVED_FROM",
        "IN_MOVED_TO",
        // Not given + or - symbol in man page
        "IN_DELETE_SELF", "IN_MOVE_SELF",
        // Convenience Macros
        "IN_MOVE", "IN_CLOSE",
        // inotify-specific mask
        "IN_DONT_FOLLOW", "IN_EXCL_UNLINK", "IN_MASK_ADD", "IN_ONESHOT",
        "IN_ONLYDIR", "IN_MASK_CREATE",
        // masks returned by read
        "IN_IGNORED", "IN_ISDIR", "IN_Q_OVERFLOW", "IN_UNMOUNT"
    };
    int masks_len = sizeof(masks)/sizeof(uint32_t);
    for(int j = 0; j < masks_len && add_mask_status == 0; j++)
    {
#define _EM_(module, name, value) PyModule_AddIntConstant(module, name, value)
        add_mask_status = _EM_(module, masknames[j], masks[j]);
#undef _EM_
    }
    if(add_mask_status == -1) {
        return NULL;
    }
    return module;
}
