# Pynauty 

__Version: 0.7__

Pynauty can be used to compare graphs for isomorphism and to determine
their automorphism group.  Pynauty is a Python/C extension module using
library components from the Nauty package by Brendan McKay.

## Installation

### Requirements

  - Python 3.5 - 3.9
  - An ANSI C compiler 
  - The most recent version of [Nauty](https://cs.anu.edu.au/people/Brendan.McKay/nauty/)

### Building

```shell
make pynauty
```

For detailed instructions for installation see the documentation.

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

For documentation, including instructions for  installation, API and
User's Guide see the docs/ directory.

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

Pynauty is distributed under the terms GPL v3 WITHOUT ANY WARRANTY.
For exact details on licensing see the file COPYING.

_Copyright (c) 2015-2021 Peter Dobsan_
