[![Build and test](https://github.com/pdobsan/pynauty/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/pdobsan/pynauty/actions/workflows/build-and-test.yml)

# Pynauty 

Pynauty can be used to compare graphs for isomorphism and to determine
their automorphism group in a Python programming environment.  Pynauty
is a Python/C extension module using library components from the
[Nauty](https://pallini.di.uniroma1.it/) package by Brendan McKay.


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

## Installation

### Requirements

- Python 3.7 - 3.9
- An ANSI C compiler 

### Installing from PyPi

```bash
pip install pynauty
```

If that fails, probably because of binary incompatibility, then download
the source distribution.

```bash
pip download --no-binary :all: pynauty
```

See the next section how to build from sources.

### Building from sources

You can download the sources either from
[PyPi](https://pypi.org/project/pynauty/) or from the
[Releases section](https://github.com/pdobsan/pynauty/releases)
of [Pynauty's Home Page](https://github.com/pdobsan/pynauty).

Please note, Nauty's source code `version 27r1` is distributed with `pynauty`.

To build and test the package first without installing it anywhere just type:

```bash
make tests
```

Here is how to create a virtualenv and install `pynauty` into it.

```bash
% make virtenv-create
python3 -m venv .venv-pynauty
Created virtualenv: .venv-pynauty
To activate it type: source ./.venv-pynauty/bin/activate
% source .venv-pynauty/bin/activate
(.venv-pynauty) % make install
...
```

You can install `pynauty` into an already existing virtualenv just skip
the creation step above and activate your own virtualenv.

```bash
% source /.../my-virtenv/bin/activate
(my-virtenv) % make install
...
```

If `make install` is invoked outside of a virtualenv that will install
`pynauty` in `~/.local` (home install).

Building the documentation from source requires the Sphinx package.

```bash
make docs
```

The Makefile is self-documenting in the sense that invoking `make`
without arguments will list all available targets with short
explanations. Please note, this *help* slightly changes depending
whether the invocation was made inside or outside of a virtualenv.


## Documentation

For detailed documentation, API and User's Guide see the `docs/`
directory after building as above.

## Contributing

Questions, bug reports, pull requests are welcome. Please, open an issue
first to discuss what you would like to change.

Pull requests must be made on a dedicated `topic-branch` of your choice
and not against `upstream/main`.  Before submitting a pull request, make
sure that your fork is up to date with upstream. Also update tests,
documentation, examples as appropriate with the changes in your PR. 

## License

Pynauty is distributed under the terms of GPL v3 WITHOUT ANY WARRANTY.
For the exact details on licensing see the file `COPYING`.

Please note, Nauty is covered by its own licensing terms. For the exact
details see the file `src/nauty27r1/COPYRIGHT`.
