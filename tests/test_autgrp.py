#!/usr/bin/env python
from __future__ import print_function

import sys
from pynauty import autgrp
import pytest

# List of graphs for testing
#
# Structure:
#   [[name, Graph, numorbit, grpsize, generators]]
#
# numorbit, grpsize, generators was calculated by dreadnut
#

def test_autgrp(graph):
    gname, g, numorbit, grpsize, gens = graph
    print('%-17s ...' % gname, end=' ')
    sys.stdout.flush()
    generators, order, o2, orbits, orbit_no = autgrp(g)
    assert generators == gens and orbit_no == numorbit and order == grpsize
