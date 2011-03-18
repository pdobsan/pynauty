Installation
============

The source and documentation of the most recent version of
**pynauty** is available as a compressed `tar-file`_.


Dependencies
------------

To build **pynauty** the following additional components are needed:

    * Python 2.5 or newer.
    * The most recent version of Nauty_.
    * An ANSI C compiler.

In general, **pynauty** should work on all platforms (Linux, Windows,
Mac, etc.) where Python is available and Nauty's source can be complied.


Build
-----

Compiling Nauty
^^^^^^^^^^^^^^^

Download the Nauty_ distribution. Unpack the tar file and change to the
``nautyXX/`` directory where ``XX`` stands for the version.
Type the following commands::

    ./configure
    make nauty.o nautil.o naugraph.o

Should you encounter any difficulties with compiling Nauty, please,
consult Nauty's documentation.


Building pynauty
^^^^^^^^^^^^^^^^

Untar the downloaded pynauty `tar-file`_ then change to the
``pynauty-x.y/`` directory. Create a symbolic link named ``nauty`` to
the directory which contains Nauty's source and compiled modules. For
example::

    ln -s ../nautyXX nauty

Replace ``XX`` with the actual version of Nauty.

To build the **pynauty** extension module type the command::

    python setup.py build


Testing
^^^^^^^

After building pynauty, you can test the module using the script(s)
in the ``tests/`` directory::

    cd tests
    PYTHONPATH=../build/lib.XXXXX python test_pynauty.py

Replace ``XXXXXX`` with the actual part of directory's name.


Install
-------

If you want to install **pynauty** to the standard location in your
system, type::

     python setup.py install

On Unix/Linux you might need root privileges to execute this command.
For alternative install methods and locations see::

    python setup.py --help install

If you don't want to install the extension module at all, you can use it
directly either by changing to the build directory::

    cd build/lib.XXXXXX

or adding the above directory to your ``PYTHONPATH`` environment
variable.  ``XXXXXX`` depends on you platform and the version of Python
you are using. Note, you must use absolute path in ``PYTHONPATH``.


.. _tar-file: pynauty-0.4.tar.gz
.. _Nauty: http://cs.anu.edu.au/~bdm/nauty/
