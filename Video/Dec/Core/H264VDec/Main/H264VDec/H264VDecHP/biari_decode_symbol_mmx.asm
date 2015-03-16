; This is an alternative implementation of the 3 biari_decode_symbol functions
; that uses extensively MMX instructions.
	PUBLIC @store_dep_mmx@4
@store_dep_mmx@4	PROC NEAR
; parameter 1: ecx

 		movd    mm0, DWORD PTR [ecx];     // Drange
 		movd    mm1, DWORD PTR [ecx+12];  // Dbuffer
        pinsrw  mm1, WORD PTR [ecx+4], 2; // Ddelta
 		movd    mm2, [ecx+8];             // Dbits_to_go
 		movd    mm3, [ecx+16];            // Dcodestrm

        ret;
        ALIGN     4

; mark_end;
@store_dep_mmx@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @store_dep_mmx@4
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @load_dep_mmx@4
; mark_begin;
       ALIGN     4

	PUBLIC @load_dep_mmx@4
@load_dep_mmx@4	PROC NEAR
; parameter 1: ecx

 		movd    DWORD PTR [ecx], mm0;     // Drange
 		movd    DWORD PTR [ecx+12], mm1;  // Dbuffer
        pextrw  eax, mm1, 2;
        mov     WORD PTR [ecx+4], ax;     // Ddelta
 		movd    [ecx+8], mm2;             // Dbits_to_go
 		movd    [ecx+16], mm3;            // Dcodestrm

        ret;
        ALIGN     4

; mark_end;
@load_dep_mmx@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @load_dep_mmx@4
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_final_no_dep_mmx@0
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_final_no_dep_mmx@0
@biari_decode_final_no_dep_mmx@0	PROC NEAR
        
        ; int 3;

        pextrw    edx, mm1, 2;            // edx = Ldelta
        add       dx, 2;                  // delta += 2
        jl        LABEL1;

        mov       eax, 1;                 // return 1

        ret;

LABEL1:
        movd      eax, mm0;               // eax = Lrange
        lea       eax, [eax-2];           // range -= 2
        movd      mm0, eax;               // Lrange = range
        pinsrw    mm1, edx, 2;            // Ldelta = delta
        cmp       eax, 256;               // compare range with QUARTER
        jl        LABEL2;

        xor       eax, eax;               // return 0

        ret;

LABEL2:
        pslld     mm0, 1;                 // Lrange<<=1
        movd      ecx, mm2;               // ecx = Lbits_to_go
        add       ecx, -1;                // ecx = Lbits_to_go-1
        jl        LABEL3;

LABEL4:
        movd      mm2, ecx;               // Lbits_to_go = ecx
        psllq     mm1, 1;                 // delta = _shld(delta,Lbuffer,1), Lbuffer <<= 1
        xor       eax, eax;               // return 0

        ret;
LABEL3:
        movd      edx, mm3;               // edx = Lcodestrm
        mov       eax, [edx];             // eax = *Lcodestrm
        lea       edx, [edx+4];           // ebx = Lcodestrm+4
        bswap     eax;                    // little-endian to big-endian conversion
        movd      mm3, edx;               // Lcodestrm = edx
        movd      mm4, eax;               // Lbuffer
        lea       ecx, [ecx+32];          // ecx = 31
        por       mm1, mm4;
        jmp       LABEL4;
        
        ALIGN     4

; mark_end;
@biari_decode_final_no_dep_mmx@0 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_final_no_dep_mmx@0
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_symbol_no_dep_mmx@4
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_symbol_no_dep_mmx@4
@biari_decode_symbol_no_dep_mmx@4	PROC NEAR
; parameter 1(bi_ct): ecx

        ; int 3;

        movd      mm7, ebx;
        movd      mm6, esi;
        
        movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
        movd      esi, mm0;                                 // esi = range = Lrange
        and       edx, 63;                                  // edx = state&0x3F
        and       esi, 192;                                 // esi = range&0xc0
        movzx     eax, BYTE PTR rLPS_table_4x64[edx+esi];   // eax = rLPS
        pextrw    ebx, mm1, 2;                              // ebx = delta = Ldelta
        movd      esi, mm0;                                 // esi = range = Lrange
        movsx     edx, bx;                                  // edx = delta (orig)
        sub       esi, eax;                                 // range -= rLPS
        add       edx, eax;                                 // delta += rLPS
        cmovge    esi, eax;                                 // range = (delta<0) ? range : rLPS
        mov       eax, edx;                                 // eax = delta
        cmovge    edx, ebx;                                 // delta = (delta<0) ? delta (new) : delta (orig)
        movd      mm0, esi;                                 // Drange = range
        pinsrw    mm1, edx, 2;                              // Ddelta = delta
        sar       eax, 31;                                  // eai = lz = delta>>31
        movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
        movzx     eax, BYTE PTR AC_next_state[2*edx+eax+1]; // eax = AC_next_state[state*2+lz+1]
        mov       BYTE PTR [ecx], al;                       // bi_ct->state = AC_next_state[state*2+lz+1]
        shr       eax, 7;                                   // bit = AC_next_bit[state*2+lz+1]
        movzx     ecx, BYTE PTR times[esi];                 // ecx = bitsneeded;
;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
        ; bsr       ecx, edi;
        ; lea       ecx, [ecx-8];
        ; neg       ecx;
        movd      mm4, ecx;                                 // mm5 = bitsneeded
        pslld     mm0, mm4;                                 // range<<=bitsneeded;
        psllq     mm1, mm4;                                 // delta = _shld(delta,Lbuffer,bitsneeded), Lbuffer <<= bitsneeded
        psubd     mm2, mm4;                                 // Lbits_to_go -= bitsneeded
        movd      ecx, mm2;
        test      ecx, ecx;
        jl        LABEL_A;                                  // Prob 38%
        
LABEL_B:
        
        movd      esi, mm6;
        movd      ebx, mm7;
        
        ; int 3;
        ret;
        
LABEL_A:
        lea       esi, [ecx+32];                            // esi = Lbits_to_go+32
        movd      edx, mm3;                                 // edx = Lcodestrm
        mov       ebx, DWORD PTR [edx];                     // ebx = *Lcodestrm
        lea       edx, [edx+4];                             // 
        bswap     ebx;                                      // little-endian to big-endian conversion
        movd      mm3, edx;                                 // Lcodestrm += 4
        neg       ecx;
        movd      mm4, ecx;                                 // mm4 = (-Lbits_to_go), so positive
        movd      mm5, ebx;                                 // mm5 = buffer
        psrld     mm1, mm4;
        por       mm1, mm5;
        psllq     mm1, mm4;
        movd      mm2, esi;                                 // Lbits_to_go+=32
        jmp       LABEL_B;
        
        ALIGN     4

; mark_end;
@biari_decode_symbol_no_dep_mmx@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_symbol_no_dep_mmx@0
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_symbol_eq_prob_no_dep_mmx@4
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_symbol_eq_prob_no_dep_mmx@0
@biari_decode_symbol_eq_prob_no_dep_mmx@0	PROC NEAR

        ; int 3;

        movd      edx, mm2;         // edx = Lbits_to_go
        add       edx, -1;          // edx = Lbits_to_go-1
        jl        LABEL_NEG;

LABEL_ELSE:
        movd      mm2, edx;         // Lbits_to_go = edx
        psllq     mm1, 1;
        pshufw    mm5, mm0, 79;     // mm5 = (0, range, 0, 0)
        paddsw    mm1, mm5;
        pextrw    eax, mm1, 2;      // eax = Ldelta
        sar       ax, 15;           // range>>=31 (lz)
        movd      mm4, eax;         // mm4 = (0, 0, 0, lz)
        psllq     mm4, 32;          // mm4 = (0, lz, 0, 0)
        pandn     mm4, mm5;         // mm4 = (0,~lz&range,0,0)
        add       ax, 1;            // return (lz +1)
        psubsw    mm1, mm4;
        
        ; int 3;
        ret;

LABEL_NEG:
        movd      ecx, mm3;         // ecx = Lcodestrm
        mov       eax, [ecx];       // eax = *Dcodestrm
        lea       ecx, [ecx+4];     // ecx = Lcodestrm+4
        bswap     eax;              // little-endian to big-endian conversion
        movd      mm3, ecx;         // Lcodestrm = ecx
        movd      mm4, eax;
        lea       edx, [edx+32];    // edx = 31
        por       mm1, mm4;         // Pack delta with buffer
        jmp       LABEL_ELSE;
        
        ALIGN     4

; mark_end;
@biari_decode_symbol_eq_prob_no_dep_mmx@0 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_symbol_eq_prob_no_dep_mmx@0
