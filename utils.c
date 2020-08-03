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
static ssize_t pos = -1;
static ssize_t buffer_size = -1;
static int _utils_errno = 0;

typedef struct inotify_event inotify_event;

static PyObject * inotipy_utils_read(PyObject *self, PyObject *args,
                                     PyObject *kwargs)
{
    // TODO: Accept a file descriptor and return a list of tuples
    char* kwlist[] = {"fd", NULL};
    int fd;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &fd))
        return NULL;
    buffer_init();
    // WIP: read the file descriptor, increasing buffer size if it fails
    ssize_t bytes_read = inotify_read(fd);
    // TODO
    
    // send read data to get_inotify_event_data()
    
    // TODO
    
    // build tuple of inotify event data
    
    // add to list of events
    
    // send list to python program
    buffer_cleanup();
}

ssize_t inotify_read(int fd)
{
    ssize_t bytes_read = read(fd, buffer, buffer_size);
    _utils_errno = errno;
    switch (_utils_errno)
    {
        case EINVAL:
            if (buffer_size + 128 < MAX_BUFFER_SIZE)
                buffer_resize(buffer_size + 128);
            else if (buffer_size + 128 > MAX_BUFFER_SIZE)
                buffer_resize(MAX_BUFFER_SIZE);
            else break;
        case EAGAIN:
        case EWOULDBLOCK:
            return inotify_read(fd);
    }
    return bytes_read;
}

static PyObject * get_inotify_event_data(void)
{
    // TODO: Read an inotify event from buffer and send it back
    ssize_t start = pos;
    if(!(start >= 0)) return NULL;
    pos += sizeof (inotify_event);
    inotify_event *read_event = PyMem_RawMalloc(sizeof(inotify_event));
    memcpy(read_event, (buffer+start), (size_t) (pos - start));
    pos += read_event->len;
    read_event = PyMem_RawRealloc(read_event,
                                  sizeof(inotify_event) + read_event->len);
    memcpy(read_event, (buffer+start), (size_t) (pos - start));
    PyObject *read_event_tuple = build_tuple(read_event);
    PyMem_RawFree(read_event);
    return read_event_tuple;
}

static void buffer_init(void)
{
    buffer_size = INITIAL_BUFFER_SIZE;
    pos = 0;
    buffer = PyMem_RawMalloc(buffer_size);
}

static void buffer_cleanup(void)
{
    buffer_size = -1;
    pos = -1;
    PyMem_RawFree(buffer);
    buffer = NULL;
}

static void buffer_resize(ssize_t bytes)
{
    if (bytes <= 0)
        break;
    buffer_size = bytes;
    buffer = PyMem_RawRealloc(buffer, (size_t) buffer_size);
}

static PyObject * build_tuple(inotify_event *read_event)
{
    // TODO: Build a tuple out of inotify event
}
#undef INT_SIZE
#undef FOUR_BYTES
#undef SIXTEEN_BYTES
#undef INITIAL_BUFFER_SIZE
#undef MAX_READABLE_BYTES
