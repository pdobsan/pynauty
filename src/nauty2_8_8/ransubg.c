/* ransubg.c  version 1.0; B D McKay, Sep 2023. */

#define USAGE \
  "ransubg [-lq] [-P#|-P#/#] [-z] [-o] [-m#] [-S#] [infile [outfile]]"

#define HELPTEXT \
" Extract random graph/digraphs of a file of graphs.\n\
\n\
    The output file has a header if and only if the input file does.\n\
    The output format is defined by the header or first graph,\n\
      but is always digraph6 if -z is used.\n\
\n\
    -z   Treat the input as a directed graph\n\
    -o   Treat the input as a directed graph and output an oriented graph\n\
    -P# or -P#/#  Specify a probability p. -P# is the same as -P1/#\n\
    -m#  Make this many outputs for each input (default 1)\n\
    -l   Canonically label outputs.\n\
    -S#  Set seed for random numbers (default nondeterministic)\n\
\n\
    Usually each edge is kept with probability p.\n\
    In the case of -o, each edge in one direction only is kept\n\
      with probability p. For edges in both directions, with probability p\n\
      keep one direction randomly chosen and with probability 1-p delete\n\
      both directions.  Thus -oP1 will make a random orientation of an\n\
      undirected graph.\n\
    -q  Suppress auxiliary information.\n"

/*************************************************************************/

#include "gtools.h" 
#include "naurng.h"
#include "nautinv.h"

/**************************************************************************/

static void
getsubgraph(graph *g, int m, int n, graph *gsub,
       boolean digraph, boolean oriented, long P1value, long P2value)
/* gsub := subgraph of g, using probability P1value/P2value */
{
    int i,j;
    setword *gi;

    EMPTYSET(gsub,m*(size_t)n);

    if (!digraph)
    {
        for (gi = g, i = 0; i < n; ++i, gi += m)
        {
            for (j = i-1; (j = nextelement(gi,m,j)) >= 0; )
                if (KRAN(P2value) < P1value) ADDONEEDGE(gsub,i,j,m);
        }
    }
    else if (oriented)
    {
        for (gi = g, i = 0; i < n; ++i, gi += m)
        {
            for (j = -1; (j = nextelement(gi,m,j)) >= 0; )
                if (KRAN(P2value) < P1value)
                {
                    if (ISELEMENT(GRAPHROW(g,j,m),i))
                    {
                        if (j > i)
                        {
                            if (KRAN(2) == 1) ADDONEARC(gsub,i,j,m);
                            else              ADDONEARC(gsub,j,i,m);
                        }
                    }
                    else
                        ADDONEARC(gsub,i,j,m);
                }
        }
    }
    else
    {
        for (gi = g, i = 0; i < n; ++i, gi += m)
        {
            for (j = -1; (j = nextelement(gi,m,j)) >= 0; )
                if (KRAN(P2value) < P1value) ADDONEARC(gsub,i,j,m);
        }
    }
}

/**************************************************************************/

int
main(int argc, char *argv[])
{
    char *infilename,*outfilename;
    FILE *infile,*outfile;
    boolean dolabel,badargs,oswitch,zswitch,Sswitch,Pswitch,mswitch,quiet;
    boolean digraph,isdig;
    int j,m,n,argnum;
    int codetype,outcode;
    graph *g;
    unsigned long long nin,nout;
    char *arg,sw;
    static graph *gq;
    unsigned long long Svalue;
    long P1value,P2value,multiplicity,ii;
    double t;
    DYNALLSTAT(graph,gsub,gsub_sz);
    DYNALLSTAT(graph,hc,hc_sz);

    HELP; PUTVERSION;

    infilename = outfilename = NULL;
    dolabel = badargs = FALSE;
    quiet = dolabel = oswitch = zswitch = Pswitch = Sswitch = mswitch = FALSE;

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
                     SWBOOLEAN('o',oswitch)
                else SWBOOLEAN('z',zswitch)
                else SWULL('S',Sswitch,Svalue,"genrang -S")
                else SWRANGE('P',"/",Pswitch,P1value,P2value,"genrang -P")
                else SWBOOLEAN('l',dolabel)
                else SWLONG('m',mswitch,multiplicity,"genrang -m")
                else SWBOOLEAN('q',quiet)
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

    if (!Pswitch)
    {
        P1value = 1;
        P2value = 2;
    }
    else if (P1value == P2value)
    {
        P1value = 1;
    }

    if (P1value < 0 || P2value <= 0 || P1value > P2value)
        gt_abort(">E genrang: bad value for -P switch\n");

    if (!Sswitch)
    {
        Svalue = INITRANBYTIME;
    }
    else
        ran_init(Svalue);

    if (!mswitch) multiplicity = 1;

    if (!quiet)
    {
        fprintf(stderr,">A ransubg");
        if (dolabel || oswitch || zswitch)
            fprintf(stderr," -%s%s%s",
                (dolabel?"l":""),(oswitch?"o":""),(zswitch?"z":""));
        fprintf(stderr," -S%llu",Svalue);
        if (mswitch) fprintf(stderr," -m%ld",multiplicity);
        if (argnum > 0) fprintf(stderr," %s",infilename);
        if (argnum > 1) fprintf(stderr," %s",outfilename);
        fprintf(stderr,"\n");
        fflush(stderr);
    }

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

    if      (codetype&SPARSE6)  outcode = SPARSE6;
    else if (codetype&DIGRAPH6) outcode = DIGRAPH6;
    else                        outcode = GRAPH6;
    if (zswitch || oswitch)     outcode = DIGRAPH6;

    if (codetype&HAS_HEADER)
    {
        if (outcode == SPARSE6)       writeline(outfile,SPARSE6_HEADER);
        else if (outcode == DIGRAPH6) writeline(outfile,DIGRAPH6_HEADER);
        else                          writeline(outfile,GRAPH6_HEADER);
    }

    nauty_check(WORDSIZE,1,1,NAUTYVERSIONID);

    nin = nout = 0;
    t = CPUTIME;
    while (TRUE)
    {
        if ((g = readgg(infile,NULL,0,&m,&n,&digraph)) == NULL) break;
        ++nin;
        DYNALLOC2(graph,gsub,gsub_sz,n,m,"ransubg");
        if (dolabel) DYNALLOC2(graph,hc,hc_sz,n,m,"ransubg");
        isdig = digraph || oswitch || zswitch;

        for (ii = 0; ii < multiplicity; ++ii)
        {
            getsubgraph(g,m,n,gsub,isdig,oswitch,P1value,P2value);
            if (dolabel)
            {
                if (isdig)
                    fcanonise_inv(gsub,m,n,hc,NULL,adjacencies,0,99,0,TRUE);
                else
                    fcanonise(gsub,m,n,hc,NULL,FALSE);
                gq = hc;
            }
            else
                gq = gsub;

            if (isdig)                   writed6(outfile,gq,m,n);
            else if (outcode == SPARSE6) writes6(outfile,gq,m,n);
            else                         writeg6(outfile,gq,m,n);
            ++nout;
        }
        FREES(g);
    }
    t = CPUTIME - t;

    if (!quiet)
        fprintf(stderr,">Z %llu graphs read from %s; "
                "%llu written to to %s; %3.2f sec.\n",
                nin,infilename,nout,outfilename,t);

    exit(0);
}
