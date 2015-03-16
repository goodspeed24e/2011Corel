
/*!
*************************************************************************************
* \file context_ini.c
*
* \brief
*    CABAC context initializations
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Detlev Marpe                    <marpe@hhi.de>
*    - Heiko Schwarz                   <hschwarz@hhi.de>
**************************************************************************************
*/

#define CONTEXT_INI_C

#include "defines.h"
#include "global.h"
#include "biaridecod.h"
#include "ctx_tables.h"


#define BIARI_CTX_INIT2(ii,jj,ctx,tab,num) \
{ \
	for (i=0; i<ii; i++) \
	for (j=0; j<jj; j++) \
	{ \
	if      (IMGPAR type==I_SLICE)  biari_init_context ARGS2(&(ctx[i][j]), &(tab ## _I[num][i][j][0])); \
		else                            biari_init_context ARGS2(&(ctx[i][j]), &(tab ## _P[num][i][j][0])); \
	} \
}
#define BIARI_CTX_INIT1(jj,ctx,tab,num) \
{ \
	for (j=0; j<jj; j++) \
	{ \
	if   (IMGPAR type==I_SLICE)  biari_init_context ARGS2(&(ctx[j]), &(tab ## _I[num][0][j][0])); \
		else                         biari_init_context ARGS2(&(ctx[j]), &(tab ## _P[num][0][j][0])); \
	} \
}


void init_contexts PARGS0()
{
	MotionInfoContexts*  mc = IMGPAR currentSlice->mot_ctx;
	TextureInfoContexts* tc = IMGPAR currentSlice->tex_ctx;
	int i, j;

	DEBUG_SHOW_SW_INFO("%d -", IMGPAR model_number);

	//--- motion coding contexts ---
	BIARI_CTX_INIT2 (3, NUM_MB_TYPE_CTX,   mc->mb_type_contexts,     INIT_MB_TYPE,    IMGPAR model_number);
	BIARI_CTX_INIT2 (2, NUM_B8_TYPE_CTX,   mc->b8_type_contexts,     INIT_B8_TYPE,    IMGPAR model_number);
	BIARI_CTX_INIT2 (2, NUM_MV_RES_CTX,    mc->mv_res_contexts,      INIT_MV_RES,     IMGPAR model_number);
	BIARI_CTX_INIT2 (2, NUM_REF_NO_CTX,    mc->ref_no_contexts,      INIT_REF_NO,     IMGPAR model_number);
	BIARI_CTX_INIT1 (   NUM_DELTA_QP_CTX,  mc->delta_qp_contexts,    INIT_DELTA_QP,   IMGPAR model_number);
	BIARI_CTX_INIT1 (   NUM_MB_AFF_CTX,    mc->mb_aff_contexts,      INIT_MB_AFF,     IMGPAR model_number);
	BIARI_CTX_INIT1 (   NUM_TRANSFORM_SIZE_CTX,  mc->transform_size_contexts,    INIT_TRANSFORM_SIZE,   IMGPAR model_number);


	//--- texture coding contexts ---
	BIARI_CTX_INIT1 (                 NUM_IPR_CTX,  tc->ipr_contexts,     INIT_IPR,       IMGPAR model_number);
	BIARI_CTX_INIT1 (                 NUM_CIPR_CTX, tc->cipr_contexts,    INIT_CIPR,      IMGPAR model_number);
	BIARI_CTX_INIT2 (3,               NUM_CBP_CTX,  tc->cbp_contexts,     INIT_CBP,       IMGPAR model_number);
	BIARI_CTX_INIT2 (NUM_BLOCK_TYPES, NUM_BCBP_CTX, tc->bcbp_contexts,    INIT_BCBP,      IMGPAR model_number);
	BIARI_CTX_INIT2 (NUM_BLOCK_TYPES, NUM_MAP_CTX,  tc->map_contexts,     INIT_MAP,       IMGPAR model_number);
	BIARI_CTX_INIT2 (NUM_BLOCK_TYPES, NUM_LAST_CTX, tc->last_contexts,    INIT_LAST,      IMGPAR model_number);
	BIARI_CTX_INIT2 (NUM_BLOCK_TYPES, NUM_ONE_CTX,  tc->one_contexts,     INIT_ONE,       IMGPAR model_number);
	BIARI_CTX_INIT2 (NUM_BLOCK_TYPES, NUM_ABS_CTX,  tc->abs_contexts,     INIT_ABS,       IMGPAR model_number);
	BIARI_CTX_INIT2 (NUM_BLOCK_TYPES, NUM_MAP_CTX,  tc->fld_map_contexts, INIT_FLD_MAP,   IMGPAR model_number);
	BIARI_CTX_INIT2 (NUM_BLOCK_TYPES, NUM_LAST_CTX, tc->fld_last_contexts,INIT_FLD_LAST,  IMGPAR model_number);
}

