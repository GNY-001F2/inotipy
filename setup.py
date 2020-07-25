from distutils.core import setup, Extension

inotipy_module = Extension('inotipy',
                    sources = ['inotipy.c'])

setup (name = 'inotipy',
       version = '0.01a',
       description = 'inotipy, a transparent interface for the inotify system '
                     'call',
       ext_modules = [inotipy_module])
