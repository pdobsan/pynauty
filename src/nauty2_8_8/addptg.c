/* addptg.c   Version 1.1; B D McKay, Oct 2023. */

#define USAGE "addptg [-lq] [-n#] [-j#:#] [-e#:#] [-ck] [-io] [infile [outfile]]"

#define HELPTEXT \
" Add a specified number of new vertices\n\
\n\
    The output file has a header if and only if the input file does.\n\
\n\
    -l  canonically label outputs\n\
    -c  join each new vertex to all the old vertices\n\
    -k  make a clique on the set of new vertices\n\
    -n# the number of new vertices (default 1)\n\
    -j# -j#:# join a new vertex to # old vertices in all possible ways\n\
    -e# -e#:# use all joins that give the new graphs # edges\n\
          (-j, -e are each incompatible with -n)\n\
    -i  for a digraph, edges go towards the old vertices\n\
    -o  for a digraph, edges go away from the old vertices\n\
          (-i is the default if neither -i nor -o is given)\n\
    -q  Suppress auxiliary information\n"

/*************************************************************************/

#include "gtools.h" 
#include "gutils.h"

static nauty_counter nout;
static boolean dolabel,digraph;
static FILE *outfile;
static int outcode;

/*************************************************************************/

static void
gotone(graph *g, int m, int n)
{
    graph *pg;
    DYNALLSTAT(graph,h,h_sz);

    if (dolabel)
    {
        DYNALLOC2(graph,h,h_sz,m,n,"addptg/gotone");
        fcanonise(g,m,n,h,NULL,FALSE);
        pg = h;
    }
    else
        pg = g;

    if (digraph)                writed6(outfile,pg,m,n);
    else if (outcode == GRAPH6) writeg6(outfile,pg,m,n);
    else                        writes6(outfile,pg,m,n);

    ++nout;
}

/*************************************************************************/

static void
dojoins(graph *g, int lastedge, int togomin, int togomax, int m, int n)
/* n is the new size */
{
    int i;
    
    if (togomin <= 0)
    {
        gotone(g,m,n);
        if (togomax == 0) return;
    }

    for (i = lastedge + 1; i < n - togomin && i < n-1; ++i)
    {
        ADDONEEDGE(g,i,n-1,m);
        dojoins(g,i,togomin-1,togomax-1,m,n);
        DELONEEDGE(g,i,n-1,m);
    }
}


static void
dojoins_dir(graph *g, int lastedge, int togomin, int togomax,
                        boolean in, boolean out, int m, int n)
/* n is the new size */
{
    int i;
    
    if (togomin <= 0)
    {
        gotone(g,m,n);
        if (togomax == 0) return;
    }

    if (in && out)
    {
        for (i = lastedge + 1; i < n - togomin && i < n-1; ++i)
        {
            ADDONEEDGE(g,i,n-1,m);
            dojoins_dir(g,i,togomin-1,togomax-1,in,out,m,n);
            DELONEEDGE(g,i,n-1,m);
        }
    }
    else if (in)
    {
        for (i = lastedge + 1; i < n - togomin && i < n-1; ++i)
        {
            ADDONEARC(g,n-1,i,m);
            dojoins_dir(g,i,togomin-1,togomax-1,in,out,m,n);
            DELONEARC(g,n-1,i,m);
        }
    }
    else
    {
        for (i = lastedge + 1; i < n - togomin && i < n-1; ++i)
        {
            ADDONEARC(g,i,n-1,m);
            dojoins_dir(g,i,togomin-1,togomax-1,in,out,m,n);
            DELONEARC(g,i,n-1,m);
        }
    }
}

/*************************************************************************/

static void
addvertices(graph *g, int extras, boolean cone, boolean clique, int m, int n)
/* n is the new size */
{
    int i,j,oldn;

    oldn = n - extras;
 
    if (cone)
    {
        for (j = oldn; j < n; ++j)
        for (i = 0; i < oldn; ++i)
            ADDONEEDGE(g,i,j,m);
    }

    if (clique)
    {
        for (j = oldn+1; j < n; ++j)
        for (i = oldn; i < j; ++i)
            ADDONEEDGE(g,i,j,m);
    }

    gotone(g,m,n);
}

static void
addvertices_dir(graph *g, int extras, boolean in, boolean out,
                           boolean cone, boolean clique, int m, int n)
/* n is the new size */
{
    int i,j,oldn;

    oldn = n - extras;
 
    if (cone)
    {
        for (j = oldn; j < n; ++j)
        for (i = 0; i < oldn; ++i)
        {
            if (in) ADDONEARC(g,j,i,m);
            if (out) ADDONEARC(g,i,j,m);
        }
    }

    if (clique)
    {
        for (j = oldn+1; j < n; ++j)
        for (i = oldn; i < j; ++i)
            ADDONEEDGE(g,i,j,m);
    }

    gotone(g,m,n);
}

/*************************************************************************/

static long
edgenumber(graph *g, boolean digraph, int m, int n)
/* Find the number of edges */
{
    long loops,count;
    graph *gi;
    int i,j;

    loops = count = 0;
    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        if (ISELEMENT(gi,i)) ++loops;
        for (j = 0; j < m; ++j) count += POPCOUNT(gi[j]);
    }

    if (digraph) return count;
    else         return (count + loops) / 2;
}

/*************************************************************************/

int
main(int argc, char *argv[])
{
    char *infilename,*outfilename;
    FILE *infile;
    boolean badargs,quiet,in,out;
    boolean cswitch,kswitch,nswitch,jswitch,eswitch;
    int extras,dmin,dmax;
    long joinmin,joinmax,emin,emax,ne;
    int i,j,m,n,m2,n2,argnum;
    int codetype;
    graph *g,*gi,*gpi;
    nauty_counter nin;
    char *arg,sw;
    double t;
#if MAXN
    graph gplus[MAXN*MAXM];
#else
    DYNALLSTAT(graph,gplus,gplus_sz);
#endif

    HELP; PUTVERSION;

    infilename = outfilename = NULL;
    dolabel = quiet = nswitch = jswitch = cswitch = kswitch = eswitch = FALSE;
    in = out = FALSE;

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
                     SWBOOLEAN('l',dolabel)
                else SWBOOLEAN('q',quiet)
                else SWBOOLEAN('c',cswitch)
                else SWBOOLEAN('k',kswitch)
                else SWBOOLEAN('i',in)
                else SWBOOLEAN('o',out)
                else SWINT('n',nswitch,extras,">E addptg -n")
                else SWRANGE('j',":-",jswitch,joinmin,joinmax,">E addptg -j")
                else SWRANGE('e',":-",eswitch,emin,emax,">E addptg -e")
                else badargs = TRUE;
            }
        }
        else
        {
            ++argnum;
            if      (argnum == 1) infilename = arg;
            else if (argnum == 2) outfilename = arg;
            else                  badargs = TRUE;
        }
    }

    if (badargs)
    {
        fprintf(stderr,">E Usage: %s\n",USAGE);
        GETHELP;
        exit(1);
    }

    if (!nswitch) extras = 1;
    if (extras > 1 && (jswitch || eswitch))
        gt_abort(">E addptg: -e and -j are incompatible with -n\n");

    if (extras < 0) gt_abort(">E addptg: negative argument for -n\n");

    if (!quiet)
    {
        fprintf(stderr,">A addptg");
        if (dolabel) fprintf(stderr," -l");
        if (nswitch) fprintf(stderr," -n%d",extras);
        if (jswitch) fprintf(stderr," -j%ld:%ld",joinmin,joinmax);
        if (eswitch) fprintf(stderr," -e%ld:%ld",emin,emax);
        if (cswitch || kswitch || in || out)
        {
            fprintf(stderr," -");
            if (cswitch) fprintf(stderr,"c");
            if (kswitch) fprintf(stderr,"k");
            if (in) fprintf(stderr,"i");
            if (out) fprintf(stderr,"o");
        }
        if (argnum > 0) fprintf(stderr," %s",infilename);
        if (argnum > 1) fprintf(stderr," %s",outfilename);
        fprintf(stderr,"\n");
        fflush(stderr);
    }

    if (!in && !out) in = TRUE;

    if (infilename && infilename[0] == '-') infilename = NULL;
    infile = opengraphfile(infilename,&codetype,FALSE,1);
    if (!infile) exit(1);
    if (!infilename) infilename = "stdin";

    if (!outfilename || outfilename[0] == '-')
    {
        outfilename = "stdout";
        outfile = stdout;
    }
    else if ((outfile = fopen(outfilename,"w")) == NULL)
        gt_abort_1(">E Can't open output file %s\n",outfilename);

    if (codetype&SPARSE6) outcode = SPARSE6;
    else                  outcode = GRAPH6;

    if (codetype&HAS_HEADER)
    {
        if (outcode == SPARSE6) writeline(outfile,SPARSE6_HEADER);
        else                    writeline(outfile,GRAPH6_HEADER);
    }

    nauty_check(WORDSIZE,1,1,NAUTYVERSIONID);

    nin = nout = 0;
    t = CPUTIME;
    while (TRUE)
    {
        if ((g = readgg(infile,NULL,0,&m,&n,&digraph)) == NULL) break;
        ++nin;

        n2 = n + extras;
        m2 = SETWORDSNEEDED(n2);
        if (m2 < m) m2 = m;
        nauty_check(WORDSIZE,m2,n2,NAUTYVERSIONID);
#if MAXN==0
        DYNALLOC2(graph,gplus,gplus_sz,m2,n2,"addptg");
#endif

        EMPTYSET(gplus,n2*(size_t)m2);
        for (i = 0, gi = g, gpi = gplus; i < n; ++i, gi += m, gpi += m2)
            for (j = 0; j < m; ++j) gpi[j] = gi[j];

        if (eswitch || jswitch)
        {
            dmin = 0; dmax = n;
            if (jswitch)
            {
                dmin = (joinmin > dmin ? joinmin : dmin);
                dmax = (joinmax < dmax ? joinmax : dmax);
            }
            if (eswitch)
            {
                ne = edgenumber(g,digraph,m,n);
                dmin = (int)(emin-ne > dmin ? emin-ne : dmin);
                dmax = (int)(emax-ne < dmax ? emax-ne : dmax);
            }
            if (dmin > dmax) continue;

            if (digraph)
                dojoins_dir(gplus,-1,dmin,dmax,in,out,m2,n2);
            else
                dojoins(gplus,-1,dmin,dmax,m2,n2);
        }
        else
        {
            if (digraph)
                addvertices_dir(gplus,extras,in,out,cswitch,kswitch,m2,n2);
            else
                addvertices(gplus,extras,cswitch,kswitch,m2,n2);
        }

        FREES(g);
    }
    t = CPUTIME - t;

    if (!quiet)
        fprintf(stderr,
          ">Z  " COUNTER_FMT " graphs read from %s, "
                       COUNTER_FMT " written to %s; %3.2f sec.\n",
                nin,infilename,nout,outfilename,t);

     exit(0);
}
