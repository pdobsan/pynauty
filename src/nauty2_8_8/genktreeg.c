/* genktreeg.c  version 1.0; B D McKay, Aug 9, 2023 */

#define USAGE \
"genktreeg [-k#] [-lq] n [res/mod] [file]"

#define HELPTEXT \
" Generate all k-trees on n vertices.\n\
\n\
      n    : the number of vertices\n\
    -k#    : the value of k (default 2)\n\
   res/mod : only generate subset res out of subsets 0..mod-1\n\
\n\
     -l    : canonically label output graphs\n\
\n\
     -u    : do not output any graphs, just generate and count them\n\
     -g    : use graph6 output (default)\n\
     -s    : use sparse6 output\n\
     -h    : write a header (only with -g or -s)\n\
\n\
     -q    : suppress auxiliary output\n\
\n"

/*
OUTPROC feature.

   By defining the C preprocessor variable OUTPROC at compile time
   (for Unix the syntax is  -DOUTPROC=procname  on the cc command),
   genktreeg can be made to call a procedure of your manufacture with each
   output graph instead of writing anything. Your procedure needs
   to have type void and the argument list (FILE *f, graph *g, int n).
   f is a stream open for writing, g is the graph in nauty format,
   and n is the number of vertices. Your procedure can be in a
   separate file so long as it is linked with genktreeg. The global
   variables nooutput, and canonise (all type boolean) can be
   used to test for the presence of the flags -u and -l,
   respectively.

PRUNE feature.

   By defining the C preprocessor variable PRUNE at compile time, genktreeg
   can be made to call
        int PRUNE(graph *g,int n,int maxn) 
   for each intermediate (and final) graph, and reject it if 
   the value returned is nonzero.  The arguments are:

     g      = the graph in nauty format (m=1)
     n      = the number of vertices in g
     maxn   = the number of vertices for output 
              (the value you gave on the command line to genktreeg)

   genktreeg constructs the graph starting with a clique in vertices 0..k-1
   then adds vertices k,k+1,...,n-1 in that order.  Each graph in the
   sequence is a k-tree.

   A call is made for all orders from k to maxn.  In testing for
   a uniform property (such as a forbidden subgraph or forbidden
   induced subgraph) it might save time to notice that a call to
   PRUNE for n implies that the call for n-1 already passed. 

   For very fast tests, it might be worthwhile using PREPRUNE as
   well or instead. It has the same meaning but is applied earlier
   and more often.

SUMMARY

   If the C preprocessor variable SUMMARY is defined at compile time, the
   procedure SUMMARY(nauty_counter nout, double cpu) is called just before
   the program exits.  The purpose is to allow reporting of statistics
   collected by PRUNE or OUTPROC.  The values nout and cpu are the output
   count and cpu time reported on the >Z line.
   Output should be written to stderr.

CALLING FROM A PROGRAM

   It is possible to call genktreeg from another program instead of using it
   as a stand-alone program.  The main requirement is to change the name
   of the main program to be other than "main".  This is done by defining
   the preprocessor variable GENGKTREEG_MAIN.  You might also like to define
   OUTPROC to be the name of a procedure to receive the graphs. To call
   the program you need to define an argument list argv[] consistent with
   the usual one; don't forget that argv[0] is the command name and not
   the first argument.  The value of argc is the number of strings in
   argv[]; that is, one more than the number of arguments.  See the
   sample program callgeng.c for how it is done with geng.

Counts:
 k=1 : https://oeis.org/A000055  (gentreeg is much faster for this case)
 k=2 : https://oeis.org/A054581
 k=3 : https://oeis.org/A078792
 k=4 : https://oeis.org/A078793
 k=5 : https://oeis.org/A201702
 k=6 : https://oeis.org/A202037
 k=7 : https://oeis.org/A322754
 k=8 : 1, 1, 1, 2, 5, 15, 64, 342, 2344, 19137, 181204, 1926782, 22638677,
       289742922, 3996857019, 58854922207, 916955507587, 14988769972628,
       255067524402905, 4487202163529135
 k=9 : 1, 1, 1, 2, 5, 15, 64, 342, 2344, 19137, 181204, 1927017, 22652254,
       290351000, 4019973352, 59642496465, 941751344429, 15724551551655,
       275926445572426, 5057692869843759
 k=10 : 1, 1, 1, 2, 5, 15, 64, 342, 2344, 19137, 181204, 1927017, 22652805,
       290391147, 4022154893, 59741455314, 945737514583, 15871943695637,
       281035862707569, 5226147900656616
 k=11 : 1, 1, 1, 2, 5, 15, 64, 342, 2344, 19137, 181204, 1927017, 22652805,
       290392448, 4022273471, 59749135094, 946147301367, 15891194890267,
       281861097237048, 5259154274072864
 k=12 : 1, 1, 1, 2, 5, 15, 64, 342, 2344, 19137, 181204, 1927017, 22652805,
       290392448, 4022276630, 59749484387, 946173920785, 15892845044945,
       281950479681057, 5263547896506226

**************************************************************************

    Author:   B. D. McKay
              Copyright  B. McKay (2023-).  All rights reserved.
              This software is subject to the conditions and waivers
              detailed in the file COPYRIGHT.

**************************************************************************/

  /* 1 = geng, 2 = genbg, 3 = gentourng, 4 = gentreeg, 5 = genktreeg */
#define NAUTY_PGM  5

#ifndef MAXN
#define MAXN WORDSIZE
#endif

#define ONE_WORD_SETS
#include "gtools.h"   /* which includes nauty.h and stdio.h */

typedef unsigned int xword;

static TLS_ATTR void (*outproc)(FILE*,graph*,int);

static TLS_ATTR FILE *outfile;           /* file for output graphs */
boolean TLS_ATTR graph6;                 /* presence of -g */
boolean TLS_ATTR sparse6;                /* presence of -s */
boolean TLS_ATTR nooutput;               /* presence of -u */
boolean TLS_ATTR canonise;               /* presence of -l */
boolean TLS_ATTR quiet;                  /* presence of -q */
boolean TLS_ATTR header;                 /* presence of -h */
static TLS_ATTR int kvalue,maxn,mod,res;
static TLS_ATTR int odometer,splitlevel;
static TLS_ATTR graph gcan[MAXN];
static TLS_ATTR nauty_counter nout;
static TLS_ATTR setword *kclique; /* List of k-cliques */
static TLS_ATTR int *cliqueorbs[MAXN];

#ifdef PLUGIN
#include PLUGIN
#endif

#ifdef OUTPROC
extern void OUTPROC(FILE*,graph*,int);
#endif
#ifdef PRUNE
extern int PRUNE(graph*,int,int);
#endif
#ifdef PREPRUNE
extern int PREPRUNE(graph*,int,int);
#endif
#ifdef SUMMARY
extern void SUMMARY(nauty_counter,double);
#endif

/************************************************************************/

void
writeg6x(FILE *f, graph *g, int n)
/* write graph g (n vertices) to file f in graph6 format */
{
    writeg6(f,g,1,n);
}

/************************************************************************/

void
writes6x(FILE *f, graph *g, int n)
/* write graph g (n vertices) to file f in sparse6 format */
{
    writes6(f,g,1,n);
}

/************************************************************************/

static void
nullwrite(FILE *f, graph *g, int n)
/* don't write graph g (n vertices) to file f */
{
}

/***********************************************************************/

static void
transposition(int v1, int v2, int n)
/* Set cliqeorbs[n] when only automorphism is (v1 v2) */
{
    int ncliques,i,*orb;
    setword *clique,w,w01;

    ncliques = 1 + kvalue*(n - kvalue);
    clique = kclique;
    orb = cliqueorbs[n];
    w01 = bit[WORDSIZE-1-v2];
    w = bit[WORDSIZE-1-v1] | w01;

    for (i = 0; i < ncliques; ++i)
        if ((clique[i] & w) == w01) orb[i] = -1;
        else                        orb[i] = i;
}

/***********************************************************************/

static void
automproc(int count, int *p, int *orbits,
          int numorbits, int stabvertex, int n)
/* form orbits on cliques; operates on data[n] */
{
    setword *clique,w,pxi,moved;
    int ncliques;
    int lo,hi,j1,j2,i,pi;
    int *orb;

    ncliques = 1 + kvalue*(n - kvalue);
    clique = kclique;
    orb = cliqueorbs[n];

    if (count == 1)                         /* first automorphism */
        for (i = 0; i < ncliques; ++i) orb[i] = i;

    moved = 0;
    for (i = 0; i < n; ++i)
        if (p[i] != i) moved |= bit[WORDSIZE-1-i];

    for (i = 0; i < ncliques; ++i)
    {
        if ((w = clique[i] & moved) == 0) continue;
        pxi = clique[i] & ~moved;
        while (w)
        {
            TAKEBIT(j1,w);
            pxi |= bit[WORDSIZE-1-p[WORDSIZE-1-j1]];
        }

        lo = 0;
        hi = ncliques - 1;
        while (lo <= hi)
        {
            pi = (lo + hi) / 2;
            if      (pxi < clique[pi]) hi = pi - 1;
            else if (pxi > clique[pi]) lo = pi + 1;
            else                       break;
        }
        if (lo > hi) gt_abort(">E binary search failed\n");

        j1 = orb[i];  while (orb[j1] != j1) j1 = orb[j1];
        j2 = orb[pi]; while (orb[j2] != j2) j2 = orb[j2];

        if      (j1 < j2) orb[j2] = orb[i] = orb[pi] = j1;
        else if (j1 > j2) orb[j1] = orb[i] = orb[pi] = j2;
    }
}

/*****************************************************************************
*                                                                            *
*  refinex(g,lab,ptn,level,numcells,count,active,goodret,code,m,n) is a      *
*  custom version of refine() which can exit quickly if required.            *
*                                                                            *
*  goodret : whether to do an early return for code 1                        *
*  code := -1 for n-1 not max, 0 for maybe, 1 for definite                   *
*                                                                            *
*****************************************************************************/

static void
refinex(graph *g, int *lab, int *ptn, int *numcells,
 int *count, set *active, boolean goodret, int *code, int n)
{
    int i,c1,c2,labc1;
    setword x,lact;
    int split1,split2,cell1,cell2;
    int cnt,bmin,bmax;
    setword workset,gs1;
    int workperm[MAXN];
    int bucket[MAXN+2];

    if (n == 1)
    {
        *code = 1;
        return;
    }

    *code = 0;
    lact = *active;

    split1 = -1;
    while (*numcells < n && lact)
    {
        TAKEBIT(split1,lact);
        
        for (split2 = split1; ptn[split2] > 0; ++split2) {}
        if (split1 == split2)       /* trivial splitting cell */
        {
//            gptr = GRAPHROW(g,lab[split1],1);
            gs1 = g[lab[split1]];
            for (cell1 = 0; cell1 < n; cell1 = cell2 + 1)
            {
                for (cell2 = cell1; ptn[cell2] > 0; ++cell2) {}
                if (cell1 == cell2) continue;

                c1 = cell1;
                c2 = cell2;
                while (c1 <= c2)
                {
                    labc1 = lab[c1];
                    if ((gs1 & bit[lab[c1]]))
//                    if (ISELEMENT1(gptr,labc1))
                        ++c1;
                    else
                    {
                        lab[c1] = lab[c2];
                        lab[c2] = labc1;
                        --c2;
                    }
                }
                if (c2 >= cell1 && c1 <= cell2)
                {
                    ptn[c2] = 0;
                    ++*numcells;
                    lact |= bit[c1];
                }
            }
        }

        else        /* nontrivial splitting cell */
        {
            workset = 0;
            for (i = split1; i <= split2; ++i) workset |= bit[lab[i]];

            for (cell1 = 0; cell1 < n; cell1 = cell2 + 1)
            {
                for (cell2 = cell1; ptn[cell2] > 0; ++cell2) {}
                if (cell1 == cell2) continue;
                i = cell1;
                if ((x = workset & g[lab[i]]) != 0) cnt = POPCOUNT(x);
                else                                cnt = 0;
                count[i] = bmin = bmax = cnt;
                bucket[cnt] = 1;
                while (++i <= cell2)
                {
                    if ((x = workset & g[lab[i]]) != 0)
                        cnt = POPCOUNT(x);
                    else
                        cnt = 0;

                    while (bmin > cnt) bucket[--bmin] = 0;
                    while (bmax < cnt) bucket[++bmax] = 0;
                    ++bucket[cnt];
                    count[i] = cnt;
                }
                if (bmin == bmax) continue;
                c1 = cell1;
                for (i = bmin; i <= bmax; ++i)
                    if (bucket[i])
                    {
                        c2 = c1 + bucket[i];
                        bucket[i] = c1;
                        if (c1 != cell1)
                        {
                            lact |= bit[c1];
                            ++*numcells;
                        }
                        if (c2 <= cell2) ptn[c2-1] = 0;
                        c1 = c2;
                    }
                for (i = cell1; i <= cell2; ++i)
                    workperm[bucket[count[i]]++] = lab[i];
                for (i = cell1; i <= cell2; ++i) lab[i] = workperm[i];
            }
        }

        if (ptn[n-2] == 0)
        {
            if (lab[n-1] == n-1)
            {
                *code = 1;
                if (goodret) return;
            }
            else
            {
                *code = -1;
                return;
            }
        }
        else
        {
            i = n - 1;
            while (TRUE)
            {
                if (lab[i] == n-1) break;
                --i;
                if (ptn[i] == 0)
                {
                    *code = -1;
                    return;
                }
            }
        }
    }
}

/**************************************************************************/

static void
makecanon(graph *g, graph *gcan, int n)
/* gcan := canonise(g) */
{
    int lab[MAXN],ptn[MAXN],orbits[MAXN];
    static TLS_ATTR DEFAULTOPTIONS_GRAPH(options);
    statsblk stats;
    setword workspace[200];

    options.getcanon = TRUE;

    nauty(g,lab,ptn,NULL,orbits,&options,&stats,workspace,200,1,n,gcan);
}

/**************************************************************************/

static boolean
accept(graph *g, int n, int maxn, boolean rigid, boolean *newrigid)
/* Test if vertex n-1 is canonical.  If can be assumed that it has
   degree kvalue. newrigid is not set for n = maxn */
{
    int count[MAXN],lab[MAXN],ptn[MAXN],orbits[MAXN];
    int deg,avdeg,numcells,code,i,k,lo,hi;
    setword avd,active;
    static TLS_ATTR DEFAULTOPTIONS_GRAPH(options);
    statsblk stats;
    setword workspace[200];
    int hits,lasthits,ii,temp;

    *newrigid = FALSE;
    k = kvalue;
    avdeg = 2*k - k*(k+1)/n;

    lo = 0; hi = n-1;
    avd = 0;
    for (i = 0; i < n; ++i)
    {
        deg = POPCOUNT(g[i]);
        if (deg == k) lab[hi--] = i;
        else          lab[lo++] = i;
        if (deg >= avdeg) avd |= bit[i];
    }

  /* Now lab[0..hi] have degree > k; lab[hi+1..n-1] have degree k */

    lasthits = POPCOUNT(g[n-1]&avd);

    ii = n;
    for (i = hi+1; i < ii; )
    {
        hits = POPCOUNT(g[lab[i]]&avd);
        if (hits < lasthits) return FALSE;  /* Aim for minimum */
        if (hits == lasthits)
        {
            --ii;
            temp = lab[i];
            lab[i] = lab[ii];
            lab[ii] = temp;
        }
        else
            ++i;
    }
 
 /* Now lab[hi+1..ii-1] are poor; lab[ii..n-1] are rich. */

    for (i = 0; i < n; ++i) ptn[i] = 1;
    ptn[n-1] = 0;
    active = bit[0];

    if (hi >= 0) { ptn[hi] = 0; active |= bit[hi+1]; }
    if (ii > 0) { ptn[ii-1] = 0; active |= bit[ii]; }

    numcells = POPCOUNT(active);

  /* Note: for n > k, k-trees have at least two vertices
     of degree k and all such vertices are non-adjacent. */

    refinex(g,lab,ptn,&numcells,count,&active,n==maxn,&code,n);

    if (code < 0) return FALSE;

    if (rigid && code == 1) *newrigid = TRUE;

    if (n == maxn)
    {
        if (code == 1) return TRUE;

        for (i = n-2; i >= 0 && ptn[i] == 1; --i)
            if (g[lab[i]] != g[lab[n-1]]) break;
        if (i < 0 || ptn[i] == 0) return TRUE;
    }
    else if (numcells == n-1)
    {
        for (i = 0; i < n; ++i) if (ptn[i] > 0) break;
        transposition(lab[i],lab[i+1],n);
        return TRUE;
    }
    
    if (!*newrigid)
    {
        options.getcanon = TRUE;
        options.defaultptn = FALSE;

        active = 0;
        if (n < maxn) options.userautomproc = automproc;
        else          options.userautomproc = NULL;
        nauty(g,lab,ptn,&active,orbits,&options,&stats,
              workspace,200,1,n,gcan);
        if (stats.numorbits == n) *newrigid = TRUE;
        return orbits[n-1] == orbits[lab[n-1]];
    }
    else
        return TRUE;
}

/**************************************************************************/

static void
scan(graph *g, int n, int maxn, boolean rigid)
/* g[0..n-1] is the current graph, maxn is the target size,
   rigid says if no symmetries of k-cliques, the group has
   already been found, and kclique updated. */
{
    setword w,basec;
    graph newg[MAXN];
    int i,ii,j,k;
    int *orb;
    boolean newrigid;

    if (n == splitlevel)
    {
        if (odometer-- != 0) return;
        odometer = mod - 1;
    }

    if (n == maxn)
    {
        if (canonise)
        {
            makecanon(g,newg,n);
            (*outproc)(outfile,newg,n);
        }
        else
            (*outproc)(outfile,g,n);
        ++nout;
        return;
    }

    k = kvalue;
    orb = cliqueorbs[n];

    for (i = 0; i < 1 + k*(n-k); ++i)
    {
        if (!rigid && orb[i] != i) continue;

        for (j = 0; j < n; ++j) newg[j] = g[j];
        newg[n] = 0;

        w = kclique[i];
        ii = 1 + k*(n-k);
        basec = w | bit[WORDSIZE-1-n];
        
        while (w)
        {
            TAKEBIT(j,w);
            kclique[ii++] = basec & ~bit[j];
            j = WORDSIZE - 1 - j;
            newg[j] |= bit[n];
            newg[n] |= bit[j];
        }
#ifdef PREPRUNE
        if (PREPRUNE(newg,n+1,maxn)) continue;
#endif

        if (accept(newg,n+1,maxn,rigid,&newrigid))
        {
#ifdef PRUNE
            if (!PRUNE(newg,n+1,maxn))
#endif
            scan(newg,n+1,maxn,newrigid);
        }
    }
}

/**************************************************************************/
/**************************************************************************/

int
#ifdef GENKTREEG_MAIN
GENKTREEG_MAIN(int argc, char *argv[])
#else
main(int argc, char *argv[])
#endif
{
    char *arg;
    boolean gotk,badargs,gotd,gotD,gotf,gotmr;
    char *outfilename,sw;
    int i,j,argnum;
    graph g[MAXN];
    double t1,t2;
    char msg[201];

    HELP; PUTVERSION;
    nauty_check(WORDSIZE,1,MAXN,NAUTYVERSIONID);

    badargs = FALSE;
    graph6 = FALSE;
    sparse6 = FALSE;
    nooutput = FALSE;
    canonise = FALSE;
    header = FALSE;
    outfilename = NULL;

    gotk = gotd = gotD = gotf = gotmr = FALSE;

    argnum = 0;
    for (j = 1; !badargs && j < argc; ++j)
    {
        arg = argv[j];
        if (arg[0] == '-' && arg[1] != '\0')
        {
            ++arg;
            while (*arg != '\0')
            {
                sw = *arg++;
                     SWBOOLEAN('u',nooutput)
                else SWBOOLEAN('g',graph6)
                else SWBOOLEAN('s',sparse6)
                else SWBOOLEAN('l',canonise)
                else SWBOOLEAN('h',header)
                else SWBOOLEAN('q',quiet)
                else SWINT('k',gotk,kvalue,"genktreeg -k")
#ifdef PLUGIN_SWITCHES
PLUGIN_SWITCHES
#endif
                else badargs = TRUE;
            }
        }
        else if (arg[0] == '-' && arg[1] == '\0')
            gotf = TRUE;
        else
        {
            if (argnum == 0)
            {
                if (sscanf(arg,"%d",&maxn) != 1) badargs = TRUE;
                ++argnum;
            }
            else if (gotf)
                badargs = TRUE;
            else
            {
                if (!gotmr)
                {
                    if (sscanf(arg,"%d/%d",&res,&mod) == 2)
                    { 
                        gotmr = TRUE; 
                        continue; 
                    }
                }
                if (!gotf)
                {
                    outfilename = arg;
                    gotf = TRUE;
                    continue;
                }
            }
        }
    }

    if (argnum == 0)
        badargs = TRUE;
    else if (maxn < 1 || maxn > MAXN)
    {
        fprintf(stderr,">E genktreeg: n must be in the range 1..%d\n",MAXN);
        if (WORDSIZE <= 32 && maxn <= 64) fprintf(stderr,
           ">E You can go further by compiling with WORDSIZE=64\n");
        badargs = TRUE;
    }

    if (!gotmr)
    {
        mod = 1;
        res = 0;
    }

    if (!gotk) kvalue = 2;

    if (maxn < kvalue)
        gt_abort(">E genktreeg: n cannot be less than k\n");

    if (!badargs && (res < 0 || res >= mod))
    {
        fprintf(stderr,">E genktreeg: must have 0 <= res < mod\n");
        badargs = TRUE;
    }

    if (badargs)
    {
        fprintf(stderr,">E Usage: %s\n",USAGE);
        GETHELP;
        exit(1);
    }

    if ((graph6!=0) + (sparse6!=0) + (nooutput!=0) > 1)
        gt_abort(">E genktreeg: -ugs are incompatible\n");

#ifdef OUTPROC
    outproc = OUTPROC;
#else
    if      (nooutput) outproc = nullwrite;
    else if (sparse6)  outproc = writes6x;
    else               outproc = writeg6x;
#endif

#ifdef PLUGIN_INIT
PLUGIN_INIT
#endif

    if (nooutput)
        outfile = stdout;
    else if (!gotf || outfilename == NULL)
    {
        outfilename = "stdout";
        outfile = stdout;
    }
    else if ((outfile = fopen(outfilename,"w")) == NULL)
        gt_abort_1(
              ">E genktreeg: can't open %s for writing\n",outfilename);

    if (nooutput) canonise = FALSE;

    if (!quiet)
    {
        msg[0] = '\0';
        if (strlen(argv[0]) > 75)
            fprintf(stderr,">A %s",argv[0]);
        else
            CATMSG1(">A %s",argv[0]);
       
        if (canonise) CATMSG0(" -l");
        CATMSG2(" k=%d n=%d",kvalue,maxn);
        if (mod > 1) CATMSG2(" class=%d/%d",res,mod);
        CATMSG0("\n");
        fputs(msg,stderr);
        fflush(stderr);
    }

    t1 = CPUTIME;

    if (header)
    {
        if (sparse6)
        {
            writeline(outfile,SPARSE6_HEADER);
            fflush(outfile);
        }
        else if (!nooutput)
        {
            writeline(outfile,GRAPH6_HEADER);
            fflush(outfile);
        }
    }

    if (mod == 1)
        splitlevel = -1;
    else
    {
        if (kvalue == 1)       splitlevel = 20;
        else if (kvalue <= 4)  splitlevel = 15;
        else if (kvalue <= 16) splitlevel = kvalue + 11;
        else                   splitlevel = kvalue + 10;

        if (splitlevel > maxn) splitlevel = maxn;
        odometer = res;
    }

    if ((kclique = malloc((1+kvalue*(maxn-kvalue))*sizeof(setword))) == NULL)
        gt_abort(">E malloc() failed in genktreeg\n");
    for (i = kvalue; i < maxn; ++i)
        if ((cliqueorbs[i] = malloc((1+i*(i-kvalue))*sizeof(int))) == NULL)
            gt_abort(">E malloc() failed in genktreeg\n");

    kclique[0] = 0;
    for (i = 0; i < kvalue; ++i)
    {
        g[i] = ALLMASK(kvalue) & ~bit[i];
        kclique[0] |= bit[WORDSIZE-1-i];
    }

    nout = 0;

    scan(g,kvalue,maxn,TRUE);

    t2 = CPUTIME;

#ifdef SUMMARY
    SUMMARY(nout,t2-t1);
#endif

    if (!quiet)
    {
        fprintf(stderr,">Z " COUNTER_FMT " graphs generated in %3.2f sec\n",
                nout,t2-t1);
    }

#ifdef GENKTREEG_MAIN
    free(kclique);
    for (i = kvalue; i < maxn; ++i) free(cliqueorbs[i]);
    return 0;
#else
    exit(0);
#endif
}
