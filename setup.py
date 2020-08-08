from distutils.core import setup, Extension

inotipy_module = Extension('inotipy', sources = ['inotipy.c'])

inotipy_utils_module = Extension('inotipyutils', sources = ['utils.c'])

setup (name = 'inotipy',
       version = '0.01a',
       description = 'inotipy, a transparent interface for the inotify system '
                     'call',
       ext_modules = [inotipy_module])

setup (name = 'inotipyutils',
       version = '0.01a',
       description = 'utils for inotipy',
       ext_modules = [inotipy_utils_module])
