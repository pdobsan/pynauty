/* Compare times for firstbitnz instructions */
/* Usage:  lztest K N
   - measures the time for 1000*N popcount operations on words
     with K one bits, comparing with the macro FIRSTBITNZ.
     Compile with values for WORDSIZE and FBNZ with FBNZ values:
     1 = __builtin_clz* */

#define MAXN WORDSIZE

#ifndef WORDSIZE 
#define WORDSIZE 32
#endif

#ifndef FBNZ
#error  Need a value for FBNZ
#endif

#if FBNZ==1
#define NEWFBNZ(x,c) {c = FIRSTBITNZ(x);}
#endif

#include "gtools.h"
#ifdef __INTEL_COMPILER
#include <nmmintrin.h>
#endif

#if  WORDSIZE==64
#define FIRSTBITMAC(x) ((x) & MSK3232 ? \
                       (x) &   MSK1648 ? \
                         (x) & MSK0856 ? \
                         0+leftbit[((x)>>56) & MSK8] : \
                         8+leftbit[(x)>>48] \
                       : (x) & MSK0840 ? \
                         16+leftbit[(x)>>40] : \
                         24+leftbit[(x)>>32] \
                     : (x) & MSK1616 ? \
                         (x) & MSK0824 ? \
                         32+leftbit[(x)>>24] : \
                         40+leftbit[(x)>>16] \
                       : (x) & MSK0808 ? \
                         48+leftbit[(x)>>8] : \
                         56+leftbit[x])
#elif WORDSIZE==32
#define FIRSTBITMAC(x) ((x) & MSK1616 ? ((x) & MSK0824 ? \
                     leftbit[((x)>>24) & MSK8] : 8+leftbit[(x)>>16]) \
                    : ((x) & MSK0808 ? 16+leftbit[(x)>>8] : 24+leftbit[x]))
#else
#define FIRSTBITMAC(x) ((x) & MSK0808 ? leftbit[((x)>>8) & MSK8] : 8+leftbit[x])
#endif

#if FBNZ==1
#define NEWFBNZ(x,c) {c = FIRSTBITNZ(x);}
#endif

static setword
ransetword(int k)     /* setword with k random bits */
{
	register setword w,rb;
	register int j;

	j = 0;
	w = 0;
	for (;;)
	{
	    if (j == k) return w;

	    rb = bit[random() % WORDSIZE];
	    if ((rb & w) == 0)
	    {
		w |= rb;
		++j;
	    }
	}
}

static double
timemac(setword *x, int n, int iters, int *sump)
{
	register int c,i,j,sum;
	register setword w;
	int it;
	double t;

	sum = 0;
	for (i = 0; i < n; ++i)
	{
	    w = x[i];
	    sum += FIRSTBITMAC(w);
	}

	t = CPUTIME;

	for (it = 0; it < iters; ++it)
	{
	    for (i = 0; i < n; ++i)
            {
                w = x[i];
                sum += FIRSTBITMAC(w);
            }
	    sum ^= it;
	}	
	    
	t = CPUTIME - t;
	*sump = sum;

	return t;
}

static double
timeold(setword *x, int n, int iters, int *sump)
{
	register int c,i,j,sum;
	register setword w;
	int it;
	double t;

	sum = 0;
	for (i = 0; i < n; ++i)
	{
	    w = x[i];
	    sum += FIRSTBITNZ(w);
	}

	t = CPUTIME;

	for (it = 0; it < iters; ++it)
	{
	    for (i = 0; i < n; ++i)
            {
                w = x[i];
                sum += FIRSTBITNZ(w);
            }
	    sum ^= it;
	}	
	    
	t = CPUTIME - t;
	*sump = sum;

	return t;
}

static double
timenew(setword *x, int n, int iters, int *sump)
{
	register int c,i,j,sum;
	register setword w;
	int it;
	double t;

	sum = 0;
	for (i = 0; i < n; ++i)
	{
	    w = x[i];
	    NEWFBNZ(w,c);
	    sum += c;
	}

	t = CPUTIME;

	for (it = 0; it < iters; ++it)
	{
	    for (i = 0; i < n; ++i)
            {
                w = x[i];
		NEWFBNZ(w,c);
                sum += c;
            }
	    sum ^= it;
	}	
	    
	t = CPUTIME - t;
	*sump = sum;

	return t;
}

static double
timenull(setword *x, int n, int iters, int *sump)
{
	register int c,i,j,sum;
	register setword w;
	int it;
	double t;

	sum = 0;
	for (i = 0; i < n; ++i)
	{
	    w = x[i];
	    NEWFBNZ(w,c);
	    sum += c;
	}

	t = CPUTIME;

	for (it = 0; it < iters; ++it)
	{
	    for (i = 0; i < n; ++i)
            {
                w = x[i];
		c = w;
                sum += c;
            }
	    sum ^= it;
	}	
	    
	t = CPUTIME - t;
	*sump = sum;

	return t;
}

int
main(int argc, char *argv[])
{
	int i,k,iters;
	setword x[1000];
	double tnull,told,tnew,tmac;
	int summac,sumold,sumnew,sumnull;

	printf("WORDSIZE=%d FBNZ=%s  ",WORDSIZE,
        FBNZ==1 ? "intrinsic" : "undefined");
#ifdef SETWORD_SHORT
        printf(" setword=unsigned short  ");
#endif
#ifdef SETWORD_INT
        printf(" setword=unsigned int ");
#endif
#ifdef SETWORD_LONG
        printf(" setword=unsigned long ");
#endif
#ifdef SETWORD_LONGLONG
        printf(" setword=unsigned long long ");
#endif
#ifdef __SSE4_2__
	printf("__SSE4_2__ ");
#endif
#ifdef __INTEL_COMPILER
	printf("__INTEL_COMPILER ");
#endif
printf("\n");

	if (argc != 3)
        {
	    fprintf(stderr,"Usage: lztest num1bits numiters\n");
	    exit(1);
        }

	k = atoi(argv[1]);
	if (k > WORDSIZE) k = WORDSIZE;
	iters = atoi(argv[2]);

	for (i = 0; i < 1000; ++i)
	    x[i] = ransetword(k);

	tnull = timenull(x,1000,iters,&sumnull);
	tmac = timemac(x,1000,iters,&summac);
	tnew = timenew(x,1000,iters,&sumnew);

	if (summac != sumnew) printf("*** sum mismatch (mac/old)\n");

	printf("macro=%3.2f new=%3.2f\n",
		tmac-tnull,tnew-tnull);

	return 0;
}
