/*!
***************************************************************************
* \file
*    biaridecod.h
*
* \brief
*    Headerfile for binary arithmetic decoder routines
*
* \author
*    Detlev Marpe,
*    Gabi Blättermann
*    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
*
* \date
*    21. Oct 2000
**************************************************************************
*/

#ifndef _BIARIDECOD_H_
#define _BIARIDECOD_H_

void arideco_start_decoding PARGS0();
DecodingEnvironment *arideco_get_dep PARGS0();
void biari_init_context PARGS2(BiContextTypePtr ctx, const int *ini);

#ifdef CONFIG_BIARI_ENABLE_ASM
extern "C" unsigned int __fastcall biari_decode_final_asm PARGS0();
extern "C" unsigned int __fastcall biari_decode_symbol_asm PARGS1( BiContextTypePtr bi_ct );
extern "C" unsigned int __fastcall biari_decode_symbol_eq_prob_asm PARGS0();
#elif defined(CONFIG_BIARI_ENABLE_MMX)
extern "C" void __fastcall store_dep_mmx PARGS0();
extern "C" void __fastcall load_dep_mmx PARGS0();
extern "C" unsigned int __fastcall biari_decode_final_mmx ();
extern "C" unsigned int __fastcall biari_decode_symbol_mmx ( BiContextTypePtr bi_ct );
extern "C" unsigned int __fastcall biari_decode_symbol_eq_prob_mmx ();
#else
extern "C" unsigned int __fastcall biari_decode_final_c PARGS0();
extern "C" unsigned int __fastcall biari_decode_symbol_c PARGS1( BiContextTypePtr bi_ct );
extern "C" unsigned int __fastcall biari_decode_symbol_eq_prob_c PARGS0();
#endif

#endif  // BIARIDECOD_H_

