User's Guide
===============

The **pynauty** module provides a Graph class to represent graphs and
functions for isomorphism testing and computing automorphism groups of
graphs. The graphs can be undirected or directed. They can contain loops
but no multiple edges. There is always a vertex-coloring associated with
them. Ordinary, that is not vertex-colored, graphs can be represented
with all vertices having the same color.


Vertex Coloring
---------------

Let V be the set of vertices of a graph G. A partition of vertices is a
collection of non-empty and pairwise disjoint subsets (parts) of V such
that the union of these subsets is V. A vertex-coloring defines a
partition of vertices, with vertices of the same color belonging to the
same part. Similarly, a partition of vertices defines a vertex-coloring
by giving the same distinct color to vertices in the same part. So
vertex-coloring and partition of vertices are essentially the same.

In **pynauty**, a vertex-coloring is specified as an ordered partition
of vertices. The order of parts is significant but the order of vertices
within each part is not. Such an ordered partition is represented by a
list of sets where each set in the list specifies a subset of the vertex
set. So the color of a vertex is the index of the part containing the
vertex. The subsets must be non-empty and pairwise disjoint. It is not a
requirement to cover all vertices, all the vertices not appearing are
put together in a single part of their own.

The significance of vertex-coloring while computing with graphs in
pynauty is the following. The partition induced by a vertex coloring
imposes the restriction on possible automorphisms of the graph that
vertices must stay in their original part (i.e. keep their color) under
any automorphism.

Two vertex-colored graphs are *isomorphic* if there is a bijection between
their vertex sets which preserves adjacency and color.


Classes
-------

.. module:: pynauty
.. autoclass:: Graph
    :members:


Functions
---------

.. autofunction:: autgrp
.. autofunction:: isomorphic
.. autofunction:: certificate


Examples
--------

These examples show using **pynauty** in interactive sessions.  We
assume that the **pynauty** module has been imported by::

    >>> from pynauty import *

Create a Graph object by connetcting some vertices step by step::

    >>> g = Graph(5)
    >>> g.connect_vertex(0, [1, 2, 3])
    >>> g.connect_vertex(2, [1, 3, 4])
    >>> g.connect_vertex(4, [3])
    >>> 
    >>> print g
    Graph(number_of_vertices=5, directed=False,
     adjacency_dict = {
      0: [1, 2, 3],
      2: [1, 3, 4],
      4: [3],
     },
     vertex_coloring = [
     ],
    )

The same graph can be created in one step by supplying the adjacency
relations to Graph::

    >>> g = Graph(number_of_vertices=5, directed=False,
    ... adjacency_dict = {
    ...  0: [1, 2, 3],
    ...  2: [1, 3, 4],
    ...  4: [3],
    ...  },
    ... )
    >>> 

Compute the automorphism group of the graph::

    >>> autgrp(g)
    ([[3, 4, 2, 0, 1]], 2.0, 0, [0, 1, 2, 0, 1], 3)
    >>> 

Let's add a new edge and see how the automorphism group would change::

    >>> g.connect_vertex(1, [3])
    >>> print g
    Graph(number_of_vertices=5, directed=False,
     adjacency_dict = {
      0: [1, 2, 3],
      1: [3],
      2: [1, 3, 4],
      4: [3],
     },
     vertex_coloring = [
     ],
    )
    >>> autgrp(g)
    ([[0, 1, 3, 2, 4], [1, 0, 2, 3, 4]], 4.0, 0, [0, 0, 2, 2, 4], 3)
    >>>

Fixing vertex 3 by coloring reduces the automorphism group::

    >>> g.set_vertex_coloring([set([3])])
    >>> print g
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

