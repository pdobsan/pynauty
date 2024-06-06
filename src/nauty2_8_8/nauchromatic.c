/* nauchromatic.c version 1.0, Aug 2023, Gordon Royle and Brendan McKay.

   This file contains functions for determining the chromatic number
   of an undirected loop-free graph. The only limitation is that the
   maximum number of colours is WORDSIZE.

   If it proves impossible to colour with less than WORDSIZE colours,
   the value WORDSIZE is returned. 

   In all cases, g,m,n have their usual meanings, with m omitted if the
   function is limited to n <= WORDSIZE.  The argument minchi,maxchi
   allow testing for particular colourabilities. The return value
   is in the interval [minchi,minchi+1,...,maxchi,maxchi+1], where:
   (1) value minchi means "chromatic number <= minchi"
   (2) values from minchi+1 to maxchi are the actual chromatic number
   (3) value maxchi+1 means "chromatic number >= maxchi+1

   Thus, to determine the chromatic number use minchi=0, maxchi=n.
   To test for k-colourability use minchi=maxchi=k and the return
      value will be either k or k+1.

   chromaticnumber1(g,n,minchi,maxchi) is restricted to n <= WORDSIZE.
     It is due to Brendan McKay and is usually the fastest for n <= 30
     approximately.   
   chromaticnumber2(g,n,minchi,maxchi) is a restriction of chromaticnumber3( )
     to the case n <= WORDSIZE.  It is always faster than chromaticnumber3
     when n <= WORDSIZE.
   chromaticnumber3(g,m,n,minchi,maxchi) is code from Gordon Royle with
     modifications by Brendan McKay. It has no restrictions on n, but of
     course finding the chromatic number is a hard problem so you can't
     expect a fast response for very large graphs.
   chromaticnumber(g,m,n,minchi,maxchi) is an umbrella function that calls
     one of the other functions depending on a crude heuristic for which
     is likely to be fastest.  This should be your usual choice.
   chromaticindex(graph *g, int m, int n, int *maxdeg) is a function that
     calculates the chromatic index (edge chromatic number) and also
     returns the maximum degree (loops counting once).
*/

#include "gtools.h"

static TLS_ATTR int *a;
static TLS_ATTR set *colourset;
static TLS_ATTR int *numcols;
static TLS_ATTR set *colours;
static TLS_ATTR set colours1[WORDSIZE];
static TLS_ATTR setword colourset1[WORDSIZE];
static TLS_ATTR setword numcols1[WORDSIZE];

/*******************************************************************
* First we have code for n <= WORDSIZE. It is generally faster than
* the other functions for n <= 30 approximately.
*******************************************************************/

int
chromaticnumber1(graph *g, int n, int minchi, int maxchi)
{
    int v[WORDSIZE];  /* List of vertices */
    int c[WORDSIZE];  /* c[i] = colour of vertex i */
    int sofar[WORDSIZE];  /* sofar[i] = # colours used up to v[i] inclusive */
    setword col[WORDSIZE];  /* col[i] = {j in v[0..t] | c[j] = i} */
    int vt,chi,s,t,i,j,lim;
    setword w,left,used;
    int deg,maxdeg,imax;

    maxdeg = -1;
    for (i = 0; i < n; ++i)
    {
        deg = POPCOUNT(g[i]);
        if (deg > maxdeg) { maxdeg = deg; imax = i; }
    }

    if (maxdeg <= 1)
    {
        chi = maxdeg + 1;
        if (chi <= minchi) return minchi;
        else if (chi > maxchi) return maxchi+1;
        else return chi;
    }

    chi = (maxdeg < maxchi ? maxdeg+1 : maxchi+1);

    v[0] = imax;
    used = bit[imax];
    left = ALLMASK(n) & ~bit[imax];
    for (i = 1; i < n; ++i)
    {
        maxdeg = -1;
        w = left;
        while (w)
        {
            TAKEBIT(j,w);
            deg = (POPCOUNT(g[j]&used) << 6) + POPCOUNT(g[j]);
            if (deg > maxdeg) { maxdeg = deg; imax = j; }
        }
        v[i] = imax;
        left &= ~bit[imax];
        used |= bit[imax];
    }
 
    for (i = 0; i < n; ++i)
    {
        col[i] = 0;
        c[i] = -1;
    }

    c[v[0]] = 0;
    sofar[0] = 1;
    col[0] = bit[v[0]];
    c[v[1]] = -1;

    for (t = 1; t >= 1; )
    {
        vt = v[t];
        s = sofar[t-1];
        lim = (s < chi-2 ? s : chi-2);
        if (c[vt] >= 0) col[c[vt]] &= ~bit[vt];
        for (i = c[vt]+1; i <= lim; ++i)
            if (!(col[i] & g[vt])) break;

        if (i > lim)
        {
            --t;
            if (c[vt] < 0)
            {
                for (; (col[c[v[t]]]&~bit[v[t]]&g[vt]); --t)
                {
                    col[c[v[t]]] &= ~bit[v[t]];
                    v[t+1] = v[t];
                }
                v[t+1] = vt;
            }
        }
        else
        {
            col[i] |= bit[vt];
            c[vt] = i;
            if (i == s) sofar[t] = s+1;
            else        sofar[t] = sofar[t-1];

            if (t == n-1)
            {
                chi = sofar[n-1];
                if (chi <= minchi) return minchi;
                for (; sofar[t] >= chi; --t)
                    col[c[v[t]]] &= ~bit[v[t]];
            }
            else
            {
                ++t;
                c[v[t]] = -1;
            }
        }
    }

    return chi;
}

/*******************************************************************
* Next we have another function for n <= WORDSIZE that is the same
* as chromaticnumber3( ) but enhanced for m=1.
*******************************************************************/

static void
updateA1(setword uncolnbr, int colour)
{
    int j;
    setword w;

    w = uncolnbr;

    while (w)
    {
        TAKEBIT(j,w);
        if (++(a+WORDSIZE*j)[colour] == 1)
        {
            colours1[numcols1[j]] &= ~bit[j];
            numcols1[j]++;
            colours1[numcols1[j]] |= bit[j];
            colourset1[j] |= bit[colour];
        }
    }
}

static void
colornext1(graph *g, int n, int numsofar, int numcolors, int *color1,
          int *minsofar, setword *uncoloured, int minchi)
{
    int i,j;
    int vtocolour,maxnumcols;
    int numnay,maxnay;
    int newnumcols;
    setword w;

    if (numsofar == n)
    {
        if (numcolors < *minsofar) *minsofar = numcolors;
        return;
    }

#if 0
    maxnumcols = WORDSIZE-1;
    while (colours1[maxnumcols] == 0) maxnumcols--;
if (maxnumcols > n-1 || maxnumcols > numcolors)  /* TEMP */
{ fprintf(stderr,"n=%d maxnumcols=%d numcolors=%d\n",n,maxnumcols,numcolors);
   fflush(stderr); }
#else
   maxnumcols = numcolors;
   while (colours1[maxnumcols] == 0) maxnumcols--;
#endif

    maxnay = -1;

    j = -1;
    w = colours1[maxnumcols];
    while (w)
    {
        TAKEBIT(j,w);
        numnay = POPCOUNT(g[j] & *uncoloured);
        if (numnay > maxnay)
        {
            vtocolour = j;
            maxnay = numnay;
        }
    }

    for (i=0;i<=numcolors;i++)
    {
        if ((colourset1[vtocolour]&bit[i])) continue;

        newnumcols = (i==numcolors) ? numcolors + 1 : numcolors;

        if (newnumcols >= *minsofar) return;

        color1[vtocolour] = i;
        *uncoloured &= ~bit[vtocolour];
        colours1[numcols1[vtocolour]] &= ~bit[vtocolour];

        w = g[vtocolour] & *uncoloured;

        while (w)
        {
            TAKEBIT(j,w);
            if (++(a+WORDSIZE*j)[i] == 1)
            {
                colours1[numcols1[j]] &= ~bit[j];
                numcols1[j]++;
                colours1[numcols1[j]] |= bit[j];
                colourset1[j] |= bit[i];
            }
        }

        colornext1(g,n,numsofar+1,
                   newnumcols,color1,minsofar,uncoloured,minchi);

        if (*minsofar <= minchi) return;

        color1[vtocolour] = -1;
        *uncoloured |= bit[vtocolour];
        colours1[numcols1[vtocolour]] |= bit[vtocolour];

        w = g[vtocolour] & *uncoloured;
        while (w)
        {
            TAKEBIT(j,w);
            if (--(a+WORDSIZE*j)[i] == 0)
            {
                colours1[numcols1[j]] &= ~bit[j];
                numcols1[j]--;
                colourset1[j] &= ~bit[i];
                colours1[numcols1[j]] |= bit[j];
            }
        }
    }
}

int
chromaticnumber2(graph *g, int n, int minchi, int maxchi)
/* Same as chromnumber3() but for m=1 */
{
    int i,d;
    setword uncoloured;
    int chi,minsofar;
    int maxvert,maxdeg,maxvert2,maxdeg2;
    setword w;
    int color1[WORDSIZE];

    maxdeg = -1;
    for (i = 0; i < n; ++i)
    {
        d = POPCOUNT(g[i]);
        if (d > maxdeg)
        {
            maxdeg = d;
            maxvert = i;
        }
    }

    if (maxdeg <= 1)
    {
        chi = maxdeg + 1;
        if (chi <= minchi) return minchi;
        else if (chi > maxchi) return maxchi+1;
        else return chi;
    }

    a = calloc(n*WORDSIZE,sizeof(int));

    minsofar = (WORDSIZE < maxdeg + 1 ? WORDSIZE : maxdeg + 1);
    if (minsofar > maxchi+1) minsofar = maxchi+1;

    for (i=0;i<n;i++) color1[i] = -1;
    for (i=0;i<n;i++) colourset1[i] = 0;
    for (i=0;i<n;i++) numcols1[i] = 0;
    for (i=0;i<WORDSIZE;i++) colours1[i] = 0;   /* Not n */

    maxdeg2 = 0;
    maxvert2 = -1;

    w = g[maxvert];
    while (w)
    {
        TAKEBIT(i,w);
        d = POPCOUNT(g[i]);
        if (d > maxdeg2)
        {
            maxdeg2 = d;
            maxvert2 = i;
        }
    }

    color1[maxvert] = 0;
    color1[maxvert2] = 1;

    uncoloured = ALLMASK(n);
    uncoloured &= ~bit[maxvert];
    uncoloured &= ~bit[maxvert2];
    colours1[0] = uncoloured;

    colourset1[maxvert] = bit[1];
    colourset1[maxvert2] = bit[0];

    numcols1[maxvert] = numcols1[maxvert2] = 1;

    updateA1(g[maxvert]&uncoloured,0);
    updateA1(g[maxvert2]&uncoloured,1);

    colornext1(g,n,2,2,color1,&minsofar,&uncoloured,minchi);

    free(a);

    if (minsofar < minchi) return minchi;
    else if (minsofar > maxchi) return maxchi+1;
    else return minsofar;
}

/*******************************************************************
* Next, code that has no limits on the size of the graph but is
* limited to WORDSIZE colours.
*******************************************************************/

static void
updateA(graph *g, int m, int vertex, int colour, set *uncoloured)
{
    int i,j,jj,jm;
    setword w;

    for (jj = 0, jm = 0; jj < m; ++jj, jm += WORDSIZE)
    {
        w = (g+m*vertex)[jj] & uncoloured[jj];
        while (w)
        {
            TAKEBIT(i,w);
            j = jm + i;
            if (++(a+WORDSIZE*j)[colour] == 1)
            {
                DELELEMENT(colours+m*numcols[j],j);
                
                numcols[j]++;
                ADDELEMENT(colourset+j,colour);
                ADDELEMENT(colours+m*numcols[j],j);
            }
        }
    }    
}

static void
updateB(graph *g, int m, int vertex, int colour, set *uncoloured)
{
    int i,j,jj,jm;
    setword w;

    for (jj = 0, jm = 0; jj < m; ++jj, jm += WORDSIZE)
    {
        w = (g+m*vertex)[jj] & uncoloured[jj];
        while (w)
        {
            TAKEBIT(i,w);
            j = jm + i;

            if (--(a+WORDSIZE*j)[colour] == 0)
            {
                DELELEMENT(colours+m*numcols[j],j);

                numcols[j]--;
                DELELEMENT(colourset+j,colour);

                ADDELEMENT(colours+m*numcols[j],j);
            }
        }
    }
}

static void
colornext(graph *g, int m, int n, int numsofar, int numcolors, int *color,
          int *minsofar, set *uncoloured, int minchi)
{
    int i,j,jj;
    int vtocolour,maxnumcols;
    int numnay,maxnay;
    int newnumcols;
    graph *gj;

    if (numsofar == n)
    {
        if (numcolors < *minsofar) *minsofar = numcolors;
        return;
    }

#if 0
    maxnumcols = WORDSIZE-1;
    while (setsize(colours+m*maxnumcols,m) == 0) maxnumcols--;
#else
    j = m*(numcolors+1) - 1;
    while (colours[j] == 0) j--;
    maxnumcols = j / m;
#endif

    maxnay = -1;

    j = -1;
    while ((j = nextelement(colours+m*maxnumcols,m,j)) >= 0)
    {
        numnay = 0;
        gj = g + m*j;
        for (jj = 0; jj < m; ++jj) numnay += POPCOUNT(gj[jj]&uncoloured[jj]);
        if (numnay > maxnay)
        {
            vtocolour = j;
            maxnay = numnay;
        }
    }

    for (i=0;i<=numcolors;i++)
    {
        if (ISELEMENT(colourset+vtocolour,i)) continue;

        newnumcols = (i==numcolors) ? numcolors + 1 : numcolors;

        if (newnumcols >= *minsofar) return;
        // if (newnumcols > minchi) return;

        color[vtocolour] = i;
        DELELEMENT(uncoloured,vtocolour);
        DELELEMENT(colours+m*numcols[vtocolour],vtocolour);

        updateA(g,m,vtocolour,i,uncoloured);

        colornext(g,m,n,numsofar+1,
                  newnumcols,color,minsofar,uncoloured,minchi);

        if (*minsofar <= minchi) return;

        color[vtocolour] = -1;
        ADDELEMENT(uncoloured,vtocolour);
        ADDELEMENT(colours+m*numcols[vtocolour],vtocolour);

        updateB(g,m,vtocolour,i,uncoloured);
    }
}

int
chromaticnumber3(graph *g, int m, int n, int minchi, int maxchi)
{
    int i,j,d;
    int *color;
    int *deg;
    set *uncoloured;
    int chi,minsofar;
    int maxvert,maxdeg,maxvert2,maxdeg2;
    graph *gp;

    deg = malloc(n*sizeof(int));

    maxdeg = -1;
    gp = g;
    for (i = 0; i < n; ++i)
    {
        d = 0;
        for (j = 0; j < m; ++j)
        {
            d += POPCOUNT(*gp);
            ++gp;
        }
        deg[i] = d;
        if (d > maxdeg)
        {
            maxdeg = d;
            maxvert = i;
        }
    }

    if (maxdeg <= 1)
    {
        free(deg);
        chi = maxdeg + 1;
        if (chi <= minchi) return minchi;
        else if (chi > maxchi) return maxchi+1;
        else return chi;
    }

    uncoloured = malloc(m*sizeof(set));
    colours = malloc(WORDSIZE*m*sizeof(set));
    colourset = malloc(n*sizeof(set));
    numcols = malloc(n*sizeof(int));
    color = malloc(n*sizeof(int));
    a = calloc(n*WORDSIZE,sizeof(int));

    for (i=0;i<m;i++) uncoloured[i] =  0;
    for (i=0;i<n;i++) ADDELEMENT(uncoloured,i);

    for (i=0;i<n;i++) color[i] = -1;

    color[maxvert] = 0;
    DELELEMENT(uncoloured,maxvert);

    maxdeg2 = 0;
    maxvert2 = -1;

    i = -1;
    while ((i = nextelement(g+m*maxvert,m,i)) >= 0)
    {
        if (deg[i] > maxdeg2)
        {
            maxdeg2 = deg[i];
            maxvert2 = i;
        }
    }

    color[maxvert2] = 1;
    DELELEMENT(uncoloured,maxvert2);

    for (i=0;i<n;i++) colourset[i] = 0;
    for (i=0;i<n;i++) numcols[i] = 0;
    memset(colours,0,(size_t)WORDSIZE*m*sizeof(set));

    ADDELEMENT(colourset+maxvert,1);
    ADDELEMENT(colourset+maxvert2,0);

    numcols[maxvert] = numcols[maxvert2] = 1;

    memcpy(colours,uncoloured,m*sizeof(set));

    updateA(g,m,maxvert,0,uncoloured);
    updateA(g,m,maxvert2,1,uncoloured);

    minsofar = (WORDSIZE < maxdeg+1 ? WORDSIZE : maxdeg+1);
    if (minsofar > maxchi) minsofar = maxchi+1;
    colornext(g,m,n,2,2,color,&minsofar,uncoloured,minchi);

    free(uncoloured); 
    free(colours); free(colourset);
    free(numcols); free(color); free(deg);
    free(a);

    if (minsofar < minchi) return minchi;
    else if (minsofar > maxchi) return maxchi+1;
    else return minsofar;
}

/***************************************************************
* Now function which selects one of the other functions  according
* to a crude heuristic.  Most programs should call this one.
****************************************************************/

int
chromaticnumber(graph *g, int m, int n, int minchi, int maxchi)
{
    int i;
    graph *gi;

    if (minchi > maxchi)
        gt_abort(">E chromaticnumber() must have minchi <= maxchi\n");

    for (i = 0, gi = g; i < n; ++i, gi += m)
        if (ISELEMENT(gi,i)) return 0;  /* loop => uncolourable */

    if (minchi < 0) minchi = 0;
    if (maxchi > n) maxchi = n;
    if (maxchi > WORDSIZE) maxchi = WORDSIZE;

    if (m == 1)
    {
        if (n <= 30) return chromaticnumber1(g,n,minchi,maxchi);
        else         return chromaticnumber2(g,n,minchi,maxchi);
    }
    return chromaticnumber3(g,m,n,minchi,maxchi);
}

/***************************************************************
* Next we have a program for the chromatic index (edge chromatic
* number. We do it by constructing the linegraph, which is probably
* a terrible approach. Since most graphs are type 1, there should
* be an initial heuristic that attempts to find a colouring with
* number of colours equal to the maximum degree. Loops are allowed,
* but digraphs are not.
****************************************************************/

int
chromaticindex(graph *g, int m, int n, int *maxdeg)
/* Return the chromatic index, also the maximum degree. */
{
    int i,j,k,d,maxd,ne,me,cindex;
    size_t e,totdeg,nloops;
    graph *gi,*inc,*lg;
    
    totdeg = nloops = 0;
    maxd = 0;
    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        if (ISELEMENT(gi,i)) ++nloops;
        d = 0;
        for (j = 0; j < m; ++j) d += POPCOUNT(gi[j]);
        totdeg += d;
        if (d > maxd) maxd = d;
    }
    *maxdeg = maxd;

    if (maxd >= WORDSIZE)
       gt_abort(">E chromaticindex() can only handle max degree WORDSIZE-1\n");
    ne = nloops + (totdeg-nloops)/2;
    if (ne != nloops + (totdeg-nloops)/2 || ne > NAUTY_INFINITY-2)
        gt_abort(">E too many edges in chromaticindex()\n");

    if (ne <= 1 || maxd <= 1) return maxd;

  /* Near-regular graphs of odd order */
    
    if (nloops == 0 && n % 2 == 1 && maxd*((n-1)/2) < ne)
        return maxd + 1;

    me = SETWORDSNEEDED(ne);

    /* Make the V*E incidence matrix */

    inc = malloc(n*sizeof(graph)*me);
    if (inc == NULL) gt_abort(">E malloc() failed in chromaticindex()\n");
    EMPTYSET(inc,n*me);

    e = 0;
    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        for (j = i-1; (j = nextelement(gi,m,j)) >= 0; )
        {
            ADDELEMENT(inc+me*i,e);
            ADDELEMENT(inc+me*j,e);
            ++e;
        }
    }
    if (e != ne) gt_abort(">E edge count error in chromaticindex()\n");
 
    /* Make the linegraph */

    lg = malloc(ne*sizeof(graph)*me);
    if (lg == NULL) gt_abort(">E malloc() failed in chromaticindex()\n");

    e = 0;
    for (i = 0, gi = g; i < n; ++i, gi += m)
    {
        for (j = i-1; (j = nextelement(gi,m,j)) >= 0; )
        {
            for (k = 0; k < me; ++k)
                (lg+e*me)[k] = (inc+i*me)[k] | (inc+j*me)[k];
            DELELEMENT(lg+e*me,e);
            ++e;
        }
    }

    free(inc);

    cindex = chromaticnumber(lg,me,ne,maxd,maxd);
    free(lg);

    return cindex;
}
