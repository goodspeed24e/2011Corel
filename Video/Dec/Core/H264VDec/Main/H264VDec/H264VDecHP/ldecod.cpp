#pragma warning ( disable : 4995 )
/*!
***********************************************************************
*  \mainpage
*     This is the H.264/AVC decoder reference software. For detailed documentation
*     see the comments in each file.
*
*  \author
*     The main contributors are listed in contributors.h
*
*  \version
*     JM 9.4 (FRExt)
*
*  \note
*     tags are used for document system "doxygen"
*     available at http://www.doxygen.org
*/
/*!
*  \file
*     ldecod.c
*  \brief
*     H.264/AVC reference decoder project main()
*  \author
*     Main contributors (see contributors.h for copyright, address and affiliation details)
*     - Inge Lille-Langøy       <inge.lille-langoy@telenor.com>
*     - Rickard Sjoberg         <rickard.sjoberg@era.ericsson.se>
*     - Stephan Wenger          <stewe@cs.tu-berlin.de>
*     - Jani Lainema            <jani.lainema@nokia.com>
*     - Sebastian Purreiter     <sebastian.purreiter@mch.siemens.de>
*     - Byeong-Moon Jeon        <jeonbm@lge.com>
*     - Gabi Blaettermann       <blaetter@hhi.de>
*     - Ye-Kui Wang             <wyk@ieee.org>
*     - Valeri George           <george@hhi.de>
*     - Karsten Suehring        <suehring@hhi.de>
*
***********************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <malloc.h>

#if defined WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>


#include <assert.h>

#define  GLOBAL_INSTANCE // create instance of global objects defined in global.h.
#include "global.h"
#ifdef _SUPPORT_RTP_
#include "rtp.h"
#endif
#include "memalloc.h"
#include "mbuffer.h"
#include "fmo.h"
#include "annexb.h"
#include "output.h"
#include "cabac.h"
#include "biaridecod.h"

#define JM          "9 (FRExt)"
#define VERSION     "9.4"
#define EXT_VERSION "(FRExt)"

// I have started to move the inp and img structures into global variables.
// They are declared in the following lines.  Since inp is defined in conio.h
// and cannot be overridden globally, it is defined here as input
//
// Everywhere, input-> and IMGPAR  can now be used either globally or with
// the local override through the formal parameter mechanism



/*!
***********************************************************************
* \brief
*    Initilize FREXT variables
***********************************************************************
*/
void init_frext PARGS0()
{
	// HP restrictions
	//pel bitdepth init
	IMGPAR bitdepth_luma_qp_scale   = 0;
	IMGPAR pic_unit_bitsize_on_disk = 8;
	IMGPAR dc_pred_value = 128;
	IMGPAR max_imgpel_value = 255;
#ifdef __SUPPORT_YUV400__
	if (active_sps->chroma_format_idc != YUV400)
	{
#endif
		//active_sps->chroma_format_idc = YUV420
		//for chrominance part
		IMGPAR bitdepth_chroma_qp_scale = 0;
		IMGPAR max_imgpel_value_uv      = 255;
		IMGPAR num_blk8x8_uv = 2;
		IMGPAR num_cdc_coeff = 4;
		IMGPAR mb_cr_size_x = 8;
		IMGPAR mb_cr_size_y = 8;
#ifdef __SUPPORT_YUV400__
	}
	else
	{
		IMGPAR bitdepth_chroma_qp_scale = 0;
		IMGPAR max_imgpel_value_uv      = 0;
		IMGPAR num_blk8x8_uv = 0;
		IMGPAR num_cdc_coeff = 0;
		IMGPAR mb_cr_size_x  = 0;
		IMGPAR mb_cr_size_y  = 0;
	}
#endif
}

/*!
************************************************************************
* \brief
*    Allocates a stand-alone partition structure.  Structure should
*    be freed by FreePartition();
*    data structures
*
* \par Input:
*    n: number of partitions in the array
* \par return
*    pointer to DecodingEnvironment Structure, zero-initialized
************************************************************************
*/

DecodingEnvironment *AllocPartition PARGS1(int n)
{
	DecodingEnvironment *dep;

	dep = arideco_get_dep ARGS0();
	return dep;
}


/*!
************************************************************************
* \brief
*    Frees a partition structure (array).  
*
* \par Input:
*    Partition to be freed, size of partition Array (Number of Partitions)
*
* \par return
*    None
*
* \note
*    n must be the same as for the corresponding call of AllocPartition
************************************************************************
*/


void FreePartition (DecodingEnvironment *dep)
{
}


/*!
************************************************************************
* \brief
*    Allocates the slice structure along with its dependent
*    data structures
*
* \par Input:
*    Input Parameters
************************************************************************
*/
#if !defined(_COLLECT_PIC_)
void malloc_slice PARGS0()
{
	Slice *currSlice;

	IMGPAR currentSlice = (Slice *) _aligned_malloc(sizeof(Slice), 16);
	if ( (currSlice = IMGPAR currentSlice) == NULL)
	{
		DEBUG_SHOW_ERROR_INFO("[ERROR]Memory allocation for Slice datastruct in NAL-mode failed");
	}
	//  IMGPAR currentSlice->rmpni_buffer=NULL;
	//! you don't know whether we do CABAC hre, hence initialize CABAC anyway
	// if (inp->symbol_mode == CABAC)
	if (1)
	{
		// create all context models
		currSlice->mot_ctx = create_contexts_MotionInfo();
		currSlice->tex_ctx = create_contexts_TextureInfo();
	}
	currSlice->max_part_nr = 3;  //! assume data partitioning (worst case) for the following mallocs()
	currSlice->partArr = AllocPartition ARGS1(currSlice->max_part_nr);
	currSlice->wp_weight = (int *) _aligned_malloc(2*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	currSlice->wp_offset = (int *) _aligned_malloc(6*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	currSlice->wbp_weight = (int *) _aligned_malloc(6*MAX_REFERENCE_PICTURES*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	IMGPAR wp_weight = currSlice->wp_weight;
	IMGPAR wp_offset = currSlice->wp_offset;
	IMGPAR wbp_weight = currSlice->wbp_weight;

}
#endif


#ifdef _COLLECT_PIC_
CREL_RETURN malloc_new_slice(Slice**  ppnewSlice)
{

	Slice* new_slice;

	new_slice = (Slice *) _aligned_malloc(sizeof(Slice), 16);
	if (new_slice == NULL)
	{
		DEBUG_SHOW_ERROR_INFO("[ERROR]Memory allocation for Slice datastruct in NAL-mode failed");
		return CREL_ERROR_H264_NOMEMORY;
	}
	memset(new_slice, 0, sizeof(Slice));
	//  IMGPAR currentSlice->rmpni_buffer=NULL;
	//! you don't know whether we do CABAC hre, hence initialize CABAC anyway
	// if (inp->symbol_mode == CABAC)

	// create all context models
	new_slice->mot_ctx = create_contexts_MotionInfo();
	new_slice->tex_ctx = create_contexts_TextureInfo();

	if ((new_slice->mot_ctx == NULL) || (new_slice->tex_ctx == NULL)) {
		return CREL_ERROR_H264_NOMEMORY;
	}

	new_slice->max_part_nr = 3;  //! assume data partitioning (worst case) for the following mallocs()
	//currSlice->partArr = AllocPartition ARGS1(currSlice->max_part_nr);
	new_slice->g_dep = (DecodingEnvironment *) _aligned_malloc(sizeof(DecodingEnvironment), 16);
	if (new_slice->g_dep == NULL)
	{
		DEBUG_SHOW_ERROR_INFO("[ERROR]AllocPartition: Memory allocation for Decoding Environment failed");
		return CREL_ERROR_H264_NOMEMORY;

	}


	new_slice->remapping_of_pic_nums_idc_l0 = NULL;
	new_slice->abs_diff_pic_num_minus1_l0 = NULL;
	new_slice->long_term_pic_idx_l0 = NULL;
	new_slice->abs_diff_view_idx_minus1_l0 = NULL;
	new_slice->remapping_of_pic_nums_idc_l1 = NULL;
	new_slice->abs_diff_pic_num_minus1_l1 = NULL;
	new_slice->long_term_pic_idx_l1 = NULL;
	new_slice->abs_diff_view_idx_minus1_l1 = NULL;

	new_slice->has_pts = 0;
	memset(&new_slice->pts, 0x0, sizeof(H264_TS));
	memset(&new_slice->dts, 0x0, sizeof(H264_TS));

	new_slice->wp_weight = (int *) _aligned_malloc(2*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	new_slice->wp_offset = (int *) _aligned_malloc(6*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	new_slice->wbp_weight = (int *) _aligned_malloc(6*MAX_REFERENCE_PICTURES*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);

	if ( (new_slice->wp_weight == NULL) || (new_slice->wp_offset == NULL) || (new_slice->wbp_weight == NULL) ) {
		return CREL_ERROR_H264_NOMEMORY;
	}

	new_slice->m_pic_combine_status = -1;

	*ppnewSlice = new_slice;

	return CREL_OK;
}

void free_new_slice(Slice *currSlice)
{		

	_aligned_free(currSlice->g_dep);	
	if (1)
	{
		// delete all context models
		delete_contexts_MotionInfo(currSlice->mot_ctx);
		delete_contexts_TextureInfo(currSlice->tex_ctx);
	}

	_aligned_free((void *) currSlice->wp_weight);
	_aligned_free((void *) currSlice->wp_offset);
	_aligned_free((void *) currSlice->wbp_weight);

	_aligned_free(currSlice);
}
#endif
/*!
************************************************************************
* \brief
*    Memory frees of the Slice structure and of its dependent
*    data structures
*
* \par Input:
*    Input Parameters 
************************************************************************

void free_slice PARGS0()
{
Slice *currSlice = IMGPAR currentSlice;

FreePartition (currSlice->partArr);
//      if (inp->symbol_mode == CABAC)
if (1)
{
// delete all context models
delete_contexts_MotionInfo(currSlice->mot_ctx);
delete_contexts_TextureInfo(currSlice->tex_ctx);
}
_aligned_free(IMGPAR currentSlice);

currSlice = NULL;
}
*/

/*!
************************************************************************
* \brief
*    Dynamic memory allocation of frame size related global buffers
*    buffers are defined in global.h, allocated memory must be freed in
*    void free_global_buffers()
*
*  \par Input:
*    Input Parameters
*
*  \par Output:
*     Number of allocated bytes
***********************************************************************
*/
CREL_RETURN init_global_buffers PARGS0()
{
	if (global_init_done)
	{
		free_global_buffers ARGS0();
	}


	// allocate memory in structure img  
	if(((IMGPAR mb_decdata) = (Macroblock *) _aligned_malloc(IMGPAR FrameSizeInMbs*sizeof(Macroblock), 16)) == NULL)	//Could be updated by single slice/multislice separation
		return CREL_ERROR_H264_NOMEMORY;//no_mem_exit("init_global_buffers: IMGPAR mb_data");

	if(((IMGPAR decoded_flag) = (BYTE *) _aligned_malloc(IMGPAR FrameSizeInMbs, 16)) == NULL)	//Could be updated by single slice/multislice separation
		return CREL_ERROR_H264_NOMEMORY;//no_mem_exit("init_global_buffers: IMGPAR mb_data");

	IMGPAR pMBpair_left = &IMGPAR mbpair[0];
	IMGPAR pMBpair_current = &IMGPAR mbpair[2];

#ifdef _COLLECT_PIC_
	IMGPAR mb_decdata_ori = IMGPAR mb_decdata;
#endif

	//#if !defined(_COLLECT_PIC_)
	//  IMGPAR wp_weight = (int *) _aligned_malloc(2*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	//  IMGPAR wp_offset = (int *) _aligned_malloc(6*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	//  IMGPAR wbp_weight = (int *) _aligned_malloc(6*MAX_REFERENCE_PICTURES*MAX_REFERENCE_PICTURES*3*sizeof(int), 16);
	//#endif

	// CAVLC mem
	IMGPAR nz_coeff = (NZCoeff *) _aligned_malloc(IMGPAR FrameSizeInMbs*sizeof(IMGPAR nz_coeff[0]), 16);
	if (IMGPAR nz_coeff == NULL) {
		return CREL_ERROR_H264_NOMEMORY;
	}

#if !defined(ONE_COF)
	if (IMGPAR array_index <= 1) {
		IMGPAR cof_array_ori = (short*) _aligned_malloc((sizeof(short)*IMGPAR FrameSizeInMbs*384), 16);
		if (IMGPAR cof_array_ori == NULL) {
			return CREL_ERROR_H264_NOMEMORY;
		}
		memset(IMGPAR cof_array_ori, 0, (sizeof(short)*IMGPAR FrameSizeInMbs*384));
	} else {
		IMGPAR cof_array_ori = (short*) _aligned_malloc((sizeof(short)*384), 16);
		if (IMGPAR cof_array_ori == NULL) {
			return CREL_ERROR_H264_NOMEMORY;
		}
		memset(IMGPAR cof_array_ori, 0, (sizeof(short)*384));
	}

	
#endif

	//Grant - the stride here must be the same as the stride in storable_picture - refer to function alloc_storable_picture()
	int stride_y = (IMGPAR width+2*BOUNDARY_EXTENSION_X+MEMORY_ALIGNMENT-1)&((int) ~(MEMORY_ALIGNMENT-1));
	int stride_uv = stride_y;

	IMGPAR m_imgY = (imgpel*) _aligned_malloc (stride_y*32*2*sizeof(imgpel), 64);
	if (IMGPAR m_imgY == NULL) {
		return CREL_ERROR_H264_NOMEMORY;
	}
	IMGPAR m_imgUV = (imgpel*) _aligned_malloc (stride_uv*16*2*sizeof(imgpel), 64);	
	if (IMGPAR m_imgUV == NULL) {
		return CREL_ERROR_H264_NOMEMORY;
	}

#ifdef _COLLECT_PIC_
	IMGPAR cof_array = IMGPAR cof_array_ori;
#endif

#ifdef _COLLECT_PIC_
	int i, j;
	for (i=0; i<2; i++)
	{
		listX[i] = (storable_picture **) _aligned_malloc(MAX_LIST_SIZE_FRAME*sizeof (StorablePicture*), 16); // +1 for reordering
		if (NULL==listX[i]) {
			return CREL_ERROR_H264_NOMEMORY;//no_mem_exit("init_dpb: listX[i]");
		}
	}

	for (i=2; i<6; i++)
	{
		listX[i] = (storable_picture **) _aligned_malloc(MAX_LIST_SIZE*sizeof (StorablePicture*), 16); // +1 for reordering
		if (NULL==listX[i]) {
			return CREL_ERROR_H264_NOMEMORY;//no_mem_exit("init_dpb: listX[i]");
		}
	}

	for (j=0;j<2;j++)
	{
		for (i=0; i<MAX_LIST_SIZE_FRAME; i++)
		{
			listX[j][i] = NULL;
		}
		listXsize[j]=0;
	}

	for (j=2;j<6;j++)
	{
		for (i=0; i<MAX_LIST_SIZE; i++)
		{
			listX[j][i] = NULL;
		}
		listXsize[j]=0;
	}

	IMGPAR last_has_mmco_5 = 0;
#endif

#if defined(_HW_ACCEL_)
	StreamParameters *stream_global = IMGPAR stream_global;

	if ( g_DXVAVer && (IviDxva1==g_DXVAVer && g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A) )
	{
		IMGPAR m_lpRESD_Intra_Luma = (byte *) _aligned_malloc(IMGPAR PicWidthInMbs * 384 * 2 * 2, 64);
		//For 2 Rows of MB pixels, each pixel take 16bit for DXVA residual data transfer
		if ( NULL == IMGPAR m_lpRESD_Intra_Luma ) {
			return CREL_ERROR_H264_NOMEMORY;//no_mem_exit("init_global_buffers: IMGPAR m_lpRESD_Intra_Luma");
		}
	}
#endif

	global_init_done = 1;
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    Free allocated memory of frame size related global buffers
*    buffers are defined in global.h, allocated memory is allocated in
*    void init_global_buffers()
*
* \par Input:
*    Input Parameters 
*
* \par Output:
*    none
*
************************************************************************
*/
void free_global_buffers PARGS0()
{

	// CAVLC free mem
	_aligned_free(IMGPAR nz_coeff);

#ifdef _COLLECT_PIC_
	//IMGPAR cof_array = IMGPAR cof_array_ori; //By Haihua
#endif
#if !defined(ONE_COF)
	//_aligned_free(IMGPAR cof_array);
	_aligned_free(IMGPAR cof_array_ori); //By Haihua
#endif

	_aligned_free(IMGPAR m_imgY);
	_aligned_free(IMGPAR m_imgUV);


	// free mem, allocated for structure img
#ifdef _COLLECT_PIC_
	IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
#endif
	if (IMGPAR mb_decdata != NULL) 
		_aligned_free(IMGPAR mb_decdata);

	if (IMGPAR decoded_flag != NULL) 
		_aligned_free(IMGPAR decoded_flag);

#if !defined(_COLLECT_PIC_)
	_aligned_free((void *) IMGPAR wp_weight);
	_aligned_free((void *) IMGPAR wp_offset);
	_aligned_free((void *) IMGPAR wbp_weight);
#endif

#ifdef _COLLECT_PIC_  
	int i;
	for (i=0; i<6; i++)
		if (listX[i])
		{
			_aligned_free (listX[i]);
			listX[i] = NULL;
		}
#endif

#if defined(_HW_ACCEL_)
		StreamParameters *stream_global = IMGPAR stream_global;

		if ( g_DXVAVer && (IviDxva1==g_DXVAVer && g_DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A) )
		{
			if (IMGPAR m_lpRESD_Intra_Luma != NULL) {
				_aligned_free(IMGPAR m_lpRESD_Intra_Luma);
			}
		}
#endif

		global_init_done = 0;

}
