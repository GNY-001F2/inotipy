# inotipy, a transparent wrapper for the Linux inotify system call
# Copyright (C) 2020  Aayush Agarwal
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


from ctypes import CDLL, get_errno, c_int, c_uint32, c_char_p
from ctypes.util import find_library
import errno


# We first need to find the library being used;
_clib = CDLL(find_library("c"), use_errno=True)


def inotify_init():
    '''
    Wrapper for inotify_init(void) in <sys/inotify.h>. Returns the file
    descriptor of the newly created event watcher.
    In case of an error during event watcher creation, returns -1 and sets
    errno.
    '''
    fd = _clib.inotify_init()
    return fd


def inotify_init1(flags: int) -> int:
    '''
    Wrapper for inotify_init1(int flags) in <sys/inotify.h>. Returns the file
    descriptor of the newly created event watcher.
    In case of an error during event watcher creation, returns -1 and sets
    errno.
    '''
    c_init_flags = c_int(flags)
    fd = _clib.inotify_init1(c_init_flags)
    return fd


def inotify_add_watch(fd: int, pathname: str, mask: int) -> int:
    '''
    Wrapper for inotify_add_watch(int fd, const char* pathname, uint32_t mask)
    in <sys/inotify.h>. Returns the watch descriptor created on adding the
    file.
    In case of error, returns -1 and sets errno.
    '''
    c_fd = c_int(fd)
    c_pathname = c_char_p(pathname)
    c_mask = c_uint32(mask)
    wd = _clib.inotify_add_watch(c_fd, c_pathname, c_mask)
    return wd


def inotify_rm_watch(fd: int, wd: int) -> int:
    '''
    Wrapper for inotify_rm_watch(int fd, int wd) in <sys/inotify.h>. Returns
    0 on success. In case of error, returns -1 and sets errno.
    '''
    c_fd = c_int(fd)
    c_wd = c_int(wd)
    status = _clib.inotify_rm_watch(c_fd, c_wd)
    return status
