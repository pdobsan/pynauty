/* genspecialg.c  version 2.1; B D McKay, June 14, 2023 */

#define USAGE \
 "genspecialg [-s|-g|-z|-d|-v] [-q] [graph options]... [outfile]"

#define HELPTEXT \
" Generate special graphs.\n\
     #  : size parameter called n in the descriptions\n\
\n\
    -s : Write in sparse6 format (default)\n\
    -g : Write in graph6 format\n\
    -z : Make digraph versions and write in digraph6 format\n\
    -d : Write in dreadnaut format (can be used with -z)\n\
    -v : For each graph, report the size to stderr\n\
    -q : Suppress summary\n\
\n\
    If defined, the digraph version is shown in parentheses:\n\
    -p#   : path (directed path) on n vertices\n\
    -c#   : cycle (directed cycle) on n vertices\n\
    -e#   : empty graph (digraph with loops only) on n vertices\n\
    -k#   : complete graph (with loops) on n vertices\n\
    -b#,#[,#] : complete bipartite graph (directed l->r) on n vertices\n\
                   minus a matching of given size if present\n\
    -m#,#... : multipartite graph\n\
    -a#   : antiprism (directed antiprism) on 2n\n\
    -l#   : moebius ladder (directed cycle, undirected rungs) with # rungs\n\
    -D#,# : de Bruijn graph (digraph). D(m,t) has order m^t\n\
    -w#   : wheel with # spokes (directed out from hub)\n\
    -f#   : flower snark on 4n vertices\n\
    -L#   : triangular graph, the linegraph of K_n\n\
    -P#,# : generalized Petersen graph; usual one is -P5,2\n\
    -Q#  -Q#,#  : hypercube on 2^n vertices. Q(n,t) has vertices {0,1}^n\n\
                    and edges for hamming distance t.  t=1 is default.\n\
    -A#   : connected antiregular graph on n vertices\n\
    -J#,# : Johnson graph J(n,k), args are n and k\n\
    -K#,# -K#,#,# : Generalized Kneser graph. K(n,k,t) is graph of k-subsets\n\
            of an n-set whose intersection is t. Default for t is 0.\n\
    -C#,#... : circulant (di)graph; first arg is the number of vertices\n\
    -T#,#... : theta (di)graph Theta(#,#,...), give path lengths\n\
    -G#,#... : (directed) grid, use negative values for open directions\n\
    -Y#   : Paley graph or digraph, depending on size. Nondeterministic.\n\
    -X# : produce a named graph, # can be an index or a partial name.\n\
          Use genspecialg --Xhelp for a list.\n\
\n\
    Any number of graphs can be generated at once.\n"

/* Ideas: full trees, friendship, chess graphs,
   double wheels, books, Blanusa snarks. */

#include "gtools.h"
#include "namedgraphs.h"

#define MAXARGS 10000  /* Maximum argument list for multi-argument parameters */
#define SWAP(x,y) {int w = x; x = y; y = w;}

static long args[MAXARGS];

static short vmark_val = 32000;
DYNALLSTAT(short,vmark,vmark_sz);
#define MARK(i) vmark[i] = vmark_val
#define UNMARK(i) vmark[i] = 0
#define ISMARKED(i) (vmark[i] == vmark_val)
#define ISNOTMARKED(i) (vmark[i] != vmark_val)
#define RESETMARKS {if (vmark_val++ >= 32000) \
    {size_t ij; for (ij=0;ij<vmark_sz;++ij) vmark[ij]=0; vmark_val=1;}}

static void
preparemarks(size_t nn)
{
    size_t oldsize;
    short *oldpos;

    oldsize = vmark_sz;
    oldpos = vmark;
    DYNALLOC1(short,vmark,vmark_sz,nn,"preparemarks");
    if (vmark_sz != oldsize || vmark != oldpos) vmark_val = 32000;
}

/**************************************************************************/

static void
writedread_sg(FILE *f, sparsegraph *sg, boolean digraph)
/* Write in dreadnaut format */
{
    size_t *v;
    int *d,*e,n,i,j,k;

    SG_VDE(sg,v,d,e);
    n = sg->nv;

    if (digraph) fprintf(f,"n=%d $=0 dg\n",n);
    else         fprintf(f,"n=%d $=0 g\n",n);

    for (i = 0; i < n; ++i)
    {
        for (j = 0; j < d[i]; ++j)
        {
            k = e[v[i]+j];
            if (k >= i || digraph) fprintf(f," %d",k);
        }
        if (i == n-1) fprintf(f,".\n$$\n");
        else          fprintf(f,";\n");
    }
} 

static void
writedread(FILE *f, graph *g, boolean digraph, int m, int n)
/* Write in dreadnaut format */
{
    int i,j;
    graph *gi;

    if (digraph) fprintf(f,"n=%d $=0 dg\n",n);
    else         fprintf(f,"n=%d $=0 g\n",n);

    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        for (j = (digraph ? -1 : i-1) ; (j = nextelement(gi,m,j)) >= 0; )
            fprintf(f," %d",j);
        if (i == n-1) fprintf(f,".\n$$\n");
        else          fprintf(f,";\n");
    }
}

/**************************************************************************/

static int binom[32][16];   /* Cached binomial coefficients */

static int
binomial(int n, int k)
/* Value of binomial(n,k), error if too big for int */
{
    int i,nki,ans;
    nauty_counter work;

    if (k > n/2) k = n - k;
    if (k < 0) return 0;

    if (n < 32  && binom[n][k] > 0) return binom[n][k];

    work = 1;
    for (i = 1; i <= k; ++i)
    {
        nki = n-k+i;
        work = (work/i) * nki + (work%i) * nki / i;
        if ((int)work != work) { fprintf(stderr,"Overflow\n"); exit(1); }
    }

    ans = (int)work;
    if (n < 32) binom[n][k] = ans;

    return ans;
}

/**************************************************************************/

static void
unrank(int r, int k, int *a)
/* r-th k-set in colex order (r=0,1,...) */
{
    int i,p;

    for (i = k; i > 0; --i)
    {
        p = i - 1;
        do ++p; while (binomial(p,i) <= r);
        r -= binomial(p-1,i);
        a[i-1] = p-1;
    }
}

static int
rank(int k, int *a)
/* Rank of a[0..k-1] in colex order */
{
    int i,r;

    r = 0;
    for (i = 0; i < k; ++i)
        r += binomial(a[i],i+1);

    return r;
}

/**************************************************************************/

static int
vnumber(long *dimen, int *index, int ndimen)
{
    int i,v;
    
    v = 0;
    for (i = 0; i < ndimen; ++i)
        v = v*dimen[i] + index[i];

    return v;
}

/**************************************************************************/

static void
makepath(long n, boolean digraph, sparsegraph *sg)
{
    int *d,*e,i;
    size_t *v,k;

    if (n < 1 || n > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -p\n");

    if (digraph) SG_ALLOC(*sg,n,n-1,"genspecialg");
    else         SG_ALLOC(*sg,n,2*(size_t)n-2,"genspecialg");

    SG_VDE(sg,v,d,e);

    if (digraph || n == 1)
    {
        sg->nv = n;
        sg->nde = n-1;

        for (i = 0; i < n-1; ++i)
        {
            d[i] = 1;
            v[i] = i;
            e[i] = i+1;
        }
        d[n-1] = 0;
        v[n-1] = 0;
    }
    else
    {
        sg->nv = n;
        sg->nde = 2*(size_t)n-2;

        d[0] = 1;
        v[0] = 0;
        e[0] = 1;
        for (i = 1, k = 1; i < n-1; ++i, k += 2)
        {
            d[i] = 2;
            v[i] = k;
            e[k] = i-1;
            e[k+1] = i+1;
        }
        d[n-1] = 1;
        v[n-1] = k;
        e[k] = n-2;
    }
}

/**************************************************************************/

static void
makecycle(long n, boolean digraph, sparsegraph *sg)
{
    int *d,*e,i;
    size_t *v;

    if (!digraph && (n < 1 || n == 2 || n > NAUTY_INFINITY-2))
        gt_abort(">E genspecialg: bad argument for -c\n");
    if (digraph && (n < 1 || n > NAUTY_INFINITY-2))
        gt_abort(">E genspecialg: bad argument for -zc\n");

    if (digraph) SG_ALLOC(*sg,n,n,"genspecialg");
    else         SG_ALLOC(*sg,n,2*(size_t)n,"genspecialg");

    SG_VDE(sg,v,d,e);

    if (digraph || n == 1)
    {
        sg->nv = n;
        sg->nde = n;

        for (i = 0; i < n-1; ++i)
        {
            d[i] = 1;
            v[i] = i;
            e[i] = i+1;
        }
        d[n-1] = 1;
        v[n-1] = n-1;
        e[n-1] = 0;
    }
    else
    {
        sg->nv = n;
        sg->nde = 2*(size_t)n;

        d[0] = 2;
        v[0] = 0;
        e[0] = 1;
        e[1] = n-1;

        for (i = 1; i < n-1; ++i)
        {
            d[i] = 2;
            v[i] = 2*(size_t)i;
            e[2*(size_t)i] = i-1;
            e[2*(size_t)i+1] = i+1;
        }
        d[n-1] = 2;
        v[n-1] = 2*(size_t)n-2;
        e[2UL*n-2] = 0;
        e[2UL*n-1] = n-2;
    }
}

/**************************************************************************/

static void
makewheel(long k, boolean digraph, sparsegraph *sg)
/* Make a wheel with k spokes. In the directed version, spokes
   are oriented out from the hub. */
{
    int n,*d,*e,i;
    size_t *v,nde;

    if (k < 3 || k > NAUTY_INFINITY-3)
        gt_abort(">E genspecialg: bad argument for -w\n");

    n = k+1;
    if (digraph) nde = 2*(size_t)k;
    else         nde = 4*(size_t)k;

    SG_ALLOC(*sg,n,nde,"genspecialg");
    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = nde;

    if (digraph)
    {
        d[0] = n-1;
        v[0] = 0;
        for (i = 1; i < n; ++i)
        {
            d[i] = 1;
            v[i] = k+i-1;
        }
        for (i = 1; i < n; ++i)
        {
            e[v[0]+i-1] = i;
            e[v[i]] = (i == n-1 ? 1 : i+1);
        }
    }
    else
    {
        d[0] = n-1;
        v[0] = 0;
        for (i = 1; i < n; ++i)
        {
            d[i] = 3;
            v[i] = k+3*(size_t)i-3;
        }
        for (i = 1; i < n; ++i)
        {
            e[v[0]+i-1] = i;
            e[v[i]] = 0;
            e[v[i]+1] = (i == n-1 ? 1 : i+1);
            e[v[i]+2] = (i == 1 ? n-1 : i-1);
        }
    }
}

/**************************************************************************/

static void
makedebruijngraph(long k, long t, boolean digraph, sparsegraph *sg)
/* Make a de Bruijn graph or digraph for words of length t over
   an alphabet of size k. */
{
    int *d,*e,i,j,n,nok;
    int jn,ki,di;
    size_t *v,nde;
    long l0,l1;

    if (k < 2 || t < 2)
        gt_abort(">E genspecialg: bad arguments for -D\n");

    l0 = 1;
    for (i = 0; i < t; ++i)
    {
        l1 = l0 * k;
        if (l1 / k != l0)
            gt_abort(">E genspecialg: too many vertices for -D\n");
        l0 = l1;
    }

    if (l0 > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: too many vertices for -D\n");
     
    n = l0;
    nok = n / k;

    if (digraph) nde = n*(size_t)k;
    else         nde = 2*(size_t)n*k;  /* upper bound, what is the real number? */
    SG_ALLOC(*sg,n,nde,"genspecialg");

    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = nde;

    if (digraph)
    {
        for (i = 0; i < n; ++i)
        {
            d[i] = k;
            v[i] = i*(size_t)k;
            ki = k * (i%nok);
            for (j = 0; j < k; ++j)
                e[v[i]+j] = ki + j;
        }
    }
    else
    {
        for (i = 0; i < n; ++i)
        {
            di = 0;
            v[i] = (i == 0 ? 0 : v[i-1] + d[i-1]);
            ki = k * (i%nok);
            for (j = 0; j < k; ++j)
            {
                e[v[i]+di] = ki + j;
                ++di;
                jn = (i/k) + j*nok;
                if (jn / k != i % nok)
                {
                    e[v[i]+di] = jn;
                    ++di;
                }
            }
            d[i] = di;
        }
        sg->nde = v[n-1]+d[n-1];
    }
}

/**************************************************************************/

static void
makemoebiusladder(long k, boolean digraph, sparsegraph *sg)
/* Make a Moebius ladder with k vertices (k even).
   This is the same as a cycle with diagonal rungs.
   In the directed version only the cycle is directed.  */
{
    int n,*d,*e,i;
    size_t *v,nde;

    if (k < 3 || k % 2 != 0 || k > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -l\n");

    n = k;

    if (digraph) nde = 2*(size_t)k;
    else         nde = 3*(size_t)k;
    SG_ALLOC(*sg,n,nde,"genspecialg");

    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = nde;

    if (digraph)
    {
        for (i = 0; i < n; ++i) d[i] = 2;
        for (i = 0; i < n; ++i) v[i] = 2*(size_t)i;
        for (i = 0; i < n; ++i) e[v[i]] = (i==n-1 ? 0 : i+1);
        for (i = 0; i < n/2; ++i)
        {
            e[v[i]+1] = i + n/2;
            e[v[i+n/2]+1] = i;
        }
    }
    else
    {
        for (i = 0; i < n; ++i) d[i] = 3;
        for (i = 0; i < n; ++i) v[i] = 3*(size_t)i;
        for (i = 0; i < n; ++i) e[v[i]] = (i==n-1 ? 0 : i+1);
        for (i = 0; i < n; ++i) e[v[i]+1] = (i==0 ? n-1 : i-1);
        for (i = 0; i < n/2; ++i)
        {
            e[v[i]+2] = i + n/2;
            e[v[i+n/2]+2] = i;
        }
    }
}

/**************************************************************************/

static void
makeantiprism(long k, boolean digraph, sparsegraph *sg)
/* Make an antiprism with 2*k vertices.
   The directed version has the rungs oriented in the same
   sense as the cycles.
*/
{
    int n,*d,*e,i;
    size_t *v,nde;

    if (k < 3 || k > (NAUTY_INFINITY-2)/2)
        gt_abort(">E genspecialg: bad argument for -a\n");

    n = 2*k;

    if (digraph) nde = 4*(size_t)k;
    else         nde = 8*(size_t)k;
    SG_ALLOC(*sg,n,nde,"genspecialg");

    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = nde;

    if (digraph)
    {
        for (i = 0; i < n; ++i)
        {
            d[i] = 2;
            v[i] = 2*(size_t)i;
        }
        for (i = 0; i < k; ++i)
        {
            e[v[i]] = (i == k-1 ? 0 : i+1);
            e[v[i]+1] = k + i;
        }
        for (i = k; i < n; ++i)
        {
            e[v[i]] = (i == n-1 ? k : i+1);
            e[v[i]+1] = (i == n-1 ? 0 : i-k+1);
        }
    }
    else
    {
        for (i = 0; i < n; ++i)
        {
            d[i] = 4;
            v[i] = 4*(size_t)i;
        }
        for (i = 0; i < k; ++i)
        {
            e[v[i]] = (i == k-1 ? 0 : i+1);
            e[v[i]+1] = (i == 0 ? k-1 : i-1);
            e[v[i]+2] = k + i;
            e[v[i]+3] = (i == 0 ? n-1 : i+k-1);
        }
        for (i = k; i < n; ++i)
        {
            e[v[i]] = (i == n-1 ? k : i+1);
            e[v[i]+1] = (i == k ? n-1 : i-1);
            e[v[i]+2] = i - k;
            e[v[i]+3] = (i == n-1 ? 0 : i-k+1);
        } 
    }
}

/**************************************************************************/

static void
makeflowersnark(long k, boolean digraph, sparsegraph *sg)
/* Flower snark on 4*k vertices, no digraph variant 
*
* The flower snark Jn can be constructed with the following process :
* Build n copies of the star graph on 4 vertices. Denote the
* central vertex of each star Ai and the outer vertices Bi, Ci and
* Di. This results in a disconnected graph on 4n vertices with 3n
* edges (Ai-Bi, Ai-Ci and Ai-Di for 1?i?n). Construct the n-cycle
* (B1... Bn). This adds n edges. Finally construct the 2n-cycle
* (C1... CnD1... Dn). This adds 2n edges. By construction, the
* Flower snark Jn is a cubic graph with 4n vertices and 6n edges.
*/

#define FSA(i) (4*(i))
#define FSB(i) (4*(i)+1)
#define FSC(i) (4*(i)+2)
#define FSD(i) (4*(i)+3)
{
    int n,*d,*e,i,j;
    size_t *v,nde;

    if (k < 3 || k > (NAUTY_INFINITY-2)/4)
        gt_abort(">E genspecialg: bad argument for -f\n");

    n = 4*k;
    nde = 12*(size_t)k;

    SG_ALLOC(*sg,n,nde,"genspecialg");

    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = nde;

    for (i = 0; i < n; ++i)
    {
        d[i] = 0;
        v[i] = 3*(size_t)i;
    }

    for (i = 0; i < k; ++i)
    {
        e[v[FSA(i)]+d[FSA(i)]++] = FSB(i);
        e[v[FSB(i)]+d[FSB(i)]++] = FSA(i);
        e[v[FSA(i)]+d[FSA(i)]++] = FSC(i);
        e[v[FSC(i)]+d[FSC(i)]++] = FSA(i);
        e[v[FSA(i)]+d[FSA(i)]++] = FSD(i);
        e[v[FSD(i)]+d[FSD(i)]++] = FSA(i);
    }

    for (i = 0; i < k; ++i)
    {
        j = FSB((i+1)%k);
        e[v[FSB(i)]+d[FSB(i)]++] = j;
        e[v[j]+d[j]++] = FSB(i);
    }

    for (i = 0; i < k-1; ++i)
    {
        e[v[FSC(i)]+d[FSC(i)]++] = FSC(i+1);
        e[v[FSC(i+1)]+d[FSC(i+1)]++] = FSC(i);
    }

    for (i = 0; i < k-1; ++i)
    {
        e[v[FSD(i)]+d[FSD(i)]++] = FSD(i+1);
        e[v[FSD(i+1)]+d[FSD(i+1)]++] = FSD(i);
    }

    e[v[FSD(0)]+d[FSD(0)]++] = FSC(k-1);
    e[v[FSC(k-1)]+d[FSC(k-1)]++] = FSD(0);
    e[v[FSC(0)]+d[FSC(0)]++] = FSD(k-1);
    e[v[FSD(k-1)]+d[FSD(k-1)]++] = FSC(0);
}


static void
maketriangular(long k, boolean digraph, sparsegraph *sg)
/* Triangular graph on k(k-1)/2 vertices, no digraph variant 
*/
#define DIRPAIR(i,j) ((j)+(i)*k-((i)+1)*((i)+2)/2)
#define PAIR(i,j) ((i)<(j) ? DIRPAIR(i,j) : DIRPAIR(j,i))
{
    int n,*d,*e,*vx,i,j,t;
    size_t *v,nde,deg;

    if (k < 2 || k*(k-1)/2 > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -L\n");

    n = k*(k-1)/2;
    deg = 2*(k-2);
    nde = deg*(size_t)n;

    SG_ALLOC(*sg,n,nde,"genspecialg");

    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = nde;

    for (i = 0; i < n; ++i)
    {
        d[i] = deg;
        v[i] = deg*(size_t)i;
    }

    for (i = 0; i < k-1; ++i)
    for (j = i+1; j < k; ++j)
    {
        vx = e + v[PAIR(i,j)];
        for (t = 0; t < k; ++t)
        {
            if (t != i && t != j)
            {
                *(vx++) = PAIR(i,t);
                *(vx++) = PAIR(j,t);
            }
        }
    }
}

/**************************************************************************/

static void
makeJohnson(long n, long k, boolean digraph, sparsegraph *sg)
{
    size_t *v;
    int *d,*e,*ep,nv,deg,i,j,s,t,u;
    int *a,*b;

    if (k > n/2) k = n - k;
    if (k < 0) gt_abort(">E genspecialg: bad parameters for -J\n");

    nv = binomial(n,k);
    if (nv > NAUTY_INFINITY-2) gt_abort(">E genspecialg: too big -J\n");
    deg = k*(n-k);

    SG_ALLOC(*sg,nv,nv*(size_t)deg,"genspecialg");
    sg->nv = nv;
    sg->nde = nv*(size_t)deg;
    SG_VDE(sg,v,d,e);

    if ((a = malloc(k*sizeof(int))) == NULL ||
        (b = malloc(k*sizeof(int))) == NULL)
        gt_abort(">E malloc failed in makeJohnson()\n");

    preparemarks(n);

    for (i = 0; i < nv; ++i)
    {
        v[i] = i*(size_t)deg;
        d[i] = deg;
        ep = e + v[i];
        unrank(i,k,a);
        RESETMARKS;
        for (j = 0; j < k; ++j) MARK(a[j]);

        for (j = 0; j < n; ++j)
        if (ISNOTMARKED(j))
        {
            for (s = 0; s < k; ++s)
            {
                for (t = 0; t < k; ++t) b[t] = a[t];
                u = s;
                while (u > 0 && b[u-1] > j)
                {
                    b[u] = b[u-1];
                    --u;
                }
                while (u < k-1 && b[u+1] < j)
                {
                    b[u] = b[u+1];
                    ++u;
                }
                b[u] = j;
                *(ep++) = rank(k,b);
            }
        }
    }
    FREES(a);
    FREES(b);
}

static setword*
subsetlist(long n, long k)
/* Allocate binomial(n,k) setwords filled with right-adjusted subsets.
   It is known that binomial(n,k) is small enough for an int. */
{
    setword *x,w,first,last;
    int i,nv;

    nv = binomial(n,k);

    if ((x = malloc(sizeof(setword)*nv)) == NULL)
        gt_abort(">E malloc() failed in subsetlist()\n");

    first = ((setword)1 << k) - 1;
    last = first << (n-k);

    if (k == 0 || k == n)
    {
        x[0] = first;
        return x;
    }

    w = ((setword)1 << k) - 1;
    last = w << (n-k);
    i = 0;
    for (;;)
    {
        if (POPCOUNT(w) == k)
        {
            x[i++] = w;
            w += SWHIBIT(w);
            if (w == last) break;
        }
        else
        {
            ++w;
        }
    }
    x[i] = last;
    if (i != nv-1) gt_abort(">E something (1) wrong in subsetlist()\n");

    return x;
}
    
/**************************************************************************/

static void
makeKneser(long n, long k, long t, boolean digraph, sparsegraph *sg)
/* Make generalized Kneser graphs.  The vertices are the k-subsets of
   an n-set and two are adjacent if there intersection is exactly t. */
{
    size_t *v;
    int *d,*e,nv,deg,i,j;
    setword *x;

    if (k < 0 || t < 0 || t >= k || 2*k-t > n)
         gt_abort(">E genspecialg: impossible parameters for -K\n");
    if (n > WORDSIZE)
        gt_abort(">E genspecialg: first parameter of -K is too big\n");

    if (t == k-1)
    {
        makeJohnson(n,k,digraph,sg);
        return;
    }

    nv = binomial(n,k);
    if (nv > NAUTY_INFINITY-2) gt_abort(">E genspecialg: too big -K\n");
    deg = binomial(k,t)*binomial(n-k,k-t);

    SG_ALLOC(*sg,nv,nv*(size_t)deg,"genspecialg");
    sg->nv = nv;
    sg->nde = nv*(size_t)deg;
    SG_VDE(sg,v,d,e);

    x = subsetlist(n,k);

    for (i = 0; i < nv; ++i)
    {
        v[i] = i*(size_t)deg;
        d[i] = 0;
    }

    for (i = 0; i < nv-1; ++i)
    for (j = i+1; j < nv; ++j)
    {
        if (POPCOUNT(x[i]&x[j]) == t)
        {
            e[v[i]+(d[i]++)] = j;
            e[v[j]+(d[j]++)] = i;
        }
    }

    if (d[0] != deg) gt_abort(">E something (2) wrong in makeKneser()\n");

    FREES(x);
}

/**************************************************************************/

static void
makeantiregular(long n, boolean digraph, graph **pg)
/* Make a connected antiregular graph on n vertices. The other
   antiregular graph is the complement of this one and is disconnected.
   The defining property is having n-1 different degrees. */
{
    int i,j,m;
    graph *g;
    int *deg;
    boolean q;

    if (n < 2 || n > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -A\n");

    m = SETWORDSNEEDED(n);
    if ((g = malloc(m*sizeof(graph)*n)) == NULL
          || (deg = malloc(sizeof(int)*n)) == NULL)
        gt_abort(">E allocation failed in makecomplete()\n");
    *pg = g;

    EMPTYSET(g,m*n);
    for (i = 0; i < n; ++i) deg[i] = 0;

    ADDONEEDGE(g,0,1,m);
    deg[0] = deg[1] = 1;

    for (j = 2; j < n; ++j)
    {
        q = FALSE;
        for (i = 0; i < j; ++i)
        {
            if (deg[i] > j/2)
            {
                ADDONEEDGE(g,i,j,m);
                ++deg[i];
                ++deg[j];
            }
            else if (deg[i] == j/2 && !q)
            {
                ADDONEEDGE(g,i,j,m);
                ++deg[i];
                ++deg[j];
                q = TRUE;
            }
        }
    }
    FREES(deg);
}

/**************************************************************************/

static void
makecomplete(long n, boolean digraph, graph **pg)
{
    int i,j,m;
    graph *g,*gj;

    if (n < 1 || n > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -k\n");

    m = SETWORDSNEEDED(n);

    if ((g = malloc(m*sizeof(graph)*n)) == NULL)
        gt_abort(">E allocation failed in makecomplete()\n");
    *pg = g;

    g[m-1] = 0;
    for (i = 0; i < n; ++i) ADDELEMENT(g,i);

    for (j = 1, gj = g+m; j < n; ++j, gj += m)
        for (i = 0; i < m; ++i) gj[i] = g[i];

    if (!digraph)
    {
        for (j = 0, gj = g; j < n; ++j, gj += m) DELELEMENT(gj,j);
    }
}

/**************************************************************************/

static void
makemultipartite(long *args, int nargs, long *size, graph **pg)
{
    graph *g,*gj,*gk;
    long ln;
    int i,j,k,kk,t,m,n;

    ln = 0;
    for (t = 0; t < nargs; ++t)
    {
        if (args[t] <= 0) gt_abort(">E bad part size for -m\n");
        ln += args[t];
    }
    *size = ln;

    if (ln < 1 || ln > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -m\n");

    n = ln;
    m = SETWORDSNEEDED(n);

    if ((g = malloc(m*sizeof(graph)*n)) == NULL)
        gt_abort(">E allocation failed in makemultipartite()\n");
    *pg = g;

    for (t = k = 0; t < nargs; ++t, k = kk)
    {
        gk = g + k*m;
        kk = k + args[t];
        gk[m-1] = 0;
        for (i = 0; i < n; ++i) ADDELEMENT(gk,i);
        for (i = k; i < kk; ++i) DELELEMENT(gk,i);
        for (j = k+1, gj = gk+m; j < kk; ++j, gj += m)
            for (i = 0; i < m; ++i) gj[i] = gk[i];
    }
}

/**************************************************************************/

static void
makeempty(long n, boolean digraph, sparsegraph *sg)
{
    int *d,*e,i;
    size_t *v;

    if (n < 1 || n > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -e\n");

    if (digraph) SG_ALLOC(*sg,n,n,"genspecialg");
    else         SG_ALLOC(*sg,n,0,"genspecialg");

    SG_VDE(sg,v,d,e);

    if (digraph)
    {
        sg->nv = n;
        sg->nde = n;

        for (i = 0; i < n; ++i)
        {
            d[i] = 1;
            v[i] = i;
            e[i] = i;
        }
    }
    else
    {
        sg->nv = n;
        sg->nde = 0;

        for (i = 0; i < n; ++i)
        {
            d[i] = 0;
            v[i] = 0;
        }
    }
}

/**************************************************************************/

static void
makehypercube(long n, long t, boolean digraph, sparsegraph *sg)
/* Make a graph whose vertices are {0,1}^n and whose edges are
   vertices at hamming distance t. */
{
    int *d,*e,i,j,deg;
    size_t *v,k,nv;
    setword *x;

    if (n < 0 || n > 30 || t < 1 || t > n)
        gt_abort(">E genspecialg: bad argument for -Q\n");

    deg = binomial(n,t);
    x = subsetlist(n,t);

    nv = 1 << n;
    SG_ALLOC(*sg,nv,nv*(size_t)deg,"genspecialg");

    SG_VDE(sg,v,d,e);

    sg->nv = nv;
    sg->nde = nv*(size_t)deg;

    for (i = 0, k = 0; i < nv; ++i, k += deg)
    {
        d[i] = deg;
        v[i] = k;
        for (j = 0; j < deg; ++j) e[k+j] = i ^ x[j];
    }

    free(x);
}

/**************************************************************************/

static void
maketheta(long *len, int npaths, boolean digraph, sparsegraph *sg)
{
    int i,j,k,n,ntemp,*d,*e;
    size_t *v,ne,etemp;
    boolean hasone;

    hasone = FALSE;
    n = 2;
    ne = 0;
    for (i = 0; i < npaths; ++i)
    {
        if (len[i] < 1)
            gt_abort(">E genspecialg: -T paths must be at least length 1\n");
        if (len[i] == 1)
        {
            if (hasone) gt_abort(
                  ">E genspecialg: -T only one path of length 1 allowed\n");
            hasone = TRUE;
        }
        ntemp = n;
        n += len[i]-1;
        if (n < ntemp)
            gt_abort(">E genspecialg: -T too many vertices\n");
        etemp = ne;
        ne += len[i];
        if (ne < etemp) gt_abort(">E genspecialg: -T too many edges\n");
    }

    if (n > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: -T size is too big\n");

    if (!digraph)
    {
        etemp = ne;
        ne *= 2;
        if (ne < etemp) gt_abort(">E genspecialg: -T too many edges\n");
    }

    SG_ALLOC(*sg,n,ne,"genspecialg");
    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = ne;    

    v[0] = 0;
    v[1] = npaths;
    if (digraph)
    {
        v[2] = v[1];
        for (i = 3; i < n; ++i) v[i] = v[i-1] + 1;
    }
    else
    {
        v[2] = v[1] + npaths;
        for (i = 3; i < n; ++i) v[i] = v[i-1] + 2;
    }

    for (i = 0; i < n; ++i) d[i] = 0;

    if (hasone)
    {
        e[v[0]+(d[0]++)] = 1;
        if (!digraph) e[v[1]+(d[1]++)] = 0;
    }

    k = 2;
    for (i = 0; i < npaths; ++i)
    {
        if (len[i] == 1) continue;

        e[v[0]+(d[0]++)] = k;
        if (!digraph) e[v[k]+(d[k]++)] = 0;
        
        for (j = 0; j < len[i]-2; ++j)
        {
            e[v[k]+(d[k]++)] = k+1;
            if (!digraph) e[v[k+1]+(d[k+1]++)] = k;
            ++k;
        }
        e[v[k]+(d[k]++)] = 1;
        if (!digraph) e[v[1]+(d[1]++)] = k;
        ++k;
    }
}

/**************************************************************************/

static void
makegrid(long *dim, int ndim, boolean digraph, sparsegraph *sg)
{
    int *d,*e,i,j,deg,n,oldn;
    size_t *v,k;
    boolean closed[30];
    int index[30];

    n = 1;
    deg = 0;
    for (i = 0; i < ndim; ++i)
    {
        if (dim[i] >= -1 && dim[i] <= 1)
            gt_abort(">E genspecialg: -G dimensions must be at least 2\n");
        if (dim[i] == 2 && !digraph)
            gt_abort(">E genspecialg: -G dimen 2 is only ok for digraphs\n");

        closed[i] = (dim[i] > 0);
        if (dim[i] < 0) dim[i] = -dim[i];

        oldn = n;
        n *= dim[i];
        if (n < 0 || n / dim[i] != oldn)
            gt_abort(">E genspecialg: -G size is too big\n");

        if (digraph || dim[i] == 2) ++deg;
        else                        deg += 2;

        index[i] = 0;
    }

    if (n > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: -G size is too big\n");

    SG_ALLOC(*sg,n,deg*(size_t)n,"genspecialg");

    SG_VDE(sg,v,d,e);

    sg->nv = n;
    sg->nde = deg*(size_t)n;

    k = 0;
    for (i = 0; i < n; ++i)
    {
        v[i] = k;
        for (j = 0; j < ndim; ++j)
        {
            if (index[j] < dim[j]-1)
            {
                ++index[j];
                e[k++] = vnumber(dim,index,ndim);
                --index[j];
            }
            if (!digraph && index[j] > 0)
            {
                --index[j];
                e[k++] = vnumber(dim,index,ndim);
                ++index[j];
            }
            if (closed[j] && index[j] == dim[j]-1)
            {
                index[j] = 0;
                e[k++] = vnumber(dim,index,ndim);
                index[j] = dim[j]-1;
            }
            if (closed[j] && !digraph && index[j] == 0)
            {
                index[j] = dim[j]-1;
                e[k++] = vnumber(dim,index,ndim);
                index[j] = 0;
            }
        }

        d[i] = k - v[i];

        for (j = ndim; --j >= 0;)
        {
            if (index[j] != dim[j]-1)
            {
                ++index[j];
                break;
            }
            else
                index[j] = 0;
        }
    }
}

/**************************************************************************/

static void
makecirculant(long n, long *conn, int nconn, boolean digraph, sparsegraph *sg)
{
    int *d,*e,i,j,deg;
    size_t *v,k;

    if (nconn > 0 && conn[0] <= 0)
        gt_abort(">E genspecialg: -C connections must be nonzero\n");

    for (i = 1; i < nconn; ++i)
        if (conn[i] <= conn[i-1])
            gt_abort(">E genspecialg: -C connections must be increasing\n");

    if (nconn == 0)
        deg = 0;
    else
    {
        if (digraph)
        {
            if (conn[nconn-1] >= n) gt_abort(
                 ">E genspecialg: -C connections must be 1..n-1\n");
            deg = nconn;
        }
        else
        {
            if (conn[nconn-1] > n/2) gt_abort(
                 ">E genspecialg: -C connections must be 1..n/2\n");
            deg = 2*nconn - (2*conn[nconn-1]==n);
        }
    }

    SG_ALLOC(*sg,n,deg*n,"genspecialg");

    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = deg*(size_t)n;

    for (i = 0; i < n; ++i)
    {
        d[i] = deg;
        v[i] = deg*(size_t)i;
    }
 
    for (i = 0; i < n; ++i)
    {
        k = v[i];
        for (j = 0; j < nconn; ++j)
        {
            e[k++] = (i + conn[j]) % n;
            if (!digraph && 2*conn[j] != n)
                e[k++] = (i - conn[j] + n) % n;
        }
    }
}

/**************************************************************************/

static void
makegenpetersen(long n1, long n2, boolean digraph, sparsegraph *sg)
{
    int *d,*e,i,n;
    size_t *v,k;

    if (digraph) gt_abort(">E no digraph version of -P is implemented\n");

    n = 2*n1;
    if (n < 1 || n1 > NAUTY_INFINITY/2-1 || n2 < 1 || 2*n2 >= n1)
        gt_abort(">E -Pm,k needs m>0,0<k<m/2; or m too large\n");

    SG_ALLOC(*sg,n,3*(size_t)n,"genspecialg");

    SG_VDE(sg,v,d,e);
    sg->nv = n;
    sg->nde = 3*(size_t)n;

    for (i = 0; i < n; ++i)
    {
        d[i] = 3;
        v[i] = 3UL*i;
    }

    for (i = 0; i < n1; ++i)
    {
        k = v[i];
        e[k] = (i + 1) % n1;
        e[k+1] = (i + n1 - 1) % n1;
        e[k+2] = i + n1;
    }
    
    for (i = 0; i < n1; ++i)
    {
        k = v[n1+i];
        e[k] = n1 + (i + n2) % n1;
        e[k+1] = n1 + (i - n2 + n1) % n1;
        e[k+2] = i;
    }
} 

/**************************************************************************/

static void
makecompletebipartite(long n1, long n2,
                      long matching, boolean digraph, sparsegraph *sg)
{
    int *d,*e,i,j,jmissing,n;
    size_t *v,k;

    n = n1 + n2;
    if (matching > n1 || matching > n2) 
        gt_abort(">E genspecialg: matching too large\n");

    if (n1 < 1 || n2 < 1 || n > NAUTY_INFINITY-2)
        gt_abort(">E genspecialg: bad argument for -b\n");

    if (digraph) SG_ALLOC(*sg,n,n1*(size_t)n2,"genspecialg");
    else         SG_ALLOC(*sg,n,2*(size_t)n1*n2,"genspecialg");

    SG_VDE(sg,v,d,e);

    if (digraph)
    {
        sg->nv = n;
        sg->nde = n1*(size_t)n2 - matching;

        for (i = 0, k = 0; i < n1; ++i)
        {
            v[i] = k;
            jmissing = (i < matching ? n1+i : -1);
            for (j = n1; j < n; ++j) if (j != jmissing) e[k++] = j;
            d[i] = k - v[i];
        }
        for (i = n1; i < n; ++i)
        {
            d[i] = 0;
            v[i] = k;
        }
    }
    else
    {
        sg->nv = n;
        sg->nde = 2*(n1*(size_t)n2 - matching);

        for (i = 0, k = 0; i < n1; ++i)
        {
            v[i] = k;
            jmissing = (i < matching ? n1+i : -1);
            for (j = n1; j < n; ++j) if (j != jmissing) e[k++] = j;
            d[i] = k - v[i];
        }
        for (i = n1; i < n; ++i)
        {
            v[i] = k;
            jmissing = (i < n1+matching ? i-n1 : -1);
            for (j = 0; j < n1; ++j) if (j != jmissing) e[k++] = j;
            d[i] = k - v[i];
        }
    }
}

/**************************************************************************/

#define LOWER(c) ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))

static int
writenamedgraph(FILE *f, int index, char *name)
/* Write named graph to f, index takes priority. 
   Return starting position in special[]. */
{
    int i,k,it;

    if (index >= 1)
    {
        for (k = 0; k < NUMSPECIALS; ++k)
            if (special[k].code == index) break;
        if (k == NUMSPECIALS)
        {
            fprintf(stderr,">E No such named graph %d\n",index);
            exit(1);
        }
        it = k;
    }
    else
    {
        it = -1;
        for (k = 0; k < NUMSPECIALS; ++k)
        if (special[k].code > 0)
        {
            for (i = 0; special[k].name[i] != '\0' && name[i] != '\0'; ++i)
                if (LOWER(special[k].name[i]) != LOWER(name[i])) break;
            if (name[i] == '\0')
            {
                if (it >= 0)
                {
                    fprintf(stderr,">E name \"%s\" is ambiguous\n",name);
                    exit(1);
                }
                it = k;
            }
        }
        if (it < 0)
        {
            fprintf(stderr,">E No such named graph \"%s\"\n",name);
            exit(1);
        }
    }

    fputs(special[it].p,f);
    for (k = it+1; k < NUMSPECIALS && special[k].code == 0; ++k) 
        fputs(special[k].p,f);    

    return it;
}

/**************************************************************************/

/* Code for making Paley graphs is here. It relies on the following theorem.
   Let n = p^k be an odd prime power and let f in Z_p[x] be a monic 
   polynomial of degree k. Then f is irreducible over Z_p iff these two
   conditions hold for all h,g in Z_p[x] of degree < k:  (1) h^2 mod f <> 0,
   (2) h^2 mod f = g^2 mod f iff h=g or h=-g. */

#define MAXPWR 19   /* Enough for p^k <= NAUTY_INFINITY-2 */

static boolean
isprimepower(int x, int *p, int *pwr)
/* Try to write x=p^pwr for prime p and pwr>0, otherwise return FALSE. */
{
    int i,j;

    if (x <= 1) return FALSE;

    if (x % 2 == 0)
    {
        for (i = 0; x%2 == 0; ++i) x /= 2;
        if (x == 1)
        {
            *p = 2;
            *pwr = i;
            return TRUE;
        }
        else
            return FALSE;
    }

    for (j = 3; j*j <= x; j += 2)
        if (x % j == 0) break;

    if (j*j > x)
    {
        *p = x;
        *pwr = 1;
        return TRUE;
    }

    for (i = 0; x%j == 0; ++i) x /= j;
    if (x == 1)
    {
        *p = j;
        *pwr = i;
        return TRUE;
    }
    else
        return FALSE;
}

static int
polysquare(int index, int *mod, int p, int deg)
/* Find poly(index)^2 mod (x^deg + mod[]), returning the index. */
{
    int ff[2*MAXPWR];
    int i,j,k,ans;
    long f[MAXPWR],a;

    for (i = deg; --i >= 0; )
    {
        f[i] = index % p;
        index /= p;
    }

    for (i = 0; i < 2*deg; ++i) ff[i] = 0;

    for (i = 0; i < deg; ++i)
    for (j = 0; j < deg; ++j)
        ff[i+j] = (ff[i+j] + f[i]*f[j]) % p;

    for (k = deg-2; k >= 0; --k)
    {
        a = ff[k+deg];
        for (i = 0; i < deg; ++i)
            ff[i+k] = (ff[i+k] + a*(p-mod[i])) % p;
    }

    ans = ff[0];
    for (i = 1; i < deg; ++i) ans = p*ans + ff[i];

    return ans;
}

static void
findsquares(int n, int p, int deg, set *sq)
/* Find an irreducible polynomial for n = p^deg and
   put the set of squares into sq. */
{
    int m,i,ii;
    int mod[MAXPWR];

    m = SETWORDSNEEDED(n);

    while (1)
    {
        mod[0] = 1 + KRAN(p-1);
        for (i = 1; i < deg; ++i) mod[i] = KRAN(p);
        EMPTYSET(sq,m);

        for (i = 1; i < n; ++i)
        {
            ii = polysquare(i,mod,p,deg);
            if (ii == 0) break;
            if (!ISELEMENT(sq,ii))
            {
                ADDELEMENT(sq,ii);
            }
        }
        if (i == n && setsize(sq,m) == (n-1)/2) break;
    }

    /* Now x^deg + mod is an irreducible polynomial */
}

static int
polyadd(int index1, int index2, int p, int deg)
/* Return the index of poly(index1)+poly(index2) */
{
    int ans,i,f[MAXPWR];

    for (i = deg; --i >= 0; )
    {
        f[i] = (index1 + index2) % p;
        index1 /= p;
        index2 /= p;
    }

    ans = f[0];
    for (i = 1; i < deg; ++i) ans = p*ans + f[i];

    return ans;
}

static void
makepaleygraph(long size, graph **pg)
{
    graph *g;
    int m,n,p,deg;
    int i,j,k;
    set *sq;

    if (size > NAUTY_INFINITY-2) gt_abort(">E Paley graph size too big\n");

    n = size;
    m = SETWORDSNEEDED(n);

    if (!isprimepower(n,&p,&deg) || p == 2)
    {
        gt_abort(">E Paley graph size is not an odd prime power.\n");
        exit(1);
    }

    if ((g = malloc(m*sizeof(graph)*n)) == NULL ||
                (sq = malloc(m*sizeof(set))) == NULL)
        gt_abort(">E allocation failed in makepaleygraph()\n");

    findsquares(n,p,deg,sq);

    *pg = g;
    EMPTYSET(g,m*(size_t)n);

    for (i = -1; (i = nextelement(sq,m,i)) >= 0; )
    {
        for (j = 0; j < n; ++j)
        {
            k = polyadd(i,j,p,deg);
            ADDONEARC(g,j,k,m);
        }
    }

    free(sq);
}

/**************************************************************************/

int
main(int argc, char *argv[])
{
    int codetype;
    int argnum,j,mm,nn;
    char *arg,sw;
    boolean badargs,quiet;
    boolean Cswitch,Pswitch,gswitch,sswitch,zswitch,Jswitch,dswitch;
    boolean pswitch,cswitch,eswitch,kswitch,bswitch,Qswitch,Gswitch;
    boolean fswitch,Tswitch,Xswitch,wswitch,Aswitch,Kswitch,Dswitch;
    boolean Yswitch,mswitch,aswitch,lswitch,Lswitch,ispaley;
    boolean havesparsegraph,havedensegraph,verbose,directed,dreadnaut;
    long size;
    static FILE *outfile;
    char *outfilename;
    graph *g;
    sparsegraph sg;
    long Dargs[2],Pargs[2],bargs[3],Jargs[2],Kargs[3],Qargs[2];
    int nDargs,nPargs,nbargs,nCargs,nGargs,nJargs,nKargs,nTargs;
    int nQargs,nmargs,Xindex;
    int numgraphs,Xout;

    HELP; PUTVERSION;

    if (argc > 1 && (strcmp(argv[1],"-Xhelp") == 0 
                           || strcmp(argv[1],"--Xhelp") == 0))
    { 
        printf("%s%s%s",SPECIALHELP1,SPECIALHELP2,SPECIALHELP3);
        return 0;
    }

    numgraphs = 0;

    gswitch = sswitch = zswitch = Pswitch = Xswitch = FALSE;
    pswitch = cswitch = eswitch = kswitch = Yswitch = FALSE;
    Gswitch = Cswitch = bswitch = Qswitch = verbose = FALSE;
    dswitch = Jswitch = fswitch = Tswitch = quiet = FALSE;
    mswitch = aswitch = wswitch = lswitch = Aswitch = FALSE;
    Kswitch = Lswitch = FALSE;

    outfilename = NULL;

    argnum = 0;
    badargs = FALSE;
    for (j = 1; !badargs && j < argc; ++j)
    {
        arg = argv[j];
        if (arg[0] == '-' && arg[1] != '\0')
        {
            ++arg;
            while (*arg != '\0')
            {
                sw = *arg++;
                     SWBOOLEAN('g',gswitch)
                else SWBOOLEAN('s',sswitch)
                else SWBOOLEAN('z',zswitch)
                else SWBOOLEAN('d',dswitch)
                else SWBOOLEAN('q',quiet)
                else SWBOOLEAN('v',verbose)
                else SWLONG('p',pswitch,size,"genspecialg -p")
                else SWLONG('c',cswitch,size,"genspecialg -c")
                else SWLONG('e',eswitch,size,"genspecialg -e")
                else SWLONG('k',kswitch,size,"genspecialg -k")
                else SWLONG('A',Aswitch,size,"genspecialg -A")
                else SWLONG('f',fswitch,size,"genspecialg -f")
                else SWLONG('a',aswitch,size,"genspecialg -a")
                else SWLONG('l',lswitch,size,"genspecialg -l")
                else SWLONG('w',wswitch,size,"genspecialg -w")
                else SWLONG('L',Lswitch,size,"genspecialg -L")
                else SWLONG('Y',Yswitch,size,"genspecialg -Y")
                else SWSEQUENCEMIN('Q',",",Qswitch,Qargs,1,2,nQargs,"genspecialg -Q")
                else SWSEQUENCEMIN('b',",",bswitch,bargs,2,3,nbargs,"genspecialg -b")
                else SWSEQUENCEMIN('J',",",Jswitch,Jargs,2,2,nJargs,"genspecialg -J")
                else SWSEQUENCEMIN('K',",",Kswitch,Kargs,2,3,nJargs,"genspecialg -K")
                else SWSEQUENCEMIN('P',",",Pswitch,Pargs,2,2,nPargs,"genspecialg -P")
                else SWSEQUENCEMIN('D',",",Dswitch,Dargs,2,2,nDargs,"genspecialg -D")
                else SWSEQUENCEMIN('m',",",mswitch,args,
                                        2,MAXARGS,nmargs,"genspecialg -m")
                else SWSEQUENCEMIN('C',",",Cswitch,args,
                                        1,MAXARGS,nCargs,"genspecialg -C")
                else SWSEQUENCEMIN('G',",",Gswitch,args,2,30,nGargs,"genspecialg -G")
                else SWSEQUENCEMIN('T',",",Tswitch,args,1,MAXARGS,
                                nTargs,"genspecialg -T")
                else if (sw == 'X')
                {
                    if (*arg >= '0' && *arg <= '9')
                    {
                        SWINT('X',Xswitch,Xindex,"genspecial -X")
                    }
                    else
                    {
                        break;
                    }
                }
                else badargs = TRUE;
            }
        }
        else
        {
            ++argnum;
            if (argnum == 1) outfilename = arg;
            else             badargs = TRUE;
        }
    }

    if ((gswitch!=0) + (sswitch!=0) + (zswitch!=0) > 1)
        gt_abort(">E genspecialg: -gsz are incompatible\n");

    if ((gswitch!=0) + (sswitch!=0) + (dswitch!=0) > 1)
        gt_abort(">E genspecialg: -gsd are incompatible\n");
 
    if (badargs)
    {
        fprintf(stderr,">E Usage: %s\n",USAGE);
        GETHELP;
        exit(1);
    }

    if (gswitch)      codetype = GRAPH6;
    else if (zswitch) codetype = DIGRAPH6;
    else              codetype = SPARSE6;
    dreadnaut = dswitch;

    if (!outfilename || outfilename[0] == '-')
    {
        outfilename = "stdout";
        outfile = stdout;
    }
    else if ((outfile = fopen(outfilename,"w")) == NULL)
        gt_abort_1(">E Can't open output file %s\n",outfilename);

    SG_INIT(sg);

    Xindex = 0;

    argnum = 0;
    badargs = FALSE;
    for (j = 1; !badargs && j < argc; ++j)
    {
        arg = argv[j];
        if (arg[0] == '-' && arg[1] != '\0')
        {
            ++arg;
            while (*arg != '\0')
            {
                havesparsegraph = havedensegraph = ispaley = FALSE;
                sw = *arg++;
                     SWBOOLEAN('g',gswitch)
                else SWBOOLEAN('s',sswitch)
                else SWBOOLEAN('z',zswitch)
                else SWBOOLEAN('d',dswitch)
                else SWBOOLEAN('q',quiet)
                else SWBOOLEAN('v',verbose)
                else if (sw == 'p')
                {
                    SWLONG('p',pswitch,size,"genspecialg -p")
                    makepath(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'c')
                {
                    SWLONG('c',cswitch,size,"genspecialg -c")
                    makecycle(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'e')
                {
                    SWLONG('e',eswitch,size,"genspecialg -e")
                    makeempty(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'k')
                {
                    SWLONG('k',kswitch,size,"genspecialg -k")
                    makecomplete(size,zswitch,&g);
                    havedensegraph = TRUE;
                }
                else if (sw == 'A')
                {
                    SWLONG('A',Aswitch,size,"genspecialg -A")
                    makeantiregular(size,zswitch,&g);
                    havedensegraph = TRUE;
                }
                else if (sw == 'a')
                {
                    SWLONG('a',aswitch,size,"genspecialg -a")
                    makeantiprism(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'L')
                {
                    SWLONG('L',Lswitch,size,"genspecialg -L")
                    maketriangular(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'l')
                {
                    SWLONG('l',lswitch,size,"genspecialg -l")
                    makemoebiusladder(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'w')
                {
                    SWLONG('w',wswitch,size,"genspecialg -w")
                    makewheel(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'f')
                {
                    SWLONG('f',fswitch,size,"genspecialg -f")
                    makeflowersnark(size,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'Y')
                {
                    SWLONG('Y',Yswitch,size,"genspecialg -Y")
                    makepaleygraph(size,&g);
                    havedensegraph = TRUE;
                    ispaley = TRUE;
                }
                else if (sw == 'b')
                {
                    SWSEQUENCEMIN('b',",",bswitch,bargs,2,3,nbargs,"genspecialg -b")
                    makecompletebipartite(bargs[0],bargs[1],
                           (nbargs==2?0:bargs[2]),zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'J')
                {
                    SWSEQUENCEMIN('J',",",Jswitch,Jargs,2,2,nJargs,"genspecialg -J")
                    makeJohnson(Jargs[0],Jargs[1],zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'Q')
                {
                    SWSEQUENCEMIN('Q',",",Qswitch,Qargs,1,2,nQargs,"genspecialg -Q")
                    if (nQargs == 1) Qargs[1] = 1;
                    makehypercube(Qargs[0],Qargs[1],zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'K')
                {
                    SWSEQUENCEMIN('K',",",Kswitch,Kargs,2,3,nKargs,"genspecialg -K")
                    if (nKargs == 2) Kargs[2] = 0;
                    makeKneser(Kargs[0],Kargs[1],Kargs[2],zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'D')
                {
                    SWSEQUENCEMIN('D',",",Dswitch,Dargs,2,2,nDargs,"genspecialg -D")
                    makedebruijngraph(Dargs[0],Dargs[1],zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'P')
                {
                    SWSEQUENCEMIN('P',",",Pswitch,Pargs,2,2,nPargs,"genspecialg -P")
                    makegenpetersen(Pargs[0],Pargs[1],zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'C')
                {
                    SWSEQUENCEMIN('C',",",Cswitch,args,
                                        1,MAXARGS,nCargs,"genspecialg -C")
                    makecirculant(args[0],args+1,nCargs-1,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'm')
                {
                    SWSEQUENCEMIN('m',",",mswitch,args,
                                        2,MAXARGS,nmargs,"genspecialg -m")
                    makemultipartite(args,nmargs,&size,&g);
                    havedensegraph = TRUE;
                }
                else if (sw == 'G')
                {
                    SWSEQUENCEMIN('G',",",Gswitch,args,2,30,nGargs,"genspecialg -G")
                    makegrid(args,nGargs,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'T')
                {
                    SWSEQUENCEMIN('T',",",Tswitch,args,1,MAXARGS,
                                nTargs,"genspecialg -T")
                    maketheta(args,nTargs,zswitch,&sg);
                    havesparsegraph = TRUE;
                }
                else if (sw == 'X')
                {
                    if (*arg >= '0' && *arg <= '9')
                    {
                        SWINT('X',Xswitch,Xindex,"genspecial -X")
                        Xout = writenamedgraph(outfile,Xindex,NULL);
                        ++numgraphs;
                        if (verbose) fprintf(stderr,"Graph %d: %s (%d vertices)\n",
                             numgraphs,special[Xout].name,graphsize(special[Xout].p));
                    }
                    else
                    {
                        Xout = writenamedgraph(outfile,0,arg);
                        ++numgraphs;
                        if (verbose) fprintf(stderr,"Graph %d: %s (%d vertices)\n",
                             numgraphs,special[Xout].name,graphsize(special[Xout].p));
                        break;
                    }
                }

                if (havedensegraph)
                {
                    nn = size;    // check it is defined for every dense case
                    mm = SETWORDSNEEDED(nn);
                    directed = zswitch || (ispaley && nn % 4 == 3);
                    if (dreadnaut)                 writedread(outfile,g,directed,mm,nn);
                    else if (codetype == DIGRAPH6 || directed)
                                                   writed6(outfile,g,mm,nn);
                    else if (codetype == GRAPH6)   writeg6(outfile,g,mm,nn);
                    else                           writes6(outfile,g,mm,nn);
                    FREES(g);
                    ++numgraphs;
                    havesparsegraph = FALSE;

                    if (verbose)
                        fprintf(stderr,"Graph %d: %d vertices %lu edges\n",numgraphs,
                           sg.nv,(unsigned long)(zswitch ? sg.nde : sg.nde/2));
                }
                else if (havesparsegraph)
                {
                    sortlists_sg(&sg);
                    if (dreadnaut)                 writedread_sg(outfile,&sg,zswitch);
                    else if (codetype == GRAPH6)   writeg6_sg(outfile,&sg);
                    else if (codetype == DIGRAPH6) writed6_sg(outfile,&sg);
                    else                           writes6_sg(outfile,&sg);
                    ++numgraphs;
                    havesparsegraph = FALSE;

                    if (verbose)
                        fprintf(stderr,"Graph %d: %d vertices %lu edges\n",numgraphs,
                           sg.nv,(unsigned long)(zswitch ? sg.nde : sg.nde/2));
                }
            }
        }
        else
        {
            ++argnum;
        }
    }

    if (!quiet)
        fprintf(stderr,">Z %d graphs written to %s\n",numgraphs,outfilename);

    exit(0);
}
