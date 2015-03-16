; itrans.asm
; inverse transform of block 4x4

.686
.mmx
.xmm
.MODEL flat, C

; *************************************************************************

.DATA
ALIGN 16
h_add_1_03	dw	1, 1, 2, 1
h_add_1_12	dw	1, -1, 1, -2
const32		dd	32, 32
const0		dw	0, 0, 0, 0

; structures
;member		equ 0h


.CODE
; parameters
dest		equ [esp+4]
pred		equ	[esp+8]
src			equ	[esp+12]
stride		equ	[esp+16]

var0		equ qword ptr[edx]
var1		equ qword ptr[edx+08h]
var2		equ qword ptr[edx+10h]
var3		equ qword ptr[edx+18h]


; *************************************************************************
; new proc
; *************************************************************************
;local variables
ALIGN 16
inverse_transform4x4_asm	PROC
			lea		edx, [esp-32]		; temp buffer
			
			mov		eax, src
			and		edx, -32			; aligned var_array fit in one cache line

			movq	mm0, [eax]			; src[0 1 2 3]
			movq	mm1, [eax+8]		; src[4 5 6 7]
			movq	mm2, [eax+16]		; src[8 9 a b]
			movq	mm3, [eax+24]		; src[c d e f]
			
			pshufw	mm4, mm0, 0d8h		; src[0 2 1 3]
			pshufw	mm5, mm1, 0d8h		; src[4 6 5 7]

			pshufw	mm0, mm0, 0d8h		; src[0 2 1 3]
			pshufw	mm1, mm1, 0d8h		; src[4 6 5 7]

			pshufw	mm2, mm2, 0d8h		; src[8 a 9 b]
			pshufw	mm3, mm3, 0d8h		; src[c e d f]

			pmaddwd	mm4, qword ptr[h_add_1_03]	; m[0 2*c] 
			pmaddwd	mm5, qword ptr[h_add_1_03]	; m[1 2*d]

			pmaddwd	mm0, qword ptr[h_add_1_12]	; m[4 2*8]
			pmaddwd	mm1, qword ptr[h_add_1_12]	; m[5 2*9]

			movq	mm6, mm4
			movq	mm7, mm0

			punpckldq	mm4, mm5		; m[0 1]
			punpckldq	mm0, mm1		; m[4 5]

			punpckhdq	mm6, mm5		; m[2*c 2*d]
			punpckhdq	mm7, mm1		; m[2*8 2*9]

			movq	mm5, mm2
			movq	mm1, mm3

			pmaddwd	mm2, qword ptr[h_add_1_03]	; m[2 2*e] 
			pmaddwd	mm3, qword ptr[h_add_1_03]	; m[3 2*f]

			psrad	mm6, 1				; m[c d]
			psrad	mm7, 1				; m[8 9]

			pmaddwd	mm5, qword ptr[h_add_1_12]	; m[6 2*a]
			pmaddwd	mm1, qword ptr[h_add_1_12]	; m[7 2*b]

			movq	var0, mm6			; save m[c d]
			movq	var1, mm7			; save m[8 9]

			movq	mm6, mm2
			movq	mm7, mm5

			punpckldq	mm2, mm3		; m[2 3]
			punpckldq	mm5, mm1		; m[6 7]

			punpckhdq	mm6, mm3		; m[2*e 2*f]
			punpckhdq	mm7, mm1		; m[2*a 2*b]

			; here we got m6
			; minimize register pressure by using store-forwarding
			movq	mm3, mm4			; m[0 1]
			movq	mm1, mm2			; m[2 3]

			psrad	mm6, 1				; m[e f]
			psrad	mm7, 1				; m[a b]

			paddd	mm4, var0			; n[0 1]
			paddd	mm2, mm6			; n[2 3]

			psubd	mm3, var0			; n[c d]
			psubd	mm1, mm6			; n[e f]
			movq	mm6, mm0			; m[4 5]

			movq	var2,mm5			; m[6 7]
			psubd	mm5, mm7			; n[a b]

			paddd	mm7, var2			; n[6 7]

			paddd	mm0, var1			; n[4 5]

			psubd	mm6, var1			; n[8 9]

			; here we got n and should process it again vertically and horizontally
			movq	var0, mm1			; n[e f]
			movq	mm1, mm5			; n[a b]
			
			movq	var1, mm3			; n[c d]
			movq	mm3, mm6			; n[8 9]

			movq	var2, mm7			; n[6 7]
			movq	mm7, mm2			; n[2 3]
			
			movq	var3, mm0			; n[4 5]
			movq	mm0, mm4			; n[0 1]
			
			punpckldq	mm5, var0		; n[a e]
			punpckhdq	mm1, var0		; n[b f]
			
			punpckldq	mm6, var1		; n[8 c]
			punpckhdq	mm3, var1		; n[9 d]
			
			punpckldq	mm7, var2		; n[2 6]
			punpckhdq	mm2, var2		; n[3 7]
			
			punpckldq	mm4, var3		; n[0 4]
			punpckhdq	mm0, var3		; n[1 5]
			
			movq	var0, mm6			; n[8 c]
			psubd	mm6, mm5			; o[6 7]
			paddd	mm5, var0			; o[2 3]

			movq	var1, mm3			; n[9 d]
			psrad	mm3, 1				; n[9 d] >> 1

			movq	var2, mm4			; n[0 4]
			psubd	mm4, mm7			; o[4 5]
			psubd	mm3, mm1			; o[a b]
			
			movq	var3, mm0			; n[1 5]
			psrad	mm1, 1				; n[b f] >> 1
			paddd	mm7, var2			; o[0 1]

			psrad	mm0, 1				; n[1 5] >> 1
			paddd	mm1, var1			; o[e f]
			
			psubd	mm0, mm2			; o[8 9]
			psrad	mm2, 1				; n[3 7] >> 1
			
			paddd	mm2, var3			; o[c d]
			
			; got o ready
			paddd	mm6, qword ptr[const32]	; o[6 7] + 0.5
			paddd	mm5, qword ptr[const32]	; o[2 3] + 0.5
			paddd	mm4, qword ptr[const32]	; o[4 5] + 0.5
			paddd	mm7, qword ptr[const32]	; o[0 1] + 0.5

			movq	var0, mm6			; o[6 7]
			psubd	mm6, mm3			; p[a b]
			paddd	mm3, var0			; p[6 7]
			
			movq	var1, mm5			; o[2 3]
			psubd	mm5, mm1			; p[e f]
			paddd	mm1, var1			; p[2 3]
			psrad	mm6, 6
			
			movq	var2, mm4			; o[4 5]
			psubd	mm4, mm0			; p[8 9]
			paddd	mm0, var2			; p[4 5]
			psrad	mm3, 6				; 
			
			movq	var3, mm7			; o[0 1]
			psubd	mm7, mm2			; p[c d]
			paddd	mm2, var3			; p[0 1]
			psrad	mm5, 6
			
			mov		eax, pred
			mov		edx, dest

			psrad	mm1, 6
			psrad	mm4, 6
			psrad	mm0, 6
			psrad	mm7, 6
			psrad	mm2, 6
			
			; got p result ready here. add pred now.
			
			packssdw	mm4, mm6		; p[8 9 a b]
			movd	mm6, dword ptr[eax]
			packssdw	mm0, mm3		; p[4 5 6 7]
			movd	mm3, dword ptr[eax+16]
			packssdw	mm2, mm1		; p[0 1 2 3]
			movd	mm1, dword ptr[eax+32]
			packssdw	mm7, mm5		; p[c d e f]
			movd	mm5, dword ptr[eax+48]

			punpcklbw	mm6, qword ptr[const0]
			punpcklbw	mm3, qword ptr[const0]
			punpcklbw	mm1, qword ptr[const0]
			punpcklbw	mm5, qword ptr[const0]

			paddsw		mm2, mm6
			paddsw		mm0, mm3
			paddsw		mm4, mm1
			paddsw		mm7, mm5

			packuswb	mm2, mm2
			packuswb	mm0, mm0
			packuswb	mm4, mm4
			packuswb	mm7, mm7

			mov		eax, stride
		
			movd	[edx], mm2
			movd	[edx+eax], mm0
			lea		edx, [edx+eax*2]
			movd	[edx], mm4
			movd	[edx+eax], mm7
			
			ret
inverse_transform4x4_asm	ENDP

END

; *************************************************************************