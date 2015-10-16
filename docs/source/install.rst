Installation
============

The source and documentation of the most recent version of
**pynauty** is available as a compressed tar-file_.


Dependencies
------------

To build **pynauty** the following additional components are needed:

    * Python 2.7 or Python 3.5
    * The most recent version of Nauty_.
    * An ANSI C compiler.

In theory, **pynauty** should work on all platforms where Python is
available and Nauty's source can be complied. The instructions below are
for Linux/Unix environment.


Build
-----

Download `pynauty's sources`_ as a compressed tar file and unpack it.
That should create a directory like ``pynauty-X.Y.Z/`` containing the
source files.

Download the most recent sources of nauty from Nauty_ or from Traces_.
That file should be something like ``nautyXYZ.tar.gz``.

Change to the directory ``pynauty-X.Y.Z/`` replacing ``X.Y.Z`` with the
actual version of pynauty:

    cd pynauty-X.Y.Z

Unpack ``nautyXYZ.tar.gz`` inside the ``pynauty-X.Y.Z/`` directory.
Create a symbolic link named ``nauty`` to nauty's source directory
replacing ``XYZ`` with the actual version of Nauty:

    ln -s nautyXYZ nauty

Pynauty can be built both for Python 2.7 or Python 3.5.

At this stage you have the option to create and activate a virtualenv
and continue the building process within it. Otherwise the building
process just picks your system wide python version which would be fine
for most users.

To build pynauty use the command:

    make pynauty

That takes care compiling the necessary object files from nauty's source
than compiling the pynauty Python extension module.

To run all the tests coming with the package type:

    make tests

The test exercises pynauty on a few graphs considered difficult.

Install
-------

To install pynauty to the standard user location in your home directory
type:

    make user-ins

That puts the pynauty package files into
~/.local/lib/pythonM.N/site-packages, where M.N is your python's version.

To uninstall type:

    make user-unins

If you used a virtualenv you can install/uninstall pynauty within it by:

    make virtenv-ins
    make virtenv-unins

Please note, the install/unistall procedures use ``pip``.

.. _pynauty's sources:
.. _tar-file: ../pynauty-0.6.0.tar.gz
.. _Nauty: https://cs.anu.edu.au/people/Brendan.McKay/nauty/
.. _Traces: http://pallini.di.uniroma1.it/
