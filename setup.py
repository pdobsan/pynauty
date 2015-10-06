
import os
from distutils.core import setup, Extension

MODULE          = 'pynauty'
VERSION         = '0.6.0'

description     = 'Automorphism and isomorphism of graphs'
long_description = '''
Package for testing isomorphism of graphs
and computing their automorphism group.
'''
author          = 'Peter Dobcsanyi'
author_email    = 'petrus@ftml.net'
url             = ''
license         = 'GNU General Public License v3'
platforms       = ['platform independent',]

pynauty_dir = 'src'
package_dir     = { MODULE : pynauty_dir}
packages        = [ MODULE ]
scripts         = []
data_files      = []

nauty_dir       = 'nauty'  # nauty's source directory
if not os.access(nauty_dir, os.R_OK | os.X_OK):
    print("Can't find nauty_dir: %s" % nauty_dir)
    raise SystemExit(1)

ext_pynauty = Extension(
        name = MODULE + '._pynauty',
        sources = [ pynauty_dir + '/' + 'pynauty.c', ],
        extra_compile_args = [ '-O4', '-fPIC' ],
        extra_objects = [ nauty_dir + '/' + 'nauty.o',
                          nauty_dir + '/' + 'nautil.o',
                          nauty_dir + '/' + 'naugraph.o',
                          nauty_dir + '/' + 'schreier.o',
                          nauty_dir + '/' + 'naurng.o',
                        ],
        include_dirs = [ nauty_dir, pynauty_dir ]
    )
ext_modules = [ ext_pynauty ]

#os.umask(022)

setup( name = MODULE, version = VERSION,
       description = description, long_description = long_description,
       author = author, author_email = author_email, url = url,
       platforms = platforms,
       license = license,
       package_dir = package_dir,
       packages = packages,
       scripts = scripts,
       data_files = data_files,
       ext_modules = ext_modules
     )

# vim: expandtab:
