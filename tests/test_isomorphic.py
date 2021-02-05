#!/usr/bin/env python
from __future__ import print_function, absolute_import

import sys
import copy
from pynauty import isomorphic, delete_random_edge
import pytest


def test_isomorphic(graph):
    print('Testing pynauty.{isomorphic(),certificate()}')
    print('Python version: ' + sys.version)
    print('Starting ...')
    passed = 0
    failed = 0
    gname, g, numorbit, grpsize, gens = graph
    print('%-37s ...' % gname, end=' ')
    sys.stdout.flush()
    x =copy.deepcopy(g)
    assert isomorphic(g,x)
    e = delete_random_edge(x)
    print('    removed random edge {:<13} ...'.format(str(e)), end=' ')
    assert not isomorphic(g,x)
