#!/usr/bin/env python

import sys
import copy
from pynauty import isomorphic, delete_random_edge, Version
import pytest


def test_isomorphic(graph):
    print(Version())
    print('Python version: ' + sys.version)
    print('Testing pynauty.{isomorphic(),certificate(),delete_random_edge(),copy()}')
    gname, g, numorbit, grpsize, gens = graph
    print('%-37s ...' % gname, end=' ')
    sys.stdout.flush()
    x = g.copy()
    assert isomorphic(g,x)
    e = delete_random_edge(x)
    print('    removed random edge {:<13} ...'.format(str(e)), end=' ')
    assert not isomorphic(g,x)
