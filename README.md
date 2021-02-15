# Pynauty 

Pynauty can be used to compare graphs for isomorphism and to determine
their automorphism group in a Python programming environment.  Pynauty
is a Python/C extension module using library components from the
[Nauty](https://pallini.di.uniroma1.it/) package by Brendan McKay.

## Usage

Here is an example of `pynauty`-s usage in an interactive session.

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

- Python 3.6 - 3.9
- An ANSI C compiler 
- The most recent version of Nauty, currently `nauty27r1`.

### Building from source

Clone this repository:

```bash
git clone https://github.com/pdobsan/pynauty
```

and change into the `pynauty/` directory created by the cloning.

Nauty's source code is not distributed with `pynauty`, however it is
automatically downloaded during the building process. 

To build and test the package:

```bash
make pynauty
make tests
```

Here is how to create a virtualenv and install `pynauty` into it.

```bash
% make virtenv-create
python3 -m venv .venv-pynauty
Created virtualenv: .venv-pynauty
To activate it type: source ./.venv-pynauty/bin/activate
% source .venv-pynauty/bin/activate
(.venv-pynauty) % make virtenv-ins
(.venv-pynauty) % make virtenv-tests
...
```

You can install `pynauty` into an already existing virtualenv just skip
the creation step above and activate your own virtualenv.

```bash
% source /.../my-virtenv/bin/activate
(my-virtenv) % make virtenv-ins
(my-virtenv) % make virtenv-tests
...
```

It is also possible to install the package in `~/.local` (home install) then
generate the documentation. Building the documentation from source requires the
Sphinx package.

```bash
make user-ins
make docs
```

Invoking `make` without arguments will list many more targets most of
them you unlikely need to use directly.

## Documentation

For detailed documentation, API and User's Guide see the `docs/`
directory after building as above.

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

Pynauty is distributed under the terms of GPL v3 WITHOUT ANY WARRANTY.
For exact details on licensing see the file COPYING.

Please note, Nauty is covered by its own licensing terms. For details,
see [Nauty's web-site](https://pallini.di.uniroma1.it/).
