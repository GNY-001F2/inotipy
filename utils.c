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
#include <unistd.h>
#include <sys/inotify.h>

#define INT_SIZE sizeof (int)
#define FOUR_BYTES sizeof (uint32_t)
#define SIXTEEN_BYTES (INT_SIZE + 3*FOUR_BYTES)
// We work with the assumption that a filename is reasonably sized
#define INITIAL_BUFFER_SIZE (SIXTEEN_BYTES + 128)
// This is the linux-specific max number of bytes read() will return
// If buffer grows bigger than this, we have a problem.
#define MAX_READABLE_BYTES 2147479552
// This is pretty much the largest a single inotify_event can get.
// On ReiserFS the max file length is 4032 bytes. On sane operating systems
// it usually does not cross 255 bytes, while reaching upto 1020 bytes on some
// rarer systems.
#define MAX_BUFFER_SIZE 4096

// The buffer which stores a read event.
static char *buffer = NULL;
// The index of a pointer to the buffer
static int buffer_pos = -1;
static ssize_t buffer_size = -1;
static int _utils_errno = 0;

static int events_read = 0;

typedef struct inotify_event inotify_event;

typedef struct inotify_event_list {
    inotify_event *event;
    struct inotify_event_list *next_event;
} inotify_event_list;

typedef struct iel_sentinels {
    inotify_event_list *head;
    inotify_event_list *tail;
} iel_head_tail;

static iel_head_tail sentinel = { .head=NULL, .tail=NULL};

static ssize_t iel_length = 0;

static void buffer_init(void);
static void buffer_cleanup(void);
static void buffer_resize(ssize_t bytes);
static ssize_t _inotify_read(int fd);
static inotify_event * extract_event_data(void);
static void add_event_to_queue(inotify_event *event);
static PyObject * build_tuple(inotify_event *event);

static PyObject * inotipy_utils_read(PyObject *self, PyObject *args,
                                     PyObject *kwargs) {
    // Accept a file descriptor and return the number of events read.
    // WARNING: Any previous contents of the buffer will be overwritten!
    char* kwlist[] = {"fd", NULL};
    int fd;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &fd))
        return NULL;
    buffer_cleanup();
    buffer_init();
    // Read the file descriptor
    ssize_t bytes_read = _inotify_read(fd);
    // If the read raised an error, raise it to the interpreter.
    if (bytes_read <= 0) {
        if (bytes_read == -1)
            PyErr_SetFromErrno(PyExc_OSError);
        else if (bytes_read == 0)
            PyErr_SetString(PyExc_EOFError, "No new events were found!");
        buffer_cleanup();
        return NULL;
    }
//     else buffer_resize(bytes_read);
//     return PyLong_FromLong(bytes_read);
    // One or more events have been read, now add them to the event queue
    while (bytes_read > 0) {
        inotify_event *read_event = extract_event_data();
//         if(read_event->cookie > 0) {
//             PyErr_SetString(PyExc_ValueError,
//                             "This line right here, officer!");
//             return NULL;
//         }
        if(read_event){
            add_event_to_queue(read_event);
        }
        else break;
    }
//     buffer_cleanup();
    return PyLong_FromLong(events_read);
}

ssize_t _inotify_read(int fd) {
    // Call read(2), adding one or more events to the buffer.
    // Shrink buffer to avoid access to illegal bytes.
    // Return the number of bytes read.
    ssize_t bytes_read = read(fd, buffer, buffer_size);
    _utils_errno = errno;
    switch (_utils_errno) {
        case 0:
            break;
        case EINVAL:
            if ((buffer_size + 128) <= MAX_BUFFER_SIZE)
                buffer_resize(buffer_size + 128);
            else if (buffer_size >= MAX_BUFFER_SIZE)
                break;
            return _inotify_read(fd);
    }
    (bytes_read >= 0) ? buffer_resize(bytes_read) : buffer_cleanup();
    return bytes_read;
}

static PyObject *get_raw_buffer(PyObject *self) {
    // Return a bytes object containing the raw buffer
    if(!buffer) {
        PyErr_SetString(PyExc_BufferError, "The buffer is empty!");
        return NULL;
    }
    PyObject *buf = PyBytes_FromStringAndSize(buffer, buffer_size);
    return buf;
}

static inotify_event * extract_event_data(void) {
    // TODO: Read an inotify event from buffer and send it back
    if(buffer_pos >= buffer_size) return NULL;
    if(buffer_pos < 0) return NULL;
    int start = buffer_pos;
//     if(start < 0 || start > buffer_size) return NULL;
    buffer_pos += sizeof (inotify_event);
    inotify_event *read_event = PyMem_RawMalloc(sizeof (inotify_event));
    memcpy(read_event, (buffer+start), (size_t) (buffer_pos - start));
    buffer_pos += read_event->len;
    read_event = PyMem_RawRealloc(read_event,
                                  sizeof (inotify_event) + read_event->len);
    memcpy(read_event, (buffer+start), (size_t) (buffer_pos - start));
    return read_event;
}

static void add_event_to_queue(inotify_event *event) {
    inotify_event_list *new_event = \
        PyMem_RawMalloc(sizeof (inotify_event_list));
    new_event->event = event;
    new_event->next_event = NULL;
    if(!(sentinel.head)) sentinel.head = new_event;
    if(!(sentinel.tail)) sentinel.tail = new_event;
    else {
        sentinel.tail->next_event = new_event;
        sentinel.tail = sentinel.tail->next_event;
    }
    events_read++;
    iel_length++;
}

static PyObject * get_event_tuple(PyObject *self) {
    // Pointer to event which is to be returned, while destructively
    // deallocating the blocks given the event and event list.
    if(!sentinel.head) {
        PyErr_SetString(PyExc_IndexError,
                        "There are no more events in the queue!");
        return NULL;
    }
    inotify_event *event = sentinel.head->event;
    inotify_event_list *next_event = sentinel.head->next_event;
    PyMem_RawFree(sentinel.head);
    sentinel.head = next_event;
    PyObject *read_event_tuple = build_tuple(event);
    if(!read_event_tuple) return NULL;
    PyMem_RawFree(event);
    iel_length--;
    events_read--;
    return read_event_tuple;
}
// 
// static PyObject * get_event_queue(PyObject *self) {
//     // TODO: Return the linked list queue as a python list of python
//     // tuples
//     return NULL;
// }
// 
static PyObject * build_tuple(inotify_event *event) {
    PyObject *py_wd = PyLong_FromLong(event->wd);
    PyObject *py_mask = PyLong_FromUnsignedLong(event->mask);
    PyObject *py_cookie = PyLong_FromUnsignedLong(event->cookie);
    PyObject *py_name = PyUnicode_FromString(event->name);
    if (!py_name) {
        PyErr_SetString(PyExc_BufferError, "Unable to read name!");
        return NULL;
    }
    Py_ssize_t len = PyUnicode_GetLength(py_name);
    if (len <= 0) {
        PyErr_SetString(PyExc_BufferError, "The length is 0!");
        return NULL;
    }
    PyObject *py_len = PyLong_FromSsize_t(len);
    if (!py_len) {
        PyErr_SetString(PyExc_BufferError, "Unable to set length in tuple!");
        return NULL;
    }
    PyObject *read_event_tuple = PyTuple_Pack(5, py_wd, py_mask, py_cookie,
                                              py_len, py_name);
    return read_event_tuple;
}

static void buffer_init(void) {
    if(buffer) buffer_cleanup;
    buffer_size = INITIAL_BUFFER_SIZE;
    buffer_pos = 0;
    buffer = PyMem_RawMalloc(buffer_size);
}

static void buffer_cleanup(void) {
    buffer_size = -1;
    buffer_pos = -1;
    PyMem_RawFree(buffer);
    buffer = NULL;
}

static void buffer_resize(ssize_t bytes) {
    // Resize buffer to number of bytes given in bytes.
    if (bytes <= 0) {
        buffer_cleanup();
        return;
    }
    buffer_size = bytes;
    buffer = PyMem_RawRealloc(buffer, (size_t) buffer_size);
}


static PyMethodDef inotipy_utils_methods[] = {
    {
        "read", inotipy_utils_read, METH_VARARGS | METH_KEYWORDS,
        "Read from the inotify file descriptor and return the number of "
        "events read."
    },
    {
        "get_event", get_event_tuple, METH_NOARGS, "Return the oldest "
        "inotify_event struct in the form of a tuple. Removes the "
        "returned event from the queue."
    },
//     {
//         "get_event_list", get_event_queue, METH_NOARGS, "Equivalent to "
//         "creating a list of get_event() tuples."
//     },
    {
        "get_raw_buffer", get_raw_buffer, METH_NOARGS, "Return the raw "
        "buffer as a python bytes object."
    },
    {
        NULL, NULL, 0, NULL
    }
};

static PyModuleDef inotipy_utils = {
    PyModuleDef_HEAD_INIT,
    "inotipyutils",
    NULL, //inotipy_doc,
    -1,
    inotipy_utils_methods
};


PyMODINIT_FUNC PyInit_inotipyutils(void) {
    PyObject *module = PyModule_Create(&inotipy_utils);
    return module;
}

#undef INT_SIZE
#undef FOUR_BYTES
#undef SIXTEEN_BYTES
#undef INITIAL_BUFFER_SIZE
#undef MAX_READABLE_BYTES
