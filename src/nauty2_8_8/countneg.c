/* countneg.c version 1.0; B D McKay, Dec 2022 */

#define USAGE "countneg [-ne] [infile]"

#define HELPTEXT \
"  Count graphs by number of vertices and/or number of edges\n\
\n\
     -n  Count by vertices\n\
     -e  Count by edges\n\
        Default and -ne are to count by both\n\
     -q  Suppress auxiliary output.\n\
  Use countg instead if incremental inputs are present.\n"

#include "gtools.h"

#define SPLAYNODE struct splay_t
struct splay_t
{
   SPLAYNODE *left,*right,*parent;
   long long n,e,count;
};

#define SCAN_ARGS
#define INSERT_ARGS , int n, size_t e
#define ACTION(p) { printf("%9lld ",(p)->count); \
                    if ((p)->n >= 0) printf(" n=%lld",(p)->n);\
                    if ((p)->e >= 0) printf(" e=%lld",(p)->e); \
                    printf("\n"); }
#define COMPARE(p) ((p)->n < n ? 1 : (p)->n > n ? -1 : e - (p)->e)
#define PRESENT(p) ++((p)->count)
#define NOT_PRESENT(p) { (p)->n = n; (p)->e = e; (p)->count = 1; }

static SPLAYNODE *root;
#include "splay.c"

int
main(int argc, char *argv[])
{
    int codetype;
    char *infilename,*line;
    FILE *infile;
    int argnum,j;
    char *arg,sw;
    boolean nswitch,eswitch,qswitch;
    boolean badargs;
    nauty_counter nin;
    int n;
    size_t e;
    long long lln,lle;
    double t0,t1;

    HELP; PUTVERSION;

    eswitch = nswitch = qswitch = FALSE;
    infilename = NULL;

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
                     SWBOOLEAN('e',eswitch)
                else SWBOOLEAN('n',nswitch)
                else SWBOOLEAN('q',qswitch)
                else badargs = TRUE;
            }
        }
        else
        {
            ++argnum;
            if      (argnum == 1) infilename = arg;
            else                  badargs = TRUE;
        }
    }

    if (badargs || argnum > 1)
    {
        fprintf(stderr,">E Usage: %s\n",USAGE);
        GETHELP;
        exit(1);
    }

    if (!nswitch && !eswitch) nswitch = eswitch = TRUE;

    if (!qswitch)
    {
        fprintf(stderr,">A %s",argv[0]);
        if (nswitch && eswitch) fprintf(stderr," -ne");
        else if (eswitch) fprintf(stderr," -e");
        else if (nswitch) fprintf(stderr," -n");
        if (argnum > 0) fprintf(stderr," %s",infilename);
        fprintf(stderr,"\n");
        fflush(stderr);
    }

    if (infilename && infilename[0] == '-') infilename = NULL;
    infile = opengraphfile(infilename,&codetype,FALSE,1);
    if (!infile) exit(1);
    if (!infilename) infilename = "stdin";

    nin = 0;
    root = NULL;
    t0 = CPUTIME;

    while ((line = gtools_getline(infile)) != NULL)
    {
        if (line[0] == ';')
            gt_abort(">E incremental sparse6 is not supported; use countg\n");
        ++nin;
        if (eswitch)
        {
            stringcounts(line,&n,&e);
            lln = (nswitch ? n : -1);
            lle = e;
        }
        else
        {
            lln = (long long) graphsize(line);
            lle = -1;
        }
 
        splay_insert(&root,lln,lle);
    }

    splay_scan(root);

    t1 = CPUTIME;

    if (!qswitch)
        fprintf(stderr,">Z " COUNTER_FMT " graphs altogether; %.2f sec\n",
                       nin,t1-t0);

    exit(0);
}
