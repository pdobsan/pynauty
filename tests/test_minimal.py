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
    aut_g = pynauty.autgrp(g)
    print('automorphism group of g:', aut_g)
    print()

    assert aut_g[0] == [[3, 4, 2, 0, 1]]
    assert aut_g[2] == 0
    assert aut_g[3] == [0, 1, 2, 0, 1]
    assert aut_g[4] == 3
    
    h = g.copy()
    h.connect_vertex(1, [3])
    aut_h = pynauty.autgrp(h)
    print('Graph h:', h)
    print('automorphism group of h:', aut_h)
    print()
    for a in aut_h[0]:
        assert a == [0, 1, 3, 2, 4] or a == [1, 0, 2, 3, 4]
    assert aut_h[1] == 4.0
    assert aut_h[2] == 0
    assert aut_h[3] == [0, 0, 2, 2, 4]
    assert aut_h[4] == 3

    g_h_iso =  pynauty.isomorphic(g,h)
    print('Are g and h isomorphic:', g_h_iso)
    print()
    assert g_h_iso == False
    
    h.set_vertex_coloring([set([3])])
    print('vertex colored h:', h)
    aut_h_colored = pynauty.autgrp(h)
    print('automorphism group of colored h:', aut_h_colored)
    assert aut_h_colored[0] == [[1, 0, 2, 3, 4]]
    assert aut_h_colored[1] == 2.0
    assert aut_h_colored[2] == 0
    assert aut_h_colored[3] == [0, 0, 2, 3, 4] 
    assert aut_h_colored[4] == 4
    
    print()
    print('Computed by', pynauty.Version())
    print()
    print('Minimal tests: OK')

if __name__ == '__main__':
    test_minimal()
