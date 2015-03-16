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

#include "qsize.h"
#include "def21020.h"

.segment /pm seg_pmco;
.file "fio21k.c";
.endseg;
.segment /dm seg_dmda;
.gcc_compiled;
.endseg;

#ifdef BITT
.segment /dm db_ser;
.var _left_port;
.var _right_port;
.endseg;
#endif

#ifdef EZ
.segment /dm hip_regs;
.var _left_port;
.var _left_output;
.var _right_port;
.var _right_output;

.var _control_0;
.var _control_1;
.endseg;
#endif

#ifdef DECODER
.segment /pm seg_pmco;
.global _output_handler;
_output_handler:
	  dm(_r2_save)=r2;
	  dm(_r3_save)=r3;
          r2 = pm(i9,m14);
	  r2 = fix f2;

#ifdef BITT
	  dm(_right_port)=r2;
	  dm(_left_port)=r2;
#endif	/* BITT */
#ifdef EZ
	  r2 = ashift r2 by 16;
	  dm(_right_output)=r2;
	  dm(_left_output)=r2;
#endif	/* EZ */

#ifdef TASK3
	  r2 = dm(sample_phase);
	  r2 = r2-1;
	  if ne jump _OLH1(db);nop;nop;
	    r2 = 5;
	    dm(_vector_end) = i9;
	    bit set IRPTL  SFT0I;	/* Trigger software interrupt */
_OLH1:	  
          dm(sample_phase) = r2;
#endif
	  bit clr mode2 0x00080000;

          rti(DB);r2=dm(_r2_save);r3=dm(_r3_save);

output_vec:
	jump _output_handler;

.global _init_output;
_init_output:
	dm(i7,-2)=r2;
	dm(-1,i6)=i13;

	  B9=_thequeue;
	  B8=_thequeue;
	  L9=QSIZE;
	  L8=QSIZE;

	r0 = 0x7f000000;	/** Max gain for input, output thru
				    1/8" jack */
	dm(_control_0) = r0;
	r0=0;
	dm(_control_1) = r0;	/* 8 kHz sampling rate */

	r0=5;
	dm(sample_phase)=r0;
	
wait2111_o:
	r0=dm(_control_1);
	btst r0 by 31;
	if sz jump wait2111_o; nop ;nop;	

	pmwait = 0x1c21;
	dmwait = 0x70a421;
	
	modify(i8,10);
	
	px=pm(output_vec);
	pm(___lib_IRQ3I)=px;
	
	bit set mode2 0x00000008;	/** make IRQ3 edge-sensitive **/
	bit clr mode2 CAFRZ;		/** Un-freeze cache ***/
	bit set mode1 IRPTEN;		/** Enable all interrupts **/
	bit set imask IRQ3I;		/** Unmask IRQ3**/
	
	i13=dm(m7,i6);
	jump(m14,i13) (DB);i7=i6; i6=dm(0,i6);
.endseg;
#endif	/* DECODER */


#ifdef CODER
.segment /pm seg_pmda;
.var ioctr = 0;		/** Number of times _input_handler has been called */
.var ioprof[2] = 0,0;	/* ioprof[0] -- number of cycles between last */
			/*                   two calls to _input_handler */
			/* ioprof[1] -- stored value of the counter  */
.endseg;

#ifdef TASK3
.segment /dm seg_dmda;
.var sample_phase = 5;
.global _vector_end;	/* End of last written vector */
.var    _vector_end;
.endseg;
#endif

.segment /pm seg_pmco;
_input_handler:
	  dm(_r2_save)=r2;
	  dm(_r3_save)=r3;
!
	  r2=pm(ioctr);
	  r2 = r2+1;
	  pm(ioctr)=r2;
!
	  r2 = emuclk;
	  r3 = pm(ioprof+1);
	  r3 = r2-r3;
  	  pm(ioprof) = r3;
	  pm(ioprof+1) = r2;
!
	  r2=dm(_right_port);
	  r3=dm(_left_port);
	  r2=r2+r3;
#ifdef EZ
	  r2 = ashift r2 by -16;
#endif

	  f2 = float r2;
	  pm(i8,m14)=r2;

#ifdef TASK3
	  r2 = dm(sample_phase);
	  r2 = r2-1;
	  if ne jump _ILH1(db);nop;nop;
	    r2 = 5;
	    dm(_vector_end) = i8;
	    bit set IRPTL  SFT0I;	/* Trigger software interrupt */
_ILH1:	  
          dm(sample_phase) = r2;
#endif

	  bit clr mode2 0x00080000;
          rti(DB);r2=dm(_r2_save);r3=dm(_r3_save);


/** This is the handler for the first 5000 (skipped) samples **/

.global _input0handler;
	
.var _skip = 1000;

_input0handler:
	dm(_r2_save)=r2;
	r2 = pm(_skip);
	r2=r2-1;
	pm(_skip) = r2;

	rti(DB);r2=dm(_r2_save);nop;

/** Jump vectors: they  are copied to ___lib_IRQ3I by _init_input**/

inputvec0:
	jump _input0handler;
inputvec:
	jump _input_handler;

!!!! Initialize Input

.global	_init_input;
_init_input:
!	FUNCTION PROLOGUE: init_input
 !	rtrts protocol, params in registers, DM stack, doubles are doubles
	dm(i7,-2)=r2;
	dm(-1,i6)=i13;
!	saving registers: 
!	END FUNCTION PROLOGUE: 

!!!   Initialize circular buffer:

	  B9=_thequeue;
	  B8=_thequeue;
	  L9=QSIZE;
	  L8=QSIZE;

#ifdef BITT
	r0=5000;
#endif /* BITT */
#ifdef EZ
	r0=2000;
#endif /* EZ */
	pm(_skip)=r0;

#ifdef NOIO
i8 = _thequeue+QSIZE-1;
#else

	px = pm(inputvec0);	/** inputvec0 skips first 5000 samples **/
	pm(___lib_IRQ3I) = px;

#ifdef EZ


!! set	0x4000 -- microphone input
!!	0x2000 -- 4*3 dB right input gain
!!	0x0400 -- 4*3 dB left  input gain
!!	  	  and no attenuations (0) on either channel.

	r0 = 0x64000000;	/* Input through microphone */
	dm(_control_0) = r0;
	r0=0;
	dm(_control_1) = r0;	/* 8 kHz sampling rate */
	
wait2111_i:
	r0=dm(_control_1);
	btst r0 by 31;
	if sz jump wait2111_i; nop ;nop;	

	pmwait = 0x1c21;
	dmwait = 0x70a421;
	
#endif	/** EZ **/

	bit set mode2 0x00000008;	/** make IRQ3 edge-sensitive **/
	bit clr mode2 CAFRZ;		/** Un-freeze cache ***/
	bit set mode1 IRPTEN;		/** Enable all interrupts **/
	bit set imask IRQ3I;		/** Unmask IRQ3**/
	
/* Wait  skipped samples to be skipped: **/


_skipwait:	
	r0 = pm(_skip);	
	r0=pass r0;
	if gt jump _skipwait(DB);nop;nop;

	px=pm(inputvec);          /** Now switch to normal io handler **/
	pm(___lib_IRQ3I)=px;

#ifdef TASK3
	bit set MODE1 NESTM;		/** Allow Nested Interrupts **/
	bit set IMASK SFT0I;		/** Unmask Software Interrupt **/
	r2 = 5;
	dm(sample_phase) = r2;
#endif

#endif   /* not NOIO */

	r2 = 0;			/** Init counter **/
	pm(ioctr)=r2;

!	FUNCTION EPILOGUE: 
	i13=dm(m7,i6);
	jump(m14,i13) (DB);i7=i6; i6=dm(0,i6);
.endseg;

#endif /* CODER */

.segment /pm seg_pmda;
.global _thequeue; .var	_thequeue[QSIZE+1];
.endseg;

.segment /dm seg_dmda;
.var _r2_save;
.var _r3_save;
.var _r4_save;
.var _r5_save;
.endseg;
