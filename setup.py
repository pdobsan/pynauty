#!/usr/bin/env python

import os
import urllib.request
import tarfile
import subprocess
from distutils.cmd import Command
from setuptools.command.install import install
from distutils.core import setup, Extension


MODULE          = 'pynauty'
VERSION         = '0.7.0'

description     = 'Automorphism and isomorphism of graphs'
long_description = '''
Package for testing isomorphism of graphs
and computing their automorphism group.
'''
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
    'Programming Language :: Python :: 3.5',
    'Programming Language :: Python :: 3.9',
    'Programming Language :: C',
    'Topic :: Scientific/Engineering',
    'Topic :: Scientific/Engineering :: Mathematics',
    'Topic :: Scientific/Engineering :: Computing Science',
    'Intended Audience :: Science/Research',
    'Intended Audience :: Education',
    'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
]

pynauty_dir = 'src'
package_dir     = { MODULE : pynauty_dir}
packages        = [ MODULE ]
scripts         = []
data_files      = []

cwd = os.getcwd()
# nauty's source directory
nauty_dir       = os.path.join(cwd,'nauty')
nauty_version   = '27r1'

class InstallNauty(Command):
    """Fetch and install nauty, if not present already"""
    description = "ensure nauty is built"
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        """Run the command.

        Fetches and installs nauty.
        """
        if os.path.exists(os.path.join(nauty_dir,'nauty.o')):
            #todo: add version check
            return

        if not os.path.exists(os.path.join(cwd,f"nauty{nauty_version}.tar.gz")):
            print("Fetching nauty.")
            urllib.request.urlretrieve(
                f"http://users.cecs.anu.edu.au/~bdm/nauty/nauty{nauty_version}.tar.gz",
                f"nauty{nauty_version}.tar.gz",)
        print("Extracting nauty.")
        with tarfile.open(f"nauty{nauty_version}.tar.gz") as tar:
            tar.extractall(cwd)
            os.rename(os.path.join(cwd,f"nauty{nauty_version}"), nauty_dir)
        print("Building nauty")
        subprocess.check_call(
            ["make", "nauty"], cwd=nauty_dir
           )

class InstallCommand(install):
    def run(self):
        self.run_command("nauty")
        # See: https://cbuelter.wordpress.com/2015/10/25/extend-the-setuptools-install-command/comment-page-1/
        self.do_egg_install()


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

setup( name = MODULE, version = VERSION,
       description = description, long_description = long_description,
       author = author, author_email = author_email, url = url,
       platforms = platforms,
       license = license,
       package_dir = package_dir,
       packages = packages,
       scripts = scripts,
       data_files = data_files,
       ext_modules = ext_modules,
       classifiers = classifiers,
       cmdclass={ 'nauty' : InstallNauty,
                  'install' : InstallCommand
                 }
     )

# vim: expandtab:
