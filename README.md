# inotipy
A simple "low-level" Python 3 wraparound for the Linux inotify API  

While an inotify API exists for Python, I wanted to try my hand at linking C
code to Python code.

The existing inotify API is also kind of poorly documented and not updtated in
the past several years.

There is also inotify-simple, but it retains compatibility with both Python2
and Python3. It also does not expose the system calls transparently.

This program will only be designed for Python3 and will try to keep the calls
as transparent as technically possible.

The objective of this program is to simply provide transparent access to the
inotify calls implemented in the kernel:

* inotify_init() and inotify_init1()
* inotify_add_watch()
* inotify_rm_watch()

Any additional functionality that a programmer may need has to be built on top
of this.

This program is licensed in GPLv3 except to the extent that any file is
required to be released under GPLv2. This will be indicated in the license
header.
