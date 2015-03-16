/* This file contains the interrupt table for the ADSP-21020		*/

/* When the C program exits either by returning from main() or by an 	*/
/* explicit or implicit call to exit(), control will transfer to the 	*/
/* label ___lib_prog_term.  Currently, the ___lib_prog_term label is 	*/
/* defined at the end of the reset vector as an IDLE instruction.    	*/
/* If your application needs to perform some operation AFTER the C   	*/
/* program has finished executing, remove the ___lib_prog_term label 	*/
/* from the runtime header, and place it at the beginning of your    	*/
/* code.							     	*/

.GLOBAL		___lib_prog_term;			    	/*Termination address	*/

.SEGMENT/PM	    seg_rth;					/*Runtime header segment*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

___lib_RSTI:	 	CALL ___lib_setup_hardware;	    	/*Reset Interrupt 	*/
		 	CALL ___lib_setup_processor;
		 	CALL ___lib_setup_environment; 
		 	JUMP _main (DB);	    		/*Begin user progam 	*/
			NOP;NOP;
___lib_prog_term:	IDLE;
			JUMP ___lib_prog_term;			/*Stay at idle */

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

/* Interrupt vector for status stack/loop stack overflow or PC stack full: 		*/
___lib_SOVFI:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/

			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

/* Interrupt vector for high priority timer interrupt: */
___lib_TMZOI:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/


/* Interrupt vector for external interrupts:*/



.global ___lib_IRQ3I;
___lib_IRQ3I:		idle;
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			nop; /* BIT SET MODE2 0x00080000; Freeze cache */
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_IRQ2I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_IRQ1I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_IRQ0I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

/* Interrupt vector for DAG1 buffer 7 circular buffer overflow				*/
___lib_CB7I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

/* Interrupt vector for DAG2 buffer 15 circular buffer overflow				*/
___lib_CB15I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt 	*/

/* Interrupt vector for lower priority timer interrupt					*/
___lib_TMZI:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

/* Interrupt vector for fixed point overflow interrupt					*/
___lib_FIXI:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

/* Interrupt vector for floating point overflow interrupt				*/
___lib_FLTOI:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

/* Interrupt vector for floating point underflow interrupt				*/
___lib_FLTUI:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

/* Interrupt vector for floating point invalid operation interrupt			*/
___lib_FLTII:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

/* Interrupt vectors for user interrupts 0 - 7						*/
___lib_USR0I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			nop;					/* DO NOT Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_USR1I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_USR2I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_USR3I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_USR4I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_USR5I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_USR6I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

___lib_USR7I:		JUMP ___lib_int_cntrl (DB);		/*Jump to dispatcher 	*/
			BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			BIT SET MODE2 0x00080000;		/*Freeze cache 		*/
			NOP;NOP;NOP;NOP;NOP;			/*Pad to next vector 	*/

.ENDSEG;

