/* These are definitions for the assembler code of the cb_index routine */
/* Essentialy they represent register allocation for the function */

#define DM1	m6
#define DMZERO	m5
#define PM1	m14
#define PMZERO	m13
#define b0	f4
#define b1	f8
#define b2	f7
#define cgm0	f2
#define cgm1	f1
#define cgm2	f0
#define cgm2save	m4
#define cor	f12
#define d	f8
#define distm	f6
#define energy	f5
#define enp	i0
#define foura	i11
#define fzero	f9
#define g2	f8
#define g2p	m2
#define gsq	f4
#define gsqp	m1
#define idxg	i1
#define ig	r10
#define is	r11
#define j	r13
#define minus5	m0
#define pcor	f3
#define pnp	i4
#define prod	f8
#define shptr	i12
#define tmp1	f0
#define tmp2	f4
#define x	f0
#define y	f4

/*		Register Usage */
/* r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13         */
/* i0 i1       i4                    i11 i12             */
/* m0 m1 m2    m4 m5 m6                      m13 m14     */

#define SAVEREGS i13 = __pmsave__; 	pm(i13,m14)=i0;	pm(i13,m14)=i1;	\
	pm(i13,m14)=i4;	pm(i13,m14)=m0;	pm(i13,m14)=m1;	pm(i13,m14)=m2;	\
	pm(i13,m14)=m4;	dm( -3,i6)=r3;	dm( -4,i6)=r5;	dm( -5,i6)=r6;	\
	dm( -6,i6)=r7;	dm( -7,i6)=r9;	dm( -8,i6)=r10;	dm( -9,i6)=r11;	\
	dm(-10,i6)=r13;	dm(-11,i6)=i11;	dm(-12,i6)=i12;	 
#define RESTOREREGS i13 = __pmsave__;	i0=pm(i13,m14);	i1=pm(i13,m14);	\
	i4=pm(i13,m14);	m0=pm(i13,m14);	m1=pm(i13,m14);	m2=pm(i13,m14);	\
	m4=pm(i13,m14);	r3=dm( -3,i6);	r5=dm( -4,i6);	r6=dm( -5,i6);	\
	r7=dm( -6,i6);	r9=dm( -7,i6);	r10=dm( -8,i6);	r11=dm( -9,i6);	\
	r13=dm(-10,i6);	i11=dm(-11,i6);	i12=dm(-12,i6);	 
