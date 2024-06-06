/* Prototypes for the functions in nauconnect.c */

#ifndef  _NAUCONNECT_H_    /* only process this file once */
#define  _NAUCONNECT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int maxvertexflow1(graph*,int,int,int,int,boolean);
extern int maxvertexflow(graph*,graph*,set*,int*,int*,
                                    int,int,int,int,int,boolean);
extern int connectivity(graph*,int,int,boolean);
extern boolean isthisconnected(graph*,int,int,int,boolean);
extern int maxedgeflow1(graph*,int,int,int,int);
extern int maxedgeflow(graph*,graph*,int,int,int,int,set*,int*,int*,int);
extern int edgeconnectivity(graph*,int,int);
extern boolean isthisedgeconnected(graph*,int,int,int);

#ifdef __cplusplus
}
#endif

#endif
