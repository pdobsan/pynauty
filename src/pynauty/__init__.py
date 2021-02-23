'''
pynauty  --  isomorphism testing and automorphism groups of graphs

A Python extension module to Brendan McKay's "nauty" C procedures
for determining the automorphism group of a vertex colored graph
and producing canonical labeling for isomorphism testing.

Classes:

    Graph   - An adjacency dictionary based graph object.
        Graph can represent vertex colored, directed or undirected graphs.

Functions:

    autgrp      - Compute the automorphism group of a graph.
    isomorphic  - Compare two graphs for isomorphism.
    certificate - Compute a "certificate" based on the canonical labeling
                  of the graph's vertices.
'''

__LICENSE__     = '''
Copyright (c) 2015-2021 Peter Dobsan

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.  This program is distributed in the hope that
it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
'''

_nauty_version   = 'nauty27r1'
_nauty_dir       = _nauty_version   # relative to src/

_pynauty_version = '1.0b2'
__version__      = _pynauty_version
__Version__      = _pynauty_version + '+' + _nauty_version

def Version():
    '''
    Return the combined version of the pynauty package.
    '''
    return('Pynauty version ' + __Version__)

try:
    from .graph import *
except ImportError:
    pass
else:
    del graph
    del nautywrap
