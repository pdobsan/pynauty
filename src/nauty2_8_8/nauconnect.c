/* nauconnect.c - version 1.0, Sep 2023, Brendan McKay.
   Routines for vertex and edge connectivity of graphs and digraphs. */

#include "gtools.h"
#include "gutils.h"
#include "nauconnect.h"

/********************************************************************
* Routines for vertex connectivity, case n <= WORDSIZE, m=1.
********************************************************************/

static boolean
vaugpath1(graph *g, graph *paths, int n, int s, int t)
/* Find a vertex-augumenting path from s to t in g.
   paths gives the existing paths in reverse order.
   If successful, update paths and return TRUE.
   Else return FALSE.  Digraphs are ok.
   n <= WORDSIZE is required. */
{
    setword w,Lvisited,Rvisited;
    int u,v,*head,*tail;
    int queue[2*WORDSIZE];
    int Lparent[WORDSIZE],Rparent[WORDSIZE];
    boolean left;

    Lvisited = Rvisited = bit[s];
    head = tail = queue;
    *(tail++) = -1 - s;

    while (head < tail)
    {
        u = *(head++);
        if (u >= 0)   /* left */
        {
            if (paths[u] == 0)
            {
                if (!(Rvisited & bit[u]))
                {
                    *(--head) = -1 - u;
                    Rvisited |= bit[u];
                    Rparent[u] = u;
                }
            }
            else
            {
                v = FIRSTBITNZ(paths[u]); 
                if (!(Rvisited & bit[v]))
                {
                    *(tail++) = -1 - v;
                    Rvisited |= bit[v];
                    Rparent[v] = u;
                }
            }
        }
        else          /* right */
        {
            u = -1 - u;
            if (paths[u] != 0 && !(Lvisited & bit[u]))
            {
                *(tail++) = u;
                Lvisited |= bit[u];
                Lparent[u] = u;
            }
            w = g[u] & ~Lvisited;
            while (w)
            {
                TAKEBIT(v,w);
                if (!(paths[v]&bit[u]))
                {
                    *(tail++) = v;
                    Lvisited |= bit[v];
                    Lparent[v] = u;
                }
            }
            if ((bit[t] & Lvisited)) break;
        }
    }

    if (!(bit[t] & Lvisited)) return FALSE;

    u = t;
    left = TRUE;
    while (left || u != s)
    {
        if (left) v = Lparent[u];
        else      v = Rparent[u];

        if (u != v)
        {
            if ((paths[v] & bit[u])) paths[v] &= ~bit[u];
            else                     paths[u] ^= bit[v];
        }

        u = v;
        left = !left;
    }
    return TRUE;
}

int
maxvertexflow1(graph *g, int n, int s, int t, int maxneeded, boolean digraph)
/* Find the maximum number of vertex-disjoint paths from s to t.
   Use digraph=TRUE if there are directed edges.
   Don't try to find more than maxneeded paths.
   Only for n <= WORDSIZE. */
{
    graph paths[WORDSIZE];
    int i,k,d;

    d = POPCOUNT(g[s]);
    if (d < maxneeded) maxneeded = d;
    if (!digraph)
    {
        d = POPCOUNT(g[t]);  /* here is where digraphs are not allowed */
        if (d < maxneeded) maxneeded = d;
    }

    for (i = 0; i < n; ++i) paths[i] = 0;

    for (k = 0; k < maxneeded; ++k)
        if (!vaugpath1(g,paths,n,s,t)) return k;

    return maxneeded;
}

static int
connectivity1(graph *g, int n)
/* Find the vertex connectivity of g. Case of n <= WORDIZE.
   Use diconnectivity1() instead for digraphs.
   Algorithm: Even p126. */
{
    int c,j,k,i,v,d,mind,f;
    setword w;

    mind = n+1;
    for (i = 0; i < n; ++i)
    {
        d = POPCOUNT(g[i] & ~bit[i]);
        if (d < mind)
        {
            v = i;
            mind = d;
        }
    }

    if (mind == n-1) return n-1;  /* complete graph */

    c = mind;

    for (k = 1; k <= c; ++k)
    {   
        if (k == 1)
        {
            i = v;
            w = (ALLMASK(n) ^ g[i]) & ~bit[v];
        }
        else if (k <= v+1)
        {
            i = k-2;
            w = (ALLMASK(n) ^ g[i]) & ~ALLMASK(i+1) & ~bit[v];
        }
        else
        {
            i = k-1;
            w = (ALLMASK(n) ^ g[i]) & ~ALLMASK(i+1) & ~bit[v];
        }
 
        while (w)
        {
            TAKEBIT(j,w);
            f = maxvertexflow1(g,n,i,j,c,FALSE);
            if (f < c) c = f;
            if (k > c) break; 
        }
    }

    return c; 
}

static int
diconnectivity1(graph *g, int n)
/* Find the vertex connectivity of g.  Digraphs are ok.
   Algorithm: Even p126. */
{
    int c,j,k,i,v,d,mind,f;
    setword w;

    mind = n+1;
    for (i = 0; i < n; ++i)
    {
        d = POPCOUNT(g[i] & ~bit[i]);
        if (d < mind)
        {
            v = i;
            mind = d;
        }
    }

    if (mind == n-1) return n-1;  /* complete graph */

    c = mind;

    for (k = 1; k <= c; ++k)
    {   
        if (k == 1)
        {
            i = v;
            w = ALLMASK(n) & ~bit[v];
        }
        else if (k <= v+1)
        {
            i = k-2;
            w = ALLMASK(n) & ~ALLMASK(i+1) & ~bit[v];
        }
        else
        {
            i = k-1;
            w = ALLMASK(n) & ~ALLMASK(i+1) & ~bit[v];
        }
 
        while (w)
        {
            TAKEBIT(j,w);
            if (!(g[i] & bit[j]))
            {
                f = maxvertexflow1(g,n,i,j,c,TRUE);
                if (f < c) c = f;
                if (k > c) break; 
            }
            if (!(g[j] & bit[i]))
            {
                f = maxvertexflow1(g,n,j,i,c,TRUE);
                if (f < c) c = f;
                if (k > c) break; 
            }
        }
    }

    return c; 
}

/********************************************************************
* Routines for vertex connectivity, general case.
********************************************************************/

static boolean
vaugpath(graph *g, graph *paths, int m, int n,
   set *visited, int *queue, int *parent, int s, int t)
/* Find a vertex-augumenting path from s to t in g.
   paths gives the existing paths in reverse order.
   If successful, update paths and return TRUE.
   Else return FALSE.  Digraphs are ok.

   visited is work space of 2*m setwords.
   queue and parent are work space of 2*n ints each. */
{
    setword w;
    int u,v,*head,*tail;
    int i,ii,j;
    boolean left;
    set *pu,*gu;

    EMPTYSET(visited,2*m);
    ADDELEMENT(visited,s);
    ADDELEMENT(visited+m,s);
    head = tail = queue;
    *(tail++) = -1 - s;

    while (head < tail)
    {
        u = *(head++);
        if (u >= 0)   /* left */
        {
            pu = paths + m*u;
            for (i = 0; i < m; ++i) if (pu[i] != 0) break;
            if (i == m)
            {
                if (!ISELEMENT(visited+m,u))
                {
                    *(--head) = -1 - u;
                    ADDELEMENT(visited+m,u);
                    parent[u+n] = u;
                }
            }
            else
            {
                v = WORDSIZE*i + FIRSTBITNZ(pu[i]);
                if (!ISELEMENT(visited+m,v))
                {
                    *(tail++) = -1 - v;
                    ADDELEMENT(visited+m,v);
                    parent[v+n] = u;
                }
            }
        }
        else          /* right */
        {
            u = -1 - u;
            pu = paths + m*u;
            for (i = 0; i < m; ++i) if (pu[i] != 0) break;

            if (i < m && !ISELEMENT(visited,u))
            {
                *(tail++) = u;
                ADDELEMENT(visited,u);
                parent[u] = u;
            }
            gu = g + u*m;
            for (i = ii = 0; i < m; ++i, ii += WORDSIZE)
            {
                w = gu[i] & ~visited[i];
                while (w)
                {
                    TAKEBIT(j,w);
                    v = ii + j;
                    if (!ISELEMENT(paths+v*m,u))
                    {
                        *(tail++) = v;
                        ADDELEMENT(visited,v);
                        parent[v] = u;
                    }
                }
            }
            if (ISELEMENT(visited,t)) break;
        }
    }

    if (!ISELEMENT(visited,t)) return FALSE;

    u = t;
    left = TRUE;
    while (left || u != s)
    {
        if (left) v = parent[u];
        else      v = parent[u+n];

        if (u != v)
        {
            if (ISELEMENT(paths+v*m,u)) { DELELEMENT(paths+v*m,u); }
            else                        { FLIPELEMENT(paths+u*m,v); }
        }

        u = v;
        left = !left;
    }
    return TRUE;
}

int
maxvertexflow(graph *g, graph *paths, set *visited, int *queue, int *parent,
              int m, int n, int s, int t, int maxneeded, boolean digraph)
/* Find the maximum number of vertex-disjoint paths from s to t.
   Use digraph=TRUE if there are directed edges.
   Don't try to find more than maxneeded paths.

   paths is work space of m*n setwords.
   visited is work space of 2*m setwords.
   queue and parent are work space of 2*n ints each. */
{
    int i,k,d;
    graph *gs,*gt;

    gs = g + s*m;
    d = 0;
    for (i = 0; i < m; ++i) d += POPCOUNT(gs[i]);

    if (d < maxneeded) maxneeded = d;

    if (!digraph)
    {
        gt = g + t*m;
        d = 0;
        for (i = 0; i < m; ++i) d += POPCOUNT(gt[i]);
        if (d < maxneeded) maxneeded = d;
    }

    EMPTYGRAPH(paths,m,n);

    for (k = 0; k < maxneeded; ++k)
        if (!vaugpath(g,paths,m,n,visited,queue,parent,s,t)) return k;

    return maxneeded;
}

int
connectivity(graph *g, int m, int n, boolean digraph)
/* Determine the vertex connectivty.  It is n-1 for complete graphs,
   and otherwise the smallest separator of two non-adjacent vertices.
   Use digraph=TRUE for digraphs.  See Even lemma 6.6. */
{
    graph *paths;
    int *parent_queue;
    set *visited;
    int i,j,d,mind,v,x,c,k,jmin,f;
    set *gi,*gx;

    if (m == 1)
    {
        if (digraph) return diconnectivity1(g,n);
        else         return connectivity1(g,n);
    }

    if (n > NAUTY_INFINITY/2)
        gt_abort(">E connectivity() requires m <= NAUTY_INFINITY/2\n");

    mind = n+1;
    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        d = 0;
        for (j = 0; j < m; ++j) d += POPCOUNT(gi[j]);
        if (ISELEMENT(gi,i)) --d;
        if (d < mind)
        {
            mind = d;
            v = i;
        }
    }

    if (mind == 0) return 0;
    if (mind == n-1) return n-1;  /* complete graph */
 
    if ((paths = malloc(m*sizeof(graph)*n)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((parent_queue = malloc(4*sizeof(int)*n)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((visited = malloc(2*sizeof(set)*m)) == NULL) 
        gt_abort(">E edgeconnectivity: malloc failed\n");

    c = mind;

    for (k = 1; k <= c; ++k)
    {   
        if (k == 1)        x = v;
        else if (k <= v+1) x = k - 2;
        else               x = k - 1;

        if (k == 1) jmin = 0;
        else        jmin = x + 1;
        gx = g + x*m;

        for (j = jmin; j < n; ++j)
        {
            if (j == v) continue;

            if (!ISELEMENT(gx,j))
            {
                f = maxvertexflow(g,paths,visited,parent_queue,
                                    parent_queue+2*n,m,n,x,j,c,digraph);
                if (f < c) c = f;
                if (k > c) break; 
            }
            if (digraph && !ISELEMENT(g + j*m,x))
            {
                f = maxvertexflow(g,paths,visited,parent_queue,
                                    parent_queue+2*n,m,n,j,x,c,digraph);
                if (f < c) c = f;
                if (k > c) break; 
            }
        }
    }

    free(visited); free(parent_queue); free(paths);

    return c; 
}

/********************************************************************
* Lower bounds for vertex connectivity, case n <= WORDSIZE-1, m=1.
********************************************************************/

static boolean
isthisconnected1(graph *g, int n, int c)
/* Test if g is c-vertex connected. For c >= 3, the case n=WORDSIZE
   is not available, since an auxiliary vertex is needed.
   Use isthisdiconnected1() instead for digraphs.
   Algorithm: Even p129. */
{
    graph aux[WORDSIZE];
    int i,j;

    if (c == 0) return TRUE;
    if (c > n-1) return FALSE;
    if (c == 1) return isconnected1(g,n);
    if (c == 2) return isbiconnected1(g,n);

    for (i = 0; i < c; ++i)
    for (j = i+1; j < c; ++j)
        if (maxvertexflow1(g,n,i,j,c,FALSE) < c) return FALSE;

    for (i = 0; i < n; ++i) aux[i] = g[i];
    aux[n] = ALLMASK(c-1);
    for (i = 0; i < c-1; ++i) aux[i] |= bit[n];

    for (j = c; j < n; ++j)
    {
        aux[n] |= bit[j-1];
        aux[j-1] |= bit[n];
        if (maxvertexflow1(aux,n+1,n,j,c,FALSE) < c) return FALSE;
    }

    return TRUE;
}

static boolean
isthisdiconnected1(graph *g, int n, int c)
/* Test if g is c-vertex connected. For c >= 3, the case n=WORDSIZE
   is not available, since an auxiliary vertex is needed.
   Digraphs ok.  Algorithm: Even p129. */
{
    graph aux[WORDSIZE];
    int i,j;

    if (c == 0) return TRUE;
    if (c > n-1) return FALSE;

    for (i = 0; i < c; ++i)
    for (j = i+1; j < c; ++j)
    {
        if (maxvertexflow1(g,n,i,j,c,TRUE) < c) return FALSE;
        if (maxvertexflow1(g,n,j,i,c,TRUE) < c) return FALSE;
    }

    for (i = 0; i < n; ++i) aux[i] = g[i];
    aux[n] = ALLMASK(c-1);
    for (i = 0; i < c-1; ++i) aux[i] |= bit[n];

    for (j = c; j < n; ++j)
    {
        aux[n] |= bit[j-1];
        aux[j-1] |= bit[n];
        if (maxvertexflow1(aux,n+1,j,n,c,TRUE) < c) return FALSE;
        if (maxvertexflow1(aux,n+1,n,j,c,TRUE) < c) return FALSE;
    }

    return TRUE;
}

/********************************************************************
* Lower bounds for vertex connectivity, general case
********************************************************************/

boolean
isthisconnected(graph *g, int m, int n, int c, boolean digraph)
/* Test if the graph or digraph is at least c vertex-connected. */
{
    int mp,np,i,j;
    graph *aux,*gi,*auxi,*paths;
    int *parent_queue;
    set *visited;
    boolean ans;

    if (c == 0) return TRUE;
    if (c > n-1) return FALSE;
    if (!digraph)
    {
        if (c == 1) return isconnected(g,m,n);
        if (c == 2) return isbiconnected(g,m,n);
    }
    else
    {
        if (c == 1) return stronglyconnected(g,m,n);
    }

    if (m == 1 && n < WORDSIZE)
    {
        if (digraph) return isthisdiconnected1(g,n,c);
        else         return isthisconnected1(g,n,c);
    }

    np = n + 1;
    mp = SETWORDSNEEDED(np);
    if (mp < m) mp = m;

    if ((paths = malloc(mp*sizeof(graph)*np)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((parent_queue = malloc(4*sizeof(int)*np)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((visited = malloc(2*sizeof(set)*mp)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");

    ans = TRUE;
    for (i = 0; i < c && ans; ++i)
    for (j = i+1; j < c; ++j)
    {
        if (maxvertexflow(g,paths,visited,parent_queue,
                             parent_queue+2*n,m,n,i,j,c,digraph) < c)
        {
            ans = FALSE;
            break;
        }
        if (digraph && maxvertexflow(g,paths,visited,parent_queue,
                             parent_queue+2*n,m,n,j,i,c,digraph) < c)
        {
            ans = FALSE;
            break;
        }
    } 

    if (!ans)
    {
        free(visited); free(parent_queue); free(paths);
        return FALSE;
    }

    if ((aux = malloc(mp*sizeof(graph)*np)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    
    EMPTYGRAPH(aux,mp,np);
    for (i = 0, gi = g, auxi = aux; i < n; ++i, gi += m, auxi += mp)
        for (j = 0; j < m; ++j) auxi[j] = gi[j];

    for (i = 0; i < c-1; ++i) ADDONEEDGE(aux,i,n,mp);

    for (j = c; j < n; ++j)
    {
        ADDONEEDGE(aux,j-1,n,mp);
        if (maxvertexflow(aux,paths,visited,parent_queue,
                             parent_queue+2*np,mp,np,j,n,c,digraph) < c)
        {
            ans = FALSE;
            break;
        }
        if (digraph && maxvertexflow(aux,paths,visited,parent_queue,
                             parent_queue+2*np,mp,np,n,j,c,digraph) < c)
        {
            ans = FALSE;
            break;
        }
    }

    free(aux); free(visited); free(parent_queue); free(paths);
    return ans;
}

/********************************************************************
* Routines for edge connectivity, case n <= WORDSIZE, m=1.
********************************************************************/

static boolean
eaugpath1(graph *g, graph *paths, int n, int s, int t)
/* Find an edge-augumenting path from s to t in g.
   g may be a digraph.
   paths gives the existing paths in reverse order.
   If successful, update paths and return TRUE.
   Else return FALSE. */
{
    setword w,visited;
    int u,v,*head,*tail;
    int queue[WORDSIZE];
    int parent[WORDSIZE];

    visited = bit[s];
    head = tail = queue;
    *(tail++) = s;

    while (head < tail)
    {
        u = *(head++);
        w = (g[u] | paths[u]) & ~visited; 
        while (w)
        {
            TAKEBIT(v,w);
            if (!(paths[v]&bit[u]))
            {
                visited |= bit[v];
                *(tail++) = v;
                parent[v] = u;
            }
        }
        if ((bit[t] & visited)) break;
    }

    if (!(bit[t] & visited)) return FALSE;

    u = t;
    while (u != s)
    {
        v = parent[u];
        if ((paths[v] & bit[u])) paths[v] &= ~bit[u];
        else                     paths[u] ^= bit[v];
        u = v;
    }
    return TRUE;
}

int
maxedgeflow1(graph *g, int n, int s, int t, int maxneeded)
/* Find the maximum number of edge-disjoint paths from s to t.
   Digraphs are ok.  Don't try to find more than maxneeded. */
{
    graph paths[WORDSIZE];
    int i,k,d;

    d = POPCOUNT(g[s]);
    if (d < maxneeded) maxneeded = d;

    if (d < maxneeded) maxneeded = d;
    for (i = 0; i < n; ++i) paths[i] = 0;

    for (k = 0; k < maxneeded; ++k)
        if (!eaugpath1(g,paths,n,s,t)) return k;

    return maxneeded;
}

static int
edgeconnectivity1(graph *g, int n)
/* Find the edge connectivity. Digraphs are ok.
   Algorithm: Even p131. */
{
    int d,i,u,v,c,f;
    
    c = n;
    for (i = 0; i < n; ++i)
    {
        d = POPCOUNT(g[i] & ~bit[i]);
        if (d < c)
        {
            u = i;
            c = d;
        }
    }

    if (c == 0) return 0;

    for (i = 0; i < n; ++i)
    {
        v = (u == n-1 ? 0 : u+1);
        f = maxedgeflow1(g,n,u,v,c);
        if (f < c) c = f;
        u = v;
    }

    return c;
}

/********************************************************************
* Routines for edge connectivity, general n.
********************************************************************/

static boolean
eaugpath(graph *g, graph *paths, int m, int n,
          set *visited, int *queue, int *parent, int s, int t)
/* Find an edge-augumenting path from s to t in g.
   g may be a digraph.
   queue[] and parent[] need length n, and visited[] length m.
   paths gives the existing paths in reverse order.
   If successful, update paths and return TRUE.
   Else return FALSE.  Use eaugpath1() instead if m=1.

   visited is work space of m setwords.
   parent and queue are work space of n ints each. */
{
    setword w;
    int u,v,*head,*tail;
    int i,ii,j;
    set *gu,*pu;

    EMPTYSET(visited,m);
    ADDELEMENT(visited,s);
    head = tail = queue;
    *(tail++) = s;

    while (head < tail)
    {
        u = *(head++);
        gu = g + m*u;
        pu = paths + m*u;
        
        for (i = ii = 0; i < m; ++i, ii += WORDSIZE)
        {
            w = (gu[i] | pu[i]) & ~visited[i];
            while (w)
            {
                TAKEBIT(j,w);
                v = ii + j;
                if (!ISELEMENT(paths+v*m,u))
                {
                    ADDELEMENT(visited,v);
                    *(tail++) = v;
                    parent[v] = u;
                }
            }
        }
        if (ISELEMENT(visited,t)) break;
    }

    if (!ISELEMENT(visited,t)) return FALSE;

    u = t;
    while (u != s)
    {
        v = parent[u];
        if (ISELEMENT(paths+v*m,u)) { DELELEMENT(paths+v*m,u); }
        else                        { FLIPELEMENT(paths+u*m,v); }
        u = v;
    }
    return TRUE;
}

int
maxedgeflow(graph *g, graph *paths, int m, int n, int s, int t,
             set *visited, int *queue, int *parent, int maxneeded)
/* Find the maximum number of edge-disjoint paths from s to t.
   Digraphs are ok.  Don't try to find more than maxneeded.

   paths is work space of m*n setwords.
   visited is work space of m setwords.
   queue and parent are work space of n ints each. */
{
    int i,k,d;
    set *gs;

    gs = g + s*m;
    d = 0;
    for (i = 0; i < m; ++i) d += POPCOUNT(gs[i]);

    if (d < maxneeded) maxneeded = d;

    EMPTYGRAPH(paths,m,n);

    for (k = 0; k < maxneeded; ++k)
    {
        if (!eaugpath(g,paths,m,n,visited,queue,parent,s,t)) return k;
    }

    return maxneeded;
}

int
edgeconnectivity(graph *g, int m, int n)
/* Find the edge connectivity. Digraphs are ok. */
{
    graph *paths,*gi;
    int *parent_queue;
    set *visited;
    int i,j,d,c,f,u,v;

    if (m == 1) return edgeconnectivity1(g,n);

    c = n;
    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        d = 0;
        for (j = 0; j < m; ++j) d += POPCOUNT(gi[j]);
        if (ISELEMENT(gi,i)) --d;
        if (d < c)
        {
            u = i;
            c = d;
            if (d == 0) return 0;
        }
    }

    if ((paths = malloc(m*sizeof(graph)*n)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((parent_queue = malloc(2*sizeof(int)*n)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((visited = malloc(m*sizeof(set))) == NULL) 
        gt_abort(">E edgeconnectivity: malloc failed\n");

    for (i = 0; i < n; ++i)
    {
        v = (u == n-1 ? 0 : u+1);
        f = maxedgeflow(g,paths,m,n,u,v,
                        visited,parent_queue,parent_queue+n,c);
        if (f < c) c = f;
        u = v;
    }

    free(visited); free(parent_queue); free(paths);

    return c;
}

/********************************************************************
* Lower bound on edge connectivity, case n <= WORDSIZE, m=1.
********************************************************************/

static boolean
isthisedgeconnected1(graph *g, int n, int c)
/* Test if g is c-edge connected, case of m=1. Digraphs are ok.
   Algorithm: Even p131. */
{
    int d,i,mind,u,v;
    
    mind = n;
    for (i = 0; i < n; ++i)
    {
        d = POPCOUNT(g[i] & ~bit[i]);
        if (d < mind)
        {
            u = i;
            mind = d;
        }
    }

    if (mind < c) return FALSE;

    for (i = 0; i < n; ++i)
    {
        v = (u == n-1 ? 0 : u+1);
        if (maxedgeflow1(g,n,u,v,c) < c) return FALSE;
        u = v;
    }

    return TRUE;
}

/********************************************************************
* Lower bound on edge connectivity, general case.
********************************************************************/

boolean
isthisedgeconnected(graph *g, int m, int n, int c)
/* Test if g is c-edge connected. Digraphs are ok.
   Algorithm: Even p131. */
{
    graph *paths,*gi;
    int *parent_queue;
    set *visited;
    int i,j,d,mind,u,v;
    boolean ans;

    if (m == 1) return isthisedgeconnected1(g,n,c);

    mind = n;
    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        d = 0;
        for (j = 0; j < m; ++j) d += POPCOUNT(gi[j]);
        if (ISELEMENT(gi,i)) --d;
        if (d < mind)
        {
            u = i;
            mind = d;
            if (d == 0) return (c >= 0);
        }
    }

    if (mind < c) return FALSE;

    if ((paths = malloc(m*sizeof(graph)*n)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((parent_queue = malloc(2*sizeof(int)*n)) == NULL)
        gt_abort(">E edgeconnectivity: malloc failed\n");
    if ((visited = malloc(m*sizeof(set))) == NULL) 
        gt_abort(">E edgeconnectivity: malloc failed\n");

    ans = TRUE;
    for (i = 0; i < n; ++i)
    {
        v = (u == n-1 ? 0 : u+1);
        if (maxedgeflow(g,paths,m,n,u,v,
                    visited,parent_queue,parent_queue+n,c) < c)
        {
            ans = FALSE;
            break;
        }
        u = v;
    }

    free(visited); free(parent_queue); free(paths);

    return ans;
}
