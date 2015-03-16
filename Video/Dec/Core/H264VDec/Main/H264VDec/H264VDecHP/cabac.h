
/*!
***************************************************************************
* \file
*    cabac.h
*
* \brief
*    Headerfile for entropy coding routines
*
* \author
*    Detlev Marpe                                                         \n
*    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
*
* \date
*    21. Oct 2000 (Changes by Tobias Oelbaum 28.08.2001)
***************************************************************************
*/

#ifndef _CABAC_H_
#define _CABAC_H_

#include "global.h"

MotionInfoContexts* create_contexts_MotionInfo(void);
TextureInfoContexts* create_contexts_TextureInfo(void);
void delete_contexts_MotionInfo(MotionInfoContexts *deco_ctx);
void delete_contexts_TextureInfo(TextureInfoContexts *deco_ctx);

void cabac_new_slice PARGS0();

int readIntraPredMode_CABAC PARGS0();
int readMVD_CABAC PARGS2(int list_idx, MotionVector *mvd);


int readCIPredMode_CABAC PARGS0();

int check_next_mb_and_get_field_mode_CABAC_even PARGS0();
void CheckAvailabilityOfNeighborsCABAC_odd PARGS0();
void CheckAvailabilityOfNeighborsCABAC_even PARGS0();

// added, used in macroblock.cpp
void set_significance_map_ctx PARGS2(int frame_flag, int type);

int cabac_startcode_follows PARGS1(int eos_bit);

byte readIPCMBytes_CABAC PARGS0();

int readMB_typeInfo_CABAC PARGS0();
int readB8_typeInfo_CABAC PARGS0();
int readRefFrame_CABAC PARGS1(int list);
int readCBP_CABAC PARGS0();
int readDquant_CABAC PARGS0();
int readMB_skip_flagInfo_CABAC PARGS0();
int readFieldModeInfo_CABAC PARGS0(); 
int readMB_transform_size_flag_CABAC PARGS0();
int read_significance_map_coefficients PARGS0();
int read_significance_map_coefficients_qp_s_4 PARGS5(const byte *scan_ptr, const short *Inv_table, short *pCof, const short qp_shift, const short qp_const);
int read_significance_map_coefficients_qp_l_4 PARGS4(const byte *scan_ptr, const short *Inv_table, short *pCof, const short qp_shift);

int read_and_store_CBP_block_bit_LUMA_16DC PARGS0();
int read_and_store_CBP_block_bit_LUMA_16AC PARGS0();
int read_and_store_CBP_block_bit_LUMA_8x8 PARGS0();
int read_and_store_CBP_block_bit_LUMA_4x4 PARGS0();
int read_and_store_CBP_block_bit_CHROMA_DC PARGS0();
int read_and_store_CBP_block_bit_CHROMA_AC PARGS0();

#endif  // _CABAC_H_

