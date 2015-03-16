#ifndef FAST_H
#define FAST_H

#define PROD5(A,B,R) \
{ typeof (R) __x,__y,__t;	\
  typeof (&A[0]) __pa=A;	\
  typeof (&B[0]) __pb=B;	\
                          __x=*__pa++;__y=*__pb++;	\
  R=R-R;    __t=__x*__y;  __x=*__pa++;__y=*__pb++;	\
  R+=__t;   __t=__x*__y;  __x=*__pa++;__y=*__pb++;	\
  R+=__t;   __t=__x*__y;  __x=*__pa++;__y=*__pb++;	\
  R+=__t;   __t=__x*__y;  __x=*__pa++;__y=*__pb++;	\
  R+=__t;   __t=__x*__y;	\
  R+=__t;}


/* Autocorrelation : R[0:K] is autocorrelation of X[M:N-1]  i.e.
      R[k] = Sum X[i]*X[i-k]   for    M<=i<N     */

#if 1

#define AUTOCORR(X,R,K,M,N)	\
{   int __k,__i,__M1=M+1;	\
    typeof (&R[0]) __rp=R;	\
    typeof (*R)    __corr=0;	\
    for(__k=0; __k<=K; __k++) {	\
        typeof (*R)  __t;	\
	typeof (&X[0]) __xip=X+M, __xkp=__xip-__k;	\
	typeof (X[0])  __xi, __xk;	\
	__corr=__corr-__corr;	\
	__xi=*__xip++;	\
	__xk=*__xkp++;	\
	for(__i=__M1; __i<N; __i++) {	\
	    __t=__xi * __xk; __xi= *__xip++;	\
	    __corr+=__t;     __xk= *__xkp++;	\
	}	\
	__t=__xi*__xk;	\
	__corr+=__t;	\
	*(__rp++)=__corr;}}
#else
#define AUTOCORR(X,R,K,M,N) \
{\
    int  ii,jj;\
    for(ii=0; ii<=K; ii++) {\
	R[ii] = 0;\
	for(jj=M; jj<N; jj++) R[ii] += X[jj]*X[jj-ii];}}
#endif


/* if AC is autocorrelation of X (as in above) , then
   R[i] = D * R[i] + AC[i]    */

#define RECACORR(X,R,K,M,N,D)	\
{   int __k,__i,__M1=M+1;	\
    typeof (&R[0]) __rp=R;	\
    for(__k=0; __k<=K; __k++) {	\
	typeof (*R) __corr, __t;	\
	typeof (&X[0]) __xip=X+M, __xkp=__xip-__k;	\
	typeof(X[0]) __xi,__xk;	\
	__corr=__corr-__corr;	\
	__xi=*__xip++;	\
	__xk=*__xkp++;	\
	for(__i=__M1;__i<N;__i++) {	\
	    __t=__xi*__xk; __xi=*__xip++;	\
	    __corr+=__t; __xk=*__xkp++;	\
	}	\
	__t=__xi*__xk;	\
	__corr+=__t;	\
	__corr=*__rp*(D)+__corr;	\
	*__rp++=__corr;}}


/** Z[i] = X[i]*Y[i] **/

#define VPROD(X,Y,Z,N)	\
{   typeof (&X[0]) __xp=X;	\
    typeof (&Y[0]) __yp=Y;	\
    typeof (&Z[0]) __zp=Z;	\
    int i;	\
    typeof (X[0]) __xi=*__xp++;	\
    typeof (Y[0]) __yi=*__yp++;	\
    typeof (Z[0]) __zi;	\
    for(i=1;i<N;i++) {	\
	__zi=__xi*__yi;	\
	__xi=*__xp++;	\
	__yi=*__yp++;	\
	*__zp++=__zi;}	\
    __zi=__xi*__yi;	\
    *__zp=__zi;}

#define RSHIFT(START,LENGTH,SHIFT) \
{ typeof (&(START)[0]) __from = &(START)[LENGTH-1]; \
  typeof (&(START)[0]) __to = &(START)[LENGTH+SHIFT-1]; \
  int __i; for(__i=0; __i<LENGTH; __i++)      *__to-- = *__from--;\
}


/** Must have at least two elements, i.e. N >= 2.
  computes sum(x[i]*y[i]) **/

#define DOTPROD(X,Y,N) \
({ int __i;	\
  typeof((X)[0]*(Y)[0]) __r=0, __t;	\
  typeof ((X)[0]) __x1;	\
  typeof ((Y)[0]) __y1;	\
  typeof (&(X)[0]) __xp = &(X)[0]; \
  typeof (&(Y)[0]) __yp = &(Y)[0]; \
                              __x1=*__xp++; __y1=*__yp++; 	\
             __t=__x1*__y1;   __x1=*__xp++; __y1=*__yp++; 	\
   for(__i=0; __i<(N)-2; __i++) {	\
   __r+=__t; __t=__x1*__y1;   __x1=*__xp++; __y1=*__yp++; }	\
   __r+=__t; __t=__x1*__y1;   	\
   __r+=__t; __r;})

#endif /*FAST_H*/
