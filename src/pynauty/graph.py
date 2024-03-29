'''
    graph.py

Module graph contains the definition of the Graph class
and utilities dealing with graph objects.
'''

__LICENSE__ = '''
Copyright (c) 2015-2023 Peter Dobsan

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.  This program is distributed in the hope that
it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
'''

__all__ = [
    'Graph',
    'autgrp',
    'isomorphic',
    'certificate',
    'canon_label',
    'canon_graph',
    'delete_random_edge',
]

from . import nautywrap
import copy
import random


class Graph(object):
    '''
    Graph instantiates an adjacency dictionary based graph object.
    It can represent vertex colored, directed or undirected graphs.
    '''

    def __init__(self, number_of_vertices, directed=False,
                 adjacency_dict={},
                 vertex_coloring=[]):
        '''
        *number_of_vertices*
            The number of vertices of the graph; the vertices are
            labeled from zero.  Mandatory argument.

        *directed*
            Indicate wether the grap is directed or not.  Optional,
            default is False.

        *adjacency_dict*
            key: a vertex, value: a list of vertices linked to the
            key vertex.  Optional, default is an empty dictionary.

        *vertex_coloring*
            A list of disjoint sets of vertices representing a
            partition of the vertex set; vertices not listed are
            placed into a single additional part.  Optional, default
            is no coloring.
        '''
        self.number_of_vertices = number_of_vertices
        self.directed = directed
        self.set_adjacency_dict(adjacency_dict)
        self.set_vertex_coloring(vertex_coloring)

    def _check_vertices(self, vs):
        for v in vs:
            if not (0 <= v and v < self.number_of_vertices):
                raise ValueError(
                    'vertex %d conflicts with number_of_vertices=%d' %
                    (v, self.number_of_vertices))

    def _get_adjacency_dict(self):
        return self._adjacency_dict

    adjacency_dict = property(_get_adjacency_dict)

    def set_adjacency_dict(self, adjacency_dict):
        '''
        Set the adjacency relations of the Graph.

        *adjacency_dict*
            key: a vertex, value: a list of vertices linked to the
            key vertex. Duplicate vertices will be removed, since there is no
            multigraph support.
        '''
        for v, vs in adjacency_dict.items():
            self._check_vertices([v])
            self._check_vertices(vs)
        self._adjacency_dict = dict([(k,list(set(vs)))
                                     for k,vs in adjacency_dict.items()])

    def connect_vertex(self, v, neighbors):
        '''
        Connect a vertex to some other vertices.

        *v*
            A vertex of the Graph. The *tail* of the arcs if the Graph
            is directed.
        *neighbors*
            A vertex or a list of vertices to which *v* should be connected.
            The *heads* of the arcs if the Graph is directed. Duplciate
            vertices are removed, since there is no multigraph support.

        '''
        self._check_vertices([v])
        self._adjacency_dict.setdefault(v, [])
        if isinstance(neighbors, list):
            self._check_vertices(neighbors)
            self._adjacency_dict[v].extend(neighbors)
        else:
            self._check_vertices([neighbors])
            self._adjacency_dict[v].append(neighbors)
        self._adjacency_dict[v] = list(set(self._adjacency_dict[v]))

    def _get_vertex_coloring(self):
        return self._vertex_coloring

    vertex_coloring = property(_get_vertex_coloring)

    def set_vertex_coloring(self, vertex_coloring):
        '''
        Define a vertex coloring of the Graph.

        *vertex_coloring*
            A list of disjoint sets of vertices representing a
            partition of the vertex set; vertices not listed are
            placed into a single additional part.
        '''
        self._vertex_coloring = []
        if vertex_coloring:
            vs = set(range(self.number_of_vertices))
            for p in vertex_coloring:
                if p <= vs:
                    self._vertex_coloring.append(p)
                    vs -= p
                else:
                    raise ValueError('Invalid partition: %s' % vertex_coloring)
            if vs:
                self._vertex_coloring.append(vs)
            if len(self._vertex_coloring) == 1:
                self._vertex_coloring = []

    def copy(self):
        '''
        Make a copy of the Graph.
        '''
        return copy.deepcopy(self)

    def __repr__(self):
        s = ['Graph(number_of_vertices=%d, directed=%s,' %
             (self.number_of_vertices, self.directed)]
        s.append(' adjacency_dict = {')
        for k, v in self._adjacency_dict.items():
            v.sort()
            s.append('  %d: %s,' % (k, v))
        s.append(' },')
        s.append(' vertex_coloring = [')
        for x in self._vertex_coloring:
            s.append('  set(%s),' % list(x))
        s.append(' ],')
        s.append(')')
        return '\n'.join(s)


def autgrp(g):
    '''
    Compute the automorphism group of a graph.

    *g*
        A Graph object.

    return -> (generators, grpsize1, grpsize2, orbits, numorbits)
        For the detailed description of the returned components, see
        Nauty's documentation.
    '''
    if not isinstance(g, Graph):
        raise TypeError
    return nautywrap.graph_autgrp(g)


def certificate(g):
    '''
    Compute a certificate based on the canonical labeling of vertices.

    *g*
        A Graph object.

    return ->
        The certificate as a byte string.
    '''
    if not isinstance(g, Graph):
        raise TypeError
    return nautywrap.graph_cert(g)


def canon_label(g):
    '''
    Finds the canonical labeling of vertices.

    *g*
        A Graph object.

    return ->
        A list with each node relabelled.
    '''
    if not isinstance(g, Graph):
        raise TypeError
    return nautywrap.graph_canonlab(g)


# This is a temporary pure Python solution due to @rburing
# It should be implemented in C for efficiency. 
def canon_graph(g):
    '''
    Compute the canonically labeled version of graph g.

    *g*
        A Graph object.

    return ->
        new canonical graph.
    '''
    if g.vertex_coloring:
        raise RuntimeError("canon_graph() is not implemented for vertex-colored graphs yet.")
    c = certificate(g)
    set_length = len(c) // g.number_of_vertices
    sets = [c[set_length*k:set_length*(k+1)] for k in range(g.number_of_vertices)]
    neighbors = [[i for i in range(set_length * 8) if st[-1 - i//8] & (1 << (7 - i%8))] for st in sets]
    return Graph(number_of_vertices=g.number_of_vertices, directed=g.directed,
                 adjacency_dict={i: neighbors[i] for i in range(g.number_of_vertices)})


def isomorphic(a, b):
    '''
    Determine if two graphs are isomorphic.

    *a,b*
        Two Graph objects.

    return ->
        True if *a* and *b* are isomorphic graphs, False otherwise,
    '''
    if a.number_of_vertices != b.number_of_vertices:
        return False
    elif list(map(len, a.vertex_coloring)) != list(map(len, b.vertex_coloring)):
        return False
    else:
        return certificate(a) == certificate(b)


def delete_random_edge(g):
    '''
    Delete a random edge from a graph.

    *g*
        A Graph object.

    return ->
        The deleted edge as a tuple or (None, None) if no edge is left.
    '''
    if g.adjacency_dict:
        # pick a random vertex 'x' which is connected
        x = random.sample(list(g.adjacency_dict), 1)[0]
        # remove a random edge connected to 'x'
        xs = g.adjacency_dict[x]
        y = xs.pop(random.randrange(len(xs)))
        if not xs:
            g.adjacency_dict.pop(x)
        # if g is not directed make sure to remove edge completely
        if (not g.directed) and y in g.adjacency_dict:
            ys = g.adjacency_dict[y]
            if x in ys:
                ys.remove(x)
    else:
        # the graph has no edges
        x, y = None, None
    return (x, y)

