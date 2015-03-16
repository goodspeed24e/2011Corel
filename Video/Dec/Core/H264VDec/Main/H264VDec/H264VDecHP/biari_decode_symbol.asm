; The following 3 flags need to be manually set in accordance to those in global.h
; The third flag (CONFIG_BIARI_ENABLE_MMX), when enabled can actually take 2 values, 1/2
CONFIG_BIARI_ENABLE_GLOBAL EQU 1
CONFIG_BIARI_ENABLE_ASM EQU 1
; CONFIG_BIARI_ENABLE_MMX EQU 1
; -- Machine type PW
; mark_description "Intel(R) C++ Compiler for 32-bit applications, Version 8.0   Build 20031017Z %s";
; mark_description "-Qvc7.1 -Qlocation,link,C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7\\Bin -c -Qvc7.1 -Qlocation";
; mark_description ",link,C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7/bin -Ox -Ob2 -Oi -Ot -G7 -QaxW -QxW -GF -FD ";
; mark_description "-EHs -EHc -MD -Gy -arch:SSE2 -Zi -nologo -W3 -D WIN32 -D NDEBUG -D _LIB -D USE_H264_INTERFACE -D _MBCS -YXSt";
; mark_description "dAfx.h -Fp.\\Release/H264VDecHP.pch -FAs -Fa.\\Release/ -Fo.\\Release/ -Fd.\\Release/ -Gr -TP";
;ident "Intel(R) C++ Compiler for 32-bit applications, Version 8.0   Build 20031017Z %s"
;ident "-Qvc7.1 -Qlocation,link,C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\Bin -c -Qvc7.1 -Qlocation,link,C:\Program Files"
	.686P
 	.387
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
	ALIGN 004H
_DATA	ENDS
_BSS	SEGMENT DWORD PUBLIC USE32 'BSS'
	ALIGN 004H
_BSS	ENDS
_RDATA	SEGMENT DWORD PUBLIC USE32 'DATA'
	ALIGN 004H
_RDATA	ENDS
_TLS	SEGMENT DWORD PUBLIC USE32 'TLS'
	ALIGN 004H
_TLS	ENDS
_DATA1	SEGMENT DWORD PUBLIC USE32 'DATA'
	ALIGN 004H
_DATA1	ENDS
_TEXT1	SEGMENT DWORD PUBLIC USE32 'CODE'
	ALIGN 004H
_TEXT1	ENDS
	ASSUME	CS:FLAT,DS:FLAT,SS:FLAT
;ident "-defaultlib:uuid.lib"
;ident "-defaultlib:uuid.lib"

IF @Version GE 612
  .MMX
  MMWORD TEXTEQU <QWORD>
ENDIF
IF @Version GE 614
  .XMM
  XMMWORD TEXTEQU <OWORD>
ENDIF

DEFINED MACRO symbol:REQ
IFDEF symbol
EXITM <-1> ;; True
ELSE
EXITM <0> ;; False
ENDIF
ENDM

IF DEFINED(CONFIG_BIARI_ENABLE_ASM)

IF NOT DEFINED(CONFIG_BIARI_ENABLE_GLOBAL)

_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_final_dep_asm@8
; mark_begin;
       ALIGN     4
	PUBLIC @biari_decode_final_dep_asm@4
@biari_decode_final_dep_asm@4	PROC NEAR
; parameter 1(dep): ecx

        mov       edx, [ecx+4];           // edx = Ddelta
        add       edx, 2;                 // delta += 2
        jl        LABEL1;

        mov       eax, 1;                 // return 1

        ret;

LABEL1:
        mov       eax, [ecx];             // eax = Drange
        lea       eax, [eax-2];           // range -= 2
        cmp       eax, 256;               // compare range with QUARTER
        jl        LABEL2;

        mov       [ecx], eax;             // Drange = range
        mov       [ecx+4], edx;           // Ddelta = delta
        xor       eax, eax;               // return 0

        ret;

LABEL2:
        lea       eax, [eax+eax];         // range<<=1
        mov       [ecx], eax;             // Drange = range
        mov       eax, [ecx+8];           // eax = Dbits_to_go
        add       eax, -1;                // eax = Dbits_to_go-1
        jl        LABEL3;

LABEL4:
        mov       [ecx+8], eax;           // Dbits_to_go = eax
        mov       eax, [ecx+12];          // eax = Dbuffer
        shld      edx, eax, 1;            // delta = _shld(delta,Dbuffer,1)
        lea       eax, [eax+eax];         // Dbuffer <<= 1
        mov       [ecx+12], eax;          // Dbuffer
        mov       [ecx+4], edx;           // Ddelta = delta
        xor       eax, eax;               // return 0

        ret;
LABEL3:
        mov       eax, [ecx+16];          // ebx = Dcodestrm
        lea       eax, [eax+4];           // eax = Dcodestrm+4
        mov       [ecx+16], eax;          // Dcodestrm = ebx
        mov       eax, [eax-4];           // eax = *Dcodestrm
        bswap     eax;                    // little-endian to big-endian conversion
        mov       [ecx+12], eax;          // Dbuffer
        mov       eax, 31;                // ebx = 31
        jmp       LABEL4;
        
        ALIGN     4

; mark_end;
@biari_decode_final_dep_asm@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_final_dep_asm@4
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_symbol_dep_asm@8
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_symbol_dep_asm@8
@biari_decode_symbol_dep_asm@8	PROC NEAR
; parameter 1(bi_ct): ecx
; parameter 2(dep): edx

        movd      mm3, ebp;
        movd      mm2, edi;
        movd      mm1, esi;
        movd      mm0, ebx;
        
        movzx     ebp, BYTE PTR [ecx];                      // ebp = state = bi_ct->state
        mov       edi, [edx];                               // edi = range = Drange
        mov       eax, ebp;                                 // eax = state
        mov       esi, edi;                                 // esi = range
        mov       ebx, [edx+4];                             // ebx = delta = Ddelta
        and       esi, 192;                                 // esi = range&0xc0
        and       eax, 63;                                  // eax = state&0x3F
        movzx     eax, BYTE PTR rLPS_table_4x64[eax+esi];   // eax = rLPS
        sub       edi, eax;                                 // range -= rLPS
        mov       esi, ebx;                                 // esi = delta
        add       esi, eax;                                 // esi = temp = delta+rLPS
        cmovge    edi, eax;                                 // range = (temp>=0) ? rLPS : range
        sar       esi, 31;                                  // esi = lz = temp>>31
        and       eax, esi;                                 // eax = rLPS&lz
        lea       ebx, [ebx+eax];                           // ebx = delta + rLPS&lz
        movzx     eax, BYTE PTR AC_next_state[2*ebp+esi+1]; // eax = AC_next_state[state*2+lz+1]
        mov       BYTE PTR [ecx], al;                       // state = bi_ct->state = AC_next_state[state*2+lz+1]
        shr       eax, 7;                                   // bit = state>>7
        movzx     ecx, BYTE PTR times[edi];                 // ecx = bitsneeded;
;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
        ; bsr       ecx, edi;
        ; lea       ecx, [ecx-8];
        ; neg       ecx;
        shl       edi, cl;                                  // range<<=bitsneeded
        mov       [edx], edi;                               // store Drange
        mov       esi, [edx+8];                             // esi = Dbits_to_go
        mov       ebp, [edx+12];                            // ebp = Dbuffer
        shld      ebx, ebp, cl;                             // delta = _shld(delta,Dbuffer,bitsneeded)
        shl       ebp, cl;                                  // Dbuffer <<= bitsneeded
        sub       esi, ecx;                                 // Dbits_to_go -= bitsneeded
        jl        LABEL_A;                                  // Prob 38%
        
LABEL_B:
        mov       [edx+4], ebx;                             // Ddelta
        mov       [edx+8], esi;                             // Dbits_to_go
        mov       [edx+12], ebp;                            // Dbuffer
        
        movd      ebx, mm0;
        movd      esi, mm1;
        movd      edi, mm2;
        movd      ebp, mm3;

        ; int 3;
        ret;
        
LABEL_A:
        mov       edi, [edx+16];                            // edi = Dcodestrm
        lea       edi, [edi+4];                             // 
        mov       [edx+16], edi;                            // Dcodestrm += 4
        mov       edi, DWORD PTR [edi-4];                   // edi = *Dcodestrm (trick...)
        bswap     edi;                                      // little-endian to big-endian conversion
        lea       ecx, [esi+32];                            // ecx = Dbits_to_go+32
        mov       ebp, edi;                                 // ebp = Lbuffer
        shr       edi, cl;                                  // edi = Lbuffer>>(32+Dbits_to_go)
        or        ebx, edi;                                 // delta |= Lbuffer>>(32+Dbits_to_go)
        lea       ecx, [ecx-32];                            // ecx = Dbits_to_go
        neg       ecx;                                      // ecx = -Dbits_to_go
        shl       ebp, cl;                                  // Lbuffer <<= (-Dbits_to_go)
        lea       esi, [esi+32];                            // esi += 32
        jmp       LABEL_B;
        
        ALIGN     4

; mark_end;
@biari_decode_symbol_dep_asm@8 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_symbol_dep_asm@8
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_symbol_eq_prob_dep_asm@4
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_symbol_eq_prob_dep_asm@4
@biari_decode_symbol_eq_prob_dep_asm@4	PROC NEAR
; parameter 1(dep): ecx

        mov       edx, [ecx+8];     // edx = Dbits_to_go
        add       edx, -1;          // edx = Dbits_to_go-1
        jl        LABEL_NEG;

        mov       eax, [ecx+12];    // eax = Dbuffer
LABEL_ELSE:
        mov       [ecx+8], edx;     // Dbits_to_go = edx
        mov       edx, [ecx+4];     // edx = Ddelta
        shld      edx, eax, 1;      // edx = _shld(delta, Dbuffer, 1)
        lea       eax, [eax+eax];   // Dbuffer<<=1;
        
        mov       [ecx+12], eax;    // Dbuffer
        mov       eax, [ecx];       // eax = Drange;
        add       eax, edx;         // range += delta
        cmovl     edx, eax;         // delta = (delta<0) ? range : delta
        sar       eax, 31;          // range>>=31 (lz)
        mov       [ecx+4], edx;     // Ddelta
        lea       eax, [eax+1];     // return (lz +1)
        
        ret;

LABEL_NEG:
        mov       edx, [ecx+16];    // edx = Dcodestrm
        lea       edx, [edx+4];     // edx = Dcodestrm+4
        mov       [ecx+16], edx;    // Dcodestrm = edx
        mov       eax, [edx-4];     // eax = *Dcodestrm
        bswap     eax;              // little-endian to big-endian conversion
        mov       edx, 31;          // edx = 31
        jmp       LABEL_ELSE;
        
        ALIGN     4

; mark_end;
@biari_decode_symbol_eq_prob_dep_asm@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_symbol_eq_prob_dep_asm@4

ELSE ; CONFIG_BIARI_ENABLE_GLOBAL

DecodingEnvironment STRUCT
        Drange      DWORD ?
        Ddelta      DWORD ?
        Dbits_to_go DWORD ?
        Dbuffer     DWORD ?
        Dcodestrm   DWORD ?
DecodingEnvironment ENDS
EXTERN C g_dep : DecodingEnvironment
Lrange      EQU g_dep.Drange
Ldelta      EQU g_dep.Ddelta
Lbits_to_go EQU g_dep.Dbits_to_go
Lbuffer     EQU g_dep.Dbuffer
Lcodestrm   EQU g_dep.Dcodestrm

_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @store_dep_asm@4
; mark_begin;
       ALIGN     4

	PUBLIC @store_dep_asm@4
@store_dep_asm@4	PROC NEAR
; parameter 1: ecx

        mov  eax, [ecx];
 		mov  DWORD PTR Lrange, eax;      // Drange
        mov  eax, [ecx+4];
 		mov  DWORD PTR Ldelta, eax;      // Ddelta
 		mov  eax, [ecx+8];
 		mov  DWORD PTR Lbits_to_go, eax; // Dbits_to_go
 		mov  eax, [ecx+12];
 		mov  DWORD PTR Lbuffer, eax;     // Dbuffer
 		mov  eax, [ecx+16];
 		mov  DWORD PTR Lcodestrm, eax;   // Dcodestrm

        ret;
        ALIGN     4

; mark_end;
@store_dep_asm@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @store_dep_asm@4
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @load_dep_asm@4
; mark_begin;
       ALIGN     4

	PUBLIC @load_dep_asm@4
@load_dep_asm@4	PROC NEAR
; parameter 1: ecx

 		mov  eax, DWORD PTR Lrange;
 		mov  [ecx], eax;                // Drange
 		mov  eax, DWORD PTR Ldelta;
 		mov  [ecx+4], eax;              // Ddelta
 		mov  eax, DWORD PTR Lbits_to_go;
 		mov  [ecx+8],  eax;             // Dbits_to_go
 		mov  eax, DWORD PTR Lbuffer;
 		mov  [ecx+12], eax;             // Dbuffer
 		mov  eax, DWORD PTR Lcodestrm;
 		mov  [ecx+16], eax;             // Dcodestrm

        ret;
        ALIGN     4

; mark_end;
@load_dep_asm@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @load_dep_asm@4
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_final_no_dep_asm@0
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_final_no_dep_asm@0
@biari_decode_final_no_dep_asm@0	PROC NEAR
        
        mov       edx, Ldelta;            // edx = Ldelta
        add       edx, 2;                 // delta += 2
        jl        LABEL1;

        mov       eax, 1;                 // return 1

        ret;

LABEL1:
        mov       eax, Lrange;            // eax = Lrange
        lea       eax, [eax-2];           // range -= 2
        cmp       eax, 256;               // compare range with QUARTER
        jl        LABEL2;

        mov       Lrange, eax;            // Lrange = range
        mov       Ldelta, edx;            // Ldelta = delta
        xor       eax, eax;               // return 0

        ret;

LABEL2:
        lea       eax, [eax+eax];         // range<<=1
        mov       Lrange, eax;            // Lrange = range
        mov       ecx, Lbits_to_go;       // ecx = Lbits_to_go
        add       ecx, -1;                // ecx = Lbits_to_go-1
        jl        LABEL3;

        mov       eax, Lbuffer;           // eax = Lbuffer
LABEL4:
        mov       Lbits_to_go, ecx;       // Lbits_to_go = ecx
        shld      edx, eax, 1;            // delta = _shld(delta,Lbuffer,1)
        lea       eax, [eax+eax];         // Lbuffer <<= 1
        mov       Lbuffer, eax;           // Lbuffer
        mov       Ldelta, edx;            // Ldelta = delta
        xor       eax, eax;               // return 0

        ret;
LABEL3:
        mov       ecx, Lcodestrm;         // ebx = Lcodestrm
        mov       eax, [ecx];             // eax = *Lcodestrm
        lea       ecx, [ecx+4];           // ebx = Lcodestrm+4
        mov       Lcodestrm, ecx;         // Lcodestrm = ebx
        bswap     eax;                    // little-endian to big-endian conversion
        mov       ecx, 31;                // ecx = 31
        jmp       LABEL4;
        
        ALIGN     4

; mark_end;
@biari_decode_final_no_dep_asm@0 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_final_no_dep_asm@0
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_symbol_no_dep_asm@4
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_symbol_no_dep_asm@4
@biari_decode_symbol_no_dep_asm@4	PROC NEAR
; parameter 1(bi_ct): ecx

        movd      mm2, edi;
        movd      mm1, esi;
        movd      mm0, ebx;
        
        movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
        mov       edi, Lrange;                              // edi = range = Lrange
        mov       eax, edx;                                 // eax = state
        mov       esi, edi;                                 // esi = range
        mov       ebx, Ldelta;                              // ebx = delta = Ldelta
        and       esi, 192;                                 // esi = range&0xc0
        and       eax, 63;                                  // eax = state&0x3F
        movzx     eax, BYTE PTR rLPS_table_4x64[eax+esi];   // eax = rLPS
        sub       edi, eax;                                 // range -= rLPS
        mov       esi, ebx;                                 // esi = delta
        add       esi, eax;                                 // esi = temp = delta+rLPS
        cmovge    edi, eax;                                 // range = (temp>=0) ? rLPS : range
        sar       esi, 31;                                  // esi = lz = temp>>31
        and       eax, esi;                                 // eax = rLPS&lz
        lea       ebx, [ebx+eax];                           // ebx = delta + rLPS&lz
        movzx     eax, BYTE PTR AC_next_state[2*edx+esi+1]; // eax = AC_next_state[state*2+lz+1]
        mov       BYTE PTR [ecx], al;                       // state = bi_ct->state = AC_next_state[state*2+lz+1]
        shr       eax, 7;                                   // bit = state>>7
        movzx     ecx, BYTE PTR times[edi];                 // ecx = bitsneeded;
;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
        ; bsr       ecx, edi;
        ; lea       ecx, [ecx-8];
        ; neg       ecx;
        shl       edi, cl;                                  // range<<=bitsneeded
        mov       Lrange, edi;                              // store Lrange
        mov       esi, Lbits_to_go;                         // esi = Lbits_to_go
        mov       edx, Lbuffer;                             // edx = Lbuffer
        shld      ebx, edx, cl;                             // delta = _shld(delta,Lbuffer,bitsneeded)
        shl       edx, cl;                                  // Lbuffer <<= bitsneeded
        sub       esi, ecx;                                 // Lbits_to_go -= bitsneeded
        jl        LABEL_A;                                  // Prob 38%
        
LABEL_B:
        mov       Ldelta, ebx;                              // Ldelta
        mov       Lbits_to_go, esi;                         // Lbits_to_go
        mov       Lbuffer, edx;                             // Lbuffer
        
        movd      ebx, mm0;
        movd      esi, mm1;
        movd      edi, mm2;

        ret;
        
LABEL_A:
        mov       edi, Lcodestrm;                           // edi = Lcodestrm
        lea       edi, [edi+4];                             // 
        mov       Lcodestrm, edi;                           // Lcodestrm += 4
        mov       edi, DWORD PTR [edi-4];                   // edi = *Lcodestrm (trick...)
        bswap     edi;                                      // little-endian to big-endian conversion
        lea       ecx, [esi+32];                            // ecx = Lbits_to_go+32
        mov       edx, edi;                                 // edx = Lbuffer
        shr       edi, cl;                                  // edi = Lbuffer>>(32+Lbits_to_go)
        or        ebx, edi;                                 // delta |= Lbuffer>>(32+Lbits_to_go)
        lea       ecx, [ecx-32];                            // ecx = Lbits_to_go
        neg       ecx;                                      // ecx = -Lbits_to_go
        shl       edx, cl;                                  // Lbuffer <<= (-Lbits_to_go)
        lea       esi, [esi+32];                            // esi += 32
        jmp       LABEL_B;
        
        ALIGN     4

; mark_end;
@biari_decode_symbol_no_dep_asm@4 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_final_no_dep_asm@0
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @biari_decode_symbol_no_dep_asm@4
; mark_begin;
       ALIGN     4

	PUBLIC @biari_decode_symbol_eq_prob_no_dep_asm@0
@biari_decode_symbol_eq_prob_no_dep_asm@0	PROC NEAR

        mov       edx, Lbits_to_go; // edx = Lbits_to_go
        add       edx, -1;          // edx = Lbits_to_go-1
        jl        LABEL_NEG;

        mov       eax, Lbuffer;     // eax = Lbuffer
LABEL_ELSE:
        mov       Lbits_to_go, edx; // Lbits_to_go = edx
        mov       edx, Ldelta;      // edx = Ldelta
        shld      edx, eax, 1;      // edx = _shld(delta, Lbuffer, 1)
; Following 3 lines are equivalent to the 1 above
        ; mov       ecx, eax;
        ; shr       ecx, 31;
        ; lea       edx, [2*edx+ecx];
        lea       eax, [eax+eax];   // Lbuffer<<=1;
        
        mov       Lbuffer, eax;     // Lbuffer
        mov       ecx, Lrange;      // ecx = Lrange;
        lea       eax, [ecx+edx];   // eax = (range+delta)
        sar       eax, 31;          // lz = (range+delta)>>=31
        and       ecx, eax;         // range&lz
        lea       edx, [ecx+edx];   // delta = delta + (range&lz)
        add       eax, 1;           // return (lz +1)
        mov       Ldelta, edx;      // Ldelta
; Following 6 lines are equivalent to the above 7
        ; mov       eax, Lrange;      // eax = Lrange
        ; add       eax, edx;         // range += delta
        ; cmovl     edx, eax;         // delta = (delta<0) ? range : delta
        ; sar       eax, 31;          // range>>=31 (lz)
        ; mov       Ldelta, edx;      // Ldelta
        ; lea       eax, [eax+1];     // return (lz+1)
        
        ret;

LABEL_NEG:
        mov       ecx, Lcodestrm;   // ecx = Lcodestrm
        lea       edx, [ecx+4];     // edx = Lcodestrm+4
        mov       eax, [ecx];       // eax = *Dcodestrm
        mov       Lcodestrm, edx;   // Lcodestrm = edx
        bswap     eax;              // little-endian to big-endian conversion
        mov       edx, 31;          // edx = 31
        jmp       LABEL_ELSE;
        
        ALIGN     4

; mark_end;
@biari_decode_symbol_eq_prob_no_dep_asm@0 ENDP
_TEXT	ENDS
_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'
_DATA	ENDS
; -- End  @biari_decode_symbol_eq_prob_no_dep_asm@0

ENDIF ; CONFIG_BIARI_ENABLE_GLOBAL or not

ELSEIF DEFINED(CONFIG_BIARI_ENABLE_MMX)

IF CONFIG_BIARI_ENABLE_MMX EQ 1

_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @store_dep_mmx@4
; mark_begin;
       ALIGN     4

	PUBLIC @store_dep_mmx@4
@store_dep_mmx@4	PROC NEAR
; parameter 1: ecx

        movd  mm7, [ecx];      // Drange
        movd  mm6, [ecx+4];    // Ddelta
 		movd  mm5, [ecx+8];    // Dbits_to_go
 		movd  mm4, [ecx+12];   // Dbuffer
 		movd  mm3, [ecx+16];   // Dcodestrm

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

 		movd  [ecx], mm7;      // Drange
 		movd  [ecx+4], mm6;    // Ddelta
 		movd  [ecx+8],  mm5;   // Dbits_to_go
 		movd  [ecx+12], mm4;   // Dbuffer
 		movd  [ecx+16], mm3;   // Dcodestrm

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
        
        movd      edx, mm6;               // edx = Ldelta
        add       edx, 2;                 // delta += 2
        jl        LABEL1;

        mov       eax, 1;                 // return 1

        ret;

LABEL1:
        movd      eax, mm7;               // eax = Lrange
        lea       eax, [eax-2];           // range -= 2
        cmp       eax, 256;               // compare range with QUARTER
        jl        LABEL2;

        movd      mm7, eax;               // Lrange = range
        movd      mm6, edx;               // Ldelta = delta
        xor       eax, eax;               // return 0

        ret;

LABEL2:
        lea       eax, [eax+eax];         // range<<=1
        movd      mm7, eax;               // Lrange = range
        movd      ecx, mm5;               // ecx = Lbits_to_go
        add       ecx, -1;                // ecx = Lbits_to_go-1
        jl        LABEL3;

        movd      eax, mm4;               // eax = Lbuffer
LABEL4:
        movd      mm5, ecx;               // Lbits_to_go = ecx
        shld      edx, eax, 1;            // delta = _shld(delta,Lbuffer,1)
        lea       eax, [eax+eax];         // Lbuffer <<= 1
        movd      mm4, eax;               // Lbuffer
        movd      mm6, edx;               // Ldelta = delta
        xor       eax, eax;               // return 0

        ret;
LABEL3:
        movd      ecx, mm3;               // ebx = Lcodestrm
        mov       eax, [ecx];             // eax = *Lcodestrm
        lea       ecx, [ecx+4];           // ebx = Lcodestrm+4
        movd      mm3, ecx;               // Lcodestrm = ebx
        bswap     eax;                    // little-endian to big-endian conversion
        mov       ecx, 31;                // ecx = 31
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

        movd      mm2, edi;
        movd      mm1, esi;
        movd      mm0, ebx;
        
        movzx     edx, BYTE PTR [ecx];                      // edx = state = bi_ct->state
        movd      edi, mm7;                                 // edi = range = Lrange
        mov       eax, edx;                                 // eax = state
        mov       esi, edi;                                 // esi = range
        movd      ebx, mm6;                                 // ebx = delta = Ldelta
        and       esi, 192;                                 // esi = range&0xc0
        and       eax, 63;                                  // eax = state&0x3F
        movzx     eax, BYTE PTR rLPS_table_4x64[eax+esi];   // eax = rLPS
        sub       edi, eax;                                 // range -= rLPS
        mov       esi, ebx;                                 // esi = delta
        add       esi, eax;                                 // esi = temp = delta+rLPS
        cmovge    edi, eax;                                 // range = (temp>=0) ? rLPS : range
        sar       esi, 31;                                  // esi = lz = temp>>31
        and       eax, esi;                                 // eax = rLPS&lz
        lea       ebx, [ebx+eax];                           // ebx = delta + rLPS&lz
        movzx     eax, BYTE PTR AC_next_state[2*edx+esi+1]; // eax = AC_next_state[state*2+lz+1]
        mov       BYTE PTR [ecx], al;                       // state = bi_ct->state = AC_next_state[state*2+lz+1]
        shr       eax, 7;                                   // bit = state>>7
        movzx     ecx, BYTE PTR times[edi];                 // ecx = bitsneeded;
;;; Following 3 lines calculate the same as above, without the lookup - SLOWER !!!
        ; bsr       ecx, edi;
        ; lea       ecx, [ecx-8];
        ; neg       ecx;
        shl       edi, cl;                                  // range<<=bitsneeded
        movd      mm7, edi;                                 // store Lrange
        movd      esi, mm5;                                 // esi = Lbits_to_go
        movd      edx, mm4;                                 // edx = Lbuffer
        shld      ebx, edx, cl;                             // delta = _shld(delta,Lbuffer,bitsneeded)
        shl       edx, cl;                                  // Lbuffer <<= bitsneeded
        sub       esi, ecx;                                 // Lbits_to_go -= bitsneeded
        jl        LABEL_A;                                  // Prob 38%
        
LABEL_B:
        movd      mm6, ebx;                                 // Ldelta
        movd      mm5, esi;                                 // Lbits_to_go
        movd      mm4, edx;                                 // Lbuffer
        
        movd      ebx, mm0;
        movd      esi, mm1;
        movd      edi, mm2;
        
        ret;
        
LABEL_A:
        movd      edi, mm3;                                 // edi = Lcodestrm
        lea       edi, [edi+4];                             // 
        movd      mm3, edi;                                 // Lcodestrm += 4
        mov       edi, DWORD PTR [edi-4];                   // edi = *Lcodestrm (trick...)
        bswap     edi;                                      // little-endian to big-endian conversion
        lea       ecx, [esi+32];                            // ecx = Lbits_to_go+32
        mov       edx, edi;                                 // edx = Lbuffer
        shr       edi, cl;                                  // edi = Lbuffer>>(32+Lbits_to_go)
        or        ebx, edi;                                 // delta |= Lbuffer>>(32+Lbits_to_go)
        lea       ecx, [ecx-32];                            // ecx = Lbits_to_go
        neg       ecx;                                      // ecx = -Lbits_to_go
        shl       edx, cl;                                  // Lbuffer <<= (-Lbits_to_go)
        lea       esi, [esi+32];                            // esi += 32
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

        movd      edx, mm5;         // edx = Lbits_to_go
        add       edx, -1;          // edx = Lbits_to_go-1
        jl        LABEL_NEG;

        movd      eax, mm4;         // eax = Lbuffer
LABEL_ELSE:
        movd      mm5, edx;         // Lbits_to_go = edx
        movd      edx, mm6;         // edx = Ldelta
        shld      edx, eax, 1;      // edx = _shld(delta, Lbuffer, 1)
; Following 3 lines are equivalent to the 1 above
        ; mov       ecx, eax;
        ; shr       ecx, 31;
        ; lea       edx, [2*edx+ecx];
        lea       eax, [eax+eax];   // Lbuffer<<=1;
        
        movd      mm4, eax;         // Lbuffer
        movd      ecx, mm7;         // ecx = Lrange;
        lea       eax, [ecx+edx];   // eax = (range+delta)
        sar       eax, 31;          // lz = (range+delta)>>=31
        and       ecx, eax;         // range&lz
        lea       edx, [ecx+edx];   // delta = delta + (range&lz)
        add       eax, 1;           // return (lz +1)
        movd      mm6, edx;         // Ldelta
; Following 6 lines are equivalent to the above 7
        ; movd      eax, mm7;         // eax = Lrange
        ; add       eax, edx;         // range += delta
        ; cmovl     edx, eax;         // delta = (delta<0) ? range : delta
        ; sar       eax, 31;          // range>>=31 (lz)
        ; movd      mm6, edx;         // Ldelta
        ; lea       eax, [eax+1];     // return (lz+1)
        
        ret;

LABEL_NEG:
        movd      ecx, mm3;         // ecx = Lcodestrm
        lea       edx, [ecx+4];     // edx = Lcodestrm+4
        mov       eax, [ecx];       // eax = *Dcodestrm
        movd      mm3, edx;         // Lcodestrm = edx
        bswap     eax;              // little-endian to big-endian conversion
        mov       edx, 31;          // edx = 31
        jmp       LABEL_ELSE;
        
        ALIGN     4

; mark_end;
@biari_decode_symbol_eq_prob_no_dep_mmx@0 ENDP
_TEXT	ENDS

ELSE ; CONFIG_BIARI_ENABLE_MMX!=1

; This is an alternative implementation of the 3 biari_decode_symbol functions
; that uses extensively MMX instructions.
_TEXT	SEGMENT DWORD PUBLIC USE32 'CODE'
; -- Begin  @store_dep_mmx@4
; mark_begin;
       ALIGN     4

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

ENDIF ; CONFIG_BIARI_ENABLE_MMX == 1 or == 2

ENDIF ; CONFIG_BIARI_ENABLE_ASM or _MMX or none

_BSS	SEGMENT DWORD PUBLIC USE32 'BSS'
_BSS	ENDS

IF DEFINED(CONFIG_BIARI_ENABLE_ASM) OR DEFINED(CONFIG_BIARI_ENABLE_MMX)
_RDATA	SEGMENT DWORD PUBLIC USE32 'DATA'
times	DB 0	; u8
	DB 8	; u8
	DB 7	; u8
	DB 7	; u8
	DB 6	; u8
	DB 6	; u8
	DB 6	; u8
	DB 6	; u8
	DB 5	; u8
	DB 5	; u8
	DB 5	; u8
	DB 5	; u8
	DB 5	; u8
	DB 5	; u8
	DB 5	; u8
	DB 5	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 4	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 3	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 2	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 1	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
	DB 0	; u8
AC_next_state	DB 1	; u8
	DB 192	; u8
	DB 2	; u8
	DB 128	; u8
	DB 3	; u8
	DB 129	; u8
	DB 4	; u8
	DB 130	; u8
	DB 5	; u8
	DB 130	; u8
	DB 6	; u8
	DB 132	; u8
	DB 7	; u8
	DB 132	; u8
	DB 8	; u8
	DB 133	; u8
	DB 9	; u8
	DB 134	; u8
	DB 10	; u8
	DB 135	; u8
	DB 11	; u8
	DB 136	; u8
	DB 12	; u8
	DB 137	; u8
	DB 13	; u8
	DB 137	; u8
	DB 14	; u8
	DB 139	; u8
	DB 15	; u8
	DB 139	; u8
	DB 16	; u8
	DB 140	; u8
	DB 17	; u8
	DB 141	; u8
	DB 18	; u8
	DB 141	; u8
	DB 19	; u8
	DB 143	; u8
	DB 20	; u8
	DB 143	; u8
	DB 21	; u8
	DB 144	; u8
	DB 22	; u8
	DB 144	; u8
	DB 23	; u8
	DB 146	; u8
	DB 24	; u8
	DB 146	; u8
	DB 25	; u8
	DB 147	; u8
	DB 26	; u8
	DB 147	; u8
	DB 27	; u8
	DB 149	; u8
	DB 28	; u8
	DB 149	; u8
	DB 29	; u8
	DB 150	; u8
	DB 30	; u8
	DB 150	; u8
	DB 31	; u8
	DB 151	; u8
	DB 32	; u8
	DB 152	; u8
	DB 33	; u8
	DB 152	; u8
	DB 34	; u8
	DB 153	; u8
	DB 35	; u8
	DB 154	; u8
	DB 36	; u8
	DB 154	; u8
	DB 37	; u8
	DB 155	; u8
	DB 38	; u8
	DB 155	; u8
	DB 39	; u8
	DB 156	; u8
	DB 40	; u8
	DB 157	; u8
	DB 41	; u8
	DB 157	; u8
	DB 42	; u8
	DB 158	; u8
	DB 43	; u8
	DB 158	; u8
	DB 44	; u8
	DB 158	; u8
	DB 45	; u8
	DB 159	; u8
	DB 46	; u8
	DB 160	; u8
	DB 47	; u8
	DB 160	; u8
	DB 48	; u8
	DB 161	; u8
	DB 49	; u8
	DB 161	; u8
	DB 50	; u8
	DB 161	; u8
	DB 51	; u8
	DB 162	; u8
	DB 52	; u8
	DB 162	; u8
	DB 53	; u8
	DB 163	; u8
	DB 54	; u8
	DB 163	; u8
	DB 55	; u8
	DB 163	; u8
	DB 56	; u8
	DB 164	; u8
	DB 57	; u8
	DB 164	; u8
	DB 58	; u8
	DB 164	; u8
	DB 59	; u8
	DB 165	; u8
	DB 60	; u8
	DB 165	; u8
	DB 61	; u8
	DB 165	; u8
	DB 62	; u8
	DB 166	; u8
	DB 62	; u8
	DB 166	; u8
	DB 63	; u8
	DB 191	; u8
	DB 193	; u8
	DB 0	; u8
	DB 194	; u8
	DB 64	; u8
	DB 195	; u8
	DB 65	; u8
	DB 196	; u8
	DB 66	; u8
	DB 197	; u8
	DB 66	; u8
	DB 198	; u8
	DB 68	; u8
	DB 199	; u8
	DB 68	; u8
	DB 200	; u8
	DB 69	; u8
	DB 201	; u8
	DB 70	; u8
	DB 202	; u8
	DB 71	; u8
	DB 203	; u8
	DB 72	; u8
	DB 204	; u8
	DB 73	; u8
	DB 205	; u8
	DB 73	; u8
	DB 206	; u8
	DB 75	; u8
	DB 207	; u8
	DB 75	; u8
	DB 208	; u8
	DB 76	; u8
	DB 209	; u8
	DB 77	; u8
	DB 210	; u8
	DB 77	; u8
	DB 211	; u8
	DB 79	; u8
	DB 212	; u8
	DB 79	; u8
	DB 213	; u8
	DB 80	; u8
	DB 214	; u8
	DB 80	; u8
	DB 215	; u8
	DB 82	; u8
	DB 216	; u8
	DB 82	; u8
	DB 217	; u8
	DB 83	; u8
	DB 218	; u8
	DB 83	; u8
	DB 219	; u8
	DB 85	; u8
	DB 220	; u8
	DB 85	; u8
	DB 221	; u8
	DB 86	; u8
	DB 222	; u8
	DB 86	; u8
	DB 223	; u8
	DB 87	; u8
	DB 224	; u8
	DB 88	; u8
	DB 225	; u8
	DB 88	; u8
	DB 226	; u8
	DB 89	; u8
	DB 227	; u8
	DB 90	; u8
	DB 228	; u8
	DB 90	; u8
	DB 229	; u8
	DB 91	; u8
	DB 230	; u8
	DB 91	; u8
	DB 231	; u8
	DB 92	; u8
	DB 232	; u8
	DB 93	; u8
	DB 233	; u8
	DB 93	; u8
	DB 234	; u8
	DB 94	; u8
	DB 235	; u8
	DB 94	; u8
	DB 236	; u8
	DB 94	; u8
	DB 237	; u8
	DB 95	; u8
	DB 238	; u8
	DB 96	; u8
	DB 239	; u8
	DB 96	; u8
	DB 240	; u8
	DB 97	; u8
	DB 241	; u8
	DB 97	; u8
	DB 242	; u8
	DB 97	; u8
	DB 243	; u8
	DB 98	; u8
	DB 244	; u8
	DB 98	; u8
	DB 245	; u8
	DB 99	; u8
	DB 246	; u8
	DB 99	; u8
	DB 247	; u8
	DB 99	; u8
	DB 248	; u8
	DB 100	; u8
	DB 249	; u8
	DB 100	; u8
	DB 250	; u8
	DB 100	; u8
	DB 251	; u8
	DB 101	; u8
	DB 252	; u8
	DB 101	; u8
	DB 253	; u8
	DB 101	; u8
	DB 254	; u8
	DB 102	; u8
	DB 254	; u8
	DB 102	; u8
	DB 255	; u8
	DB 127	; u8
	DB 1	; u8
	DB 192	; u8
	DB 2	; u8
	DB 128	; u8
	DB 3	; u8
	DB 129	; u8
	DB 4	; u8
	DB 130	; u8
	DB 5	; u8
	DB 130	; u8
	DB 6	; u8
	DB 132	; u8
	DB 7	; u8
	DB 132	; u8
	DB 8	; u8
	DB 133	; u8
	DB 9	; u8
	DB 134	; u8
	DB 10	; u8
	DB 135	; u8
	DB 11	; u8
	DB 136	; u8
	DB 12	; u8
	DB 137	; u8
	DB 13	; u8
	DB 137	; u8
	DB 14	; u8
	DB 139	; u8
	DB 15	; u8
	DB 139	; u8
	DB 16	; u8
	DB 140	; u8
	DB 17	; u8
	DB 141	; u8
	DB 18	; u8
	DB 141	; u8
	DB 19	; u8
	DB 143	; u8
	DB 20	; u8
	DB 143	; u8
	DB 21	; u8
	DB 144	; u8
	DB 22	; u8
	DB 144	; u8
	DB 23	; u8
	DB 146	; u8
	DB 24	; u8
	DB 146	; u8
	DB 25	; u8
	DB 147	; u8
	DB 26	; u8
	DB 147	; u8
	DB 27	; u8
	DB 149	; u8
	DB 28	; u8
	DB 149	; u8
	DB 29	; u8
	DB 150	; u8
	DB 30	; u8
	DB 150	; u8
	DB 31	; u8
	DB 151	; u8
	DB 32	; u8
	DB 152	; u8
	DB 33	; u8
	DB 152	; u8
	DB 34	; u8
	DB 153	; u8
	DB 35	; u8
	DB 154	; u8
	DB 36	; u8
	DB 154	; u8
	DB 37	; u8
	DB 155	; u8
	DB 38	; u8
	DB 155	; u8
	DB 39	; u8
	DB 156	; u8
	DB 40	; u8
	DB 157	; u8
	DB 41	; u8
	DB 157	; u8
	DB 42	; u8
	DB 158	; u8
	DB 43	; u8
	DB 158	; u8
	DB 44	; u8
	DB 158	; u8
	DB 45	; u8
	DB 159	; u8
	DB 46	; u8
	DB 160	; u8
	DB 47	; u8
	DB 160	; u8
	DB 48	; u8
	DB 161	; u8
	DB 49	; u8
	DB 161	; u8
	DB 50	; u8
	DB 161	; u8
	DB 51	; u8
	DB 162	; u8
	DB 52	; u8
	DB 162	; u8
	DB 53	; u8
	DB 163	; u8
	DB 54	; u8
	DB 163	; u8
	DB 55	; u8
	DB 163	; u8
	DB 56	; u8
	DB 164	; u8
	DB 57	; u8
	DB 164	; u8
	DB 58	; u8
	DB 164	; u8
	DB 59	; u8
	DB 165	; u8
	DB 60	; u8
	DB 165	; u8
	DB 61	; u8
	DB 165	; u8
	DB 62	; u8
	DB 166	; u8
	DB 62	; u8
	DB 166	; u8
	DB 63	; u8
	DB 191	; u8
	DB 193	; u8
	DB 0	; u8
	DB 194	; u8
	DB 64	; u8
	DB 195	; u8
	DB 65	; u8
	DB 196	; u8
	DB 66	; u8
	DB 197	; u8
	DB 66	; u8
	DB 198	; u8
	DB 68	; u8
	DB 199	; u8
	DB 68	; u8
	DB 200	; u8
	DB 69	; u8
	DB 201	; u8
	DB 70	; u8
	DB 202	; u8
	DB 71	; u8
	DB 203	; u8
	DB 72	; u8
	DB 204	; u8
	DB 73	; u8
	DB 205	; u8
	DB 73	; u8
	DB 206	; u8
	DB 75	; u8
	DB 207	; u8
	DB 75	; u8
	DB 208	; u8
	DB 76	; u8
	DB 209	; u8
	DB 77	; u8
	DB 210	; u8
	DB 77	; u8
	DB 211	; u8
	DB 79	; u8
	DB 212	; u8
	DB 79	; u8
	DB 213	; u8
	DB 80	; u8
	DB 214	; u8
	DB 80	; u8
	DB 215	; u8
	DB 82	; u8
	DB 216	; u8
	DB 82	; u8
	DB 217	; u8
	DB 83	; u8
	DB 218	; u8
	DB 83	; u8
	DB 219	; u8
	DB 85	; u8
	DB 220	; u8
	DB 85	; u8
	DB 221	; u8
	DB 86	; u8
	DB 222	; u8
	DB 86	; u8
	DB 223	; u8
	DB 87	; u8
	DB 224	; u8
	DB 88	; u8
	DB 225	; u8
	DB 88	; u8
	DB 226	; u8
	DB 89	; u8
	DB 227	; u8
	DB 90	; u8
	DB 228	; u8
	DB 90	; u8
	DB 229	; u8
	DB 91	; u8
	DB 230	; u8
	DB 91	; u8
	DB 231	; u8
	DB 92	; u8
	DB 232	; u8
	DB 93	; u8
	DB 233	; u8
	DB 93	; u8
	DB 234	; u8
	DB 94	; u8
	DB 235	; u8
	DB 94	; u8
	DB 236	; u8
	DB 94	; u8
	DB 237	; u8
	DB 95	; u8
	DB 238	; u8
	DB 96	; u8
	DB 239	; u8
	DB 96	; u8
	DB 240	; u8
	DB 97	; u8
	DB 241	; u8
	DB 97	; u8
	DB 242	; u8
	DB 97	; u8
	DB 243	; u8
	DB 98	; u8
	DB 244	; u8
	DB 98	; u8
	DB 245	; u8
	DB 99	; u8
	DB 246	; u8
	DB 99	; u8
	DB 247	; u8
	DB 99	; u8
	DB 248	; u8
	DB 100	; u8
	DB 249	; u8
	DB 100	; u8
	DB 250	; u8
	DB 100	; u8
	DB 251	; u8
	DB 101	; u8
	DB 252	; u8
	DB 101	; u8
	DB 253	; u8
	DB 101	; u8
	DB 254	; u8
	DB 102	; u8
	DB 254	; u8
	DB 102	; u8
	DB 255	; u8
	DB 127	; u8
rLPS_table_4x64	DB 128	; u8
	DB 128	; u8
	DB 128	; u8
	DB 123	; u8
	DB 116	; u8
	DB 111	; u8
	DB 105	; u8
	DB 100	; u8
	DB 95	; u8
	DB 90	; u8
	DB 85	; u8
	DB 81	; u8
	DB 77	; u8
	DB 73	; u8
	DB 69	; u8
	DB 66	; u8
	DB 62	; u8
	DB 59	; u8
	DB 56	; u8
	DB 53	; u8
	DB 51	; u8
	DB 48	; u8
	DB 46	; u8
	DB 43	; u8
	DB 41	; u8
	DB 39	; u8
	DB 37	; u8
	DB 35	; u8
	DB 33	; u8
	DB 32	; u8
	DB 30	; u8
	DB 29	; u8
	DB 27	; u8
	DB 26	; u8
	DB 24	; u8
	DB 23	; u8
	DB 22	; u8
	DB 21	; u8
	DB 20	; u8
	DB 19	; u8
	DB 18	; u8
	DB 17	; u8
	DB 16	; u8
	DB 15	; u8
	DB 14	; u8
	DB 14	; u8
	DB 13	; u8
	DB 12	; u8
	DB 12	; u8
	DB 11	; u8
	DB 11	; u8
	DB 10	; u8
	DB 10	; u8
	DB 9	; u8
	DB 9	; u8
	DB 8	; u8
	DB 8	; u8
	DB 7	; u8
	DB 7	; u8
	DB 7	; u8
	DB 6	; u8
	DB 6	; u8
	DB 6	; u8
	DB 2	; u8
	DB 176	; u8
	DB 167	; u8
	DB 158	; u8
	DB 150	; u8
	DB 142	; u8
	DB 135	; u8
	DB 128	; u8
	DB 122	; u8
	DB 116	; u8
	DB 110	; u8
	DB 104	; u8
	DB 99	; u8
	DB 94	; u8
	DB 89	; u8
	DB 85	; u8
	DB 80	; u8
	DB 76	; u8
	DB 72	; u8
	DB 69	; u8
	DB 65	; u8
	DB 62	; u8
	DB 59	; u8
	DB 56	; u8
	DB 53	; u8
	DB 50	; u8
	DB 48	; u8
	DB 45	; u8
	DB 43	; u8
	DB 41	; u8
	DB 39	; u8
	DB 37	; u8
	DB 35	; u8
	DB 33	; u8
	DB 31	; u8
	DB 30	; u8
	DB 28	; u8
	DB 27	; u8
	DB 26	; u8
	DB 24	; u8
	DB 23	; u8
	DB 22	; u8
	DB 21	; u8
	DB 20	; u8
	DB 19	; u8
	DB 18	; u8
	DB 17	; u8
	DB 16	; u8
	DB 15	; u8
	DB 14	; u8
	DB 14	; u8
	DB 13	; u8
	DB 12	; u8
	DB 12	; u8
	DB 11	; u8
	DB 11	; u8
	DB 10	; u8
	DB 9	; u8
	DB 9	; u8
	DB 9	; u8
	DB 8	; u8
	DB 8	; u8
	DB 7	; u8
	DB 7	; u8
	DB 2	; u8
	DB 208	; u8
	DB 197	; u8
	DB 187	; u8
	DB 178	; u8
	DB 169	; u8
	DB 160	; u8
	DB 152	; u8
	DB 144	; u8
	DB 137	; u8
	DB 130	; u8
	DB 123	; u8
	DB 117	; u8
	DB 111	; u8
	DB 105	; u8
	DB 100	; u8
	DB 95	; u8
	DB 90	; u8
	DB 86	; u8
	DB 81	; u8
	DB 77	; u8
	DB 73	; u8
	DB 69	; u8
	DB 66	; u8
	DB 63	; u8
	DB 59	; u8
	DB 56	; u8
	DB 54	; u8
	DB 51	; u8
	DB 48	; u8
	DB 46	; u8
	DB 43	; u8
	DB 41	; u8
	DB 39	; u8
	DB 37	; u8
	DB 35	; u8
	DB 33	; u8
	DB 32	; u8
	DB 30	; u8
	DB 29	; u8
	DB 27	; u8
	DB 26	; u8
	DB 25	; u8
	DB 23	; u8
	DB 22	; u8
	DB 21	; u8
	DB 20	; u8
	DB 19	; u8
	DB 18	; u8
	DB 17	; u8
	DB 16	; u8
	DB 15	; u8
	DB 15	; u8
	DB 14	; u8
	DB 13	; u8
	DB 12	; u8
	DB 12	; u8
	DB 11	; u8
	DB 11	; u8
	DB 10	; u8
	DB 10	; u8
	DB 9	; u8
	DB 9	; u8
	DB 8	; u8
	DB 2	; u8
	DB 240	; u8
	DB 227	; u8
	DB 216	; u8
	DB 205	; u8
	DB 195	; u8
	DB 185	; u8
	DB 175	; u8
	DB 166	; u8
	DB 158	; u8
	DB 150	; u8
	DB 142	; u8
	DB 135	; u8
	DB 128	; u8
	DB 122	; u8
	DB 116	; u8
	DB 110	; u8
	DB 104	; u8
	DB 99	; u8
	DB 94	; u8
	DB 89	; u8
	DB 85	; u8
	DB 80	; u8
	DB 76	; u8
	DB 72	; u8
	DB 69	; u8
	DB 65	; u8
	DB 62	; u8
	DB 59	; u8
	DB 56	; u8
	DB 53	; u8
	DB 50	; u8
	DB 48	; u8
	DB 45	; u8
	DB 43	; u8
	DB 41	; u8
	DB 39	; u8
	DB 37	; u8
	DB 35	; u8
	DB 33	; u8
	DB 31	; u8
	DB 30	; u8
	DB 28	; u8
	DB 27	; u8
	DB 25	; u8
	DB 24	; u8
	DB 23	; u8
	DB 22	; u8
	DB 21	; u8
	DB 20	; u8
	DB 19	; u8
	DB 18	; u8
	DB 17	; u8
	DB 16	; u8
	DB 15	; u8
	DB 14	; u8
	DB 14	; u8
	DB 13	; u8
	DB 12	; u8
	DB 12	; u8
	DB 11	; u8
	DB 11	; u8
	DB 10	; u8
	DB 9	; u8
	DB 2	; u8
_RDATA	ENDS
ENDIF ; CONFIG_BIARI_ENABLE_ASM or CONFIG_BIARI_ENABLE_MMX
	END
