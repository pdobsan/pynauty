[![Build and test](https://github.com/pdobsan/pynauty/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/pdobsan/pynauty/actions/workflows/build-and-test.yml)

# Pynauty 

Pynauty can be used to compare graphs for isomorphism and to determine
their automorphism group in a Python programming environment.  Pynauty
is a Python/C extension module using library components from the
[Nauty](https://pallini.di.uniroma1.it/) package by Brendan McKay.

Current version is distributed and has been tested with `nauty2_8_8`.

## Usage

Here is an example of `pynauty`'s usage in an interactive session.

```python
>>> from pynauty import *
>>> g = Graph(5)
>>> g.connect_vertex(0, [1, 2, 3])
>>> g.connect_vertex(2, [1, 3, 4])
>>> g.connect_vertex(4, [3])
>>> print(g)
Graph(number_of_vertices=5, directed=False,
 adjacency_dict = {
  0: [1, 2, 3],
  2: [1, 3, 4],
  4: [3],
 },
 vertex_coloring = [
 ],
)
>>> autgrp(g)
([[3, 4, 2, 0, 1]], 2.0, 0, [0, 1, 2, 0, 1], 3)
>>> 
>>> g.connect_vertex(1, [3])
>>> autgrp(g)
([[0, 1, 3, 2, 4], [1, 0, 2, 3, 4]], 4.0, 0, [0, 0, 2, 2, 4], 3)
>>>
>>> g.set_vertex_coloring([set([3])])
>>> print(g)
Graph(number_of_vertices=5, directed=False,
 adjacency_dict = {
  0: [1, 2, 3],
  1: [3],
  2: [1, 3, 4],
  4: [3],
 },
 vertex_coloring = [
  set([3]),
  set([0, 1, 2, 4]),
 ],
)
>>> autgrp(g)
([[1, 0, 2, 3, 4]], 2.0, 0, [0, 0, 2, 3, 4], 4)
>>>
```

Please note that multigraphs are not supported, but this limitation can be
worked around by encoding multigraphs as simple graphs. For details, see the
documentation and Nauty's manual.

## Installation

### Installing from [PyPi](https://pypi.org/project/pynauty/)

You can install `pynauty` using `pip`, just type

```bash
pip install --upgrade pynauty
```

Many binary wheels are provided for recent Linux and macOS systems.
When your system is not compatible with any of the provided binary
wheels `pip` attempts to build the wheel of the extension module in your
local system. This happens automatically and the process is transparent
to the user.  For local builds `pip` expects that the [required
tools](#requirements) are available.

You can force the local (re)compilation during install by issuing

```bash
pip install --no-binary pynauty pynauty
```

Regardless of which way the installation was accomplished, afterwards
you can test the result by running the provided tests.

```bash
python <path-to-installed-package>/pynauty/tests/test_minimal.py
```

If you have `pytest` installed then you can run the full test suit by
issuing

```bash
pip install pytest
python -m pytest <path-to-installed-package>/pynauty
```

That takes a few seconds but then you can be sure in having a binary
compatible module.

## Documentation

The `pynauty` package comes with an HTML documentation with examples.
You can read it with your favorite browser:

```bash
<your-browser>  <path-to-installed-package>/pynauty/docs/html/index.html
```

### Building manually from sources

#### Requirements

Apart from Python the requirements are the same as for building Nauty.

- Python 3.8 - 3.10 
- An ANSI C compiler 
- GNU make

#### Download sources

You can download the source distribution form
[PyPi](https://pypi.org/project/pynauty/) by issuing:

```bash
pip download --no-binary pynauty pynauty
```

Please note, the source distribution also contains Nauty2_8_8's source.

#### Build, test, install

If you really are interested in the low level details then unpack the
downloaded source distribution, change into `pynauty`'s directory and
type `make`.  The Makefile is self-documenting in the sense that
invoking `make` without arguments will list all available targets with
short explanations.

## Contributing

Questions, bug reports, pull requests, especially the ones extending the
Python module's functionality, are welcome. Please, open an issue first
to discuss what you would like to change.

Pull requests must be made on a dedicated `topic-branch` of your choice
and not against `upstream/main`.  Before submitting a pull request, make
sure that your fork is up to date with upstream. Also update tests,
documentation, examples as appropriate with the changes in your PR. 

### Contributors

  - @goens helped creating `pytest` components.

  - Sam Morley-Short (@sammorley-short) added `canon_label()` function.
  
  - Frank Bryce fixed the issue of misleadingly accepting multigraphs.

## License

Pynauty is distributed under the terms of GPL v3 WITHOUT ANY WARRANTY.
For the exact details on licensing see the file `COPYING`.

Please note, Nauty is covered by its own licensing terms. For the exact
details see the file `src/nauty2_8_8/COPYRIGHT`.
