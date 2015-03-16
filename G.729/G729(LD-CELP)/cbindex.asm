/*************************************************************************/
/*                                                                       */
/*                            LD-CELP  G.728                             */
/*                                                                       */
/*    Low-Delay Code Excitation Linear Prediction speech compression.    */
/*                                                                       */
/*                 Copyright: Analog Devices, Inc., 1993                 */
/*                                                                       */
/*                         Author: Alex Zatsman.                         */
/*                                                                       */
/*  This program was written mostly for testing  Analog Devices' g21k C  */
/*  compiler for the  ADSP21000 architecture  family. While the program  */
/*  works  on  Sparc and ADSP21020, it  has  NOT  been  tested with the  */
/*  official test data from CCITT/ITU.                                   */
/*                                                                       */
/*  The program  is   distributed as  is,  WITHOUT ANY WARRANTY, EITHER  */
/*  EXPLICIT OR IMPLIED.                                                 */
/*                                                                       */
/*************************************************************************/

#include "cbindex.h"
.segment /pm seg_pmco; .file "cbindex.S";
.var __pmsave__[15];
.var _four = 4;
.endseg;

.segment /pm seg_pmco;
.extern	_cb_gain2;
.extern	_cb_gain_sq;
.extern	_shape_energy;

.global	_cb_index;
_cb_index:
	dm(i7,-18)=r2;
	dm(-1,i6)=i13;
	SAVEREGS;

	pnp	=r4;
	cgm0	= 0x3f358000;
	cgm1	= 0x3f9ed000;
	enp	=_shape_energy;
	cgm2save = 0x400af600;
	shptr	=_cb_shape;
	minus5	=-5;
	distm	= 0x72fc6f7c;
	g2p	=_cb_gain2;
	is	= is-is;
	ig	= 0;
	cor	= 0;
	gsqp	=_cb_gain_sq;
	foura	= _four;
	j	= -1;

	lcntr = 128,  do _L41-1 until lce;

	cor=cor-cor,            y=dm(pnp,DM1), x=pm(shptr,PM1);	

	prod=x*y,               y=dm(pnp,DM1), x=pm(shptr,PM1);	 

	prod=x*y, cor=prod+cor, y=dm(pnp,DM1), x=pm(shptr,PM1);	 

	prod=x*y, cor=prod+cor, y=dm(pnp,DM1), x=pm(shptr,PM1);	 

	prod=x*y, cor=prod+cor, y=dm(pnp,DM1), x=pm(shptr,PM1);	 

	prod=x*y, cor=prod+cor, energy=dm(enp,DM1);	 

	b0=cgm0*energy, 	cor=prod+cor, cgm2=cgm2save;

	b1=cgm1*energy,	pcor = fzero - cor, idxg = pm(foura,PMZERO);

	if lt  pcor = pass cor, idxg=DMZERO;

	b2=cgm2*energy,		modify(pnp, minus5);

	comp(pcor,b0);	
	
	if gt	comp(pcor,b1), modify(idxg, DM1);

	if gt	comp(pcor,b2), modify(idxg, DM1);

	if gt                  modify(idxg, DM1);
	
	j=j+1, gsq=dm(gsqp,idxg);

	tmp1=gsq*energy,g2=dm(g2p,idxg);

	tmp2=g2*cor;

	d=tmp1-tmp2;

	comp(d,distm);

	if lt is=pass j, ig = idxg;

	distm = min(d,distm);

_L41:

	is = ashift is by 3;
	r0 = is + ig;

	RESTOREREGS;
	i13=dm(m7,i6);
	jump(m14,i13) (DB);i7=i6; i6=dm(0,i6);
.endseg;
