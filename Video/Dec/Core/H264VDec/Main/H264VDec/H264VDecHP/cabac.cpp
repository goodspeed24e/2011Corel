
//
// 1) Need to cache the results of the neighboring block code.
// 2) Need to optimize the run/level to avoid unnecessary function calls and
// to allow SSE inverse quantization. (Requires symmetric CAVLC change).
// 3) Avoid the function pointer indirection through readSyntaxElement_CABAC()... that
// function should not exist and always mispredicts.
// 4) Add memory prefetches to contexts before using.
//

/*!
*************************************************************************************
* \file cabac.c
*
* \brief
*    CABAC entropy coding routines
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Detlev Marpe                    <marpe@hhi.de>
**************************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

#include "global.h"
#include "memalloc.h"
#include "elements.h"
#include "image.h"
#include "mb_access.h"
#include "cabac.h"
#include "biaridecod.h"
#include "defines.h"

//#define CONFIG_BIARI_ENABLE_BRANCH
#define CONFIG_BIARI_ENABLE_BRANCH_2
#if defined(__linux__) && !defined(__INTEL_COMPILER)
#define GOTO_RETURN goto local_return; //__asm__ ("jmp local_return")
#else
#define GOTO_RETURN _asm jmp local_return;
#endif

static unsigned int unary_bin_decode P_BARGS1(
	BiContextTypePtr ctx);

static unsigned int unary_bin_max_decode P_BARGS1(
	BiContextTypePtr ctx);

#if 0
static unsigned int exp_golomb_decode_eq_prob P_BARGS1(
	int k);
#else
static unsigned int exp_golomb_decode_eq_prob0 P_BARGS0();
static unsigned int exp_golomb_decode_eq_prob3 P_BARGS0();
#endif

static unsigned int unary_exp_golomb_level_decode P_BARGS1(
	BiContextTypePtr ctx);

static unsigned int unary_exp_golomb_mv_decode P_BARGS1(
	BiContextTypePtr ctx);

void CheckAvailabilityOfNeighborsCABAC_odd PARGS0()
{

	if(IMGPAR MbaffFrameFlag)
	{
		Macroblock * pMBTemp;
		pMBTemp = IMGPAR pLeftMB_r;
		if(pMBTemp && currMB_r->mb_field==pMBTemp->mb_field) {			
			pMBTemp++;
		}
		IMGPAR pLeftMB_temp = pMBTemp;

		pMBTemp = IMGPAR pUpMB_r;

		if(currMB_r->mb_field) { //field pair, bottom	
			if (pMBTemp) {
				IMGPAR pUpMB_temp = pMBTemp + 1;
				//IMGPAR pUpMB_r = pMBTemp + 1;
			} else {
				IMGPAR pUpMB_temp = NULL;				
			}
		} else {	// frame pair, bottom
			IMGPAR pUpMB_temp = currMB_r - 1;
			//IMGPAR pUpMB_r = currMB_r - 1;
		}		

	} else {

		IMGPAR pUpMB_temp = IMGPAR pUpMB_r;
		IMGPAR pLeftMB_temp = IMGPAR pLeftMB_r;
	}

}

void CheckAvailabilityOfNeighborsCABAC_even PARGS0()
{
	IMGPAR pLeftMB_temp = IMGPAR pLeftMB_r;

	if(IMGPAR MbaffFrameFlag) {		
		Macroblock* pUpMB;

		pUpMB = IMGPAR pUpMB_r;
		if ( pUpMB == NULL ) {
			IMGPAR pUpMB_temp = NULL;
		} else {
			if(!(currMB_r->mb_field && pUpMB->mb_field) ) {
				pUpMB++;
			}
			IMGPAR pUpMB_temp = pUpMB;
		}

	} else {
		IMGPAR pUpMB_temp = IMGPAR pUpMB_r;
	}
}

void cabac_new_slice PARGS0()
{
	last_dquant=0;
}

/*!
************************************************************************
* \brief
*    Allocation of contexts models for the motion info
*    used for arithmetic decoding
*
************************************************************************
*/
MotionInfoContexts* create_contexts_MotionInfo(void)
{
	MotionInfoContexts *deco_ctx;

	deco_ctx = (MotionInfoContexts*) _aligned_malloc(sizeof(MotionInfoContexts), 16);
	if( deco_ctx == NULL )
		no_mem_exit("create_contexts_MotionInfo: deco_ctx");

	return deco_ctx;
}


/*!
************************************************************************
* \brief
*    Allocates of contexts models for the texture info
*    used for arithmetic decoding
************************************************************************
*/
TextureInfoContexts* create_contexts_TextureInfo(void)
{
	TextureInfoContexts *deco_ctx;

	deco_ctx = (TextureInfoContexts*) _aligned_malloc(sizeof(TextureInfoContexts), 16);
	if( deco_ctx == NULL )
		no_mem_exit("create_contexts_TextureInfo: deco_ctx");

	return deco_ctx;
}




/*!
************************************************************************
* \brief
*    Frees the memory of the contexts models
*    used for arithmetic decoding of the motion info.
************************************************************************
*/
void delete_contexts_MotionInfo(MotionInfoContexts *deco_ctx)
{
	if( deco_ctx == NULL )
		return;

	_aligned_free( deco_ctx );
	deco_ctx = NULL;

	return;
}


/*!
************************************************************************
* \brief
*    Frees the memory of the contexts models
*    used for arithmetic decoding of the texture info.
************************************************************************
*/
void delete_contexts_TextureInfo(TextureInfoContexts *deco_ctx)
{
	if( deco_ctx == NULL )
		return;

	_aligned_free( deco_ctx );
	deco_ctx = NULL;

	return;
}

int readFieldModeInfo_CABAC PARGS0()
{
	int act_ctx;

	act_ctx = 0;
	if(IMGPAR pLeftMB_r)
		act_ctx = IMGPAR pLeftMB_r->mb_field;
	if(IMGPAR pUpMB_r)
		act_ctx += IMGPAR pUpMB_r->mb_field;
	return biari_decode_symbol BARGS1(&IMGPAR currentSlice->mot_ctx->mb_aff_contexts[act_ctx]);
}


//#define CONFIG_CABAC_DYNAMIC_ALLOC
int check_next_mb_and_get_field_mode_CABAC_even PARGS0()
{
	int skip_flag;
	int skip;
#if !defined(CONFIG_BIARI_ENABLE_MMX)
	DecodingEnvironment dep_copy;
#endif

#ifdef CONFIG_CABAC_DYNAMIC_ALLOC
	BiContextType *mb_aff_ctx_copy;
	BiContextType (*mb_type_ctx_copy)[NUM_MB_TYPE_CTX];

	mb_aff_ctx_copy = (BiContextType *) _aligned_malloc((NUM_MB_AFF_CTX+4*NUM_MB_TYPE_CTX)*sizeof(BiContextType), 16);
	mb_type_ctx_copy = (BiContextType (*)[NUM_MB_TYPE_CTX])(mb_aff_ctx_copy+NUM_MB_AFF_CTX);
#else
	BiContextType mb_aff_ctx_copy[NUM_MB_AFF_CTX];
	BiContextType mb_type_ctx_copy[4][NUM_MB_TYPE_CTX];
#endif
	//get next MB
	{
		Macroblock * pMBTemp;
		pMBTemp = IMGPAR pLeftMB_r;
		if(pMBTemp && currMB_r->mb_field==pMBTemp->mb_field) {			
			pMBTemp++;
		}
		IMGPAR pLeftMB_temp = pMBTemp;

		pMBTemp = IMGPAR pUpMB_r;

		if(currMB_r->mb_field) { //field pair, bottom	
			if (pMBTemp) {
				IMGPAR pUpMB_temp = pMBTemp + 1;
			} else {
				IMGPAR pUpMB_temp = NULL;
			}
		} else {	// frame pair, bottom
			IMGPAR pUpMB_temp = currMB_r;	//Tricky method, using 1st MB in pair as 2nd MB's mb_field calculation
		}
	}
	//copy
#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0(); // save dep
#else
	memcpy(&dep_copy, &(IMGPAR g_dep), sizeof(dep_copy));
#endif /* CONFIG_BIARI_ENABLE_MMX */
	memcpy(mb_aff_ctx_copy, IMGPAR currentSlice->mot_ctx->mb_aff_contexts, sizeof(IMGPAR currentSlice->mot_ctx->mb_aff_contexts));
	memcpy(mb_type_ctx_copy, IMGPAR currentSlice->mot_ctx->mb_type_contexts, sizeof(IMGPAR currentSlice->mot_ctx->mb_type_contexts));
	//check_next_mb
	last_dquant = 0;
	skip_flag = readMB_skip_flagInfo_CABAC ARGS0();
	skip = (skip_flag==0);
	if(!skip)
	{
		currMB_r->mb_field = readFieldModeInfo_CABAC ARGS0();
	}
	//reset
	//IMGPAR current_mb_nr_r--;
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0(); // restore dep
#else
	memcpy(&(IMGPAR g_dep), &dep_copy, sizeof(dep_copy));
#endif /* CONFIG_BIARI_ENABLE_MMX */
	memcpy(IMGPAR currentSlice->mot_ctx->mb_aff_contexts, mb_aff_ctx_copy, sizeof(IMGPAR currentSlice->mot_ctx->mb_aff_contexts));
	memcpy(IMGPAR currentSlice->mot_ctx->mb_type_contexts, mb_type_ctx_copy, sizeof(IMGPAR currentSlice->mot_ctx->mb_type_contexts));
	//CheckAvailabilityOfNeighborsCABAC ARGS0();
#ifdef CONFIG_CABAC_DYNAMIC_ALLOC
	_aligned_free(mb_aff_ctx_copy);
#endif
	return skip;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the motion
*    vector data of a B-frame MB.
************************************************************************
*/
int readMVD_CABAC PARGS2(int list_idx, MotionVector *mvd)
{
	int a_x, a_y, b_x, b_y;
	int i = IMGPAR subblock_x;
	int j = IMGPAR subblock_y;
	int mb_nr = IMGPAR current_mb_nr_r;
	MotionInfoContexts *ctx = IMGPAR currentSlice->mot_ctx;

	PixelPos block_a, block_b;
	PixelPos *p_block_a;
	Macroblock *mb_a, *mb_b;

	mb_a = mb_b = 0;

#if !defined(CONFIG_BIARI_ENABLE_BRANCH_2)
	int curr_mbAff_field = IMGPAR MbaffFrameFlag&currMB_r->mb_field;
	int curr_mbAff_frame = IMGPAR MbaffFrameFlag&(!currMB_r->mb_field);
#endif

	list_idx &= 1;

	if(i)
	{
		p_block_a = &block_a;		
		p_block_a->x = i-1;
		p_block_a->y = j;

		mb_a = currMB_r;

		a_x = fast_abs_short(IMGPAR curr_mvd[list_idx][(p_block_a->y<<2)+p_block_a->x].x);
		a_y = fast_abs_short(IMGPAR curr_mvd[list_idx][(p_block_a->y<<2)+p_block_a->x].y);
	}
	else
	{
		p_block_a = &(IMGPAR left[j]);
		if(p_block_a->pMB)
		{
			mb_a = p_block_a->pMB;

			if(IS_INTERMV(mb_a))
			{
				a_x = fast_abs_short(IMGPAR left_mvd[p_block_a->mb_addr&IMGPAR mvd_pairs_mask][list_idx][p_block_a->y].x);
				a_y = fast_abs_short(IMGPAR left_mvd[p_block_a->mb_addr&IMGPAR mvd_pairs_mask][list_idx][p_block_a->y].y);
			}
			else
			{
				a_x=a_y=0;
			}
#if !defined(CONFIG_BIARI_ENABLE_BRANCH_2)
			int lshift = curr_mbAff_frame&(mb_a->mb_field);
			a_y <<= lshift;
			int rshift = curr_mbAff_field&(!mb_a->mb_field);
			a_y >>= rshift;
#else
			if(IMGPAR MbaffFrameFlag) 
			{
				if(currMB_r->mb_field)
				{
					if(!mb_a->mb_field)
						a_y >>= 1;
				}
				else
				{
					if(mb_a->mb_field)
						a_y <<= 1;
				}
			}
#endif
		}
		else
			a_x = a_y = 0;
	}

	if(j)
	{
		block_b.x = i;
		block_b.y = j-1;

		mb_b = currMB_r;

		b_x = fast_abs_short(IMGPAR curr_mvd[list_idx][(block_b.y<<2)+block_b.x].x);
		b_y = fast_abs_short(IMGPAR curr_mvd[list_idx][(block_b.y<<2)+block_b.x].y);
	}
	else
	{
		if(IMGPAR pUpMB_r)
		{
			mb_b = IMGPAR pUpMB_r;

			if(IS_INTERMV(mb_b))
			{
				b_x = fast_abs_short(mb_b->upmvd[list_idx][i].x);
				b_y = fast_abs_short(mb_b->upmvd[list_idx][i].y);
			}
			else
			{
				b_x=b_y=0;
			}

#if !defined(CONFIG_BIARI_ENABLE_BRANCH_2)
			int lshift = curr_mbAff_frame&(mb_b->mb_field);
			b_y <<= lshift;
			int rshift = curr_mbAff_field&(!mb_b->mb_field);
			b_y >>= rshift;
#else
			if(IMGPAR MbaffFrameFlag)
			{
				if(currMB_r->mb_field)
				{
					if(!mb_b->mb_field)
						b_y >>= 1;
				}
				else
				{
					if(mb_b->mb_field)
						b_y <<= 1;
				}			
			}
#endif
		}
		else
			b_x = b_y = 0;
	}

	a_x += b_x;
	a_y += b_y;

#if !defined(CONFIG_BIARI_ENABLE_BRANCH_2)
	a_x = 5*0 + 2*((a_x-3)>>31)+((a_x-33)>>31) + 3;
	a_y = 5*1 + 2*((a_y-3)>>31)+((a_y-33)>>31) + 3;
#else
	a_x = ((a_x>2)<<1) + (a_x>32);
	a_y = ((a_y>2)<<1) + (a_y>32) + 5;
#endif

	if(biari_decode_symbol BARGS1(&ctx->mv_res_contexts[0][a_x]))
	{
		a_x = unary_exp_golomb_mv_decode BARGS1(ctx->mv_res_contexts[1]);
		b_x = biari_decode_symbol_eq_prob BARGS0();
		mvd->x = (a_x^-b_x) + b_x;
	}
	else
		mvd->x = 0;


	if(biari_decode_symbol BARGS1(&ctx->mv_res_contexts[0][a_y]))
	{
		a_y = unary_exp_golomb_mv_decode BARGS1(ctx->mv_res_contexts[1]+5);
		b_y = biari_decode_symbol_eq_prob BARGS0();
		mvd->y = (a_y^-b_y) + b_y;
	}
	else
		mvd->y = 0;

	return 0;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the 8x8 block type.
************************************************************************
*/

int readB8_typeInfo_CABAC PARGS0()
{
	int ret_val;
	MotionInfoContexts *ctx = IMGPAR currentSlice->mot_ctx;

	if(IMGPAR type!=B_SLICE)
	{
		if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[0][1]))
		{
			ret_val = 0;
local_return:
			return ret_val;
		}
		if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[0][3]))
		{
			ret_val = 3 - biari_decode_symbol BARGS1(&ctx->b8_type_contexts[0][4]);
			GOTO_RETURN;
		}
		ret_val = 1;
		GOTO_RETURN;
	}
	int	act_sym;

	if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][0]))
	{
		if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][1]))
		{
			if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][2]))
			{
				if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][3]))
				{
					ret_val = 11 + 
						biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][3]);
					GOTO_RETURN;
				}
				act_sym = 7;
check_twice:
#ifdef CONFIG_BIARI_ENABLE_BRANCH
				if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][3]))
					act_sym += 2;
				if(biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][3]))
					act_sym += 1;
#else
				act_sym += biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][3])<<1;
				act_sym += biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][3]);
#endif
				ret_val = act_sym;
				GOTO_RETURN;
			}
			act_sym = 3;
			goto check_twice;
		}
		ret_val = 1 + biari_decode_symbol BARGS1(&ctx->b8_type_contexts[1][3]);
		GOTO_RETURN;
	}
	ret_val = 0;
	return ret_val;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the macroblock
*    type info of a given MB.
************************************************************************
*/
int readMB_skip_flagInfo_CABAC PARGS0()
{
	int ret_val;
	int act_ctx;
	MotionInfoContexts *ctx = (IMGPAR currentSlice)->mot_ctx;

	act_ctx = 0;
	if(IMGPAR pUpMB_temp && IMGPAR pUpMB_temp->skip_flag==0)
		act_ctx = 1;
	if(IMGPAR pLeftMB_temp && IMGPAR pLeftMB_temp->skip_flag==0)
		act_ctx++;
	if(IMGPAR type==B_SLICE)
		ret_val = biari_decode_symbol BARGS1(&ctx->mb_type_contexts[2][act_ctx+7])^1;
	else
		ret_val = biari_decode_symbol BARGS1(&ctx->mb_type_contexts[1][act_ctx])^1;
	if(!ret_val)
		last_dquant=0;
	return ret_val;
}

/*!
***************************************************************************
* \brief
*    This function is used to arithmetically decode the macroblock
*    intra_pred_size flag info of a given MB.
***************************************************************************
*/
int readMB_transform_size_flag_CABAC PARGS0()
{
	int act_ctx;

	//	currMB_r  = &dec_picture->mb_data[IMGPAR current_mb_nr_r];
	act_ctx = 0;
	if(IMGPAR pUpMB_r)
		act_ctx = IMGPAR pUpMB_r->luma_transform_size_8x8_flag;
	if(IMGPAR pLeftMB_r)
		act_ctx += IMGPAR pLeftMB_r->luma_transform_size_8x8_flag;

	return biari_decode_symbol BARGS1((IMGPAR currentSlice->mot_ctx->transform_size_contexts + act_ctx));
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the macroblock
*    type info of a given MB.
************************************************************************
*/


#ifdef CONFIG_CABAC_ENABLE_BRANCH

int readMB_typeInfo_CABAC PARGS0()
{
	int ret_val;
	int act_sym;
	BiContextType (*ctx)[NUM_MB_TYPE_CTX];

	ctx = IMGPAR currentSlice->mot_ctx->mb_type_contexts;
	//	currMB_r = &dec_picture->mb_data[IMGPAR current_mb_nr_r];
	if(IMGPAR type==I_SLICE)  // INTRA-frame
	{
		act_sym = 0;
		if(currMB_r->mb_available_up && 
			currMB_r->mb_available_up->mb_type!=I4MB &&
			currMB_r->mb_available_up->mb_type!=I8MB)
			act_sym = 1;
		if(currMB_r->mb_available_left &&
			currMB_r->mb_available_left->mb_type!=I4MB &&
			currMB_r->mb_available_left->mb_type!=I8MB)
			act_sym += 1;

		// 4x4 Intra
		if(biari_decode_symbol BARGS1(&ctx[0][act_sym])==0)
		{
			ret_val = 0;
local_return:
			return ret_val;
		}
		// 16x16 Intra
		if(biari_decode_final BARGS0())
		{
			ret_val = 25;
			GOTO_RETURN;
		}
		// decoding of AC/no AC
		act_sym = biari_decode_symbol BARGS1(&ctx[0][4])*12 + 1;
		// decoding of cbp: 0,1,2
		if(biari_decode_symbol BARGS1(&ctx[0][5]))
			act_sym += (biari_decode_symbol BARGS1(&ctx[0][6])<<2) + 4;
		// decoding of I pred-mode: 0,1,2,3
		if(biari_decode_symbol BARGS1(&ctx[0][7]))
			act_sym += 2;
		if(biari_decode_symbol BARGS1(&ctx[0][8]))
			act_sym += 1;
		ret_val = act_sym;
		GOTO_RETURN;
	}
	if(IMGPAR type==B_SLICE)
	{
		act_sym = 0;
		if(currMB_r->mb_available_up && currMB_r->mb_available_up->mb_type)
			act_sym = 1;
		if(currMB_r->mb_available_left && currMB_r->mb_available_left->mb_type)
			act_sym += 1;
		if(biari_decode_symbol BARGS1(&ctx[2][act_sym])==0)
		{
			ret_val = 0;
			GOTO_RETURN;
		}
		if(biari_decode_symbol BARGS1(&ctx[2][4])==0)
		{
			ret_val = biari_decode_symbol BARGS1(&ctx[2][6]) + 1;
			GOTO_RETURN;
		}
		if(biari_decode_symbol BARGS1(&ctx[2][5])==0)
		{
			act_sym = 3 + (biari_decode_symbol BARGS1(&ctx[2][6])<<2);
			if(biari_decode_symbol BARGS1(&ctx[2][6]))
				act_sym += 2;
			if(biari_decode_symbol BARGS1(&ctx[2][6]))
				act_sym += 1;
			ret_val = act_sym;
			GOTO_RETURN;
		}
		act_sym = 12 + (biari_decode_symbol BARGS1(&ctx[2][6])<<3);
		if(biari_decode_symbol BARGS1(&ctx[2][6]))
			act_sym += 4;
		if(biari_decode_symbol BARGS1(&ctx[2][6]))
			act_sym += 2;
		if(act_sym==24)
		{
			ret_val = 11;
			GOTO_RETURN;
		}
		if(act_sym==26)
		{
			ret_val = 22;
			GOTO_RETURN;
		}
		if(act_sym==22)
			act_sym = 23;
		if(biari_decode_symbol BARGS1(&ctx[2][6]))
			act_sym += 1;
		if(act_sym<=23)
		{
			ret_val = act_sym;
			GOTO_RETURN;
		}
		if(biari_decode_final BARGS0())
		{
			ret_val = 48;
			GOTO_RETURN;
		}
	}
	else // P-frame
	{
		if(biari_decode_symbol BARGS1(&ctx[1][4])==0) 
		{
			if(biari_decode_symbol BARGS1(&ctx[1][5]))
			{
				ret_val = 3 - biari_decode_symbol BARGS1(&ctx[1][7]);
				GOTO_RETURN;
			}
			ret_val = biari_decode_symbol BARGS1(&ctx[1][6])*3 + 1;
			GOTO_RETURN;
		}
		if(biari_decode_symbol BARGS1(&ctx[1][7])==0)
		{
			ret_val = 6;
			GOTO_RETURN;
		}
		if(biari_decode_final BARGS0())
		{
			ret_val = 31;
			GOTO_RETURN;
		}
		act_sym = 7;
	}
	// additional info for 16x16 Intra-mode
	// decoding of AC/no AC
	if(biari_decode_symbol BARGS1(&ctx[1][8]))
		act_sym += 12;
	// decoding of cbp: 0,1,2
	if(biari_decode_symbol BARGS1(&ctx[1][9]))
		act_sym += 4 + (biari_decode_symbol BARGS1(&ctx[1][9])<<2);
	// decoding of I pred-mode: 0,1,2,3
	if(biari_decode_symbol BARGS1(&ctx[1][10]))
		act_sym += 2;
	if(biari_decode_symbol BARGS1(&ctx[1][10]))
		act_sym += 1;
	ret_val = act_sym;
	//  if(curr_mb_type >= 23)       DEBUG_SHOW_SW_INFO(" stopx");
	GOTO_RETURN;
}

#else

int readMB_typeInfo_CABAC PARGS0()
{
	int ret_val;
	int act_sym;
	BiContextType (*ctx)[NUM_MB_TYPE_CTX];

	ctx = IMGPAR currentSlice->mot_ctx->mb_type_contexts;
	//	currMB_r = &dec_picture->mb_data[IMGPAR current_mb_nr_r];
	if(IMGPAR type==I_SLICE)  // INTRA-frame
	{
		act_sym = 0;
		if(IMGPAR pUpMB_r && 
			IMGPAR pUpMB_r->mb_type!=I4MB &&
			IMGPAR pUpMB_r->mb_type!=I8MB)
			act_sym = 1;
		if(IMGPAR pLeftMB_r &&
			IMGPAR pLeftMB_r->mb_type!=I4MB &&
			IMGPAR pLeftMB_r->mb_type!=I8MB)
			act_sym += 1;

		// 4x4 Intra
		if(biari_decode_symbol BARGS1(&ctx[0][act_sym])==0)
		{
			ret_val = 0;
local_return:
			return ret_val;
		}
		// 16x16 Intra
		if(biari_decode_final BARGS0())
		{
			ret_val = 25;
			GOTO_RETURN;
		}
		// decoding of AC/no AC
		act_sym = biari_decode_symbol BARGS1(&ctx[0][4])*12 + 1;
		// decoding of cbp: 0,1,2
		if(biari_decode_symbol BARGS1(&ctx[0][5]))
			act_sym += (biari_decode_symbol BARGS1(&ctx[0][6])<<2) + 4;
		// decoding of I pred-mode: 0,1,2,3
		act_sym += biari_decode_symbol BARGS1(&ctx[0][7])<<1;
		act_sym += biari_decode_symbol BARGS1(&ctx[0][8]);
		ret_val = act_sym;
		GOTO_RETURN;
	}
	if(IMGPAR type==B_SLICE)
	{
		act_sym = 0;
		if(IMGPAR pUpMB_r && IMGPAR pUpMB_r->mb_type)
			act_sym = 1;
		if(IMGPAR pLeftMB_r && IMGPAR pLeftMB_r->mb_type)
			act_sym += 1;
		if(biari_decode_symbol BARGS1(&ctx[2][act_sym])==0)
		{
			ret_val = 0;
			GOTO_RETURN;
		}
		if(biari_decode_symbol BARGS1(&ctx[2][4])==0)
		{
			ret_val = biari_decode_symbol BARGS1(&ctx[2][6]) + 1;
			GOTO_RETURN;
		}
		if(biari_decode_symbol BARGS1(&ctx[2][5])==0)
		{
			act_sym = 3 + (biari_decode_symbol BARGS1(&ctx[2][6])<<2);
			act_sym += biari_decode_symbol BARGS1(&ctx[2][6])<<1;
			act_sym += biari_decode_symbol BARGS1(&ctx[2][6]);
			ret_val = act_sym;
			GOTO_RETURN;
		}
		act_sym = 12 + (biari_decode_symbol BARGS1(&ctx[2][6])<<3);
		act_sym += biari_decode_symbol BARGS1(&ctx[2][6])<<2;
		act_sym += biari_decode_symbol BARGS1(&ctx[2][6])<<1;
		if(act_sym==24)
		{
			ret_val = 11;
			GOTO_RETURN;
		}
		if(act_sym==26)
		{
			ret_val = 22;
			GOTO_RETURN;
		}
		if(act_sym==22)
			act_sym = 23;
		act_sym += biari_decode_symbol BARGS1(&ctx[2][6]);
		if(act_sym<=23)
		{
			ret_val = act_sym;
			GOTO_RETURN;
		}
		if(biari_decode_final BARGS0())
		{
			ret_val = 48;
			GOTO_RETURN;
		}
	}
	else // P-frame
	{
		if(biari_decode_symbol BARGS1(&ctx[1][4])==0) 
		{
			if(biari_decode_symbol BARGS1(&ctx[1][5]))
			{
				ret_val = 3 - biari_decode_symbol BARGS1(&ctx[1][7]);
				GOTO_RETURN;
			}
			ret_val = biari_decode_symbol BARGS1(&ctx[1][6])*3 + 1;
			GOTO_RETURN;
		}
		if(biari_decode_symbol BARGS1(&ctx[1][7])==0)
		{
			ret_val = 6;
			GOTO_RETURN;
		}
		if(biari_decode_final BARGS0())
		{
			ret_val = 31;
			GOTO_RETURN;
		}
		act_sym = 7;
	}

	// additional info for 16x16 Intra-mode
	// decoding of AC/no AC
	act_sym += biari_decode_symbol BARGS1(&ctx[1][8])*12;
	// decoding of cbp: 0,1,2
	if(biari_decode_symbol BARGS1(&ctx[1][9]))
		act_sym += 4 + (biari_decode_symbol BARGS1(&ctx[1][9])<<2);
	// decoding of I pred-mode: 0,1,2,3
	act_sym += biari_decode_symbol BARGS1(&ctx[1][10])<<1;
	act_sym += biari_decode_symbol BARGS1(&ctx[1][10]);
	ret_val = act_sym;
	//  if(curr_mb_type >= 23)       DEBUG_SHOW_SW_INFO(" stopx");
	GOTO_RETURN;
}

#endif

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode a pair of
*    intra prediction modes of a given MB.
************************************************************************
*/

int readIntraPredMode_CABAC PARGS0()
{
	int ret_val;
	TextureInfoContexts *ctx;

	ctx = IMGPAR currentSlice->tex_ctx;
	if(biari_decode_symbol BARGS1(ctx->ipr_contexts))
		ret_val = -1;
	else
	{
		ret_val  = biari_decode_symbol BARGS1(ctx->ipr_contexts+1);
		ret_val |= biari_decode_symbol BARGS1(ctx->ipr_contexts+1)<<1;
		ret_val |= biari_decode_symbol BARGS1(ctx->ipr_contexts+1)<<2;
	}
	return ret_val;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the reference
*    parameter of a given MB.
************************************************************************
*/
int readRefFrame_CABAC PARGS1(int list)
{
	const int addctx  = 0;
	int ret_val, act_sym;
	int a = IMGPAR subblock_x; 
	int b = IMGPAR subblock_y;
	MotionInfoContexts *ctx = IMGPAR currentSlice->mot_ctx;

	PixelPos block_a, block_b;
	PixelPos* p_block_a;
	Macroblock *mb;


	if(a)
	{
		p_block_a = &block_a;
		block_a.mb_addr = IMGPAR current_mb_nr_r;
		block_a.pMB = currMB_r;
		block_a.x = a-1;
		block_a.y = b;
	}
	else
	{
		p_block_a = &(IMGPAR left[b]);
	}

	if(b)
	{
		block_b.mb_addr = IMGPAR current_mb_nr_r;
		block_b.pMB = currMB_r;
		block_b.x = a;
		block_b.y = b-1;
	}
	else
	{
		block_b.mb_addr = IMGPAR mbAddrB;
		block_b.pMB = IMGPAR pUpMB_r;
		block_b.x = a;
		block_b.y = 3;
	}

	act_sym = 0;
#if !defined(CONFIG_BIARI_ENABLE_BRANCH_2)
	int curr_mbAff_frame = (IMGPAR MbaffFrameFlag)&(!currMB_s_r->mb_field);
#endif

	if (mb = block_b.pMB)
	{
		b  = ((block_b.x>>1)&1) + (block_b.y&2);
#if !defined(CONFIG_BIARI_ENABLE_BRANCH_2)
		if(  mb->mb_type!=IPCM &&
			!IS_DIRECT(mb) &&
			!(mb->b8mode[b]==0 && mb->b8pdir[b]==2) &&
			(dec_picture->mb_data[block_b.mb_addr].pred_info.ref_idx[list][b]>
			(curr_mbAff_frame&mb->mb_field)))
			act_sym = 2;
#else
		if(  mb->mb_type!=IPCM &&
			!IS_DIRECT(mb) &&
			!(mb->b8mode[b]==0 && mb->b8pdir[b]==2))
		{
			if(IMGPAR MbaffFrameFlag && currMB_r->mb_field==0 && mb->mb_field==1)
			{
				if(dec_picture->mb_data[block_b.mb_addr].pred_info.ref_idx[list][b]>1)
					act_sym = 2;
			}
			else
			{
				if(dec_picture->mb_data[block_b.mb_addr].pred_info.ref_idx[list][b]>0)
					act_sym = 2;
			}
		}
#endif
	}


	if (mb = p_block_a->pMB)
	{
		a  = ((p_block_a->x>>1)&1) + (p_block_a->y&2);
#if !defined(CONFIG_BIARI_ENABLE_BRANCH_2)
		if( mb->mb_type!=IPCM &&
			!IS_DIRECT(mb) &&
			!(mb->b8mode[a]==0 && mb->b8pdir[a]==2)  &&
			(dec_picture->mb_data[p_block_a->mb_addr].pred_info.ref_idx[list][a]>
			(curr_mbAff_frame&mb->mb_field)))
			act_sym++;
#else
		if( mb->mb_type!=IPCM &&
			!IS_DIRECT(mb) &&
			!(mb->b8mode[a]==0 && mb->b8pdir[a]==2))
		{
			if(IMGPAR MbaffFrameFlag && currMB_r->mb_field==0 && mb->mb_field==1)
			{
				if(dec_picture->mb_data[p_block_a->mb_addr].pred_info.ref_idx[list][a]>1)
					act_sym++;
			}
			else
			{
				if(dec_picture->mb_data[p_block_a->mb_addr].pred_info.ref_idx[list][a]>0)
					act_sym++;
			}
		}
#endif
	}

	if(biari_decode_symbol BARGS1(&ctx->ref_no_contexts[addctx][act_sym]))
		ret_val = unary_bin_decode BARGS1(&ctx->ref_no_contexts[addctx][4]) + 1;
	else
		ret_val = 0;

	return ret_val;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the delta qp
*     of a given MB.
************************************************************************
*/
int readDquant_CABAC PARGS0()
{
	BiContextTypePtr ctx;
	int dquant;
	unsigned int act_sym;

	ctx = IMGPAR currentSlice->mot_ctx->delta_qp_contexts;
	if(biari_decode_symbol BARGS1((ctx + (last_dquant!=0)))==0)
	{
		last_dquant = 0;
		return last_dquant;
	}
	act_sym = unary_bin_decode BARGS1((ctx + 2));
	dquant = act_sym/2 + 1;
#ifdef CONFIG_BIARI_ENABLE_BRANCH
	if(act_sym & 1)                           // lsb is signed bit
		dquant = -dquant;
#else
	act_sym &= 1;
	dquant = (dquant ^ -(signed)act_sym) + act_sym;
#endif
	last_dquant = dquant;
	return dquant;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the coded
*    block pattern of a given MB.
************************************************************************
*/
int readCBP_CABAC PARGS0()
{
	TextureInfoContexts *ctx;
	// int mb_x, mb_y;
	int b, cbp;
	PixelPos block_a;
	PixelPos *p_block_a = &block_a;
	int da, db;

	ctx = IMGPAR currentSlice->tex_ctx;
	//	currMB_r = &dec_picture->mb_data[IMGPAR current_mb_nr_r];
	//  coding of luma part (bit by bit)
	// this might be better unrolled as there are only 4 cases!
	if(IMGPAR pUpMB_r &&
		IMGPAR pUpMB_r->mb_type!=IPCM)
		db = IMGPAR pUpMB_r->cbp;
	else
		db = 0xFFFF;
	//mb_y=0;
	//mb_x=0;
	b = (db>>1 & 2)^2;

	p_block_a = &(IMGPAR left[0]);
	if(p_block_a->pMB &&
		p_block_a->pMB->mb_type!=IPCM &&
		((p_block_a->pMB->cbp>>(p_block_a->y|1)) & 1)==0)
		b |= 1;
	cbp = biari_decode_symbol BARGS1(ctx->cbp_contexts[0]+b);
	//mb_y=0;
	//mb_x=2;
	b = ((db>>2 & 2)|(cbp & 1))^3;
	cbp += biari_decode_symbol BARGS1(ctx->cbp_contexts[0]+b)<<1;
	//mb_y=2;
	//mb_x=0;
	b = ((cbp & 1)^1)<<1;

	p_block_a = &(IMGPAR left[2]);
	if(p_block_a->pMB &&
		p_block_a->pMB->mb_type!=IPCM &&
		((p_block_a->pMB->cbp>>(p_block_a->y|1)) & 1)==0)
		b |= 1;
	cbp += biari_decode_symbol BARGS1(ctx->cbp_contexts[0]+b)<<2;
	//mb_y=2;
	//mb_x=2;
	b = ((cbp & 2)|(cbp>>2 & 1))^3;
	cbp += biari_decode_symbol BARGS1(ctx->cbp_contexts[0]+b)<<3;
	// coding of chroma part
#ifdef __SUPPORT_YUV400__
	if(dec_picture->chroma_format_idc!=YUV400)
	{
#endif
		da = 0;
		db = 0;
#define USE_PARTIAL
#ifdef USE_PARTIAL
		// CABAC decoding for BinIdx 0
		b = 0;
		if(IMGPAR pUpMB_r &&
			(IMGPAR pUpMB_r->mb_type==IPCM ||
			(db = IMGPAR pUpMB_r->cbp>>4)))
			b = 2;
		if(IMGPAR pLeftMB_r &&
			(IMGPAR pLeftMB_r->mb_type==IPCM ||
			(da = IMGPAR pLeftMB_r->cbp>>4)))
			b += 1;
#else
		// CABAC decoding for BinIdx 0
		b = 0;
		if(currMB_r->mb_available_up &&
			(currMB_r->mb_available_up->mb_type==IPCM ||
			currMB_r->mb_available_up->cbp>15))
			b = 2;
		if(currMB_r->mb_available_left &&
			(currMB_r->mb_available_left->mb_type==IPCM ||
			currMB_r->mb_available_left->cbp>15))
			b += 1;
#endif
		// CABAC decoding for BinIdx 1 
		if(biari_decode_symbol BARGS1((ctx->cbp_contexts[1] + b))) // set the chroma bits
		{
#ifdef USE_PARTIAL
			b &= (((db&~2)==0)<<1) + ((da&~2)==0);	// hope to use LEA
#else
			b = 0;
			if(currMB_r->mb_available_up &&
				(currMB_r->mb_available_up->mb_type==IPCM ||
				(currMB_r->mb_available_up->cbp>15 &&
				currMB_r->mb_available_up->cbp>>4==2)))
				b = 2;
			if(currMB_r->mb_available_left &&
				(currMB_r->mb_available_left->mb_type==IPCM ||
				(currMB_r->mb_available_left->cbp>15 && 
				currMB_r->mb_available_left->cbp>>4==2)))
				b += 1;
#endif
			cbp += 16 << biari_decode_symbol BARGS1((ctx->cbp_contexts[2] + b));
		}
#ifdef __SUPPORT_YUV400__
	}
#endif
	if(!cbp)
		last_dquant = 0;

	return cbp;
}

/*!
************************************************************************
* \brief
*    This function is used to arithmetically decode the chroma
*    intra prediction mode of a given MB.
************************************************************************
*/  //GB

int readCIPredMode_CABAC PARGS0()
{
	TextureInfoContexts *ctx;
	int b, act_sym;

	//	currMB_r  = &dec_picture->mb_data[IMGPAR current_mb_nr_r];
	b = 0;
	if(IMGPAR pUpMB_r &&
		IMGPAR pUpMB_r->mb_type!=IPCM &&
		IMGPAR pUpMB_r->c_ipred_mode!=0)
		b = 1;
	if(IMGPAR pLeftMB_r &&
		IMGPAR pLeftMB_r->mb_type!=IPCM &&
		IMGPAR pLeftMB_r->c_ipred_mode!=0)
		b += 1;
	ctx = IMGPAR currentSlice->tex_ctx;
	if((act_sym = biari_decode_symbol BARGS1((ctx->cipr_contexts + b))))
		act_sym = unary_bin_max_decode BARGS1(ctx->cipr_contexts+3);
	return act_sym;
}

// These tables used to have 10 entries, but the last 2 were deleted since they are only used for the case of YUV422 & YUV444
static const __declspec(align(16)) unsigned char c1isdc_maxpos[][2]  = { 
	{0, 15}, {1, 15}, {0, 63}, {0, 31},
	{0, 31}, {0, 15}, {0,  3}, {1, 15},
	//{0,  7}, {0, 15}
};
static const unsigned char type2ctx_bcbp[]     = {
	0,  1,  2,  2,
	3,  4,  5,  6,
	//5,  5
}; // 7
// type2ctx_map and type2ctx_last are the same - use only one
static const unsigned char type2ctx_map_last[] = {
	0,  1,  2,  3,
	4,  5,  6,  7,
	//6,  6
}; // 8
// type2ctx_one and type2ctx_abs are the same - use only one
static const unsigned char type2ctx_one_abs[]  = {
	0,  1,  2,  3,
	3,  4,  5,  6,
	//5,  5
}; // 7
// We can eliminate these multiple table for High Profile, since max number for CHROMA_DC is 3
static const unsigned char new_c2[][5]         = {
	{ 1, 2, 3, 4, 4},
	{ 1, 2, 3, 4, 4},
	{ 1, 2, 3, 4, 4},
	{ 1, 2, 3, 4, 4},
	{ 1, 2, 3, 4, 4},
	{ 1, 2, 3, 4, 4},
	{ 1, 2, 3, 3, 3},
	{ 1, 2, 3, 4, 4},
	//{ 1, 2, 3, 3, 3},
	//{ 1, 2, 3, 3, 3}
}; // (type, old_c2)

//#define LUMA_16DC       0
//#define LUMA_16AC       1
//#define LUMA_8x8        2
//#define LUMA_8x4        3
//#define LUMA_4x8        4
//#define LUMA_4x4        5
//#define CHROMA_DC       6
//#define CHROMA_AC       7
// CHROMA_DC_2x4 and CHROMA_DC_2x4 are only used in YUV422 and YUV444 profiles ?
//#define CHROMA_DC_2x4   8
//#define CHROMA_DC_4x4   9
//#define NUM_BLOCK_TYPES 10 - changed to 8 in defines.h

#define CBP_FLAG_LUMA	1
#define CBP_FLAG_CHROMA	2
#define CBP_FLAG_DC		4
#define CBP_FLAG_AC		8

#if 0
typedef struct {
	unsigned char flags;
	unsigned char bit_base;
	unsigned char bit_mask;
} cbp_struct;

static const cbp_struct cbp_array[] = 
{
	CBP_FLAG_LUMA|CBP_FLAG_DC,0,1,
	CBP_FLAG_LUMA|CBP_FLAG_AC,1,1,
	CBP_FLAG_LUMA|CBP_FLAG_AC,1,0x33,
	CBP_FLAG_LUMA|CBP_FLAG_AC,1,0x03,
	CBP_FLAG_LUMA|CBP_FLAG_AC,1,0x11,
	CBP_FLAG_LUMA|CBP_FLAG_AC,1,1,
	CBP_FLAG_CHROMA|CBP_FLAG_DC,17,1,
	CBP_FLAG_CHROMA|CBP_FLAG_AC,19,1,
	//CBP_FLAG_CHROMA|CBP_FLAG_DC,17,1,
	//CBP_FLAG_CHROMA|CBP_FLAG_DC,17,1};
#else
typedef struct {
	unsigned char flags;
	unsigned char bit_mask;
} cbp_struct;

static const cbp_struct cbp_array[] = 
{
	{CBP_FLAG_LUMA|CBP_FLAG_DC,1},
	{CBP_FLAG_LUMA|CBP_FLAG_AC,1},
	{CBP_FLAG_LUMA|CBP_FLAG_AC,0x33},
	{CBP_FLAG_LUMA|CBP_FLAG_AC,0x03},
	{CBP_FLAG_LUMA|CBP_FLAG_AC,0x11},
	{CBP_FLAG_LUMA|CBP_FLAG_AC,1},
	{CBP_FLAG_CHROMA|CBP_FLAG_DC,1},
	{CBP_FLAG_CHROMA|CBP_FLAG_AC,1},
	//{CBP_FLAG_CHROMA|CBP_FLAG_DC,1},
	//{CBP_FLAG_CHROMA|CBP_FLAG_DC,1}
};
#endif

int read_and_store_CBP_block_bit_LUMA_16DC PARGS0()
{
	int cbp_bit;  // always one for 8x8 mode
	int bit_pos_a;
	int bit_pos_b;
	int  bit_base, bit_scan;

	PixelPos* p_block_a;
	Macroblock *mb_a, *mb_b;

	int mb_nr = IMGPAR current_mb_nr_r;
	//--- get bits from neighbouring blocks ---
	int upper_bit, left_bit, default_bit;

	default_bit = IMGPAR is_intra_block!=0;	  

	mb_a = mb_b = 0;
	bit_scan = 0;
	bit_base = 0; // LUMA-DC: 0
	bit_pos_a = bit_pos_b = 0;

	p_block_a = &(IMGPAR left[0]);
	if(p_block_a->pMB)
	{
		mb_a = p_block_a->pMB;
		if(mb_a->mb_type==IPCM)
			left_bit = 1;
		else
			left_bit = (mb_a->cbp_bits >> (bit_base+bit_pos_a) & 1);
	}
	else
		left_bit = default_bit;

	if(IMGPAR pUpMB_r)
	{
		mb_b = IMGPAR pUpMB_r;
		if(mb_b->mb_type==IPCM)
			upper_bit = 1;
		else
			upper_bit = (mb_b->cbp_bits >> (bit_base+bit_pos_b) & 1);
	}
	else
		upper_bit = default_bit;

	cbp_bit = biari_decode_symbol BARGS1((IMGPAR currentSlice->tex_ctx->bcbp_contexts[0] + (upper_bit<<1) + left_bit));

	if(cbp_bit)
	{   //--- set bits for current block ---
		currMB_r->cbp_bits |= ((unsigned int) 1) << (bit_base + bit_scan);
	}
	return cbp_bit;
}

int read_and_store_CBP_block_bit_LUMA_16AC PARGS0()
{
	int cbp_bit;  // always one for 8x8 mode
	int bit_pos_a;
	int bit_pos_b;
	int i, j, bit_base, bit_scan;
	PixelPos* p_block_a;
	Macroblock *mb_a, *mb_b;	

	int mb_nr = IMGPAR current_mb_nr_r;
	//--- get bits from neighbouring blocks ---
	int upper_bit, left_bit, default_bit;

	default_bit = IMGPAR is_intra_block!=0;

	mb_a = mb_b = 0;
	j = IMGPAR subblock_y;
	i = IMGPAR subblock_x;
	bit_base = 1; // LUMA - AC: 1-16
	bit_scan = (j<<2) + i;

	if(i)
	{ // Same MB
		bit_pos_a = bit_scan - 1; // Left 4x4 block
		left_bit = (currMB_r->cbp_bits >> (bit_base+bit_pos_a) & 1);
	}
	else
	{
		p_block_a = &(IMGPAR left[j]);
		if(p_block_a->pMB)
		{
			mb_a = p_block_a->pMB;
			bit_pos_a = (p_block_a->y<<2) + p_block_a->x;
			if(mb_a->mb_type==IPCM)
				left_bit = 1;
			else
				left_bit = (mb_a->cbp_bits >> (bit_base+bit_pos_a) & 1);
		}
		else
			left_bit = default_bit;
	}

	if(j)
	{ // Same MB
		bit_pos_b = bit_scan - 4; // Upper 4x4 block
		upper_bit = (currMB_r->cbp_bits >> (bit_base+bit_pos_b) & 1);
	}
	else
	{
		if(IMGPAR pUpMB_r)
		{
			mb_b = IMGPAR pUpMB_r;
			bit_pos_b = (3<<2) + i;
			if(mb_b->mb_type==IPCM)
				upper_bit = 1;
			else
				upper_bit = (mb_b->cbp_bits >> (bit_base+bit_pos_b) & 1);
		}
		else
			upper_bit = default_bit;
	}

	cbp_bit = biari_decode_symbol BARGS1((IMGPAR currentSlice->tex_ctx->bcbp_contexts[1] + (upper_bit<<1) + left_bit));

	if(cbp_bit)
	{   //--- set bits for current block ---
		currMB_r->cbp_bits |= ((unsigned int) 1) << (bit_base + bit_scan);
	}
	return cbp_bit;
}

int read_and_store_CBP_block_bit_LUMA_8x8 PARGS0()
{
	int i, j;

	j = IMGPAR subblock_y;
	i = IMGPAR subblock_x;
	currMB_r->cbp_bits |= (0x33) << ((j<<2) + i + 1);
	return 1;
}


int read_and_store_CBP_block_bit_LUMA_4x4 PARGS0()
{
	int cbp_bit;  // always one for 8x8 mode
	int bit_pos_a;
	int bit_pos_b;
	int i, j, bit_base, bit_scan;
	PixelPos* p_block_a;
	Macroblock *mb_a, *mb_b;

	int mb_nr = IMGPAR current_mb_nr_r;
	//--- get bits from neighbouring blocks ---
	int upper_bit, left_bit, default_bit;

	default_bit = IMGPAR is_intra_block!=0;

	mb_a = mb_b = 0;

	j = IMGPAR subblock_y;
	i = IMGPAR subblock_x;
	bit_base = 1; // LUMA - AC: 1-16
	bit_scan = (j<<2) + i;

	if(i)
	{ // Same MB
		bit_pos_a = bit_scan - 1; // Left 4x4 block
		left_bit = (currMB_r->cbp_bits >> (bit_base+bit_pos_a) & 1);
	}
	else
	{
		p_block_a = &(IMGPAR left[j]);
		if(p_block_a->pMB)
		{
			mb_a = p_block_a->pMB;
			bit_pos_a = (p_block_a->y<<2) + p_block_a->x;
			if(mb_a->mb_type==IPCM)
				left_bit = 1;
			else
				left_bit = (mb_a->cbp_bits >> (bit_base+bit_pos_a) & 1);
		}
		else
			left_bit = default_bit;
	}

	if(j)
	{ // Same MB
		bit_pos_b = bit_scan - 4; // Upper 4x4 block
		upper_bit = (currMB_r->cbp_bits >> (bit_base+bit_pos_b) & 1);
	}
	else
	{
		if(IMGPAR pUpMB_r)
		{
			mb_b = IMGPAR pUpMB_r;
			bit_pos_b = (3<<2) + i;
			if(mb_b->mb_type==IPCM)
				upper_bit = 1;
			else
				upper_bit = (mb_b->cbp_bits >> (bit_base+bit_pos_b) & 1);
		}
		else
			upper_bit = default_bit;
	}

	cbp_bit = biari_decode_symbol BARGS1((IMGPAR currentSlice->tex_ctx->bcbp_contexts[4] + (upper_bit<<1) + left_bit));

	if(cbp_bit)
	{   //--- set bits for current block ---
		currMB_r->cbp_bits |= ((unsigned int) 1) << (bit_base + bit_scan);
	}
	return cbp_bit;
}

int read_and_store_CBP_block_bit_CHROMA_DC PARGS0()
{
	int cbp_bit;  // always one for 8x8 mode
	int bit_pos_a;
	int bit_pos_b;
	int bit_base, bit_scan;
	PixelPos block_a;
	PixelPos* p_block_a;
	Macroblock *mb_a, *mb_b;	

	int mb_nr = IMGPAR current_mb_nr_r;
	//--- get bits from neighbouring blocks ---
	int upper_bit, left_bit, default_bit;

	default_bit = IMGPAR is_intra_block!=0;

	mb_a = mb_b = 0;
	bit_scan  = 0;
	bit_base  = 17 + IMGPAR is_v_block; // CHROMA-DC(U):17, CHROMA-DC(V):18
	bit_pos_a = bit_pos_b = 0;
	p_block_a = &block_a;

	memcpy(p_block_a, &(IMGPAR left[0]), sizeof(PixelPos));

	if(p_block_a->pMB)
	{
		mb_a = p_block_a->pMB;
		if(mb_a->mb_type==IPCM)
			left_bit = 1;
		else
			left_bit = (mb_a->cbp_bits >> (bit_base+bit_pos_a) & 1);
	}
	else
		left_bit = default_bit;

	if(IMGPAR pUpMB_r)
	{
		mb_b = IMGPAR pUpMB_r;
		if(mb_b->mb_type==IPCM)
			upper_bit = 1;
		else
			upper_bit = (mb_b->cbp_bits >> (bit_base+bit_pos_b) & 1);
	}
	else
		upper_bit = default_bit;

	cbp_bit = biari_decode_symbol BARGS1((IMGPAR currentSlice->tex_ctx->bcbp_contexts[5] + (upper_bit<<1) + left_bit));

	if(cbp_bit)
	{   //--- set bits for current block ---
		currMB_r->cbp_bits |= ((unsigned int) 1) << (bit_base + bit_scan);
	}
	return cbp_bit;
}


int read_and_store_CBP_block_bit_CHROMA_AC PARGS0()
{
	int cbp_bit;  // always one for 8x8 mode
	int bit_pos_a;
	int bit_pos_b;
	int i, j, bit_base, bit_scan;	
	PixelPos* p_block_a;

	Macroblock *mb_a, *mb_b;	

	int mb_nr = IMGPAR current_mb_nr_r;
	//--- get bits from neighbouring blocks ---
	int upper_bit, left_bit, default_bit;

	default_bit = IMGPAR is_intra_block!=0;

	mb_a = mb_b = 0;

	j = IMGPAR subblock_y;
	i = IMGPAR subblock_x;
	bit_base  = 19 + (IMGPAR is_v_block<<2); // CHROMA-AC(U): 19-22, CHROMA-AC(V): 23-26
	bit_scan = (j<<1) + i; // High Profile restriction	

	if (i) //===> i=1
	{
		//bit_pos_a = (j<<1) + (i-1);
		bit_pos_a = (j<<1);
		left_bit = (currMB_r->cbp_bits >> (bit_base+bit_pos_a) & 1);
	}
	else
	{
		p_block_a = &(IMGPAR left[j<<1]);		


		if(mb_a = p_block_a->pMB)
		{			
			bit_pos_a = ((p_block_a->y>>1)<<1) + (p_block_a->x>>1);
			if(mb_a->mb_type==IPCM)
				left_bit = 1;
			else
				left_bit = (mb_a->cbp_bits >> (bit_base+bit_pos_a) & 1);
		}  
		else
			left_bit = default_bit;
	}

	if (j) //===> j=1
	{
		//bit_pos_b = ((j-1)<<1) + i;
		bit_pos_b = i;
		upper_bit = (currMB_r->cbp_bits >> (bit_base+bit_pos_b) & 1);
	}
	else
	{        
		if(mb_b=IMGPAR pUpMB_r)
		{			
			bit_pos_b = ((3>>1)<<1) + i;
			if(mb_b->mb_type==IPCM)
				upper_bit = 1;
			else
				upper_bit = (mb_b->cbp_bits >> (bit_base+bit_pos_b) & 1);
		}
		else
			upper_bit = default_bit;
	}

	cbp_bit = biari_decode_symbol BARGS1((IMGPAR currentSlice->tex_ctx->bcbp_contexts[6] + (upper_bit<<1) + left_bit));

	if(cbp_bit)
	{   //--- set bits for current block ---
		currMB_r->cbp_bits |= ((unsigned int) 1) << (bit_base + bit_scan);
	}
	return cbp_bit;
}



//===== position -> ctx for MAP =====
//--- zig-zag scan ----
static const __declspec(align(16)) unsigned char pos2ctx_map8x8[] =
{
	0,  1,  2,  3,  4,  5,  5,  4,  4,  3,  3,  4,  4,  4,  5,  5,
	4,  4,  4,  4,  3,  3,  6,  7,  7,  7,  8,  9, 10,  9,  8,  7,
	7,  6, 11, 12, 13, 11,  6,  7,  8,  9, 14, 10,  9,  8,  6, 11,
	12, 13, 11,  6,  9, 14, 10,  9, 11, 12, 13, 11, 14, 10, 12, 14
}; // 15 CTX
static const unsigned char pos2ctx_map8x4[] =
{
	0,  1,  2,  3,  4,  5,  7,  8,  9, 10, 11,  9,  8,  6,  7,  8,
	9, 10, 11,  9,  8,  6, 12,  8,  9, 10, 11,  9, 13, 13, 14, 14
}; // 15 CTX
static const unsigned char pos2ctx_map4x4[] =
{
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 14
}; // 15 CTX
// The following 2 should be used only for YUV422 & YUV444
/*
static const unsigned char pos2ctx_map2x4c[] =
{
0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX
static const unsigned char pos2ctx_map4x4c[] =
{
0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX
*/

//--- interlace scan ----
//taken from ABT
static const unsigned char pos2ctx_map8x8i[] =
{
	0,  1,  1,  2,  2,  3,  3,  4,  5,  6,  7,  7,  7,  8,  4,  5,
	6,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 11, 12, 11,
	9,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 13, 13,  9,
	9, 10, 10,  8, 13, 13,  9,  9, 10, 10, 14, 14, 14, 14, 14, 14
}; // 15 CTX
static const unsigned char pos2ctx_map8x4i[] =
{
	0,  1,  2,  3,  4,  5,  6,  3,  4,  5,  6,  3,  4,  7,  6,  8,
	9,  7,  6,  8,  9, 10, 11, 12, 12, 10, 11, 13, 13, 14, 14, 14
}; // 15 CTX
static const unsigned char pos2ctx_map4x8i[] =
{
	0,  1,  1,  1,  2,  3,  3,  4,  4,  4,  5,  6,  2,  7,  7,  8,
	8,  8,  5,  6,  9, 10, 10, 11, 11, 11, 12, 13, 13, 14, 14, 14
}; // 15 CTX
// Combined table for field/frame (0/1)
static const unsigned char *pos2ctx_map[][2] = 
{
	{pos2ctx_map4x4, pos2ctx_map4x4 }, {pos2ctx_map4x4, pos2ctx_map4x4 },
	{pos2ctx_map8x8i,pos2ctx_map8x8 }, {pos2ctx_map8x4i,pos2ctx_map8x4 },
	{pos2ctx_map4x8i,pos2ctx_map8x4 }, {pos2ctx_map4x4, pos2ctx_map4x4 },
	{pos2ctx_map4x4, pos2ctx_map4x4 }, {pos2ctx_map4x4, pos2ctx_map4x4 },
	//	{pos2ctx_map2x4c,pos2ctx_map2x4c}, {pos2ctx_map4x4c,pos2ctx_map4x4c}
};

//===== position -> ctx for LAST =====
static const unsigned char pos2ctx_last8x8[] =
{
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,
	5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8
}; //  9 CTX
static const unsigned char pos2ctx_last8x4[] =
{
	0,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,
	3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8
}; //  9 CTX
static const unsigned char pos2ctx_last4x4[] =
{
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 14
}; // 15 CTX
// The following 2 should be used only for YUV422 & YUV444
/*
static const unsigned char pos2ctx_last2x4c[] =
{
0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX
static const unsigned char pos2ctx_last4x4c[] =
{
0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX
*/
static const unsigned char *pos2ctx_last[] =
{
	pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8, pos2ctx_last8x4,
	pos2ctx_last8x4, pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last4x4,
	//    pos2ctx_last2x4c, pos2ctx_last4x4c
};

//move as global
//static const unsigned char *map, *maplast;
//static BiContextTypePtr map_ctx, last_ctx;
//static BiContextTypePtr ctxone, ctxlvl;
//static const unsigned char *c1isdc_maxpos_ptr, *new_c2_ptr;

void set_significance_map_ctx PARGS2(int frame_flag, int type)
{
	/*
	if(!frame_flag)
	{
	map_ctx = IMGPAR currentSlice->tex_ctx->fld_map_contexts[type2ctx_map_last[type]];
	last_ctx = IMGPAR currentSlice->tex_ctx->fld_last_contexts[type2ctx_map_last[type]];
	map = (unsigned char *) pos2ctx_map_int[type];
	}
	else
	{
	map_ctx = IMGPAR currentSlice->tex_ctx->map_contexts[type2ctx_map_last[type]];
	last_ctx = IMGPAR currentSlice->tex_ctx->last_contexts[type2ctx_map_last[type]];
	map = (unsigned char *) pos2ctx_map[type];
	}
	*/
	// We will use the fact that fld_map_contexts and fld_last_contexts are consecutive, and so do
	// map_contexts & last_contexts - look in global.h for details
	// Also, for High Profile (not YUV422 and YUV444), type2ctx_map_last[type] = type
	//map_ctx = IMGPAR currentSlice->tex_ctx->map_contexts[type2ctx_map_last[type]];
	//last_ctx = IMGPAR currentSlice->tex_ctx->last_contexts[type2ctx_map_last[type]];
	map_ctx  = IMGPAR currentSlice->tex_ctx->map_contexts[type] + (MAP_LAST_CTX_FRAME_FIELD_DIFF&(frame_flag-1));
	last_ctx = IMGPAR currentSlice->tex_ctx->last_contexts[type] + (MAP_LAST_CTX_FRAME_FIELD_DIFF&(frame_flag-1));
	map      = (unsigned char *) pos2ctx_map[type][frame_flag];
	maplast  = (unsigned char *) pos2ctx_last[type];
	ctxone   = IMGPAR currentSlice->tex_ctx->one_contexts[type2ctx_one_abs[type]];
	ctxlvl   = IMGPAR currentSlice->tex_ctx->abs_contexts[type2ctx_one_abs[type]];
	c1isdc_maxpos_ptr = (unsigned char *) &c1isdc_maxpos[type][0];
	new_c2_ptr = (unsigned char *) &new_c2[type][0];
}

int read_significance_map_coefficients PARGS0()
{
	static const unsigned char new_c1[5] = { 0, 2, 3, 4, 4 }; // (old_c1)
	int i, i0, i1;
	unsigned int c1,c2;
	int bit, level;
	int coeff_ctr;

	i0 = c1isdc_maxpos_ptr[0];
	i1 = c1isdc_maxpos_ptr[1];
	for(coeff_ctr=0, i=i0; i<i1; i++) // if last coeff is reached, it has to be significant
	{
		if(biari_decode_symbol BARGS1((map_ctx + map[i])))
		{
			coeff_pos[coeff_ctr++] = i;
			if(biari_decode_symbol BARGS1((last_ctx + maplast[i])))
				goto _coefficients;
		}
	}
	coeff_pos[coeff_ctr++] = i;

_coefficients:
	for(c1=1,c2=0,i=coeff_ctr-1; i>=0; i--)
	{
		level = biari_decode_symbol BARGS1((ctxone + c1));
		if(level)
		{
			level += unary_exp_golomb_level_decode BARGS1((ctxlvl + c2));
			c2 = new_c2_ptr[c2];
			c1 = 0;
			bit = biari_decode_symbol_eq_prob BARGS0();
			coeff_level[i] = ((level+1)^-bit) + bit;
			//coeff_level[i] = (((level+1)<<1)&(bit-1)) - (level+1);
		}
		else
		{
			c1 = new_c1[c1];
			bit = biari_decode_symbol_eq_prob BARGS0();
			coeff_level[i] = ((-bit)<<1)+1;
		}
	}

	return coeff_ctr;
}

int read_significance_map_coefficients_qp_s_4 PARGS5(const byte *scan_ptr, const short *Inv_table, short *pCof, const short qp_shift, const short qp_const)
{
	static const unsigned char new_c1[5] = { 0, 2, 3, 4, 4 }; // (old_c1)
	int i, i0, i1, j0;
	unsigned int c1,c2;
	int bit, level;
	int coeff_ctr;
	short pos[64];

	i0 = c1isdc_maxpos_ptr[0];
	i1 = c1isdc_maxpos_ptr[1];
	for(coeff_ctr=0, i=i0; i<i1; i++) // if last coeff is reached, it has to be significant
	{
		if(biari_decode_symbol BARGS1((map_ctx + map[i])))
		{
			pos[coeff_ctr++] = scan_ptr[i];
			if(biari_decode_symbol BARGS1((last_ctx + maplast[i])))
				goto _coefficients;
		}
	}
	pos[coeff_ctr++] = i;

_coefficients:
	for(c1=1,c2=0,i=coeff_ctr-1; i>=0; i--)
	{
		level = biari_decode_symbol BARGS1((ctxone + c1));
		if(level)
		{
			level += unary_exp_golomb_level_decode BARGS1((ctxlvl + c2));
			c2 = new_c2_ptr[c2];
			c1 = 0;
			bit = biari_decode_symbol_eq_prob BARGS0();			
			level = ((level+1)^-bit) + bit;	
		}
		else
		{
			c1 = new_c1[c1];
			bit = biari_decode_symbol_eq_prob BARGS0();			
			level = ((-bit)<<1)+1;			
		}

		j0=pos[i];
		pCof[j0] = (level*Inv_table[j0]+qp_const)>>qp_shift;
	}

	return coeff_ctr;
}

int read_significance_map_coefficients_qp_l_4 PARGS4(const byte *scan_ptr, const short *Inv_table, short *pCof, const short qp_shift)
{
	static const unsigned char new_c1[5] = { 0, 2, 3, 4, 4 }; // (old_c1)
	int i, i0, i1, j0;
	unsigned int c1,c2;
	int bit, level;
	int coeff_ctr;
	short pos[64];

	i0 = c1isdc_maxpos_ptr[0];
	i1 = c1isdc_maxpos_ptr[1];
	for(coeff_ctr=0, i=i0; i<i1; i++) // if last coeff is reached, it has to be significant
	{
		if(biari_decode_symbol BARGS1((map_ctx + map[i])))
		{
			pos[coeff_ctr++] = scan_ptr[i];
			if(biari_decode_symbol BARGS1((last_ctx + maplast[i])))
				goto _coefficients;
		}
	}
	pos[coeff_ctr++] = i;

_coefficients:
	for(c1=1,c2=0,i=coeff_ctr-1; i>=0; i--)
	{
		level = biari_decode_symbol BARGS1((ctxone + c1));
		if(level)
		{
			level += unary_exp_golomb_level_decode BARGS1((ctxlvl + c2));
			c2 = new_c2_ptr[c2];
			c1 = 0;
			bit = biari_decode_symbol_eq_prob BARGS0();			
			level = ((level+1)^-bit) + bit;			
		}
		else
		{
			c1 = new_c1[c1];
			bit = biari_decode_symbol_eq_prob BARGS0();			
			level = ((-bit)<<1)+1;		
		}
		j0=pos[i];	
		pCof[j0] = (level*Inv_table[j0])<<qp_shift;
	}

	return coeff_ctr;
}

unsigned int unary_bin_max_decode P_BARGS1(BiContextTypePtr ctx)
{
	if(biari_decode_symbol BARGS1(ctx)==0)
		return 1;
	if(biari_decode_symbol BARGS1(ctx)==0)
		return 2;
	return 3;
}

unsigned int unary_bin_decode P_BARGS1(BiContextTypePtr ctx)
{
	unsigned int symbol;
	const int ctx_offset = 1;

	if(biari_decode_symbol BARGS1(ctx)==0)
		return 0;
	ctx += ctx_offset;
	for(symbol=1;;symbol++)
	{
		if(biari_decode_symbol BARGS1(ctx)==0)
			return symbol;
	}
}

#if 0
unsigned int exp_golomb_decode_eq_prob P_BARGS1(int k)
{
	unsigned int symbol, binary_symbol;
	int init_k;

	for(init_k=0; biari_decode_symbol_eq_prob BARGS0(); init_k++);
	symbol = ((1<<init_k)-1)<<k;
	for(k+=init_k, binary_symbol=0; k--;) //next binary part
		//binary_symbol = (binary_symbol<<1) + biari_decode_symbol_eq_prob BARGS0();
		binary_symbol |= biari_decode_symbol_eq_prob BARGS0()<<k;
	return symbol + binary_symbol;
}

unsigned int unary_exp_golomb_level_decode P_BARGS1(BiContextTypePtr ctx)
{
	unsigned int k;
	const unsigned int exp_start = 13;

	for(k=0;k<exp_start;k++)
	{
		if(biari_decode_symbol BARGS1(ctx)==0)
			return k;
	}
	//return exp_golomb_decode_eq_prob BARGS1(0) + exp_start;
	return exp_golomb_decode_eq_prob BARGS1(0) + k;
}

unsigned int unary_exp_golomb_mv_decode P_BARGS1(BiContextTypePtr ctx)
{
	unsigned int k;
	const unsigned int exp_start = 8;
	const unsigned int max_bin = 3;

	for(k=1;k<=max_bin;k++)
	{
		if(biari_decode_symbol BARGS1(ctx)==0)
			return k;
		ctx++;
	}
	for(;k<=exp_start;k++)
	{
		if(biari_decode_symbol BARGS1(ctx)==0)
			return k;
	}
	//return exp_golomb_decode_eq_prob BARGS1(3) + exp_start + 1;
	return exp_golomb_decode_eq_prob BARGS1(3) + k;
}

#else
unsigned int exp_golomb_decode_eq_prob0 P_BARGS0()
{
	unsigned int symbol, binary_symbol;
	int k;

	for(k=0; biari_decode_symbol_eq_prob BARGS0(); k++);
	symbol = (1<<k)-1;
	for(binary_symbol=0; k--;) //next binary part
		//binary_symbol = (binary_symbol<<1) + biari_decode_symbol_eq_prob BARGS0();
		binary_symbol |= biari_decode_symbol_eq_prob BARGS0()<<k;
	return symbol + binary_symbol;
}

unsigned int exp_golomb_decode_eq_prob3 P_BARGS0()
{
	unsigned int symbol, binary_symbol;
	int k;

	for(k=0; biari_decode_symbol_eq_prob BARGS0(); k++);
	symbol = ((1<<k)-1)<<3;
	for(k+=3, binary_symbol=0; k--;) //next binary part
		//binary_symbol = (binary_symbol<<1) + biari_decode_symbol_eq_prob BARGS0();
		binary_symbol |= biari_decode_symbol_eq_prob BARGS0()<<k;
	return symbol + binary_symbol;
}

unsigned int unary_exp_golomb_level_decode P_BARGS1(BiContextTypePtr ctx)
{
	unsigned int k;
	const unsigned int exp_start = 13;

	for(k=0;k<exp_start;k++)
	{
		if(biari_decode_symbol BARGS1(ctx)==0)
			return k;
	}
	//return exp_golomb_decode_eq_prob0 BARGS0() + exp_start;
	return exp_golomb_decode_eq_prob0 BARGS0() + k;
}

unsigned int unary_exp_golomb_mv_decode P_BARGS1(BiContextTypePtr ctx)
{
	unsigned int k;
	const unsigned int exp_start = 8;
	const unsigned int max_bin = 3;

	for(k=1;k<=max_bin;k++)
	{
		if(biari_decode_symbol BARGS1(ctx)==0)
			return k;
		ctx++;
	}
	for(;k<=exp_start;k++)
	{
		if(biari_decode_symbol BARGS1(ctx)==0)
			return k;
	}
	//return exp_golomb_decode_eq_prob3 BARGS0() + exp_start + 1;
	return exp_golomb_decode_eq_prob3 BARGS0() + k;
}

#endif

/*!
************************************************************************
* \brief
*    finding end of a slice in case this is not the end of a frame
*
* Unsure whether the "correction" below actually solves an off-by-one
* problem or whether it introduces one in some cases :-(  Anyway,
* with this change the bit stream format works with CABAC again.
* StW, 8.7.02
************************************************************************
*/
int cabac_startcode_follows PARGS1(int eos_bit)
{
	if(eos_bit==0)
		return 0;
#ifdef CONFIG_BIARI_ENABLE_MMX
	store_dep ARGS0();
#endif
	eos_bit = biari_decode_final BARGS0(); //GB
#ifdef CONFIG_BIARI_ENABLE_MMX
	load_dep ARGS0();
#endif
	return eos_bit;
}

/*!
************************************************************************
* \brief
*    Read one byte from CABAC-partition. 
*    DecodingEnvironment will be modified
*    (for IPCM CABAC  28/11/2003)
*
* \author
*    Dong Wang <Dong.Wang@bristol.ac.uk>  
************************************************************************
*/
byte readIPCMBytes_CABAC PARGS0()
{
	byte *read_ptr = IMGPAR g_dep.Dcodestrm;
	byte ret_val;
	const int bitsneeded = 8;

	if(IMGPAR g_dep.Dbits_to_go<bitsneeded)
	{
		ret_val = *read_ptr++;
	}
	else{
		ret_val = read_ptr[-IMGPAR g_dep.Dbits_to_go/8];
		IMGPAR g_dep.Dbits_to_go -= 8;
		IMGPAR g_dep.Dbuffer <<= 8;
	}

	IMGPAR g_dep.Dcodestrm = read_ptr;

	return ret_val;
}
