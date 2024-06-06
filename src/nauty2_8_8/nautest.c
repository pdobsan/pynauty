/* Test for basic nauty functions (but not nauty itself) */

#include "nauty.h"

int
main(int argc, char *argv[])
{
    int i,j,n,bad,sz;
    setword w,ww;
    int curfile;
    FILE *f;
    set ss[4];

    bad = 0;

    printf("NAUTYVERSION=%s  NAUTYVERSIONID=%d  HAVE_TLS=%d\n",
            NAUTYVERSION,NAUTYVERSIONID,HAVE_TLS);
    printf("MAXN=%d  MAXM=%d  WORDSIZE=%d  NAUTY_INFINITY=%d\n",
            MAXN,MAXM,WORDSIZE,NAUTY_INFINITY);
    printf("sizes: short=%d int=%d long=%d double=%d boolean=%d"
           " pointer=%d setword=%d\n",
            (int)sizeof(short),(int)sizeof(int),(int)sizeof(long),
            (int)sizeof(double),(int)sizeof(boolean),
            (int)sizeof(void*),(int)sizeof(setword));
    printf("CLZ=%d,%d,%d  POPCNT=%d,%d,%d;%d,%d",
            HAVE_CLZ,HAVE_CLZL,HAVE_CLZLL,HAVE_POPCNT,
            HAVE_POPCNTL,HAVE_POPCNTLL,HAVE_MMPOP32,HAVE_MMPOP64);
    printf("  LONG_LONG_COUNTERS=%d  COUNTER_FMT=%s\n",
            LONG_LONG_COUNTERS,COUNTER_FMT);

    printf("Defined: ");
#ifdef __STDC__
    printf(" __STDC__=%d",__STDC__);
#endif
#ifdef BIGNAUTY
    printf(" BIGNAUTY(obsolete!)");
#endif
#ifdef SYS_UNIX
    printf(" SYS_UNIX");
#endif
#ifdef SYS_CRAY
    printf(" SYS_CRAY");
#endif
#ifdef IS_ARM64
    printf(" IS_ARM64=%d",IS_ARM64);
#endif
#ifdef _MSC_VER
    printf(" _MSC_VER=%d",_MSC_VER);
#endif
#ifdef __INTEL_COMPILER
    printf(" __INTEL_COMPILER");
#endif
#ifdef __POPCNT__
    printf(" __POPCNT__");
#endif
#ifdef SETWORD_SHORT
    printf(" SETWORD_SHORT");
#endif
#ifdef SETWORD_INT
    printf(" SETWORD_INT");
#endif
#ifdef SETWORD_LONG
    printf(" SETWORD_LONG");
#endif
#ifdef SETWORD_LONGLONG
    printf(" SETWORD_LONGLONG");
#endif
#ifdef SETWORD_128
    printf(" SETWORD_128");
#endif
#if SIZEOF_UNINT128 > 0
    printf(" unsigned __int128");
#endif
#if SIZEOF_UINT128_T > 0
    printf(" __uint128_t");
#endif
    printf("\n");

    printf("Capabilities: ");

#if defined(__x86_64__)
    printf("x86_64");
#elif defined(__x86_32__)
    printf("x86_32");
#elif defined(__x86__)
    printf("x86");
#endif

#if defined(__SSE4_2__)
    printf(" SSE4_2");
#elif defined(__SSE4_1__)
    printf(" SSE4_1");
#elif defined(__SSE3__)
    printf(" SSE3");
#elif defined(__SSE2__)
    printf(" SSE2");
#elif defined(__SSE__)
    printf(" SSE");
#endif

#if defined(__MMX__)
    printf(" MMX");
#endif

#if defined(__AVX2__)
    printf(" AVX2");
#elif defined(__AVX__)
    printf(" AVX");
#endif

#if defined(__BMI2__)
    printf(" BMI2");
#elif defined(__BMI__)
    printf(" BMI");
#endif

#if defined(__ABM__)
    printf(" ABM");
#endif

    printf("\n");

#if SIZEOF_LONGLONG > 0
    if (sizeof(long long) != SIZEOF_LONGLONG)
    {
        printf("\nSIZEOF_LONGLONG is wrong (%d, should be %d)\n\n",
                    SIZEOF_LONGLONG,(int)sizeof(long long));
        ++bad;
    }
    printf("sizeof(long long)=%d\n",sizeof(long long));
#endif

    if (sizeof(long) != SIZEOF_LONG)
    {
        printf("\n ***** SIZEOF_LONG is wrong (%d, should be %d) ***** \n\n",
                    SIZEOF_LONG,(int)sizeof(long));
        ++bad;
    }

    if (sizeof(int) != SIZEOF_INT)
    {
        printf("\n ***** SIZEOF_INT is wrong (%d, should be %d) ***** \n\n",
                    SIZEOF_INT,(int)sizeof(int));
        ++bad;
    }

    if (sizeof(void*) != SIZEOF_POINTER)
    {
        printf("\n ***** SIZEOF_POINTER is wrong (%d, should be %d) ***** \n\n",
                    SIZEOF_POINTER,(int)sizeof(void*));
        ++bad;
    }

    if (8*sizeof(setword) != WORDSIZE)
    {
        printf("\n ***** WORDSIZE is not 8 times sizeof(setword) ***** \n\n");
        ++bad;
    }

    for (i = 0; i < WORDSIZE; ++i)
    {
        if (!(BITT[i]&ALLBITS))
        {
            printf("\n ***** ALLBITS error %d *****\n\n",i);
            ++bad;
        }
    }

    for (i = 0; i <= WORDSIZE; ++i)
    {
        w = ALLMASK(i);
        if (POPCOUNT(w) != i)
        {
            printf("\n ***** POPCOUNT(ALLMASK) error %d,%d *****\n\n",
                    i,POPCOUNT(w));
            ++bad;
        }
    }

    for (i = 0; i <= WORDSIZE; ++i)
    {
        w = ALLMASK(i);
        if (POPCOUNTMAC(w) != i)
        {
            printf("\n ***** POPCOUNTMAC(ALLMASK) error %d,%d *****\n\n",
                    i,POPCOUNTMAC(w));
            ++bad;
        }
    }

    for (i = 0; i < WORDSIZE; ++i)
    {
        w = BITMASK(i);
        if (POPCOUNT(w) != WORDSIZE-i-1)
        {
            printf("\n ***** POPCOUNT(BITMASK) error %d *****\n\n",i);
            ++bad;
        }
    }

    for (i = 0; i < WORDSIZE; ++i)
        if (FIRSTBIT(BITT[i]) != i)
        {
            printf("\n ***** FIRSTBIT(BITT) error %d,%d *****\n\n",
                   i,FIRSTBIT(BITT[i]));
            ++bad;
        }

    if (FIRSTBIT((setword)0) != WORDSIZE)
    {
        printf("\n ***** FIRSTBIT(0) error *****\n\n");
        ++bad;
    }
    
    if (FIRSTBITMAC((setword)0) != WORDSIZE)
    {
        printf("\n ***** FIRSTBITMAC(0) error *****\n\n");
        ++bad;
    }
    
    for (i = 0; i < WORDSIZE; ++i)
        if (POPCOUNT(BITT[i]) != 1)
        {
            printf("\n ***** POPCOUNT(BITT) error %d *****\n\n",i);
            ++bad;
        }
    
    for (i = 0; i < WORDSIZE; ++i)
        if (POPCOUNTMAC(BITT[i]) != 1)
        {
            printf("\n ***** POPCOUNTMAC(BITT) error %d *****\n\n",i);
            ++bad;
        }

    for (i = 0; i < WORDSIZE; ++i)
    {
        w = 0;
        for (j = 1; j <= WORDSIZE; ++j)
        {
            w |= BITT[(j*97+i)%WORDSIZE];
            if (POPCOUNT(w) != j)
            {
                printf("\n ***** POPCOUNT(w) error %d %d *****\n\n",i,j);
                ++bad;
            }
        }
    }

    for (i = 0; i < 4*WORDSIZE; ++i)
    {
        EMPTYSET(ss,4);
        ADDELEMENT(ss,i);
        if (!ISELEMENT(ss,i))
        {
            printf("\n ***** ISELEMENT failed *****\n");
            ++bad;
            break;
        }
    }

    for (n = 0; n <= 3*WORDSIZE; ++n)
    {
        ss[0] = ss[1] = ss[2] = ss[3] = 1;
        FILLSET(ss,3,n);
        if (ss[3] != 1)
        {
            printf("\n ***** FILLSET overfilled *****\n\n");
            ++bad;
        }
        else
        {
            for (i = 0; i < 3*WORDSIZE; ++i)
            {
                if (ISELEMENT(ss,i))
                {
                    if (i >= n)
                    {
                        printf("\n ***** FILLSET overfilled *****\n\n");
                        ++bad;
                    }
                    break;
                }
                else
                {
                    if (i < n)
                    {
                        printf("\n ***** FILLSET underfilled *****\n\n");
                        ++bad;
                    }
                    break;
                }
            }
        }
        SETSIZE(sz,ss,3);
        if (sz != n)
        {
            printf("\n ***** SETSIZE error *****\n\n");
            ++bad;
        }
    }

    if (!bad) printf("No errors found\n\n");
    else      printf("\nXXXXXXX %d errors found XXXXXXX\n\n",bad);

    exit(bad != 0);
} 
