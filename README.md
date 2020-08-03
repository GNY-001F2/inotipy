# inotipy
A simple "low-level" Python 3 wrapper for the Linux inotify API

While an inotify API exists for Python, I wanted to try my hand at linking C
code to Python code.

This program will only be designed for Python3 and will try to keep the calls
as transparent as technically possible.

This program attempts to provide transparent access to the following system
calls, which are declared in sys/inotify.h:

* inotify_init() and inotify_init1()
* inotify_add_watch()
* inotify_rm_watch()
* uint32_t mask values
* int flags
* errno when it is set by any of the above API calls.

Any additional functionality that a programmer may need has to be built on top
of this.

Specifically, the use of the following built-in modules/functions is highly
recommended:

* os.read(): this function is essential to actually read the inotify events
             that are triggered by your watch descriptors
* errno module: if you need to compare the errno set by the inotify calls
