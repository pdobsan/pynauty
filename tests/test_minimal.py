#!/usr/bin/env python3

import sys
import os

def test_minimal():
    print()
    print('Python system prefix:', sys.prefix)
    print('Virtualenv:', os.environ.get('VIRTUAL_ENV'))
    
    import pynauty
    
    print('Pynauty module file:', pynauty.__file__)
    print()
    
    g = pynauty.Graph(number_of_vertices=5, directed=False,
         adjacency_dict = {
          0: [1, 2, 3],
          2: [1, 3, 4],
          4: [3],
         },
         vertex_coloring = [
         ],
        )
    
    print('Graph g:', g)
    print('automorphism group of g:', pynauty.autgrp(g))
    print()
    
    h = g.copy()
    h.connect_vertex(1, [3])
    print('Graph h:', h)
    print('automorphism group of h:', pynauty.autgrp(h))
    print()
    
    print('Is g and h isomorphic:', pynauty.isomorphic(g,h))
    print()
    
    h.set_vertex_coloring([set([3])])
    print('vertex colored h:', h)
    print('automorphism group of colored h:', pynauty.autgrp(g))
    
    print()
    print('Computed by', pynauty.Version())
    print()

if __name__ == '__main__':
    test_minimal()
