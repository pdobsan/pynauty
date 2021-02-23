#!/usr/bin/env python3

import os
from setuptools import setup, Extension
from src import pynauty
from glob import glob

MODULE = 'pynauty'

description      = 'Isomorphism testing and automorphisms of graphs.'
long_description_content_type='text/markdown'
with open('README.md') as f: long_description = f.read()

author          = 'Peter Dobsan'
author_email    = 'pdobsan@gmail.com'
url             = 'https://github.com/pdobsan/pynauty'
license         = 'GNU General Public License v3'
platforms       = ['Linux', 'Unix', 'OS X']
classifiers     = [
    'Environment :: Console',
    'Operating System :: POSIX :: Linux',
    'Operating System :: Unix',
    'Programming Language :: Python',
    'Programming Language :: Python :: 3.6',
    'Programming Language :: Python :: 3.7',
    'Programming Language :: Python :: 3.8',
    'Programming Language :: Python :: 3.9',
    'Programming Language :: C',
    'Topic :: Scientific/Engineering',
    'Topic :: Scientific/Engineering :: Mathematics',
    'Intended Audience :: Science/Research',
    'Intended Audience :: Education',
    'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
]

nauty_dir   = 'src/' + pynauty._nauty_dir
pynauty_dir = 'src/' + MODULE
package_dir = { MODULE : pynauty_dir}
packages    = [ MODULE ]

ext_pynauty = Extension(
        name = MODULE + '.nautywrap',
        sources = [ pynauty_dir + '/' + 'nautywrap.c', ],
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

package_data = {
        MODULE : [f for f in glob('docs/build/*/**/**',  recursive=True) if os.path.isfile(f)],
        }

setup( name = MODULE, version = pynauty.__version__,
       description = description, long_description = long_description,
       author = author, author_email = author_email, url = url,
       platforms = platforms,
       license = license,
       package_dir = package_dir,
       packages = packages,
       ext_modules = ext_modules,
       include_package_data = True,
       #packege_data = package_data,
       classifiers = classifiers,
     )
