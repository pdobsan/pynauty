# Pynauty 

Pynauty can be used to compare graphs for isomorphism and to determine
their automorphism group in a Python programming environment.  Pynauty
is a Python/C extension module using library components from the
[Nauty](https://pallini.di.uniroma1.it/) package by Brendan McKay and
Adolfo Piperno.

## Installation

### Requirements

  - Python 3.6 - 3.9
  - An ANSI C compiler 
  - The most recent version of Nauty, currently `nauty27r1`.

### Building

Download [naughty27r1](https://pallini.di.uniroma1.it/nauty27r1.tar.gz)
source code into the `src/` subdirectory and `untar` it there.

```bash
  cd src/
  curl -O https://pallini.di.uniroma1.it/nauty27r1.tar.gz
  sha1sum -c nauty27r1.tar.gz.sha1sum
  tar xf nauty27r1.tar.gz
```

Build, test, install `pynauty` using `make` in the root directory of the
package.

```bash
  make pynauty
  make tests
  make user-ins
  make docs
```

Invoking `make` without arguments will list all possible targets.

## Usage

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
```

## Documentation

For detailed documentation, API and User's Guide see the `docs/`
directory after building as above.

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

Pynauty is distributed under the terms GPL v3 WITHOUT ANY WARRANTY.
For exact details on licensing see the file COPYING.

_Copyright (c) 2015-2021 Peter Dobsan_
