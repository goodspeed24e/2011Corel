/*!
***********************************************************************
* \file image.c
*
* \brief
*    Decode a Slice
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
*    - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
*    - Jani Lainema                    <jani.lainema@nokia.com>
*    - Sebastian Purreiter             <sebastian.purreiter@mch.siemens.de>
*    - Byeong-Moon Jeon                <jeonbm@lge.com>
*    - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
*    - Gabi Blaettermann               <blaetter@hhi.de>
*    - Ye-Kui Wang                     <wyk@ieee.org>
*    - Antti Hallapuro                 <antti.hallapuro@nokia.com>
*    - Alexis Tourapis                 <alexismt@ieee.org>
***********************************************************************
*/

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include <io.h>
#include "h264dxvabase.h"
#else
#include <unistd.h>
#endif

#include "global.h"
#include "image.h"
#include "mbuffer.h"
#include "fmo.h"
#include "nalu.h"
#include "parsetcommon.h"
#include "parset.h"
#include "header.h"
#include "rtp.h"
#include "sei.h"
#include "output.h"
#include "biaridecod.h"
#include "mb_access.h"
#include "memalloc.h"
#include "annexb.h"
#include "context_ini.h"
#include "cabac.h"
#include "loopfilter.h"
#include "get_block.h"
#include "mb_chroma.h"
#if !defined(_GLOBAL_IMG_)
#include <process.h>
#endif
#include "ATSCDec.h"

#ifdef H264_ENABLE_INTRINSICS
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#define MM_PREFETCH(a,b) _mm_prefetch(a,b)
#else
#define MM_PREFETCH(a,b)
#endif

#include "vlc.h"
#include "videoframe.h"

#include "mb_motcomp.h"

pad_boundary_luma_PX *pad_boundary_luma;
pad_boundary_chroma_PX *pad_boundary_chroma;

extern HRESULT NvH264_DMATransfer(StorablePicture *sp);
extern HRESULT NvH264_GetDeblockingParaBuffer(void **pSurf, StorablePicture *sp);
extern HRESULT NvH264_ReleaseDeblockingParaBuffer();
extern HRESULT NvH264_Deblocking(void *pNvH264DeblockingBuf, StorablePicture *sp);
extern void NvH264_FillDeblockParaBuf(LPVOID pSurf, StorablePicture *p);

extern void calc_chroma_vector_adjustment PARGS2(int list_offset, int curr_mb_field);
extern CREL_RETURN record_reference_picIds PARGS2(int list_offset, Macroblock_s *currMB_s);

#ifdef _COLLECT_PIC_
typedef CREL_RETURN (*pfReadOneMacroblock) PARGS0();
static pfReadOneMacroblock pf_read_one_macroblock[2][3][2] = 
{{{&read_one_macroblock_UVLC_P_even, &read_one_macroblock_UVLC_P_odd}, 
{&read_one_macroblock_UVLC_B_even, &read_one_macroblock_UVLC_B_odd}, 
{&read_one_macroblock_UVLC_I_even, &read_one_macroblock_UVLC_I_odd}},
{{&read_one_macroblock_CABAC_P_even, &read_one_macroblock_CABAC_P_odd}, 
{&read_one_macroblock_CABAC_B_even, &read_one_macroblock_CABAC_B_odd}, 
{&read_one_macroblock_CABAC_I_even, &read_one_macroblock_CABAC_I_odd}},
};
#endif

extern void SelectB8Mode_B PARGS0();
extern void SelectB8Mode_P PARGS0();

CREL_RETURN fnDoReadAndDecodeOneSlice PARGS0();
CREL_RETURN fnDoDecodePictureReadAndDecodeIP PARGS1(int nImgID);

extern void UpdatePTS PARGS0();

#ifdef ANALYZE_CODE_READ_DATA_IN
int get_mb_bits_data(char *buf, char *token, char *tmp_string, int *test_mb);
#endif

/*!
***********************************************************************
* \brief
*    decodes one I- or P-frame
*
***********************************************************************
*/

//int decode_one_frame PARGS0()
//{
//	int header;
//	Slice *currSlice = IMGPAR currentSlice;
//
//	IMGPAR current_slice_nr = 0;
//	IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4711;     // initialized to an impossible value for debugging -- correct value is taken from slice header
//	currSlice->next_header = -8888; // initialized to an impossible value for debugging -- correct value is taken from slice header
//	IMGPAR num_dec_mb = 0;
//
//	while ((currSlice->next_header != EOS && currSlice->next_header != SOP))
//	{
//		header = read_new_slice ARGS0();
//		if(header<0)
//			return header;
//		// IoK: check for HD-profile violation due to incorrect nal_unit_type (2-4), ASO or incorrect slice types (SI/SP)
//		// current_header == -1, it means HD-profile violation and the reason is recorded in g_HDProfileFault.
//		else if (header == EOS)
//		{
//			exit_picture ARGS0();
//			return EOS;
//		}
//
//		decode_slice ARGS1(header);
//
//		if (dec_picture->MbaffFrameFlag)
//			MbAffPostProc ARGS2(currSlice->start_mb_nr, IMGPAR current_mb_nr_r-currSlice->start_mb_nr);
//
//		//software deblocking for frame or field
//		DeblockSlice ARGS3( dec_picture, currSlice->start_mb_nr, IMGPAR current_mb_nr_r-currSlice->start_mb_nr );  
//
//		IMGPAR current_slice_nr++;
//	}
//
//	exit_picture ARGS0();
//
//	return (SOP);
//}

#if 0
/*!
************************************************************************
* \brief
*    Convert file read buffer to source picture structure
* \param imgX
*    Pointer to image plane
* \param buf
*    Buffer for file output
* \param size
*    image size in pixel
************************************************************************
*/
void buf2img (imgpel* imgX, unsigned char* buf, int size_x, int size_y, int symbol_size_in_bytes, int stride)
{
	int i,j;

#ifdef USE_BIGENDIAN
	unsigned short tmp16, ui16;
	unsigned long  tmp32, ui32;
#endif

	if (symbol_size_in_bytes> sizeof(imgpel))
	{
		DEBUG_SHOW_ERROR_INFO ("[ERROR]Source picture has higher bit depth than imgpel data type. Please recompile with larger data type for imgpel.", 500);
	}

	if (( sizeof(char) == sizeof (imgpel)) && ( sizeof(char) == symbol_size_in_bytes))
	{
		// imgpel == pixel_in_file == 1 byte -> simple copy
		for(j=0;j<size_y;j++)
			memcpy(imgX+j*stride, buf+j*size_x, size_x);
	}
	else
	{
		// sizeof (imgpel) > sizeof(char)
#ifdef USE_BIGENDIAN
		// big endian
		switch (symbol_size_in_bytes)
		{
		case 1:
			{
				for(j=0;j<size_y;j++)
					for(i=0;i<size_x;i++)
					{
						*(imgX+j*stride+i)= buf[i+j*size_x];
					}
					break;
			}
		case 2:
			{
				for(j=0;j<size_y;j++)
					for(i=0;i<size_x;i++)
					{
						memcpy(&tmp16, buf+((i+j*size_x)*2), 2);
						ui16  = (tmp16 >> 8) | ((tmp16&0xFF)<<8);
						*(imgX+j*stride+i) = (imgpel) ui16;
					}
					break;
			}
		case 4:
			{
				for(j=0;j<size_y;j++)
					for(i=0;i<size_x;i++)
					{
						memcpy(&tmp32, buf+((i+j*size_x)*4), 4);
						ui32  = ((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24);
						*(imgX+j*stride+i) = (imgpel) ui32;
					}
			}
		default:
			{
				DEBUG_SHOW_ERROR_INFO ("[ERROR]reading only from formats of 8, 16 or 32 bit allowed on big endian architecture", 500);
				break;
			}
		}

	}
#else
		// little endian
		for (j=0; j < size_y; j++)
			for (i=0; i < size_x; i++)
			{
				*(imgX+j*stride+i)=0;
				memcpy(imgX+j*stride+i, buf +((i+j*size_x)*symbol_size_in_bytes), symbol_size_in_bytes);
			}

#endif
	}
}
#endif


CREL_RETURN reorder_lists PARGS2(int currSliceType, Slice * currSlice)
{

	if (currSliceType != I_SLICE)
	{
		if (currSlice->ref_pic_list_reordering_flag_l0)
		{
#if !defined(_COLLECT_PIC_)
			reorder_ref_pic_list ARGS8(listX[0], &listXsize[0], 
				IMGPAR num_ref_idx_l0_active - 1, 
				currSlice->remapping_of_pic_nums_idc_l0, 
				currSlice->abs_diff_pic_num_minus1_l0, 
				currSlice->long_term_pic_idx_l0,
				currSlice->abs_diff_view_idx_minus1_l0,
				0);
#else
			reorder_ref_pic_list ARGS8(listX[0], &listXsize[0],
				currSlice->num_ref_idx_l0_active - 1,
				currSlice->remapping_of_pic_nums_idc_l0, 
				currSlice->abs_diff_pic_num_minus1_l0, 
				currSlice->long_term_pic_idx_l0,
				currSlice->abs_diff_view_idx_minus1_l0,
				0);
#endif
		}
#if !defined(_COLLECT_PIC_)
		if (NULL == listX[0][IMGPAR num_ref_idx_l0_active-1])
#else
		if (currSlice->num_ref_idx_l0_active > 0 && NULL == listX[0][currSlice->num_ref_idx_l0_active-1])
#endif
		{
			DEBUG_SHOW_ERROR_INFO("[ERROR]RefPicList0[ num_ref_idx_l0_active_minus1 ] is equal to 'no reference picture', invalid bitstream",500);

			if ( listXsize[0] == 0 ) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1 | CREL_WARNING_H264_ERROR_DPB;
			}
#if !defined(_COLLECT_PIC_)
			for (int i=0; i<(IMGPAR num_ref_idx_l0_active-listXsize[0]); i++)
#else
			for (int i=0; i<(currSlice->num_ref_idx_l0_active-listXsize[0]); i++)
#endif
				listX[0][listXsize[0]+i] = listX[0][listXsize[0]-1];	// Error Concealment
		}
		// that's a definition
#if !defined(_COLLECT_PIC_)
		listXsize[0] = IMGPAR num_ref_idx_l0_active;
#else
		listXsize[0] = currSlice->num_ref_idx_l0_active;
#endif
	}
	if (currSliceType == B_SLICE)
	{
		if (currSlice->ref_pic_list_reordering_flag_l1)
		{
#if !defined(_COLLECT_PIC_)
			reorder_ref_pic_list ARGS8(listX[1], &listXsize[1], 
				IMGPAR num_ref_idx_l1_active - 1, 
				currSlice->remapping_of_pic_nums_idc_l1, 
				currSlice->abs_diff_pic_num_minus1_l1, 
				currSlice->long_term_pic_idx_l1,
				currSlice->abs_diff_view_idx_minus1_l1,
				1);
#else
			reorder_ref_pic_list ARGS8(listX[1], &listXsize[1], 
				currSlice->num_ref_idx_l1_active - 1, 
				currSlice->remapping_of_pic_nums_idc_l1, 
				currSlice->abs_diff_pic_num_minus1_l1, 
				currSlice->long_term_pic_idx_l1,
				currSlice->abs_diff_view_idx_minus1_l1,
				1);
#endif
		}
#if !defined(_COLLECT_PIC_)
		if (NULL == listX[1][IMGPAR num_ref_idx_l1_active-1])
#else
		if (currSlice->num_ref_idx_l1_active > 0 && NULL == listX[1][currSlice->num_ref_idx_l1_active-1])
#endif
		{
			DEBUG_SHOW_ERROR_INFO("[ERROR]RefPicList1[ num_ref_idx_l1_active_minus1 ] is equal to 'no reference picture', invalid bitstream",500);

			if ( listXsize[1] == 0 ) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1 | CREL_WARNING_H264_ERROR_DPB;
			}

#if !defined(_COLLECT_PIC_)
			for (int i=0; i<(IMGPAR num_ref_idx_l1_active-listXsize[1]); i++)
#else
			for (int i=0; i<(currSlice->num_ref_idx_l1_active-listXsize[1]); i++)
#endif
				listX[1][listXsize[1]+i] = listX[1][listXsize[1]-1]; // Error Concealment
		}
		// that's a definition
#if !defined(_COLLECT_PIC_)
		listXsize[1] = IMGPAR num_ref_idx_l1_active;
#else
		listXsize[1] = currSlice->num_ref_idx_l1_active;
#endif
	}  

	return CREL_OK;
}

CREL_RETURN check_lists PARGS0(){

	StorablePicture *list_temp_forward = NULL;
	StorablePicture *list_temp_backward = NULL;
	/*
	#if !defined(_COLLECT_PIC_)
	int dpb_used_size = dpb.used_size;
	#else
	int dpb_used_size = IMGPAR stream_global->m_dpb.used_size;
	#endif
	*/

	if (listX[0][0] && listX[0][0]->imgY && listX[0][0]->imgUV){
		list_temp_forward = listX[0][0];
	}

	if (listX[1][0] && listX[1][0]->imgY && listX[1][0]->imgUV){
		list_temp_backward = listX[1][0];
	}

	if (list_temp_forward == NULL && listXsize[0]>0) {
		if (list_temp_backward) {
			list_temp_forward = list_temp_backward;
		} else {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1 | CREL_WARNING_H264_ERROR_DPB;
		}
	}

	if (list_temp_backward == NULL && listXsize[1]>0) {
		if (list_temp_forward) {
			list_temp_backward = list_temp_forward;
		} else {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1 | CREL_WARNING_H264_ERROR_DPB;
		}
	}

	for (int i = 0; i < listXsize[0]; i++) {
		if ( listX[0][i] == NULL || listX[0][i]->imgY == NULL || listX[0][i]->imgUV == NULL) {
			listX[0][i]		= list_temp_forward;
		}
	}

	for (int i = 0; i < listXsize[1]; i++) {
		if ( listX[1][i] == NULL || listX[1][i]->imgY == NULL || listX[1][i]->imgUV == NULL) {
			listX[1][i]		= list_temp_backward;
		}
	}

	return CREL_OK;


};

#ifdef DO_REF_PIC_NUM
/*!
************************************************************************
* \brief
*    initialize ref_pic_num array
************************************************************************
*/
void set_ref_pic_num()
{
	int i,j;

	int slice_id=IMGPAR current_slice_nr;

	for (i=0;i<listXsize[LIST_0];i++)
	{
		dec_picture->ref_pic_num        [slice_id][LIST_0][i]=listX[LIST_0][i]->poc * 2 + ((listX[LIST_0][i]->structure==BOTTOM_FIELD)?1:0) ; 
		dec_picture->frm_ref_pic_num    [slice_id][LIST_0][i]=listX[LIST_0][i]->frame_poc * 2; 
		dec_picture->top_ref_pic_num    [slice_id][LIST_0][i]=listX[LIST_0][i]->top_poc * 2; 
		dec_picture->bottom_ref_pic_num [slice_id][LIST_0][i]=listX[LIST_0][i]->bottom_poc * 2 + 1; 
		DEBUG_INFO("POCS %d %d %d %d ",listX[LIST_0][i]->frame_poc,listX[LIST_0][i]->bottom_poc,listX[LIST_0][i]->top_poc,listX[LIST_0][i]->poc);
		DEBUG_INFO("refid %d %d %d %d\n",(int) dec_picture->frm_ref_pic_num[LIST_0][i],(int) dec_picture->top_ref_pic_num[LIST_0][i],(int) dec_picture->bottom_ref_pic_num[LIST_0][i],(int) dec_picture->ref_pic_num[LIST_0][i]);
	}

	for (i=0;i<listXsize[LIST_1];i++)
	{
		dec_picture->ref_pic_num        [slice_id][LIST_1][i]=listX[LIST_1][i]->poc  *2 + ((listX[LIST_1][i]->structure==BOTTOM_FIELD)?1:0);
		dec_picture->frm_ref_pic_num    [slice_id][LIST_1][i]=listX[LIST_1][i]->frame_poc * 2; 
		dec_picture->top_ref_pic_num    [slice_id][LIST_1][i]=listX[LIST_1][i]->top_poc * 2; 
		dec_picture->bottom_ref_pic_num [slice_id][LIST_1][i]=listX[LIST_1][i]->bottom_poc * 2 + 1; 
	}

	if (!active_sps.frame_mbs_only_flag)
	{
		if (IMGPAR structure==FRAME)
			for (j=2;j<6;j++)
				for (i=0;i<listXsize[j];i++)
				{
					dec_picture->ref_pic_num        [slice_id][j][i] = listX[j][i]->poc * 2 + ((listX[j][i]->structure==BOTTOM_FIELD)?1:0);
					dec_picture->frm_ref_pic_num    [slice_id][j][i] = listX[j][i]->frame_poc * 2 ;
					dec_picture->top_ref_pic_num    [slice_id][j][i] = listX[j][i]->top_poc * 2 ;
					dec_picture->bottom_ref_pic_num [slice_id][j][i] = listX[j][i]->bottom_poc * 2 + 1;
				}
	}

}
#endif /* DO_REF_PIC_NUM */

#ifdef _COLLECT_PIC_
void Check_MultiThreadModel PARGS0 ()
{
	stream_par * stream_global = IMGPAR stream_global;

	stream_global->m_bMultiThreadModeSwitch = 0;

	if (stream_global->m_is_MTMS < 0) {
		if ((IMGPAR firstSlice->structure==0 && IMGPAR slice_number >= 4) || (IMGPAR firstSlice->structure!=0 && IMGPAR slice_number >= 8)) {
			stream_global->m_is_MTMS = 1;
			if ( (g_pfnDecodePicture == DecodePicture_MultiThread_SingleSlice_IPRD_Seperate) || (g_pfnDecodePicture == DecodePicture_MultiThread_SingleSlice_IPRD_Merge) ){
				g_pfnDecodePicture = DecodePicture_MultiThread_MultiSlice;
				stream_global->m_bMultiThreadModeSwitch = 1;
			}
		} else {
			stream_global->m_is_MTMS = 0;
			if (g_pfnDecodePicture == DecodePicture_MultiThread_MultiSlice) {
#ifdef IP_RD_MERGE	
				g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Merge;
#else
				g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Seperate;
#endif
				stream_global->m_bMultiThreadModeSwitch = 2;
			}
		}
	} else {

		if (stream_global->m_is_MTMS == 1 || stream_global->m_bIsSingleThreadMode == TRUE) {

			if ( ((IMGPAR firstSlice->structure==0 && IMGPAR slice_number < 4) || (IMGPAR firstSlice->structure!=0 && IMGPAR slice_number < 8)) 
				&& (IMGPAR firstSlice->picture_type == I_SLICE) ){

#ifdef IP_RD_MERGE	
					g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Merge;
#else
					g_pfnDecodePicture = DecodePicture_MultiThread_SingleSlice_IPRD_Seperate;
#endif
					stream_global->m_bMultiThreadModeSwitch = 2;

					stream_global->is_first = 1;
					//stream_global->m_is_MTMS = 0;

			}

		} else {

			if ( ((IMGPAR firstSlice->structure==0 && IMGPAR slice_number >= 4) || (IMGPAR firstSlice->structure!=0 && IMGPAR slice_number >= 8)) 
				&& (IMGPAR firstSlice->picture_type == I_SLICE) ){

					g_pfnDecodePicture = DecodePicture_MultiThread_MultiSlice;			

					stream_global->m_bMultiThreadModeSwitch = 1;

					//stream_global->m_is_MTMS = 1;

			}
		}
	}	
}

CREL_RETURN read_new_picture PARGS1(int* header)
{
	int AU_HasSPS = 0;
	int primary_pic_type = -1;
	CREL_RETURN ret;
	DecodingEnvironment *dep;	
	int new_picture = 0;
	int new_pps     = 0;
	Slice *newSlice;		
	StreamParameters *stream_global = IMGPAR stream_global;
	int pos;
	static BOOL bSeekIDR_backup;
	BOOL bFirstSlice;
	//static int count = 0;		//Debugging

	nalu->buf = IMGPAR ori_nalu_buf;

	img->slice_number = 0;	//Error Resillience by Haihua

	while (1)
	{
		if(nalu_global_available == 0)
		{
			DEBUG_SHOW_SW_INFO("Read new bitstream");
			if (IMGPAR FileFormat == PAR_OF_ANNEXB)
				ret=GetAnnexbNALU ARGS2(nalu, &pos);
#ifdef _SUPPORT_RTP_
			else
				ret=GetRTPNALU (nalu);
#endif

			//In some cases, zero_byte shall be present. If current NALU is a VCL NALU, we can't tell
			//whether it is the first VCL NALU at this point, so only non-VCL NAL unit is checked here.
			CheckZeroByteNonVCL ARGS1(nalu);

			//NALUtoRBSP(nalu);
			DEBUG_SHOW_SW_INFO_DETAIL("nalu->len %d\n", nalu->len);

			if (FAILED(ret))
			{
#ifdef _SUPPORT_RTP_
				DEBUG_SHOW_ERROR_INFO ("[ERROR]Error while getting the NALU in file format %s, exit\n", IMGPAR FileFormat==PAR_OF_ANNEXB?"Annex B":"RTP");
#else
				DEBUG_SHOW_ERROR_INFO ("[ERROR]Error while getting the NALU in file format %s, exit\n", "Annex B");
#endif
				return ret;
			}

			if (pos == 0)
			{
				DEBUG_SHOW_SW_INFO("-- EOS --");
				DEBUG_SHOW_SW_INFO_DETAIL("read_new_slice: returning %s\n", "EOS");
				*header = EOS;
				if (nalu_global_available == 1)
				{
					DEBUG_SHOW_SW_INFO("Already existed bitstream when received EOS!!");
					next_image_type = IMGPAR firstSlice->picture_type;
					nalu_global_available = 0;
				}
				if (IMGPAR slice_number > 0)
					IMGPAR currentSlice->exit_flag = 1;
				return CREL_OK;
			}

			// Got a NALU
			if (nalu->forbidden_bit)
			{
				DEBUG_SHOW_SW_INFO("Found NALU w/ forbidden_bit set, bit error?  Let's try...\n");
				g_bReceivedFirst = 0;
			}

		}
		else
		{
			DEBUG_SHOW_SW_INFO("Already has bitstream");
			nalu->startcodeprefix_len	= nalu_global->startcodeprefix_len;
			nalu->len					= nalu_global->len;
			nalu->max_size				= nalu_global->max_size;
			nalu->nal_unit_type			= nalu_global->nal_unit_type;
			nalu->nal_reference_idc		= nalu_global->nal_reference_idc;
			nalu->forbidden_bit			= nalu_global->forbidden_bit;
			nalu->pts					= nalu_global->pts;
			memcpy(nalu->buf, nalu_global->buf, nalu_global->len);

			nalu_global_available = 0;			
		}

		
		switch (nalu->nal_unit_type)
		{
		case NALU_TYPE_SLICE:
		case NALU_TYPE_IDR:
			DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SLICE -- NALU_TYPE_IDR --");

			ret = malloc_new_slice(&newSlice);

			if (FAILED(ret)) {
				return ret;
			}

			bFirstSlice = (IMGPAR slice_number == 0) ? TRUE:FALSE;

			/*
			count++;

			if ( count > 1000 ) {
			count = count;
			}
			*/

			// Frame management: This is the part we have to reset pic_combine_status
			//IMGPAR idr_flag = (nalu->nal_unit_type == NALU_TYPE_IDR);
			if(nalu->nal_unit_type == NALU_TYPE_IDR)
			{ 
				DEBUG_SHOW_SW_INFO("-- This is IDR --");
				IMGPAR idr_flag = newSlice->idr_flag = 1;
				stream_global->m_bSeekIDR = FALSE;
				
			} else {
				IMGPAR idr_flag = newSlice->idr_flag = 0;
			}
			

			IMGPAR nal_reference_idc = newSlice->nal_reference_idc = nalu->nal_reference_idc;
			IMGPAR disposable_flag = newSlice->disposable_flag = (nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE);

			newSlice->dp_mode = PAR_DP_1;
			newSlice->max_part_nr = 1;
			newSlice->ei_flag = 0;
			newSlice->nextSlice = NULL;
			dep              = newSlice->g_dep;
			dep->Dei_flag    = 0;
			dep->Dbits_to_go = 0;
			dep->Dbuffer     = 0;						
			dep->Dbasestrm = &(nalu->buf[1]);
			dep->Dstrmlength = nalu->len-1;
			ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
			if (FAILED(ret)) {				

				free_new_slice(newSlice);
				newSlice = NULL;				

				break;				
			}
			dep->Dcodestrm   = dep->Dbasestrm;

			if(bFirstSlice){				
				IMGPAR prevSlice  = NULL;
				IMGPAR firstSlice = newSlice;
				IMGPAR currentSlice  = IMGPAR firstSlice;
			} else {
				IMGPAR prevSlice  = IMGPAR currentSlice;
				IMGPAR currentSlice->nextSlice = (void*)newSlice;
				IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
			}

			if (bFirstSlice) {
				bSeekIDR_backup = stream_global->m_bSeekIDR;
			}

			ret = ParseSliceHeader ARGS0();

			if ( FAILED(ret) ) {

				if (bSeekIDR_backup) {
					free_new_slice(newSlice);
					newSlice = NULL;		

					stream_global->m_bSeekIDR = TRUE;

					nalu->buf = IMGPAR ori_nalu_buf;					
					IMGPAR slice_number = 0;
					new_picture = 0;
					break;

				}

				if ( ISWARNING(ret) ) {

					free_new_slice(newSlice);
					newSlice = NULL;					

					if (stream_global->m_bSeekIDR) {
						nalu->buf = IMGPAR ori_nalu_buf;					
						IMGPAR slice_number = 0;
						new_picture = 0;

					} else if (nalu->nal_unit_type == NALU_TYPE_IDR) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
						new_picture = 0;

					}									

				} 
				break;

			}

			//this picture type is for CheckSkipFrame().
			IMGPAR currentSlice->AU_type = IMGPAR currentSlice->picture_type;			
			if(AU_HasSPS && IMGPAR currentSlice->AU_type==I_SLICE)
				IMGPAR currentSlice->AU_type = I_GOP;			
			if(IMGPAR currentSlice->idr_flag)
				IMGPAR currentSlice->AU_type = IDR_SLICE;
			if(IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->nal_reference_idc )
				IMGPAR currentSlice->AU_type = RB_SLICE;

			//Yolk Chuang
			if ((stream_global->profile_idc == 0) || 
				((g_bReceivedFirst == 0) && (IMGPAR firstSlice->picture_type != I_SLICE))
				)
			{
				DEBUG_SHOW_SW_INFO("ERROR! First frame is not intra after reset decoder!!");
				DEBUG_SHOW_SW_INFO("profile_idc: %d  number: %d  type: %d  ref_idx: %d", stream_global->profile_idc, stream_global->number, IMGPAR type, IMGPAR num_ref_idx_l0_active);
				nalu->buf = IMGPAR ori_nalu_buf;

				free_new_slice(newSlice);

				newSlice = NULL;
				new_picture = 0;
				IMGPAR slice_number = 0;

				//Yolk: Reset this value!
				for ( unsigned int i = 0; i < stream_global->num_views; i++) {
					stream_global->m_active_sps_on_view[i]= 0;
				}

				break;
			}
#if 0
			else if ((g_bReceivedFirst == 0) && 
				(IMGPAR currentSlice->picture_type != I_SLICE) && 
				(IMGPAR num_ref_idx_l0_active > 1) && 
				(IMGPAR firstSlice->idr_flag==0)
				)
			{
				DEBUG_SHOW_SW_INFO("ERROR! OpenGOP!!\n profile_idc: %d  number: %d  type: %d  ref_idx: %d", stream_global->profile_idc, stream_global->number, IMGPAR type, IMGPAR num_ref_idx_l0_active);

				nalu->buf = IMGPAR ori_nalu_buf;

				IMGPAR currentSlice = IMGPAR firstSlice;
				for (int i=0; i<IMGPAR slice_number; i++)
				{
					IMGPAR prevSlice = IMGPAR currentSlice;
					IMGPAR currentSlice = (Slice*)IMGPAR currentSlice->nextSlice;
					free_new_slice(IMGPAR prevSlice);
				}
				free_new_slice(IMGPAR currentSlice);

				newSlice = NULL;
				new_picture = 0;
				IMGPAR slice_number = 0;
				break;
			}
#endif

			if(is_new_picture ARGS0())
			{
				seq_parameter_set_rbsp_t *sps;
				pic_parameter_set_rbsp_t *pps;

				DEBUG_SHOW_SW_INFO("This is new Picture");
				ret = decode_poc ARGS0();
				if (FAILED(ret)) {

					free_new_slice(newSlice);
					newSlice = NULL;

					break;
				}

				DEBUG_SHOW_SW_INFO( "This Picture POC: %d", IMGPAR ThisPOC);

				IMGPAR currentSlice->framerate1000 = g_framerate1000;

				if (IMGPAR prevSlice)
					IMGPAR prevSlice->exit_flag = 1;

				//Picture or Field
				if(IMGPAR currentSlice->field_pic_flag && new_picture != 2)
				{   //Field Picture

					if( (IMGPAR prevSlice!= NULL && IMGPAR prevSlice->field_pic_flag)
						&&					
						( (IMGPAR currentSlice->structure==BOTTOM_FIELD && IMGPAR prevSlice!=NULL && 
						IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==TOP_FIELD)
						||
						(IMGPAR currentSlice->structure==TOP_FIELD && IMGPAR prevSlice!=NULL && 
						IMGPAR prevSlice->frame_num == IMGPAR currentSlice->frame_num && IMGPAR prevSlice->m_pic_combine_status==BOTTOM_FIELD) )
						)
					{
						//store these parameters to next collect_pic
						stream_global->PreviousFrameNum[0]		= IMGPAR PreviousFrameNum;
						stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
						stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
						stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
						stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
						stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;
						//Combine Two Filed
						IMGPAR currentSlice->header = SOP;
						IMGPAR currentSlice->m_pic_combine_status = 0; //Picture is Full
						IMGPAR slice_number++;
						new_picture = 2;
						nalu->buf += nalu->len;

						g_bReceivedFirst = 1;

						sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
							&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
						pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
						ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
						if (FAILED(ret)) {

							if (ISWARNING(ret)) {

								if (ISWARNINGLEVEL_1(ret)) {
									stream_global->m_bSeekIDR = TRUE;
									nalu->buf = IMGPAR ori_nalu_buf;

									new_picture = 0;
									IMGPAR slice_number = 0;
									//Yolk: Reset this value!									
									for ( unsigned int i = 0; i < stream_global->num_views; i++) {
										stream_global->m_active_sps_on_view[i]= 0;
									}
								}								

								free_new_slice(newSlice);
								newSlice = NULL;

								break;

							} else {
								return ret;
							}
						}
						activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

					}
					else if (IMGPAR slice_number == 0)				
					{
						//First Filed of New Picture
						UpdatePTS ARGS0();

						ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
						SetH264CCCode ARGS0();

						IMGPAR currentSlice->header = SOP;
						IMGPAR currentSlice->m_pic_combine_status = IMGPAR currentSlice->structure; //first field
						IMGPAR slice_number++;
						new_picture = 1;
						nalu->buf += nalu->len;

						sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
							&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
						pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
						ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
						if (FAILED(ret)) {

							if (ISWARNING(ret)) {

								if (ISWARNINGLEVEL_1(ret)) {
									stream_global->m_bSeekIDR = TRUE;
									nalu->buf = IMGPAR ori_nalu_buf;
									new_picture = 0;
									IMGPAR slice_number = 0;									
									for ( unsigned int i = 0; i < stream_global->num_views; i++) {
										stream_global->m_active_sps_on_view[i]= 0;
									}
								}								

								free_new_slice(newSlice);
								newSlice = NULL;

								break;

							} else {
								return ret;
							}
						}
						activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

						IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;
					}
					else
					{
						//Cpoy naul to nalu_global
						nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
						nalu_global->len					= nalu->len;
						nalu_global->max_size				= nalu->max_size;
						nalu_global->nal_unit_type			= nalu->nal_unit_type;
						nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
						nalu_global->forbidden_bit			= nalu->forbidden_bit;
						nalu_global->pts					= nalu->pts;
						memcpy(nalu_global->buf, nalu->buf, nalu->len);
						nalu_global_available = 1;

						IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
						IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
						IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
						IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
						IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
						IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

						next_image_type = IMGPAR type;

						IMGPAR currentSlice = IMGPAR prevSlice;
						IMGPAR currentSlice->nextSlice = NULL;

						free_new_slice(newSlice);
						newSlice = NULL;

						//Check_MultiThreadModel ARGS0 ();
						*header = SOP;

						return CREL_OK;
					}
				}
				else 
				{   //Frame Picture
					if(new_picture != 0)
					{					
						//Cpoy naul to nalu_global
						nalu_global->startcodeprefix_len	= nalu->startcodeprefix_len;
						nalu_global->len					= nalu->len;
						nalu_global->max_size				= nalu->max_size;
						nalu_global->nal_unit_type			= nalu->nal_unit_type;
						nalu_global->nal_reference_idc		= nalu->nal_reference_idc;
						nalu_global->forbidden_bit			= nalu->forbidden_bit;
						nalu_global->pts					= nalu->pts;
						memcpy(nalu_global->buf, nalu->buf, nalu->len);
						nalu_global_available = 1;

						IMGPAR PreviousFrameNum				= stream_global->PreviousFrameNum[0];
						IMGPAR PreviousFrameNumOffset = stream_global->PreviousFrameNumOffset[0];
						IMGPAR PreviousPOC						= stream_global->PreviousPOC[0];
						IMGPAR ThisPOC								= stream_global->ThisPOC[0]; 
						IMGPAR PrevPicOrderCntLsb			= stream_global->PrevPicOrderCntLsb[0];
						IMGPAR PrevPicOrderCntMsb			= stream_global->PrevPicOrderCntMsb[0];

						next_image_type = IMGPAR type;

						IMGPAR currentSlice = IMGPAR prevSlice;
						IMGPAR currentSlice->nextSlice = NULL;

						free_new_slice(newSlice);
						newSlice = NULL;

						//Check_MultiThreadModel ARGS0 ();
						*header = SOP;

						return CREL_OK;
					}
					else
					{
						UpdatePTS ARGS0();

						ZeroMemory(&(IMGPAR m_CCCode), sizeof(H264_CC));
						SetH264CCCode ARGS0();

						IMGPAR currentSlice->header = SOP;
						IMGPAR currentSlice->m_pic_combine_status = FRAME;
						IMGPAR slice_number++;
						new_picture = 1;
						nalu->buf += nalu->len;

						sps =  (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
							&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
						pps =  &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
						ret = activate_sps ARGS2(sps, IMGPAR currentSlice->viewId);
						if (FAILED(ret)) {

							if (ISWARNING(ret)) {

								if (ISWARNINGLEVEL_1(ret)) {
									stream_global->m_bSeekIDR = TRUE;
									nalu->buf = IMGPAR ori_nalu_buf;
									new_picture = 0;
									IMGPAR slice_number = 0;
									for ( unsigned int i = 0; i < stream_global->num_views; i++) {
										stream_global->m_active_sps_on_view[i]= 0;
									}
								}							

								free_new_slice(newSlice);
								newSlice = NULL;

								break;

							} else {
								return ret;
							}
						}
						activate_pps ARGS2(pps, IMGPAR currentSlice->viewIndex);

						stream_global->PreviousFrameNum[0]				= IMGPAR PreviousFrameNum;
						stream_global->PreviousFrameNumOffset[0]	= IMGPAR PreviousFrameNumOffset;
						stream_global->PreviousPOC[0]						= IMGPAR PreviousPOC;
						stream_global->ThisPOC[0]								= IMGPAR ThisPOC; 
						stream_global->PrevPicOrderCntLsb[0]			= IMGPAR PrevPicOrderCntLsb;
						stream_global->PrevPicOrderCntMsb[0]			= IMGPAR PrevPicOrderCntMsb;

						IMGPAR currentSlice->m_nDispPicStructure = sps->vui_seq_parameters.picture_structure;

						g_bReceivedFirst = 1;
					}
				}												
			}
			else
			{   //Slice
				IMGPAR currentSlice->header = *header = SOS;				
				IMGPAR currentSlice->m_pic_combine_status = IMGPAR prevSlice->m_pic_combine_status;
				IMGPAR slice_number++;

				if (IMGPAR prevSlice)
				{
					IMGPAR prevSlice->exit_flag = 0;
					IMGPAR currentSlice->pts = IMGPAR prevSlice->pts;
					IMGPAR currentSlice->dts = IMGPAR prevSlice->dts;
					IMGPAR currentSlice->has_pts = IMGPAR prevSlice->has_pts;
					IMGPAR currentSlice->framerate1000 = IMGPAR prevSlice->framerate1000;
					IMGPAR currentSlice->NumClockTs = IMGPAR prevSlice->NumClockTs;
				}
				nalu->buf += nalu->len;

				IMGPAR currentSlice->m_nDispPicStructure = IMGPAR prevSlice->m_nDispPicStructure;
			}		
			break;
		case NALU_TYPE_DPA:
			return CREL_ERROR_H264_NOTSUPPORTED;
			break;
		case NALU_TYPE_DPC:			
			return CREL_ERROR_H264_NOTSUPPORTED;
			break;
		case NALU_TYPE_SEI:
			DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SEI --");
			DEBUG_SHOW_SW_INFO_DETAIL("read_new_slice: Found NALU_TYPE_SEI, len %d\n", nalu->len);
			/*
			if ( count >= 992 ) {
			count = count;
			}
			*/
			ret = InterpretSEIMessage ARGS2(nalu->buf,nalu->len);
			//Terry: For PIP's sub bitstream, it may has no SPS before pasing SEI nalu during enable fast forward (FF) function.
			if (FAILED(ret)) {

				if (!ISWARNINGLEVEL_3(ret) ) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
					for ( unsigned int i = 0; i < stream_global->num_views; i++) {
						stream_global->m_active_sps_on_view[i]= 0;
					}
				}				
			}			
			break;
		case NALU_TYPE_PPS:
			DEBUG_SHOW_SW_INFO("-- NALU_TYPE_PPS --");
			//if(new_pps == 1)
			//{
			//	//Cpoy naul to nalu_global				
			//	nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
			//	nalu_global->len = nalu->len;
			//	nalu_global->max_size = nalu->max_size;
			//	nalu_global->nal_unit_type = nalu->nal_unit_type;
			//	nalu_global->nal_reference_idc = nalu->nal_reference_idc;
			//	nalu_global->forbidden_bit= nalu->forbidden_bit;
			//	memcpy(nalu_global->buf, nalu->buf, nalu->len);
			//	nalu_global_available = 1;
			//return SOP;
			//}
			//else
			//   new_pps = 1;

			ret = ProcessPPS ARGS1(nalu);

			if (FAILED(ret)) {				

				if (!ISWARNINGLEVEL_3(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
				}			
				
			}			
			break;
		case NALU_TYPE_SPS:
			DEBUG_SHOW_SW_INFO("-- NALU_TYPE_SPS --");
			AU_HasSPS = 1;

			//If bitstream is IDR only and IDR are all the same, decoder needs to return here.
			if (IMGPAR slice_number > 0 && IMGPAR firstSlice->idr_flag)
			{
				nalu_global->startcodeprefix_len = nalu->startcodeprefix_len;
				nalu_global->len                 = nalu->len;
				nalu_global->max_size            = nalu->max_size;
				nalu_global->nal_unit_type       = nalu->nal_unit_type;
				nalu_global->nal_reference_idc   = nalu->nal_reference_idc;
				nalu_global->forbidden_bit       = nalu->forbidden_bit;
				nalu_global->pts                 = nalu->pts;
				memcpy(nalu_global->buf, nalu->buf, nalu->len);
				nalu_global_available = 1;

				IMGPAR currentSlice->exit_flag = 1;

				*header = NALU_TYPE_SPS;

				return CREL_OK;
			}

			ret = ProcessSPS ARGS1(nalu);
			if (FAILED(ret)) {

				if (!ISWARNINGLEVEL_3(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
				}
			}		
			break;
		case NALU_TYPE_AUD:
			ret = ProcessAUD ARGS2(nalu, &primary_pic_type);			
			break;
		case NALU_TYPE_EOSEQ:
			DEBUG_SHOW_SW_INFO("-- NALU_TYPE_EndOfSequence --");
			DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found 'End of Sequence' NAL unit, len %d\n", nalu->len);

			*header = EOS;
			if (nalu_global_available == 1)
			{
				DEBUG_SHOW_SW_INFO("Already existed bitstream when received EOS!!");
				next_image_type = IMGPAR firstSlice->picture_type;
				nalu_global_available = 0;
			}
			if (IMGPAR slice_number > 0)
				IMGPAR currentSlice->exit_flag = 1;
			return CREL_OK;
			break;
		case NALU_TYPE_EOSTREAM:
			DEBUG_SHOW_SW_INFO("-- NALU_TYPE_EndOfStream --");
			DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found 'End of Stream' NAL unit, len %d\n", nalu->len);

			*header = EOS;
			if (nalu_global_available == 1)
			{
				DEBUG_SHOW_SW_INFO("Already existed bitstream when received EOS!!");
				next_image_type = IMGPAR firstSlice->picture_type;
				nalu_global_available = 0;
			}
			if (IMGPAR slice_number > 0)
				IMGPAR currentSlice->exit_flag = 1;
			return CREL_OK;
			break;
		case NALU_TYPE_FILL:
			DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found NALU_TYPE_FILL, len %d\n", nalu->len);
			DEBUG_SHOW_SW_INFO_DETAIL ("Skipping these filling bits, proceeding w/ next NALU\n");
			break;
		default:
			DEBUG_SHOW_SW_INFO_DETAIL("-- NALU_TYPE_default --");
			DEBUG_SHOW_SW_INFO_DETAIL ("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", nalu->nal_unit_type, nalu->len);
		}
	}
	//FreeNALU(nalu);

	return  CREL_OK;
}

void CopySliceParameter PARGS0()
{	
	Slice *currSlice = IMGPAR currentSlice;
	const int dP_nr = 0;	
	StreamParameters *stream_global = IMGPAR stream_global;

	//IMGPAR g_dep = currSlice->g_dep;
	IMGPAR g_dep.Dei_flag    = currSlice->g_dep->Dei_flag;
	IMGPAR g_dep.Dbits_to_go = currSlice->g_dep->Dbits_to_go;
	IMGPAR g_dep.Dbuffer     = currSlice->g_dep->Dbuffer;
	//memcpy(IMGPAR g_dep.Dbasestrm, currSlice->g_dep->Dbasestrm, currSlice->g_dep->Dstrmlength);
	IMGPAR g_dep.Dbasestrm = currSlice->g_dep->Dbasestrm;
	IMGPAR g_dep.Dstrmlength = currSlice->g_dep->Dstrmlength;
	IMGPAR g_dep.Dcodestrm   = currSlice->g_dep->Dcodestrm;

	IMGPAR idr_flag = currSlice->idr_flag;
	IMGPAR nal_reference_idc = currSlice->nal_reference_idc;
	IMGPAR disposable_flag = currSlice->disposable_flag;
	IMGPAR type = currSlice->picture_type;
	//IMGPAR pps_id = currSlice->pic_parameter_set_id;
	IMGPAR Transform8x8Mode = currSlice->Transform8x8Mode;
	IMGPAR frame_num = currSlice->frame_num;
	//if (currSlice->idr_flag)
	//{
	IMGPAR pre_frame_num = currSlice->pre_frame_num;
	//}
	IMGPAR structure = currSlice->structure;
	IMGPAR field_pic_flag = currSlice->field_pic_flag;
	IMGPAR bottom_field_flag = currSlice->bottom_field_flag;		
	IMGPAR MbaffFrameFlag= currSlice->MbaffFrameFlag;
	IMGPAR idr_pic_id = currSlice->idr_pic_id;
	IMGPAR pic_order_cnt_lsb = currSlice->pic_order_cnt_lsb;
	IMGPAR delta_pic_order_cnt_bottom = currSlice->delta_pic_order_cnt_bottom;
	IMGPAR delta_pic_order_cnt_bottom = currSlice->delta_pic_order_cnt_bottom;	
	IMGPAR delta_pic_order_cnt[ 0 ] = currSlice->delta_pic_order_cnt[0];
	IMGPAR delta_pic_order_cnt[ 1 ] = currSlice->delta_pic_order_cnt[1];
	IMGPAR direct_spatial_mv_pred_flag = currSlice->direct_spatial_mv_pred_flag;
	IMGPAR num_ref_idx_l0_active = currSlice->num_ref_idx_l0_active;
	IMGPAR num_ref_idx_l1_active = currSlice->num_ref_idx_l1_active;
	*static_cast<H264_TS *>(&streampts) = currSlice->pts;
	IMGPAR framerate1000 = currSlice->framerate1000;

	//ref_pic_list_reordering ARGS0();

	IMGPAR apply_weights = currSlice->apply_weights;

	if (currSlice->header == SOP)
	{
		if(IMGPAR structure==BOTTOM_FIELD && prev_dec_picture!=NULL && 
			(int)prev_dec_picture->frame_num == IMGPAR frame_num && pic_combine_status==TOP_FIELD)
			pic_combine_status = 0;	// Frame is full

		else if(IMGPAR structure==TOP_FIELD && prev_dec_picture!=NULL && 
			(int)prev_dec_picture->frame_num == IMGPAR frame_num && pic_combine_status==BOTTOM_FIELD)
			pic_combine_status = 0;

		else
			pic_combine_status = IMGPAR structure;
	}

#if defined(_HW_ACCEL_)
	if(g_DXVAVer == IviNotDxva || IMGPAR Hybrid_Decoding == 5)
#endif
	{
		if(active_pps.num_slice_groups_minus1 == 0)
		{
			IMGPAR FP_decode_one_macroblock_I = decode_one_macroblock_I;
			IMGPAR FP_decode_one_macroblock_P_Intra = decode_one_macroblock_I;
			IMGPAR FP_decode_one_macroblock_B_Intra = decode_one_macroblock_I;
			if (IMGPAR apply_weights)
			{
				IMGPAR FP_decode_one_macroblock_P = decode_one_macroblock_P_1;
				IMGPAR FP_decode_one_macroblock_B = decode_one_macroblock_B_1;
			}
			else
			{
				IMGPAR FP_decode_one_macroblock_P = decode_one_macroblock_P_0;
				IMGPAR FP_decode_one_macroblock_B = decode_one_macroblock_B_0;
			}
		}
		else
		{
			IMGPAR FP_decode_one_macroblock_I = decode_one_macroblock_I_FMO;
			IMGPAR FP_decode_one_macroblock_P_Intra = decode_one_macroblock_I_FMO;
			IMGPAR FP_decode_one_macroblock_B_Intra = decode_one_macroblock_I;

			if (IMGPAR apply_weights)
			{
				IMGPAR FP_decode_one_macroblock_P = decode_one_macroblock_P_1_FMO;
				IMGPAR FP_decode_one_macroblock_B = decode_one_macroblock_B_1;
			}
			else
			{
				IMGPAR FP_decode_one_macroblock_P = decode_one_macroblock_P_0_FMO;
				IMGPAR FP_decode_one_macroblock_B = decode_one_macroblock_B_0;
			}
		}
		IMGPAR FP_StoreImgRowToImgPic = StoreImgRowToImgPic;
		IMGPAR FP_TransferData_at_SliceEnd = TransferData_at_SliceEnd;
	}
#if defined(_HW_ACCEL_)
	else		//GPU acceleration
	{
		if(IMGPAR Hybrid_Decoding == 0 )
		{
			IMGPAR FP_decode_one_macroblock_I = HW_decode_one_macroblock_Intra;
			IMGPAR FP_decode_one_macroblock_P_Intra = HW_decode_one_macroblock_Intra;
			IMGPAR FP_decode_one_macroblock_B_Intra = HW_decode_one_macroblock_Intra;

			IMGPAR FP_decode_one_macroblock_P = HW_decode_one_macroblock_Inter;
			IMGPAR FP_decode_one_macroblock_B = HW_decode_one_macroblock_Inter;

		}
		else if (IMGPAR Hybrid_Decoding == 1)
		{
			IMGPAR FP_decode_one_macroblock_I = decode_one_macroblock_I;
			IMGPAR FP_decode_one_macroblock_P_Intra = HW_decode_one_macroblock_Intra;
			IMGPAR FP_decode_one_macroblock_B_Intra = HW_decode_one_macroblock_Intra;

			IMGPAR FP_decode_one_macroblock_P = HW_decode_one_macroblock_Inter;
			IMGPAR FP_decode_one_macroblock_B = HW_decode_one_macroblock_Inter;
		}
		else
		{
			IMGPAR FP_decode_one_macroblock_I = decode_one_macroblock_I;
			IMGPAR FP_decode_one_macroblock_P_Intra = decode_one_macroblock_I;
			IMGPAR FP_decode_one_macroblock_B_Intra = HW_decode_one_macroblock_Intra;

			if(IMGPAR apply_weights)
			{
				IMGPAR FP_decode_one_macroblock_P = decode_one_macroblock_P_1;
				IMGPAR FP_decode_one_macroblock_B = HW_decode_one_macroblock_Inter;
			}else{
				IMGPAR FP_decode_one_macroblock_P = decode_one_macroblock_P_0;
				IMGPAR FP_decode_one_macroblock_B = HW_decode_one_macroblock_Inter;
			}
		}

		IMGPAR FP_StoreImgRowToImgPic = HW_StoreImgRowToImgPic;
		IMGPAR FP_TransferData_at_SliceEnd = HW_TransferData_at_SliceEnd;

	}
#endif

	if ((active_pps.weighted_pred_flag&&(IMGPAR type==P_SLICE) ||
		(active_pps.weighted_bipred_idc==1) && (IMGPAR type==B_SLICE)))
	{
		IMGPAR luma_log2_weight_denom = currSlice->luma_log2_weight_denom;
		IMGPAR chroma_log2_weight_denom = currSlice->chroma_log2_weight_denom;
		IMGPAR wp_weight = currSlice->wp_weight;
		IMGPAR wp_offset = currSlice->wp_offset;
		IMGPAR wbp_weight = currSlice->wbp_weight;
		IMGPAR wp_round_luma = currSlice->wp_round_luma;
		IMGPAR wp_round_chroma = currSlice->wp_round_chroma;
	}
	else if (IMGPAR apply_weights)
	{
		IMGPAR wp_weight = currSlice->wp_weight;
		IMGPAR wp_offset = currSlice->wp_offset;
		IMGPAR wbp_weight = currSlice->wbp_weight;
	}

	/*if ((active_pps.weighted_pred_flag&&(IMGPAR type==P_SLICE) ||
	(active_pps.weighted_bipred_idc==1) && (IMGPAR type==B_SLICE)))
	{
	pred_weight_table ARGS0();
	}

	if (IMGPAR nal_reference_idc)
	dec_ref_pic_marking ARGS0();*/

	/*if (active_pps.entropy_coding_mode_flag && IMGPAR type!=I_SLICE)
	{
	IMGPAR model_number = ue_v ("SH: cabac_init_idc");
	}
	else 
	{
	IMGPAR model_number = 0;
	}*/

	//val = se_v ("SH: slice_qp_delta");
	IMGPAR qp = currSlice->qp;// = IMGPAR qp = 26 + active_pps.pic_init_qp_minus26 + val;


	//currSlice->slice_qp_delta = val;

	/*if (active_pps.deblocking_filter_control_present_flag)
	{
	currSlice->LFDisableIdc = ue_v ("SH: disable_deblocking_filter_idc");

	if (currSlice->LFDisableIdc!=1)
	{
	currSlice->LFAlphaC0Offset = 2 * se_v ("SH: slice_alpha_c0_offset_div2");
	currSlice->LFBetaOffset = 2 * se_v ("SH: slice_beta_offset_div2");
	}
	else
	{
	currSlice->LFAlphaC0Offset = currSlice->LFBetaOffset = 0;
	}
	}
	else 
	{
	currSlice->LFDisableIdc = currSlice->LFAlphaC0Offset = currSlice->LFBetaOffset = 0;
	}*/

	/*if (active_pps.num_slice_groups_minus1>0 && active_pps.slice_group_map_type>=3 &&
	active_pps.slice_group_map_type<=5)
	{
	len = (active_sps.pic_height_in_map_units_minus1+1)*(active_sps.pic_width_in_mbs_minus1+1)/ 
	(active_pps.slice_group_change_rate_minus1+1);
	if (((active_sps.pic_height_in_map_units_minus1+1)*(active_sps.pic_width_in_mbs_minus1+1))% 
	(active_pps.slice_group_change_rate_minus1+1))
	len +=1;

	len = CeilLog2(len+1);

	IMGPAR slice_group_change_cycle = u_v (len, "SH: slice_group_change_cycle");
	}*/

	IMGPAR PicHeightInMbs = IMGPAR FrameHeightInMbs / ( 1 + IMGPAR field_pic_flag );
	IMGPAR PicSizeInMbs   = IMGPAR PicWidthInMbs * IMGPAR PicHeightInMbs;
	IMGPAR FrameSizeInMbs = IMGPAR PicWidthInMbs * IMGPAR FrameHeightInMbs;

	//reset_read_functions ARGS0(); 

	// currSlice->dp_mode is set by read_new_slice (NALU first byte available there)
	if (active_pps.entropy_coding_mode_flag == UVLC)
		nal_startcode_follows = uvlc_startcode_follows;
	else
		nal_startcode_follows = cabac_startcode_follows;
}
#endif

#if !defined (_COLLECT_PIC_)
/*!
************************************************************************
* \brief
*    Reads new slice from bit_stream
************************************************************************
*/
int read_new_slice PARGS0()
{
	//NALU_t *nalu = AllocNALU(MAX_CODED_FRAME_SIZE);
	int header;
	int ret;
	int BitsUsedByHeader;
	Slice *currSlice = IMGPAR currentSlice;
	DecodingEnvironment *dep;
	int slice_id_a, slice_id_b, slice_id_c;
	int redundant_pic_cnt_b, redundant_pic_cnt_c;
	int pos;

#ifdef _COLLECT_PIC_
	StreamParameters *stream_global = IMGPAR stream_global;
#endif

	while (1)
	{
		if (IMGPAR FileFormat == PAR_OF_ANNEXB)
			ret=GetAnnexbNALU ARGS2(nalu, &pos);
#ifdef _SUPPORT_RTP_
		else
			ret=GetRTPNALU (nalu);
#endif
		// IoK: This is constrained to be different than 2,3,4 for HD-DVD streams
		if(nalu->nal_unit_type==NALU_TYPE_DPA || nalu->nal_unit_type==NALU_TYPE_DPB || nalu->nal_unit_type==NALU_TYPE_DPC)
		{
			//g_HDProfileFault = HD_ProfileFault_NALU_nal_unit_type;
			return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
		}

		//In some cases, zero_byte shall be present. If current NALU is a VCL NALU, we can't tell
		//whether it is the first VCL NALU at this point, so only non-VCL NAL unit is checked here.
		CheckZeroByteNonVCL ARGS1(nalu);

		//NALUtoRBSP(nalu);
		DEBUG_SHOW_SW_INFO_DETAIL("nalu->len %d\n", nalu->len);

		if (ret < 0)
			DEBUG_SHOW_ERROR_INFO("Error while getting the NALU in file format, exit\n");

		if (ret == 0)
		{
			DEBUG_SHOW_SW_INFO_DETAIL("read_new_slice: returning EOS\n");
			return EOS;
		}

		// Got a NALU
		if (nalu->forbidden_bit)
			DEBUG_SHOW_SW_INFO_DETAIL("Found NALU w/ forbidden_bit set, bit error?  Let's try...\n");

		switch (nalu->nal_unit_type)
		{
		case NALU_TYPE_SLICE:
		case NALU_TYPE_IDR:
			// Frame management: This is the part we have to reset pic_combine_status
			if(nalu->nal_unit_type == NALU_TYPE_IDR)
			{ 
				IMGPAR idr_flag = 1;
			}
			else
			{
				if(IMGPAR smart_dec & SMART_DEC_ONLY_IDR)
				{
					g_dwSkipFrameCounter++;
					break;
				}
				IMGPAR idr_flag = 0;
			}
			IMGPAR nal_reference_idc = nalu->nal_reference_idc;
			IMGPAR disposable_flag = (nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE);
			currSlice->dp_mode = PAR_DP_1;
			currSlice->max_part_nr = 1;
			currSlice->ei_flag = 0;
			dep              = &(IMGPAR g_dep);
			dep->Dei_flag    = 0;
			dep->Dbits_to_go = 0;
			dep->Dbuffer     = 0;
			dep->Dbasestrm   = &nalu->buf[1];
			dep->Dstrmlength = nalu->len-1;
			ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
			if (FAILED(ret)) {
				return ret;
			}
			dep->Dcodestrm   = dep->Dbasestrm;

			// Some syntax of the Slice Header depends on the parameter set, which depends on
			// the parameter set ID of the SLice header.  Hence, read the pic_parameter_set_id
			// of the slice header first, then setup the active parameter sets, and then read
			// the rest of the slice header
			BitsUsedByHeader = FirstPartOfSliceHeader ARGS0();

			// IoK: check for HD-profile violation due to incorrect slice type and/or ASO
			//if(g_HDProfileFault)
			//	return(-1);

			ret = UseParameterSet ARGS1(currSlice->pic_parameter_set_id);
			if (FAILED(ret)) {
				return ret;
			}
			BitsUsedByHeader+= RestOfSliceHeader ARGS0();

			//Terry: we should skip all B-frames between I and P frames after seeking.
			if(IMGPAR SkipThisFrame)
			{
				if(IMGPAR type == B_SLICE)
					break;
				else
					IMGPAR SkipThisFrame = 0;
			}

			if((IMGPAR smart_dec & SMART_DEC_SKIP_1B) && (IMGPAR type==B_SLICE))
			{
				if((!IMGPAR field_pic_flag) || ((dec_picture->slice_type == B_SLICE) && (dec_picture->frame_num == IMGPAR frame_num)))
					g_dwSkipFrameCounter++;
				break;
			}
			else if((IMGPAR smart_dec & SMART_DEC_SKIP_PB) && (IMGPAR type==B_SLICE || IMGPAR type==P_SLICE))
			{
				if((!IMGPAR field_pic_flag) || ((dec_picture->slice_type != I_SLICE) && (dec_picture->frame_num == IMGPAR frame_num)))
					g_dwSkipFrameCounter++;
				break;
			}

			//FmoInit ARGS2(active_pps, active_sps);

			AssignQuantParam ARGS2(&active_pps, &active_sps);

			//Terry: error handle for no SEI frame.
			IMGPAR NumClockTs = (active_sps.vui_seq_parameters.NumClockTs) ? active_sps.vui_seq_parameters.NumClockTs : 1;
			IMGPAR framerate1000 = g_framerate1000;

			IMGPAR SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][1] = 0;

			if(is_new_picture ARGS0())
			{
				if(nalu->pts.ts)
				{
					IMGPAR has_pts = 1;
					*static_cast<H264_TS *>(&streampts) = nalu->pts;
				}
				else
				{
					IMGPAR has_pts = 0;
				}

				ret = init_picture ARGS0();
				if(FAILED(ret)) {
					return ret;
				}
				header = SOP;
				//check zero_byte if it is also the first NAL unit in the access unit
				CheckZeroByteVCL ARGS1(nalu);
				// IoK: for each new picture, reset first_mb_in_slice
				g_prev_start_mb_nr = (-1);
			}
			else
				header = SOS;
			// IoK: start_mb_nr can not be smaller than previously decoded start_mb_nr for HD-profile
			if(currSlice->start_mb_nr < g_prev_start_mb_nr)
			{
				//g_HDProfileFault = HD_ProfileFault_SLICE_first_mb_in_slice;
				return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
			}
			g_prev_start_mb_nr = currSlice->start_mb_nr;

			ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
			if (FAILED(ret)) {
				return ret;
			}
			ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
			if (FAILED(ret)) {
				return ret;
			}

			free_ref_pic_list_reordering_buffer(currSlice);

			if (IMGPAR structure==FRAME)
			{
				init_mbaff_lists ARGS0();
				if (IMGPAR direct_spatial_mv_pred_flag)
					compute_colocated_SUBMB = compute_colocated_SUBMB_frame_spatial;
				else
					compute_colocated_SUBMB = compute_colocated_SUBMB_frame_temporal;
			}
			else
			{
				if (IMGPAR direct_spatial_mv_pred_flag)
					compute_colocated_SUBMB = compute_colocated_SUBMB_field_spatial;
				else
					compute_colocated_SUBMB = compute_colocated_SUBMB_field_temporal;
			}

			// From here on, active_sps, active_pps and the slice header are valid
			if (IMGPAR MbaffFrameFlag)
				IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r = currSlice->start_mb_nr << 1;
			else
				IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r = currSlice->start_mb_nr;

			if (active_pps.entropy_coding_mode_flag)
			{
				// Byte-align
				dep->Dcodestrm -= dep->Dbits_to_go>>3;
				arideco_start_decoding ARGS0();
			}
			DEBUG_SHOW_SW_INFO_DETAIL("read_new_slice: returning %s\n", header == SOP?"SOP":"SOS");
			//FreeNALU(nalu);
			return header;
			break;
		case NALU_TYPE_DPA:
			//! The state machine here should follow the same ideas as the old readSliceRTP()
			//! basically:
			//! work on DPA (as above)
			//! read and process all following SEI/SPS/PPS/PD/Filler NALUs
			//! if next video NALU is dpB, 
			//!   then read and check whether it belongs to DPA, if yes, use it
			//! else
			//!   ;   // nothing
			//! read and process all following SEI/SPS/PPS/PD/Filler NALUs
			//! if next video NALU is dpC
			//!   then read and check whether it belongs to DPA (and DPB, if present), if yes, use it, done
			//! else
			//!   use the DPA (and the DPB if present)

			/* 
			LC: inserting the code related to DP processing, mainly copying some of the parts
			related to NALU_TYPE_SLICE, NALU_TYPE_IDR.
			*/
			IMGPAR idr_flag          = (nalu->nal_unit_type == NALU_TYPE_IDR);
			IMGPAR nal_reference_idc = nalu->nal_reference_idc;
			IMGPAR disposable_flag   = (nalu->nal_reference_idc == NALU_PRIORITY_DISPOSABLE);
			currSlice->dp_mode     = PAR_DP_3;
			currSlice->max_part_nr = 3;
			currSlice->ei_flag     = 0;
			dep                    = &(IMGPAR g_dep);
			dep->Dei_flag          = 0;
			dep->Dbits_to_go       = 0;
			dep->Dbuffer           = 0;
			//memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
			dep->Dbasestrm         = &nalu->buf[1];
			dep->Dstrmlength = nalu->len-1;
			ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
			if (FAILED(ret)) {
				return ret;
			}
			dep->Dcodestrm         = dep->Dbasestrm;

			BitsUsedByHeader     = FirstPartOfSliceHeader ARGS0();

			// IoK: check for HD-profile violation due to incorrect slice type and/or ASO
			//if(g_HDProfileFault)
			//	return(-1);

			ret = UseParameterSet ARGS1(currSlice->pic_parameter_set_id);
			if (FAILED(ret)) {
				return ret;
			}
			BitsUsedByHeader    += RestOfSliceHeader ARGS0();

			//Terry: we should skip all B-frames between I and P frames after seeking.
			if(IMGPAR SkipThisFrame)
			{
				if(IMGPAR type == B_SLICE)
					break;
				else
					IMGPAR SkipThisFrame = 0;
			}

			if((IMGPAR smart_dec & SMART_DEC_SKIP_1B) && (IMGPAR type==B_SLICE))
			{
				if((!IMGPAR field_pic_flag) || ((dec_picture->slice_type == B_SLICE) && (dec_picture->frame_num == IMGPAR frame_num)))
					g_dwSkipFrameCounter++;
				break;
			}
			else if((IMGPAR smart_dec & SMART_DEC_SKIP_PB) && (IMGPAR type==B_SLICE || IMGPAR type==P_SLICE))
			{
				if((!IMGPAR field_pic_flag) || ((dec_picture->slice_type != I_SLICE) && (dec_picture->frame_num == IMGPAR frame_num)))
					g_dwSkipFrameCounter++;
				break;
			}

			//FmoInit ARGS2(active_pps, active_sps);

			if(g_has_pts)
			{
				g_has_pts = 0;
				IMGPAR has_pts = 1;
				*static_cast<H264_TS *>(&streampts) = g_pts;
			}
			else
			{
				IMGPAR has_pts = 0;
			}

			IMGPAR SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][1] = 0;

			if(is_new_picture ARGS0())
			{
				ret = init_picture ARGS0();
				if(FAILED(ret)) {
					return ret;
				}
				header = SOP;
				CheckZeroByteVCL ARGS1(nalu);
				// IoK: for each new picture, reset first_mb_in_slice
				g_prev_start_mb_nr = (-1);
			}
			else
				header = SOS;
			// IoK: start_mb_nr can not be smaller than previously decoded start_mb_nr for HD-profile
			if(currSlice->start_mb_nr < g_prev_start_mb_nr)
			{
				//g_HDProfileFault = HD_ProfileFault_SLICE_first_mb_in_slice;
				return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
			}
			g_prev_start_mb_nr = currSlice->start_mb_nr;

			ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
			if (FAILED(ret)) {
				return ret;
			}
			ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
			if (FAILED(ret)) {
				return ret;
			}
			free_ref_pic_list_reordering_buffer(currSlice);

			if (IMGPAR structure==FRAME)
			{
				init_mbaff_lists ARGS0();
			}

			// From here on, active_sps, active_pps and the slice header are valid
			if (IMGPAR MbaffFrameFlag)
				IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r = currSlice->start_mb_nr << 1;
			else
				IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r = currSlice->start_mb_nr;


			/* 
			LC:
			Now I need to read the slice ID, which depends on the value of 
			redundant_pic_cnt_present_flag (pag.49). 
			*/

			slice_id_a  = ue_v ("NALU:SLICE_A slice_idr");
			if (active_pps.entropy_coding_mode_flag)
			{
				// Byte-align
				dep->Dcodestrm -= dep->Dbits_to_go>>3;
				arideco_start_decoding ARGS0();
			}
			DEBUG_SHOW_SW_INFO_DETAIL("read_new_slice: returning %s\n", header == SOP?"SOP":"SOS");
			break;
		case NALU_TYPE_DPB:
			/* LC: inserting the code related to DP processing */

			dep              = &(IMGPAR g_dep);
			dep->Dei_flag    = 0;
			dep->Dbits_to_go = 0;
			dep->Dbuffer     = 0;
			//memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
			dep->Dbasestrm   = &nalu->buf[1];
			dep->Dstrmlength = nalu->len-1;
			ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
			if (FAILED(ret)) {
				return ret;
			}
			dep->Dcodestrm   = dep->Dbasestrm;

			slice_id_b  = ue_v ("NALU:SLICE_B slice_idr");
			if (active_pps.redundant_pic_cnt_present_flag)
				redundant_pic_cnt_b = ue_v ("NALU:SLICE_B redudand_pic_cnt");
			else
				redundant_pic_cnt_b = 0;

			/*  LC: Initializing CABAC for the current data stream. */

			if (active_pps.entropy_coding_mode_flag)
			{
				// Byte-align
				dep->Dcodestrm -= dep->Dbits_to_go>>3;
				arideco_start_decoding ARGS0();

			}

			/* LC: resilience code to be inserted */
			/*         FreeNALU(nalu); */
			/*         return header; */

			break;
		case NALU_TYPE_DPC:
			/* LC: inserting the code related to DP processing */
			dep              = &(IMGPAR g_dep);
			dep->Dei_flag    = 0;
			dep->Dbits_to_go = 0;
			dep->Dbuffer     = 0;
			//memcpy (currStream->streamBuffer, &nalu->buf[1], nalu->len-1);
			dep->Dbasestrm   = &nalu->buf[1];
			dep->Dstrmlength = nalu->len-1;
			ret = RBSPtoSODB(dep->Dbasestrm, &(dep->Dstrmlength));
			if (FAILED(ret)) {
				return ret;
			}
			dep->Dcodestrm   = dep->Dbasestrm;

			slice_id_c  = ue_v ("NALU:SLICE_C slice_idr");
			if (active_pps.redundant_pic_cnt_present_flag)
				redundant_pic_cnt_c = ue_v ("NALU:SLICE_C redudand_pic_cnt");
			else
				redundant_pic_cnt_c = 0;

			/* LC: Initializing CABAC for the current data stream. */

			if (active_pps.entropy_coding_mode_flag)
			{
				// Byte-align
				dep->Dcodestrm -= dep->Dbits_to_go>>3;
				arideco_start_decoding ARGS0();
			}

			/* LC: resilience code to be inserted */

			//FreeNALU(nalu);
			return header;

			break;
		case NALU_TYPE_SEI:
			DEBUG_SHOW_SW_INFO_DETAIL("read_new_slice: Found NALU_TYPE_SEI, len %d\n", nalu->len);
			ret = InterpretSEIMessage ARGS2(nalu->buf,nalu->len);
			if (FAILED(ret)) {
				return ret;
			}
			break;
		case NALU_TYPE_PPS:
			ret = ProcessPPS ARGS1(nalu);
			if (FAILED(ret)) {
				return ret;
			}			
			break;

		case NALU_TYPE_SPS:
			ret = ProcessSPS ARGS1(nalu);
			if (FAILED(ret)) {
				return ret;
			}			
			break;
		case NALU_TYPE_AUD:
			DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found 'Access Unit Delimiter' NAL unit, len %d, ignored\n", nalu->len);
			break;
		case NALU_TYPE_EOSEQ:
			DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found 'End of Sequence' NAL unit, len %d, ignored\n", nalu->len);
			break;
		case NALU_TYPE_EOSTREAM:
			DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found 'End of Stream' NAL unit, len %d, ignored\n", nalu->len);
			break;
		case NALU_TYPE_FILL:
			DEBUG_SHOW_SW_INFO_DETAIL ("read_new_slice: Found NALU_TYPE_FILL, len %d\n", nalu->len);
			DEBUG_SHOW_SW_INFO_DETAIL ("Skipping these filling bits, proceeding w/ next NALU\n");
			break;
		default:
			DEBUG_SHOW_SW_INFO_DETAIL ("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", nalu->nal_unit_type, nalu->len);
		}
	}
	//FreeNALU(nalu);

	return  header;
}
#endif

/*!
************************************************************************
* \brief
*    Initializes the parameters for a new picture
************************************************************************
*/
CREL_RETURN init_picture PARGS0()
{
	int i, nSeekFlag;
	Slice *currSlice = IMGPAR currentSlice;
	int ret;
	int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

#ifdef  _COLLECT_PIC_
	StreamParameters *stream_global = IMGPAR stream_global;
#endif

	if(IMGPAR idr_flag==1)
	{
		if(!prev_dec_picture)
			pic_combine_status = 0;
		else if(prev_dec_picture->frame_num != IMGPAR frame_num)
			pic_combine_status = 0;
	}

#if !defined(_COLLECT_PIC_)
	if (dec_picture)
	{
		// this may only happen on slice loss
		ret = exit_picture ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	}
#endif

	if (IMGPAR frame_num != IMGPAR pre_frame_num && IMGPAR frame_num != (IMGPAR pre_frame_num + 1) % IMGPAR MaxFrameNum) 
	{
		// the PreviousFrameNum should not be updated in the previous decode_poc function.
		IMGPAR PreviousFrameNum = IMGPAR pre_frame_num;

		if (active_sps.gaps_in_frame_num_value_allowed_flag == 0)
		{
			/* Advanced Error Concealment would be called here to combat unintentional loss of pictures. */
			DEBUG_SHOW_ERROR_INFO("[ERROR]An unintentional loss of pictures occurs! Exit\n", 100);
			//return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}
		// IoK: This commenting-out is necessary to make cases SPEED_2 and SPEED_3 work
		// IoK: Uncommented to fix crash in bitstreams with gap in frame number...
		nSeekFlag = g_framemgr->GetDisplayCount() + dpb.used_size_on_view[view_index];
		// Terry: For seeking case (nSeekFlag==0), the first frame should be I frame.
#if !defined(_COLLECT_PIC_)
		if((nSeekFlag == 0) && (IMGPAR type != I_SLICE))
#else
		if((nSeekFlag == 0) && (IMGPAR firstSlice->picture_type != I_SLICE))
#endif
		{
			DEBUG_SHOW_ERROR_INFO("ERROR: Real Frame Gap!!");

#if defined(_COLLECT_PIC_)
			DEBUG_SHOW_ERROR_INFO("Picture Type: %d  Structure: %d  MBAff: %d  Direct Spital Mode: %d", IMGPAR firstSlice->picture_type, IMGPAR firstSlice->structure,						IMGPAR firstSlice->MbaffFrameFlag, IMGPAR firstSlice->direct_spatial_mv_pred_flag);
#endif
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}
		// Terry: For seeking to non_closed GOP, we should skip all B frames between I and P frames.
		if((nSeekFlag == 0) && (IMGPAR idr_flag == 0))
		{
#ifdef _COLLECT_PIC_
			stream_global->bSeekToOpenGOP = 1;
#else
			IMGPAR SkipThisFrame = 1;
#endif
		}
		// Terry: For seeking case (nSeekFlag==0), we should skip this function.
		if((IMGPAR smart_dec <= SMART_DEC_LEVEL_3 || IMGPAR smart_dec == SMART_DEC_LEVEL_0) && nSeekFlag && IMGPAR dwFillFrameNumGap) {
			ret = fill_frame_num_gap ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
		}
	}
	IMGPAR pre_frame_num = IMGPAR frame_num;
	IMGPAR num_dec_mb = 0;

#ifdef _COLLECT_PIC_
	IMGPAR currentSlice->pre_frame_num = IMGPAR pre_frame_num;
#endif

	//calculate POC
	ret = decode_poc ARGS0();
	if (FAILED(ret)) {
		return ret;
	}
	//  dumppoc ARGS0();

#ifdef DEBUG_SHOW_PROCESS_TIME
	if (IMGPAR structure==FRAME ||IMGPAR structure==TOP_FIELD)
	{

#ifdef WIN32
		_ftime (&(IMGPAR tstruct_start));             // start time ms
#else
		ftime (&(IMGPAR tstruct_start));              // start time ms
#endif
		time( &(IMGPAR ltime_start));                // start time s
	}
#endif

	if(IMGPAR structure==BOTTOM_FIELD && prev_dec_picture!=NULL && 
		(int)prev_dec_picture->frame_num == IMGPAR frame_num && pic_combine_status==TOP_FIELD)
	{
		dec_picture = get_storable_picture ARGS7((PictureStructure)IMGPAR structure, IMGPAR width, IMGPAR height, IMGPAR width_cr, IMGPAR height_cr, 0, 0);
		if (dec_picture == NULL) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}
#ifdef _HW_ACCEL_
		if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
#else
		if(1)
#endif
		{
			dec_picture->imgY        = prev_dec_picture->imgY + (prev_dec_picture->Y_stride>>1);
			dec_picture->imgUV       = prev_dec_picture->imgUV + (prev_dec_picture->UV_stride>>1);
			dec_picture->pDownSampleY	= prev_dec_picture->pDownSampleY;
			dec_picture->pDownSampleUV	= prev_dec_picture->pDownSampleUV;
		}
		pic_combine_status = 0;	// Frame is full
#if !defined (_COLLECT_PIC_)
		if( (IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) && (IMGPAR type == B_SLICE) )
		{
			for (i=0; i<prev_dec_picture->size_y; i++)
				memcpy(dec_picture->imgY+(i*prev_dec_picture->Y_stride), prev_dec_picture->imgY+(i*prev_dec_picture->Y_stride), prev_dec_picture->Y_stride>>1);
			for (i=0; i<prev_dec_picture->size_y_cr; i++)
			{
				memcpy(dec_picture->imgUV+(i*prev_dec_picture->UV_stride), prev_dec_picture->imgUV+(i*prev_dec_picture->UV_stride), prev_dec_picture->UV_stride>>1);
			}
		}
#endif

#if defined (_COLLECT_PIC_)		
		IMGPAR m_dec_picture_bottom = dec_picture;	
		currSlice->m_mb_nr_offset = IMGPAR PicSizeInMbs;
		IMGPAR cof_array_bottom = IMGPAR cof_array_ori + (IMGPAR PicSizeInMbs*384);
		IMGPAR mb_decdata_bottom = IMGPAR mb_decdata_ori + IMGPAR PicSizeInMbs;

		/*currSlice->toppoc = IMGPAR toppoc;
		currSlice->bottompoc = IMGPAR bottompoc;
		currSlice->framepoc = IMGPAR framepoc;*/
		currSlice->PreviousPOC = IMGPAR PreviousPOC;
		currSlice->ThisPOC = IMGPAR ThisPOC;
#endif
	}
	else if(IMGPAR structure==TOP_FIELD && prev_dec_picture!=NULL && 
		(int)prev_dec_picture->frame_num == IMGPAR frame_num && pic_combine_status==BOTTOM_FIELD)
	{
		dec_picture =get_storable_picture ARGS7((PictureStructure)IMGPAR structure, IMGPAR width, IMGPAR height, IMGPAR width_cr, IMGPAR height_cr, 0, 0);
		if (dec_picture == NULL) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}
#ifdef _HW_ACCEL_
		if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
#else
		if(1)
#endif
		{
			dec_picture->imgY        = prev_dec_picture->imgY - (prev_dec_picture->Y_stride>>1);
			dec_picture->imgUV       = prev_dec_picture->imgUV - (prev_dec_picture->UV_stride>>1);
			dec_picture->pDownSampleY	= prev_dec_picture->pDownSampleY;
			dec_picture->pDownSampleUV	= prev_dec_picture->pDownSampleUV;
		}
		pic_combine_status = 0;

#ifdef _COLLECT_PIC_	
		IMGPAR m_dec_picture_top = dec_picture;
		currSlice->m_mb_nr_offset = IMGPAR PicSizeInMbs;

		//IMGPAR cof_array_top = IMGPAR cof_array + (currSlice->m_mb_nr_offset*384);
		IMGPAR cof_array_top = IMGPAR cof_array_ori;	//By Haihua
		IMGPAR mb_decdata_top = IMGPAR mb_decdata_ori + IMGPAR PicSizeInMbs;

		/*currSlice->toppoc = IMGPAR toppoc;
		currSlice->bottompoc = IMGPAR bottompoc;
		currSlice->framepoc = IMGPAR framepoc;*/
		currSlice->PreviousPOC = IMGPAR PreviousPOC;
		currSlice->ThisPOC = IMGPAR ThisPOC;
#endif

	}
	else
	{


#if defined(_HW_ACCEL_)
		if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
			dec_picture =get_storable_picture ARGS7((PictureStructure)IMGPAR structure, IMGPAR width, IMGPAR height, IMGPAR width_cr, IMGPAR height_cr, 2, 0);
		else
			dec_picture =get_storable_picture ARGS7((PictureStructure)IMGPAR structure, IMGPAR width, IMGPAR height, IMGPAR width_cr, IMGPAR height_cr, 0, 0);
#else
		dec_picture =get_storable_picture ARGS7((PictureStructure)IMGPAR structure, IMGPAR width, IMGPAR height, IMGPAR width_cr, IMGPAR height_cr, 2, 0);
#endif
		if (dec_picture == NULL) {
			return CREL_WARNING_H264_STREAMERROR_LEVEL_1|CREL_WARNING_H264_ERROR_DPB;
		}

		pic_combine_status = IMGPAR structure;

#if defined (_COLLECT_PIC_)
		currSlice->m_mb_nr_offset = 0;
		if (IMGPAR structure == TOP_FIELD)
		{
			IMGPAR m_dec_picture_top = dec_picture;
			//IMGPAR cof_array_top = IMGPAR cof_array;
			IMGPAR cof_array_top = IMGPAR cof_array_ori;	//Bye Haihua
			IMGPAR mb_decdata_top = IMGPAR mb_decdata_ori;

			/*currSlice->toppoc = IMGPAR toppoc;
			currSlice->bottompoc = IMGPAR bottompoc;
			currSlice->framepoc = IMGPAR framepoc;*/
			currSlice->PreviousPOC = IMGPAR PreviousPOC;
			currSlice->ThisPOC = IMGPAR ThisPOC;
			prev_dec_picture = dec_picture;
		}
		else
			if (IMGPAR structure == BOTTOM_FIELD)
			{
				IMGPAR m_dec_picture_bottom = dec_picture;		
				//IMGPAR cof_array_bottom = IMGPAR cof_array;
				IMGPAR cof_array_bottom = IMGPAR cof_array_ori + IMGPAR PicSizeInMbs * 384; //Bye Haihua
				IMGPAR mb_decdata_bottom = IMGPAR mb_decdata_ori + currSlice->m_mb_nr_offset;//IMGPAR mb_decdata_bottom;

				/*currSlice->toppoc = IMGPAR toppoc;
				currSlice->bottompoc = IMGPAR bottompoc;
				currSlice->framepoc = IMGPAR framepoc;*/
				currSlice->PreviousPOC = IMGPAR PreviousPOC;
				currSlice->ThisPOC = IMGPAR ThisPOC;
				prev_dec_picture = dec_picture;
			}
#endif
	}
/*
	if (currSlice->ThisPOC == 10690) {
		img->stream_global->number = img->stream_global->number;
	}
*/
	if(stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode == TRUE)
		memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));
	dec_picture->top_poc=IMGPAR toppoc;
	dec_picture->bottom_poc=IMGPAR bottompoc;
	dec_picture->frame_poc=IMGPAR framepoc;
	dec_picture->qp=IMGPAR qp;
	dec_picture->slice_qp_delta=currSlice->slice_qp_delta;
	dec_picture->chroma_qp_offset[0] = active_pps.chroma_qp_index_offset;
	dec_picture->chroma_qp_offset[1] = active_pps.second_chroma_qp_index_offset;

	switch (IMGPAR structure )
	{
	case TOP_FIELD:
		{
			dec_picture->poc=IMGPAR toppoc;
			IMGPAR number *= 2;
			break;
		}
	case BOTTOM_FIELD:
		{
			dec_picture->poc=IMGPAR bottompoc;
			IMGPAR number++;
			break;
		}
	case FRAME:
		{
			dec_picture->poc=IMGPAR framepoc;
			break;
		}
	default:
		{
			//DEBUG_SHOW_SW_INFO("IMGPAR structure not initialized", 235);
			//return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;

		}
		break;

	}

	IMGPAR current_slice_nr=0;

	//assert(IMGPAR type < SP_SLICE);
	if (IMGPAR type >= SP_SLICE) {
		return CREL_ERROR_H264_NOTSUPPORTED;
	}

	// CAVLC init
	if(active_pps.entropy_coding_mode_flag == UVLC)
		memset(IMGPAR nz_coeff, 0xFF, (int)IMGPAR PicSizeInMbs*sizeof(IMGPAR nz_coeff[0]));
	/*
	if(active_pps.constrained_intra_pred_flag)
	{
	for (i=0; i<(int)IMGPAR PicSizeInMbs; i++)
	{
	IMGPAR mb_decdata[i].intra_block = 1;
	}		
	}
	*/

	// Set the slice_nr member of each MB to -1, to ensure correct when packet loss occurs
	// TO set Macroblock Map (mark all MBs as 'have to be concealed')
	if (stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode == TRUE)
	{
		for(i=0; i<(int)IMGPAR PicSizeInMbs; i++)
		{
			dec_picture->mb_data[i].slice_nr = -1; 
			IMGPAR mb_decdata[i].ei_flag = 1;
		}
	}

	IMGPAR mb_y_r = IMGPAR mb_x_r = IMGPAR mb_left_x_r_FMO = 0;
	IMGPAR mb_y_d = IMGPAR mb_x_d = 0;
	//IMGPAR block_y_r = IMGPAR pix_y_r = IMGPAR pix_c_y_r = 0; // define vertical positions
	//IMGPAR block_x_r = IMGPAR pix_x_r = IMGPAR pix_c_x_r = 0; // define horizontal positions
	IMGPAR block_y_d = IMGPAR pix_y_d = IMGPAR pix_c_y_d = 0; // define vertical positions
	IMGPAR block_x_d = IMGPAR pix_x_d = IMGPAR pix_c_x_d = 0; // define horizontal positions

	dec_picture->slice_type = IMGPAR type;
	dec_picture->used_for_reference = (IMGPAR nal_reference_idc != 0);
	dec_picture->idr_flag = IMGPAR idr_flag;
	dec_picture->no_output_of_prior_pics_flag = IMGPAR no_output_of_prior_pics_flag;
	dec_picture->long_term_reference_flag = IMGPAR long_term_reference_flag;
	dec_picture->adaptive_ref_pic_buffering_flag = IMGPAR adaptive_ref_pic_buffering_flag;

	dec_picture->dec_ref_pic_marking_buffer = IMGPAR dec_ref_pic_marking_buffer;
	IMGPAR dec_ref_pic_marking_buffer = NULL;

	dec_picture->MbaffFrameFlag = IMGPAR MbaffFrameFlag;
	dec_picture->PicWidthInMbs = IMGPAR PicWidthInMbs;
	dec_picture->pts = *static_cast<H264_TS *>(&streampts);
	dec_picture->framerate1000 = IMGPAR framerate1000;
	dec_picture->has_pts = IMGPAR has_pts;
	dec_picture->NumClockTs = IMGPAR NumClockTs;
	dec_picture->SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][0];
	dec_picture->SkipedBFrames[view_index][1] = IMGPAR SkipedBFrames[view_index][1];

#ifdef __MB_POS_TABLE_LOOKUP__
	if(pic_width_in_mbs!=dec_picture->PicWidthInMbs || mb_pos_table_size<dec_picture->PicSizeInMbs)
	{
#if 0
		int i, x=0, y=0, j;
		mb_pos_table_size = dec_picture->PicSizeInMbs;
		pic_width_in_mbs = dec_picture->PicWidthInMbs;
		for(i=j=0;i<mb_pos_table_size;i++,j++)
		{
			if(j==dec_picture->PicWidthInMbs)
			{
				x = j = 0;
				y++;
			}
			mb_pos_table[i].x = x;
			mb_pos_table[i].y = y;
			x++;
		}
#else
		int i, x, y;
		mb_pos_table_size = dec_picture->PicSizeInMbs;
		pic_width_in_mbs = dec_picture->PicWidthInMbs;
		x = y = 0;
		for (i=0;i<mb_pos_table_size;i++)
		{
			if (x == pic_width_in_mbs)
			{
				x = 0;
				y++;
			}
			mb_pos_table[i].x = x;
			mb_pos_table[i].y = y;
			x++;
		}
#endif
	}
#endif

	dec_picture->pic_num = IMGPAR frame_num;
	dec_picture->frame_num = IMGPAR frame_num;
	dec_picture->coded_frame = (IMGPAR structure==FRAME);

	dec_picture->chroma_format_idc = active_sps.chroma_format_idc;

	dec_picture->frame_mbs_only_flag = active_sps.frame_mbs_only_flag;
	dec_picture->frame_cropping_flag = active_sps.frame_cropping_flag;

	if (dec_picture->frame_cropping_flag)
	{
		dec_picture->frame_cropping_rect_left_offset   = active_sps.frame_cropping_rect_left_offset;
		dec_picture->frame_cropping_rect_right_offset  = active_sps.frame_cropping_rect_right_offset;
		dec_picture->frame_cropping_rect_top_offset    = active_sps.frame_cropping_rect_top_offset;
		dec_picture->frame_cropping_rect_bottom_offset = active_sps.frame_cropping_rect_bottom_offset;
	}

	//get display aspect ratio 
	if (active_sps.Valid && active_sps.vui_seq_parameters.aspect_ratio_info_present_flag)
	{
		dec_picture->nAspectRatio = active_sps.vui_seq_parameters.aspect_ratio_idc;//Table E-1

		if(dec_picture->nAspectRatio == 1)
		{
			if((IMGPAR width == 1280 && IMGPAR height == 720) || (IMGPAR width == 1920 && IMGPAR height == 1088))
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
			else if(IMGPAR width == 640 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
		}
		else if(dec_picture->nAspectRatio == 2)
		{
			if((IMGPAR width == 720 && IMGPAR height == 576) || (IMGPAR width == 352 && IMGPAR height == 288))
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
		}
		else if(dec_picture->nAspectRatio == 3)
		{
			if((IMGPAR width == 720 && IMGPAR height == 480) || (IMGPAR width == 352 && IMGPAR height == 240))
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
		}
		else if(dec_picture->nAspectRatio == 4)
		{
			if(IMGPAR width == 720 && IMGPAR height == 576)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
			else if(IMGPAR width == 540 && IMGPAR height == 576)
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
		}
		else if(dec_picture->nAspectRatio == 5)
		{
			if(IMGPAR width == 720 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
			else if(IMGPAR width == 540 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
		}
		else if(dec_picture->nAspectRatio == 6)
		{
			if(IMGPAR width == 352 && IMGPAR height == 576)
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
			else if(IMGPAR width == 540 && IMGPAR height == 576)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 7)
		{
			if(IMGPAR width == 352 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
			else if(IMGPAR width == 480 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 8)
		{
			if(IMGPAR width == 352 && IMGPAR height == 576)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 9)
		{
			if(IMGPAR width == 352 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 10)
		{
			if(IMGPAR width == 480 && IMGPAR height == 576)
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
		}
		else if(dec_picture->nAspectRatio == 11)
		{
			if(IMGPAR width == 480 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect =  4;
				dec_picture->dwYAspect =  3;
			}
		}
		else if(dec_picture->nAspectRatio == 12)
		{
			if(IMGPAR width == 540 && IMGPAR height == 576)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 13)
		{
			if(IMGPAR width == 540 && IMGPAR height == 480)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 14)
		{
			if(IMGPAR width == 1440 && IMGPAR height == 1088)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 15)
		{
			if(IMGPAR width == 1280 && IMGPAR height == 1088)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 16)
		{
			if(IMGPAR width == 960 && IMGPAR height == 1088)
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect =  9;
			}
		}
		else if(dec_picture->nAspectRatio == 255)
		{
			int nX, nY;
			float fAspect;
			nX = active_sps.vui_seq_parameters.sar_width * IMGPAR width;
			nY = active_sps.vui_seq_parameters.sar_height * IMGPAR height;
			fAspect = (float)nX / (float)nY;
			if (fAspect < 1.5)
			{
				dec_picture->dwXAspect = 4;
				dec_picture->dwYAspect = 3;
			}
			else
			{
				dec_picture->dwXAspect = 16;
				dec_picture->dwYAspect = 9;
			}
		}
	}//aspect ratio

	//BitRate Annex E.2.2
	if(active_sps.Valid && active_sps.vui_seq_parameters.nal_hrd_parameters_present_flag)
	{
		dec_picture->dwBitRate = ((active_sps.vui_seq_parameters.nal_hrd_parameters.bit_rate_value_minus1[0]+1)<<(6+active_sps.vui_seq_parameters.nal_hrd_parameters.bit_rate_scale));
	}
	else if(active_sps.Valid && active_sps.vui_seq_parameters.vcl_hrd_parameters_present_flag)
	{
		dec_picture->dwBitRate = ((active_sps.vui_seq_parameters.vcl_hrd_parameters.bit_rate_value_minus1[0]+1)<<(6+active_sps.vui_seq_parameters.vcl_hrd_parameters.bit_rate_scale));
	}
	else
		dec_picture->dwBitRate = 0;

	//interlace/progressive
	//dec_picture->progressive_frame = (!active_sps.mb_adaptive_frame_field_flag);
	dec_picture->progressive_frame = ((IMGPAR structure==FRAME) && !(IMGPAR MbaffFrameFlag));
	dec_picture->pull_down_flag = ( active_sps.vui_parameters_present_flag?(currSlice->m_nDispPicStructure>2):0 );

	return CREL_OK;
}

// Boundary extension stuff
//#ifdef H264_ENABLE_INTRINSICS
void pad_boundary_luma_sse PARGS4(imgpel *ptr, int width, int height, int stride)
{
	int j;
	unsigned int val1,val2;

		__m64 mm0,mm1;

		for(j=height;j>0;j--)
		{
			val1 = ptr[0];
			val2 = ptr[width-1];
			mm0 = _mm_cvtsi32_si64(val1);
			mm1 = _mm_cvtsi32_si64(val2);
			mm0 = _mm_unpacklo_pi8(mm0, mm0);
			mm1 = _mm_unpacklo_pi8(mm1, mm1); //Interleaves the lower 8 signed or unsigned 8-bit integers in a with the lower 8 signed or unsigned 8-bit integers in b.
			mm0 = _mm_shuffle_pi16(mm0, 0);  //Shuffles the lower 4 signed or unsigned 16-bit integers in a as specified by imm.
			mm1 = _mm_shuffle_pi16(mm1, 0);
			//mm0 = _mm_shuffle_epi32(xmm0, 0); //Shuffles the 4 signed or unsigned 32-bit integers in a as specified by imm.
			//mm1 = _mm_shuffle_epi32(xmm1, 0);
			*(__m64 *) (ptr-32) = mm0;
			*(__m64 *) (ptr-24) = mm0;
			*(__m64 *) (ptr-16) = mm0;
			*(__m64 *) (ptr-8) = mm0;
			*(__m64 *) (ptr+width) = mm1;
			*(__m64 *) (ptr+width+8) = mm1;
			*(__m64 *) (ptr+width+16) = mm1;
			*(__m64 *) (ptr+width+24) = mm1;
			ptr += stride;
		}
	
}
void pad_boundary_luma_sse2 PARGS4(imgpel *ptr, int width, int height, int stride)
{
	int j;
	unsigned int val1,val2;

		__m128i xmm0,xmm1;

		for(j=height;j>0;j--)
		{
			val1 = ptr[0];
			val2 = ptr[width-1];
			xmm0 = _mm_cvtsi32_si128(val1);
			xmm1 = _mm_cvtsi32_si128(val2);
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm0);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm1); //Interleaves the lower 8 signed or unsigned 8-bit integers in a with the lower 8 signed or unsigned 8-bit integers in b.
			xmm0 = _mm_shufflelo_epi16(xmm0, 0);  //Shuffles the lower 4 signed or unsigned 16-bit integers in a as specified by imm.
			xmm1 = _mm_shufflelo_epi16(xmm1, 0);
			xmm0 = _mm_shuffle_epi32(xmm0, 0); //Shuffles the 4 signed or unsigned 32-bit integers in a as specified by imm.
			xmm1 = _mm_shuffle_epi32(xmm1, 0);
			*(__m128i *) (ptr-32) = xmm0;
			*(__m128i *) (ptr-16) = xmm0;
			*(__m128i *) (ptr+width) = xmm1;
			*(__m128i *) (ptr+width+16) = xmm1;
			ptr += stride;
		}
	
}
//#else
void pad_boundary_luma_c PARGS4(imgpel *ptr, int width, int height, int stride)
{
	int j;
	unsigned int val1,val2;

	for(j=height;j>0;j--)
	{
		val1 = ptr[0];
		val2 = ptr[width-1];
		val1 = val1 | (val1<<8);
		val1 = val1 | (val1<<16);
		val2 = val2 | (val2<<8);
		val2 = val2 | (val2<<16);
		*(int *) &ptr[-32] = val1;
		*(int *) &ptr[-28] = val1;
		*(int *) &ptr[-24] = val1;
		*(int *) &ptr[-20] = val1;
		*(int *) &ptr[-16] = val1;
		*(int *) &ptr[-12] = val1;
		*(int *) &ptr[-8 ] = val1;
		*(int *) &ptr[-4 ] = val1;

		*(int *) &ptr[width+0 ] = val2;
		*(int *) &ptr[width+4 ] = val2;
		*(int *) &ptr[width+8 ] = val2;
		*(int *) &ptr[width+12] = val2;
		*(int *) &ptr[width+16] = val2;
		*(int *) &ptr[width+20] = val2;
		*(int *) &ptr[width+24] = val2;
		*(int *) &ptr[width+28] = val2;

		ptr += stride;
	}
}
//#endif

//#ifdef H264_ENABLE_INTRINSICS
void pad_boundary_chroma_sse PARGS4(imgpel *ptr, int width, int height, int stride)
{
	int j;
	unsigned int val1,val2;

		__m64 mm0,mm1;

		width<<=1;

		for(j=height;j>0;j--)
		{
			val1 = *((unsigned short*)&ptr[0]);
			val2 = *((unsigned short*)&ptr[width-2]);
			mm0 = _mm_cvtsi32_si64(val1); //Moves 32-bit integer a to the least significant 32 bits of an __m128 object, zero extending the upper bits.
			mm1 = _mm_cvtsi32_si64(val2);
			mm0 = _mm_shuffle_pi16(mm0, 0); //Shuffles the lower 4 signed or unsigned 16-bit integers in a as specified by imm.
			mm1 = _mm_shuffle_pi16(mm1, 0);
			*((__m64*)(ptr-32)) = mm0;
			*((__m64*)(ptr-24)) = mm0;
			*((__m64*)(ptr-16)) = mm0;
			*((__m64*)(ptr-8)) = mm0;
			*((__m64*)(ptr+width)) = mm1;
			*((__m64*)(ptr+width+8)) = mm1;
			*((__m64*)(ptr+width+16)) = mm1;
			*((__m64*)(ptr+width+24)) = mm1;
			ptr += stride;
		}
	
}

void pad_boundary_chroma_sse2 PARGS4(imgpel *ptr, int width, int height, int stride)
{
	int j;
	unsigned int val1,val2;

		__m128i xmm0,xmm1;

		width<<=1;

		for(j=height;j>0;j--)
		{
			val1 = *((unsigned short*)&ptr[0]);
			val2 = *((unsigned short*)&ptr[width-2]);
			xmm0 = _mm_cvtsi32_si128(val1); //Moves 32-bit integer a to the least significant 32 bits of an __m128 object, zero extending the upper bits.
			xmm1 = _mm_cvtsi32_si128(val2);
			xmm0 = _mm_shufflelo_epi16(xmm0, 0); //Shuffles the lower 4 signed or unsigned 16-bit integers in a as specified by imm.
			xmm1 = _mm_shufflelo_epi16(xmm1, 0);
			_mm_storel_epi64((__m128i *) (ptr-32), xmm0);  //Reads the lower 64 bits of b and stores them into the lower 64 bits of a.
			_mm_storel_epi64((__m128i *) (ptr-24), xmm0);
			_mm_storel_epi64((__m128i *) (ptr-16), xmm0);
			_mm_storel_epi64((__m128i *) (ptr-8), xmm0);
			_mm_storel_epi64((__m128i *) (ptr+width), xmm1);
			_mm_storel_epi64((__m128i *) (ptr+width+8), xmm1);
			_mm_storel_epi64((__m128i *) (ptr+width+16), xmm1);
			_mm_storel_epi64((__m128i *) (ptr+width+24), xmm1);
			ptr += stride;
		}
	
}
//#else
void pad_boundary_chroma_c PARGS4(imgpel *ptr, int width, int height, int stride)
{
	int j;
	unsigned int val1,val2;
	unsigned int val3,val4;

	width<<=1;

	for(j=height;j>0;j--)
	{
		val1 = ptr[0];
	    val3 = ptr[1];
		val2 = ptr[width-1];
		val4 = ptr[width-2];
		
		val1 = val1 | (val3<<8);
		val1 = val1 | (val1<<16);
		val2 = val4 | (val2<<8);
		val2 = val2 | (val2<<16);
		*(int *) &ptr[-16] = val1;
		*(int *) &ptr[-12] = val1;
		*(int *) &ptr[-8 ] = val1;
		*(int *) &ptr[-4 ] = val1;
		*(int *) &ptr[width+0 ] = val2;
		*(int *) &ptr[width+4 ] = val2;
		*(int *) &ptr[width+8 ] = val2;
		*(int *) &ptr[width+12] = val2;
		ptr += stride;
	}
}
//#endif
/*!
************************************************************************
* \brief
*    finish decoding of a picture, conceal errors and store it 
*    into the DPB
************************************************************************
*/
CREL_RETURN exit_picture PARGS0()
{
	int structure, frame_poc, slice_type, refpic, qp, pic_num, chroma_format_idc;
	int ret;

	int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	DEBUG_SHOW_SW_INFO_DETAIL("--Start Exit This Picture--");

	/*
	if ( ( img->framepoc == 90 ) ) {
	img->framepoc = img->framepoc;
	}
	*/
	if (dec_picture == NULL) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
	}

#if defined (_HW_ACCEL_)
	if(imgDXVAVer && (IviDxva2==(IMGPAR stream_global)->lDXVAVer || (IviDxva1==(IMGPAR stream_global)->lDXVAVer && (IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_ATI_PROPRIETARY_A)))
	{
		if (!( IMGPAR Hybrid_Decoding == 5 || (IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status == 0 && !IMGPAR nal_reference_idc))
		{
			if (IMGPAR Hybrid_Decoding)
			{
				if (IMGPAR Hybrid_Decoding == 1 && IMGPAR type != I_SLICE)
					EXECUTE ARGS0();
				else if (IMGPAR Hybrid_Decoding == 2 && IMGPAR type == B_SLICE)
					EXECUTE ARGS0();
			}
			else
				EXECUTE ARGS0();
		}
	}
#endif

#ifdef DEBUG_SHOW_PROCESS_TIME
	char yuv_types[4][6]= {"4:0:0","4:2:0","4:2:2","4:4:4"};
	int tmp_time;                   // time used by decoding the last frame
	char yuvFormat[10];
#endif

	// return if the last picture has already been finished
#if !defined(_COLLECT_PIC_)
	if ((dec_picture == NULL))
	{
		return CREL_OK;
	}
#else
	if (active_pps.num_slice_groups_minus1 == 0)
	{

		if (dec_picture != NULL) {

			if (IMGPAR stream_global->m_bSeekIDR) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
			}

			if (IMGPAR current_mb_nr_d != IMGPAR PicSizeInMbs) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
		}
	}
	else
	{
		if (dec_picture != NULL) {

			if (IMGPAR stream_global->m_bSeekIDR) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
			}

			if (/*IMGPAR current_mb_nr_d*/dec_picture->decoded_mb_number != IMGPAR PicSizeInMbs) {
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
		}

	}
#endif

	if (IMGPAR currentSlice->m_pic_combine_status == 0 && (!(IMGPAR stream_global->m_is_MTMS || IMGPAR stream_global->m_bIsSingleThreadMode == TRUE)))
	{
		memcpy(&(dec_picture->m_CCCode), &(IMGPAR m_CCCode), sizeof(H264_CC));
	}

	if (IMGPAR structure != FRAME)         // buffer mgt. for frame mode
		IMGPAR number >>= 1;   // reset all interlaced variables

	structure  = dec_picture->structure;
	slice_type = dec_picture->slice_type;
	frame_poc  = dec_picture->frame_poc;
	refpic     = dec_picture->used_for_reference;
	qp         = dec_picture->qp;
	pic_num    = dec_picture->pic_num;

	chroma_format_idc= dec_picture->chroma_format_idc;

#if 1
#ifdef _HW_ACCEL_
	if ((imgDXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding) && (dec_picture->used_for_reference ||IMGPAR currentSlice->interViewFlag) )
#else
	if (dec_picture->used_for_reference)
#endif
	{
		if(dec_picture->structure==TOP_FIELD || dec_picture->structure==BOTTOM_FIELD)
		{
			// Do boundary extension inside get_block & mb_chroma_pred when needed
			pad_boundary_luma ARGS4(dec_picture->imgY, dec_picture->size_x, dec_picture->size_y, 
				dec_picture->Y_stride);
			if(dec_picture->chroma_format_idc)
			{
				pad_boundary_chroma ARGS4(dec_picture->imgUV, dec_picture->size_x_cr, dec_picture->size_y_cr, 
					dec_picture->UV_stride);
			}
		}
		else
		{
			// Do boundary extension inside get_block & mb_chroma_pred when needed
			pad_boundary_luma ARGS4(dec_picture->imgY, dec_picture->size_x, dec_picture->size_y, 
				dec_picture->Y_stride);
			if(dec_picture->chroma_format_idc)
			{
				pad_boundary_chroma ARGS4(dec_picture->imgUV, dec_picture->size_x_cr, dec_picture->size_y_cr, 
					dec_picture->UV_stride);
			}
		}
	}
#endif

	StreamParameters *stream_global = IMGPAR stream_global;
	if (stream_global->m_is_MTMS != 1 && stream_global->m_bIsSingleThreadMode != TRUE)
	{	
		//Terry: this counter may be updated during decoding this image.
		dec_picture->SkipedBFrames[view_index][0] = IMGPAR SkipedBFrames[view_index][0];
		dec_picture->SkipedBFrames[view_index][1] = IMGPAR SkipedBFrames[view_index][1];
	}

	DEBUG_SHOW_SW_INFO_DETAIL("--Store This Picture in DPB--");
	
	ret = store_picture_in_dpb ARGS1(dec_picture);
	if (FAILED(ret)) {
		return ret;
	}
	IMGPAR FP_EndDecodeFrame ARGS0();
	prev_dec_picture = dec_picture;
	dec_picture=NULL;

	if (IMGPAR last_has_mmco_5)
	{
		IMGPAR pre_frame_num = 0;
	}

	DEBUG_SHOW_SW_INFO_DETAIL("--End Exit This Picture--");

	if( (structure==FRAME) || (structure==BOTTOM_FIELD) || (IMGPAR smart_dec >= SMART_DEC_LEVEL_4 && IMGPAR smart_dec != SMART_DEC_LEVEL_0) )
	{
#ifdef DEBUG_SHOW_PROCESS_TIME

#ifdef WIN32
		_ftime (&(IMGPAR tstruct_end));             // start time ms
#else
		ftime (&(IMGPAR tstruct_end));              // start time ms
#endif

		time( &(IMGPAR ltime_end));                // start time s

		tmp_time=(int) (IMGPAR ltime_end*1000+IMGPAR tstruct_end.millitm) - (IMGPAR ltime_start*1000+IMGPAR tstruct_start.millitm);
		tot_time=tot_time + tmp_time;

		sprintf(yuvFormat,"%s", yuv_types[chroma_format_idc]);

		if(slice_type == I_SLICE) // I picture
			DEBUG_SHOW_SW_INFO("(I)  %8d %5d %5d  %s %5d\n",
			frame_poc, pic_num, qp, yuvFormat, tmp_time);
		else if(slice_type == P_SLICE) // P pictures
			DEBUG_SHOW_SW_INFO("(P)  %8d %5d %5d  %s %5d\n",
			frame_poc, pic_num, qp, yuvFormat, tmp_time);	
		else if(refpic) // stored B pictures
			DEBUG_SHOW_SW_INFO("(RB) %8d %5d %5d  %s %5d\n",
			frame_poc, pic_num, qp, yuvFormat, tmp_time);
		else // B pictures
			DEBUG_SHOW_SW_INFO("(B)  %8d %5d %5d  %s %5d\n",
			frame_poc, pic_num, qp, yuvFormat, tmp_time);
#endif

		if(slice_type == I_SLICE || slice_type == P_SLICE || refpic)   // I or P pictures
			IMGPAR number++;

#if !defined(_COLLECT_PIC_)
		g_llDecCount[view_index]++;
#endif

	}
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    write the encoding mode and motion vectors of current 
*    MB to the buffer of the error concealment module.
************************************************************************
*/
#ifdef ENABLE_ERROR_RESILIENCE
void ercWriteMBMODEandMV()
{
	extern objectBuffer_t *erc_object_list;
	int i, ii, jj, currMBNum = IMGPAR current_mb_nr_d;
	int mbx = xPosMB(currMBNum,dec_picture->size_x), mby = yPosMB(currMBNum,dec_picture->size_x);
	objectBuffer_t *currRegion, *pRegion;
	currMB_d = &dec_picture->mb_data[currMBNum];
	short*  mv;

	currRegion = erc_object_list + (currMBNum<<2);

	if(IMGPAR type != B_SLICE) //non-B frame
	{
		for (i=0; i<4; i++)
		{
			pRegion             = currRegion + i;
			pRegion->regionMode = (currMB_d->mb_type  ==I16MB  ? REGMODE_INTRA      :
				currMB_d->b8mode[i]==IBLOCK ? REGMODE_INTRA_8x8  :
				currMB_d->b8mode[i]==0      ? REGMODE_INTER_COPY :
				currMB_d->b8mode[i]==1      ? REGMODE_INTER_PRED : REGMODE_INTER_PRED_8x8);
			if (currMB_d->b8mode[i]==0 || currMB_d->b8mode[i]==IBLOCK)  // INTRA OR COPY
			{
				pRegion->mv[0]    = 0;
				pRegion->mv[1]    = 0;
				pRegion->mv[2]    = 0;
			}
			else
			{
				ii              = 4*mbx + (i%2)*2;// + BLOCK_SIZE;
				jj              = 4*mby + (i/2)*2;
				if (currMB_d->b8mode[i]>=5 && currMB_d->b8mode[i]<=7)  // SMALL BLOCKS
				{
					pRegion->mv[0]  = (dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2].x + dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2+1].x +
						dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2+4].x + dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2+5].x + 2)/4;
					pRegion->mv[1]  = (dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2].y + dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2+1].y +
						dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2+4].y + dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2+5].y + 2)/4;
				}
				else // 16x16, 16x8, 8x16, 8x8
				{
					pRegion->mv[0]  = dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2].x;
					pRegion->mv[1]  = dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2].y;
					//          pRegion->mv[0]  = dec_picture->mv[LIST_0][4*mby+(i/2)*2][4*mbx+(i%2)*2+BLOCK_SIZE][0];
					//          pRegion->mv[1]  = dec_picture->mv[LIST_0][4*mby+(i/2)*2][4*mbx+(i%2)*2+BLOCK_SIZE][1];
				}
				erc_mvperMB      += mabs(pRegion->mv[0]) + mabs(pRegion->mv[1]);
				pRegion->mv[2]    = dec_picture->mb_data[currMBNum].pred_info.ref_idx[LIST_0][i];
			}
		}
	}
	else  //B-frame
	{
		for (i=0; i<4; i++)
		{
			ii                  = 4*mbx + (i%2)*2;// + BLOCK_SIZE;
			jj                  = 4*mby + (i/2)*2;
			pRegion             = currRegion + i;
			pRegion->regionMode = (currMB_d->mb_type  ==I16MB  ? REGMODE_INTRA      :
				currMB_d->b8mode[i]==IBLOCK ? REGMODE_INTRA_8x8  : REGMODE_INTER_PRED_8x8);
			if (currMB_d->mb_type==I16MB || currMB_d->b8mode[i]==IBLOCK)  // INTRA
			{
				pRegion->mv[0]    = 0;
				pRegion->mv[1]    = 0;
				pRegion->mv[2]    = 0;
			}
			else
			{
				int idx = (dec_picture->mb_data[currMBNum].pred_info.ref_idx[LIST_0][i]<0)?1:0;
				//        int idx = (currMB_d->b8mode[i]==0 && currMB_d->b8pdir[i]==2 ? LIST_0 : currMB_d->b8pdir[i]==1 ? LIST_1 : LIST_0);
				//        int idx = currMB_d->b8pdir[i]==0 ? LIST_0 : LIST_1;
				pRegion->mv[0]    = dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2].x;
				pRegion->mv[1]    = dec_picture->mb_data[currMBNum].pred_info.mv[LIST_0][(i/2)*8+(i%2)*2].y;
				erc_mvperMB      += mabs(pRegion->mv[0]) + mabs(pRegion->mv[1]);

				pRegion->mv[2]    = dec_picture->mb_data[currMBNum].pred_info.ref_idx[LIST_0][i];
				/*
				if (currMB_d->b8pdir[i]==0 || (currMB_d->b8pdir[i]==2 && currMB_d->b8mode[i]!=0)) // forward or bidirect
				{
				pRegion->mv[2]  = (dec_picture->ref_idx[LIST_0][jj][ii]);
				///???? is it right, not only "IMGPAR fw_refFrArr[jj][ii-4]"
				}
				else
				{
				pRegion->mv[2]  = (dec_picture->ref_idx[LIST_1][jj][ii]);
				//          pRegion->mv[2]  = 0;
				}
				*/
			}
		}
	}
}
#endif

/*!
************************************************************************
* \brief
*    set defaults for old_slice
*    NAL unit of a picture"
************************************************************************
*/
void init_old_slice PARGS0()
{
	old_slice.field_pic_flag = 0;

	old_slice.pps_id = INT_MAX;

	old_slice.frame_num = INT_MAX;

	old_slice.nal_ref_idc = INT_MAX;

	old_slice.idr_flag = 0;

	old_slice.pic_oder_cnt_lsb          = UINT_MAX;
	old_slice.delta_pic_oder_cnt_bottom = INT_MAX;

	old_slice.delta_pic_order_cnt[0] = INT_MAX;
	old_slice.delta_pic_order_cnt[1] = INT_MAX;

}

/*!
************************************************************************
* \brief
*    save slice parameters that are needed for checking of "first VCL
*    NAL unit of a picture"
************************************************************************
*/
void exit_slice PARGS0()
{

	old_slice.pps_id = IMGPAR currentSlice->pic_parameter_set_id;

	old_slice.frame_num = IMGPAR frame_num;

	old_slice.field_pic_flag = IMGPAR field_pic_flag;

	if(IMGPAR field_pic_flag)
	{
		old_slice.bottom_field_flag = IMGPAR bottom_field_flag;
	}

	old_slice.nal_ref_idc   = IMGPAR nal_reference_idc;

	old_slice.idr_flag = IMGPAR idr_flag;
	if (IMGPAR idr_flag)
	{
		old_slice.idr_pic_id = IMGPAR idr_pic_id;
	}

	if (active_sps.pic_order_cnt_type == 0)
	{
		old_slice.pic_oder_cnt_lsb          = IMGPAR pic_order_cnt_lsb;
		old_slice.delta_pic_oder_cnt_bottom = IMGPAR delta_pic_order_cnt_bottom;
	}

	if (active_sps.pic_order_cnt_type == 1)
	{
		old_slice.delta_pic_order_cnt[0] = IMGPAR delta_pic_order_cnt[0];
		old_slice.delta_pic_order_cnt[1] = IMGPAR delta_pic_order_cnt[1];
	}

}



/*!
************************************************************************
* \brief
*    detect if current slice is "first VCL NAL unit of a picture"
************************************************************************
*/

#if !defined(_COLLECT_PIC_)
int is_new_picture PARGS0()
{
	int result=0;

	result |= (old_slice.pps_id != IMGPAR currentSlice->pic_parameter_set_id);

	result |= (old_slice.frame_num != IMGPAR frame_num);

	result |= (old_slice.field_pic_flag != IMGPAR field_pic_flag);

	if(IMGPAR field_pic_flag && old_slice.field_pic_flag)
	{
		result |= (old_slice.bottom_field_flag != IMGPAR bottom_field_flag);
	}

	result |= (old_slice.nal_ref_idc   != IMGPAR nal_reference_idc);

	result |= ( old_slice.idr_flag != IMGPAR idr_flag);

	if (IMGPAR idr_flag && old_slice.idr_flag)
	{
		result |= ((IMGPAR smart_dec & SMART_DEC_ONLY_IDR) ? 1 : (old_slice.idr_pic_id != IMGPAR idr_pic_id));
	}

	if (active_sps.pic_order_cnt_type == 0)
	{
		result |=  (old_slice.pic_oder_cnt_lsb          != IMGPAR pic_order_cnt_lsb);
		result |=  (old_slice.delta_pic_oder_cnt_bottom != IMGPAR delta_pic_order_cnt_bottom);
	}

	if (active_sps.pic_order_cnt_type == 1)
	{
		result |= (old_slice.delta_pic_order_cnt[0] != IMGPAR delta_pic_order_cnt[0]);
		result |= (old_slice.delta_pic_order_cnt[1] != IMGPAR delta_pic_order_cnt[1]);
	}

	return result;
}

#else

int is_new_picture PARGS0()
{
	Slice *currSlice = IMGPAR currentSlice;
	Slice *prevSlice = IMGPAR prevSlice;
	StreamParameters *stream_global = IMGPAR stream_global;
	seq_parameter_set_rbsp_t *sps = (currSlice->bIsBaseView) ? &SeqParSet[PicParSet[currSlice->pic_parameter_set_id].seq_parameter_set_id] : 
		&SeqParSubset[PicParSet[currSlice->pic_parameter_set_id].seq_parameter_set_id];
	int result=0;

	if(currSlice->idr_flag==1)
	{
		if(prevSlice == NULL)
		{
			currSlice->m_pic_combine_status = 0;
			return 1;
		}
		else if(prevSlice->frame_num != currSlice->frame_num)
			currSlice->m_pic_combine_status = 0;
	}

	if (prevSlice == NULL)
		return 1;

	result |= (prevSlice->viewId != currSlice->viewId);

	result |= (prevSlice->interViewFlag != currSlice->interViewFlag);

	result |= (prevSlice->anchorPicFlag != currSlice->anchorPicFlag);

	result |= (prevSlice->pic_parameter_set_id != currSlice->pic_parameter_set_id);

	result |= (prevSlice->frame_num != currSlice->frame_num);

	result |= (prevSlice->field_pic_flag != currSlice->field_pic_flag);

	if(currSlice->field_pic_flag && prevSlice->field_pic_flag)
	{
		result |= (prevSlice->bottom_field_flag != currSlice->bottom_field_flag);		
	}

	result |= (prevSlice->nal_reference_idc != currSlice->nal_reference_idc) && ((prevSlice->nal_reference_idc == 0) || (currSlice->nal_reference_idc == 0));

	result |= ( prevSlice->idr_flag != currSlice->idr_flag);

	if (currSlice->idr_flag && prevSlice->idr_flag)
	{
		result |= ((IMGPAR smart_dec & SMART_DEC_ONLY_IDR) ? 1 : (prevSlice->idr_pic_id != currSlice->idr_pic_id));
	}

	if (sps->pic_order_cnt_type == 0)
	{
		result |=  (prevSlice->pic_order_cnt_lsb          != currSlice->pic_order_cnt_lsb);
		result |=  (prevSlice->delta_pic_order_cnt_bottom != currSlice->delta_pic_order_cnt_bottom);
	}

	if (sps->pic_order_cnt_type == 1)
	{
		result |= (prevSlice->delta_pic_order_cnt[0] != currSlice->delta_pic_order_cnt[0]);
		result |= (prevSlice->delta_pic_order_cnt[1] != currSlice->delta_pic_order_cnt[1]);
	}

	return result;
}
#endif

void mem8( LPBYTE d, const BYTE* s, int _size ) 
{
	_asm 
	{
		mov edi,d;
		mov esi,s;
		mov ecx,_size;
		shr ecx,3;
lx8:
		movq mm0,[esi]; 

		lea esi,[esi+8];
		movntq [edi],mm0;
		lea edi,[edi+8];
		dec ecx;
		jnz lx8;
	}
}

void mem8_2( LPBYTE dUV, LPBYTE dU, LPBYTE dV, const BYTE* sU, const BYTE* sV, int _size ) 
{
	_asm 
	{
		mov edi,sU;
		mov esi,sV;
		mov eax,dU;
		mov ebx,dV;
		mov edx,dUV;
		mov ecx,_size;
		shr ecx,4;
lx8_2:
		movdqa xmm0,XMMWORD PTR [esi];	// V
		movdqa xmm1,XMMWORD PTR [edi];	// U
		movdqa xmm2,xmm1;				// U

		movntdq [eax],xmm1;				// Store U

		punpcklbw xmm1, xmm0;
		punpckhbw xmm2, xmm0;

		movntdq [ebx], xmm0			// Store V

			movdqa XMMWORD PTR [edx], xmm1;
		movdqa XMMWORD PTR [edx+16], xmm2;		

		lea esi,[esi+16];
		lea edi,[edi+16];		
		lea eax,[eax+16];
		lea ebx,[ebx+16];
		lea edx,[edx+32];

		dec ecx;
		jnz lx8_2;
	}
}

void mem16( LPBYTE d, const BYTE* s, int _size ) 
{
	_asm 
	{
		mov edi,d;
		mov esi,s;
		mov ecx,_size;
		shr ecx,4;
lx16:
		movaps xmm0,[esi]; 

		lea esi,[esi+16];
		movntps [edi],xmm0;
		lea edi,[edi+16];
		dec ecx;
		jnz lx16;
	}
}

void mem16_2( LPBYTE d1, LPBYTE d2, const BYTE* s, int _size ) 
{
	_asm 
	{
		mov edi,d1;
		mov	edx,d2;
		mov esi,s;
		mov ecx,_size;
		shr ecx,4;
lx16_2:
		movaps xmm0,[esi]; 

		lea esi,[esi+16];
		//movntps [edi],xmm0;		
		//movntps [edx],xmm0;
		movntdq [edi], xmm0;
		movntdq [edx], xmm0;

		lea edi,[edi+16];
		lea edx,[edx+16];
		dec ecx;
		jnz lx16_2;
	}
}


int StoreImgRowToImgPic PARGS2(int start_x, int end_x)
{
	//write back to picture	
	int j;
	imgpel *imgY;		
	imgpel *imgUV;
	int stride = dec_picture->Y_stride;
	int stride_UV = dec_picture->UV_stride;
	int width;
	imgpel *imgY_row;
	imgpel *imgUV_row;

	width = (end_x-start_x + 1) << 4;

	if ( IMGPAR MbaffFrameFlag ) 
	{
		if ( IMGPAR mb_y_d & 1 )
		{		
			imgY     = dec_picture->imgY + (IMGPAR pix_y_d-16)*stride+(start_x<<4);
			imgUV = dec_picture->imgUV + (IMGPAR pix_c_y_d-8)*stride_UV+(start_x<<4); 

			imgY_row = IMGPAR m_imgY + (IMGPAR pix_y_rows-16)*stride+(start_x<<4);
			imgUV_row = IMGPAR m_imgUV + (IMGPAR pix_c_y_rows-8)*stride_UV+(start_x<<4); 


			for(j=0;j<32;j++)
			{
				mem16(imgY, imgY_row, width);
				imgY += stride;
				imgY_row += stride;
			}

			for(j=0;j<16;j++)
			{
				mem8(imgUV, imgUV_row, width);
				imgUV += stride_UV;
				imgUV_row += stride_UV;
			}
		}
	} 
	else 
	{		
		imgY     = dec_picture->imgY + IMGPAR pix_y_d*stride+(start_x<<4);
		imgUV = dec_picture->imgUV + IMGPAR pix_c_y_d*stride_UV+(start_x<<4);  

		imgY_row = IMGPAR m_imgY + IMGPAR pix_y_rows*stride+(start_x<<4);
		imgUV_row = IMGPAR m_imgUV + IMGPAR pix_c_y_rows*stride_UV+(start_x<<4); 

		for(j=0;j<16;j++)
		{
			mem16(imgY, imgY_row, width);
			imgY += stride;
			imgY_row += stride;
		}

		for(j=0;j<8;j++)
		{
			mem8(imgUV, imgUV_row, width);
			imgUV += stride_UV;
			imgUV_row += stride_UV;
		}
	}

	return 0;
}

int TransferData_at_SliceEnd PARGS0() {

	StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);	

	return 0;
}

#ifdef _COLLECT_PIC_
CREL_RETURN fnDoDecodePictureDecodeIP PARGS1(int nImgID)
{
	Slice *currSlice;
	int i, j;
	int decode_buffer_pivot = 0;
	int list_offset;
	int MbaffFrameFlag;
	int prev_field_type;
	imgpel *pImgY, *pImgUV;
	bool de_blocking_flag;
	CREL_RETURN ret = CREL_OK;

	StreamParameters *stream_global = IMGPAR stream_global;

	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;
	int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;


	if (img->slice_number && IMGPAR bReadCompleted && (!IMGPAR SkipThisFrame)) {

		prev_field_type = IMGPAR currentSlice->picture_type;
	}
	//IMGPAR start_mb_x = 100000; //set a big number

	DEBUG_SHOW_SW_INFO_DETAIL("--01_Start Decode This Frame--");
	/*
	if (!IMGPAR SkipThisFrame && !IMGPAR bReadCompleted)
	{
	DEBUG_SHOW_ERROR_INFO("ERROR!! [DecodeIP]This Frame doesn't read complete!!");
	IMGPAR SkipThisFrame = 1;
	}
	*/
	//for(i=0; (!IMGPAR SkipThisFrame) && (IMGPAR bReadCompleted) && (i<IMGPAR slice_number); i++)
	for(i=0; (!IMGPAR SkipThisFrame) && (i<IMGPAR slice_number); i++)
	{
		currSlice = IMGPAR currentSlice;

		IMGPAR start_mb_x = 100000; //set a big number


		if (currSlice->picture_type == B_SLICE && (currSlice->structure!=FRAME && currSlice->m_pic_combine_status==FRAME) && currSlice->header == SOP) //put bottom field read and decode on decoding thread
		/*if (currSlice->picture_type == B_SLICE 
			&& ((currSlice->structure == BOTTOM_FIELD && img->firstSlice->structure == TOP_FIELD)|| (currSlice->structure == TOP_FIELD && img->firstSlice->structure == BOTTOM_FIELD)) 
			&& currSlice->header == SOP)*/
		{
			stream_global->m_bRefBWaitBRead = TRUE;

			SetEvent(event_RB_1stfield_decode_complete);
			WaitForSingleObject(event_RB_2ndfield_read_complete, INFINITE);
			ResetEvent(event_RB_2ndfield_read_complete);

			if (IMGPAR bReadCompleted == FALSE) {
				//if ((IMGPAR bReadCompleted == FALSE) || (stream_global->m_bSeekIDR)){  //Future improvement
				return CREL_OK;
			}

			IMGPAR currentSlice = IMGPAR firstSlice;
			for(j=0; j<i; j++)
			{
				IMGPAR prevSlice = IMGPAR currentSlice;
				IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
			}
			
		}

		if (IMGPAR bReadCompleted) {

#if defined(_HW_ACCEL_)
			if(IMGPAR slice_number > 1 && currSlice->header == SOP)
				IMGPAR m_bFirstMB = TRUE;
#endif

			DEBUG_SHOW_SW_INFO_DETAIL("--01_Copy Slice Parameter--");

			CopySliceParameter ARGS0();
			//IMGPAR m_bFirstMB = TRUE;
			//FmoInit ARGS2(active_pps, active_sps);
			AssignQuantParam ARGS2(&active_pps, &active_sps);

			if (IMGPAR structure == TOP_FIELD)
			{
				dec_picture = IMGPAR m_dec_picture_top;
				IMGPAR mb_decdata = IMGPAR mb_decdata_top;
				IMGPAR cof_array = IMGPAR cof_array_top;

				IMGPAR toppoc = dec_picture->top_poc;
				IMGPAR bottompoc = dec_picture->bottom_poc;
				IMGPAR framepoc = dec_picture->frame_poc;
				IMGPAR PreviousPOC = currSlice->PreviousPOC;
				IMGPAR ThisPOC = currSlice->ThisPOC;
			}
			else if (IMGPAR structure == BOTTOM_FIELD)
			{
				dec_picture = IMGPAR m_dec_picture_bottom;
				IMGPAR mb_decdata = IMGPAR mb_decdata_bottom;				
				IMGPAR cof_array = IMGPAR cof_array_bottom;

				IMGPAR toppoc = dec_picture->top_poc;
				IMGPAR bottompoc = dec_picture->bottom_poc;
				IMGPAR framepoc = dec_picture->frame_poc;
				IMGPAR PreviousPOC = currSlice->PreviousPOC;
				IMGPAR ThisPOC = currSlice->ThisPOC;
			}


			if (IMGPAR MbaffFrameFlag)
				IMGPAR current_mb_nr_d = currSlice->start_mb_nr << 1;
			else
				IMGPAR current_mb_nr_d = currSlice->start_mb_nr;


			if (currSlice->header == SOP)
			{

				IMGPAR FP_BeginDecodeFrame ARGS0();

				ret = init_lists ARGS2(IMGPAR type, currSlice->structure);

				if (FAILED(ret)) {
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}
					break;
				}


				ret = reorder_lists ARGS2((IMGPAR type), (currSlice));
				if (FAILED(ret)) {
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}
					break;
				}

				free_ref_pic_list_reordering_buffer(currSlice);

				ret = check_lists ARGS0();
				if (FAILED(ret)) {
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}					
					break;
				}

				if (IMGPAR structure==FRAME)
					init_mbaff_lists ARGS0();
			}

			if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
				fill_wp_params ARGS0();

			decode_buffer_pivot = currSlice->read_mb_nr;

			currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
			currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];

			MbaffFrameFlag = currSlice->MbaffFrameFlag;

			//Terry: +2 is for field case in this decoding stage.
			if(g_llDecCount[view_index] < NO_SKIP_FRAME_FOR_PULLDOWN+2 || !(IMGPAR smart_dec & (SMART_DEC_SKIP_PB | SMART_DEC_ONLY_IDR)) || (prev_field_type == IMGPAR type))
			{
				DEBUG_SHOW_SW_INFO_DETAIL("--01_Start Decode MBs in One Slice--");

				while (decode_buffer_pivot)
				{
#if !defined(ONE_COF)
					IMGPAR cof_d = (IMGPAR cof_array + IMGPAR current_mb_nr_d*384);
#endif				
					int remainder = IMGPAR current_mb_nr_d;
					if (MbaffFrameFlag)
					{
						IMGPAR mb_x_d = mb_pos_table[remainder>>1].x;
						IMGPAR mb_y_d = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);
					}
					else
					{
						IMGPAR mb_x_d = (((int)mb_pos_table[remainder].x));   
						IMGPAR mb_y_d = (((int)mb_pos_table[remainder].y));
					}

					/* Define vertical positions */
					IMGPAR block_y_d = IMGPAR mb_y_d * BLOCK_SIZE;      /* luma block position */
					IMGPAR pix_y_d   = IMGPAR mb_y_d * MB_BLOCK_SIZE;   /* luma macroblock position */
					IMGPAR pix_c_y_d = IMGPAR mb_y_d * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

					/* Define horizontal positions */
					IMGPAR block_x_d = IMGPAR mb_x_d * BLOCK_SIZE;      /* luma block position */
					IMGPAR pix_x_d   = IMGPAR mb_x_d * MB_BLOCK_SIZE;   /* luma pixel position */
					IMGPAR pix_c_x_d = IMGPAR mb_x_d * (MB_BLOCK_SIZE/2); /* chroma pixel position */

					if(IMGPAR mb_x_d < IMGPAR start_mb_x)
						IMGPAR start_mb_x = IMGPAR mb_x_d;

					if(IMGPAR MbaffFrameFlag)
					{
						IMGPAR pix_y_rows = IMGPAR pix_y_d&63;		
						IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&31;
					}
					else
					{
						IMGPAR pix_y_rows = IMGPAR pix_y_d&31;		
						IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&15;
					}

					list_offset = (MbaffFrameFlag&&(currMB_s_d->mb_field))? (decode_buffer_pivot&1) ? 4 : 2 : 0;					

					if (currMB_s_d->do_record == 1) {
						ret = record_reference_picIds ARGS2(list_offset, currMB_s_d);

						if (FAILED(ret)) {		// Error or WarningLevel_1

							if (ISWARNINGLEVEL_1(ret)) {
								stream_global->m_bSeekIDR = TRUE;
								nalu->buf = IMGPAR ori_nalu_buf;
								break;
							} else {
								//return ret;
								break;
							}
						}
					}

					if (IS_COPY(currMB_d))
						memset(&(currMB_s_d->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);

					if (!currMB_d->luma_transform_size_8x8_flag)
						IMGPAR FP_MB_itrans_Luma = MB_itrans4x4_Luma;
					else
						IMGPAR FP_MB_itrans_Luma = MB_itrans8x8_Luma;

					if(IMGPAR type == I_SLICE)
						ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
					else if(IMGPAR type == P_SLICE)
					{
						if(IS_INTRA(currMB_d))
							ret = IMGPAR FP_decode_one_macroblock_P_Intra ARGS0();
						else 
							ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
					}
					else
					{
						if(IS_INTRA(currMB_d))
							ret = IMGPAR FP_decode_one_macroblock_B_Intra ARGS0();
						else
							ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
					}

					if (FAILED(ret)) {
						break;
					}

					decode_buffer_pivot--;
					//IMGPAR current_mb_nr_d = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_d));
					if (active_pps.num_slice_groups_minus1 == 0)
					{
						IMGPAR current_mb_nr_d++;
						currMB_d++;
						currMB_s_d++;
					}
					else
					{
						IMGPAR current_mb_nr_d = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_d));
						currMB_d   = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d]; 
						currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d]; 
					}
#ifdef _SLEEP_FOR_AUDIO_
					if (IMGPAR current_mb_nr_d % 2040 == 0)
						Sleep(0);
#endif
				}

				if (FAILED(ret)) {
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
						break;
					}
				}

				if (active_pps.num_slice_groups_minus1 == 0)
				{
					if ( ( (IMGPAR mb_x_d+ 1) != IMGPAR PicWidthInMbs ) && (IMGPAR error_mb_nr != 0) ) { //In case error_mb_nr = 0, no one MB read sucessfully
						IMGPAR FP_TransferData_at_SliceEnd ARGS0();
					}
				}

				DEBUG_SHOW_SW_INFO_DETAIL("--01_End Decode MBs in One Slice--");
			}
			else
			{
				DEBUG_SHOW_SW_INFO_DETAIL("--01_Process Skip I or P--");

				for(i=0; i<IMGPAR PicSizeInMbs; i++)
				{
					memset(currMB_s_d->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
					memset(currMB_s_d->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/4);
					IMGPAR current_mb_nr_d++;
					currMB_d++;
					currMB_s_d++;
				}

				if(IMGPAR structure == TOP_FIELD)
				{
					pImgY = IMGPAR m_dec_picture_bottom->imgY;
					pImgUV = IMGPAR m_dec_picture_bottom->imgUV;
				}
				else//(IMGPAR structure == BOTTOM_FIELD)
				{
					pImgY = IMGPAR m_dec_picture_top->imgY;
					pImgUV = IMGPAR m_dec_picture_top->imgUV;
				}

				for (i=0; i<dec_picture->size_y; i++)
					memcpy(dec_picture->imgY+(i*dec_picture->Y_stride), pImgY+(i*dec_picture->Y_stride), dec_picture->Y_stride>>1);
				for (i=0; i<dec_picture->size_y_cr; i++)
					memcpy(dec_picture->imgUV+(i*dec_picture->UV_stride), pImgUV+(i*dec_picture->UV_stride), dec_picture->UV_stride>>1);
			}		

			prev_field_type = IMGPAR type;
			exit_slice ARGS0();

			if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
				de_blocking_flag = TRUE;
			else
				de_blocking_flag = FALSE;

			DEBUG_SHOW_SW_INFO_DETAIL("--01_Deblocking This Slice--");

			if(active_pps.num_slice_groups_minus1 == 0)
			{
#if defined(_HW_ACCEL_)
				if(de_blocking_flag && g_DXVAVer==IviNotDxva && (IMGPAR error_mb_nr < 0) && (decode_buffer_pivot == 0) )	// No Deblocking while error detected
#else
				if(de_blocking_flag && (IMGPAR error_mb_nr < 0) && (decode_buffer_pivot == 0) )
#endif
				{
					IMGPAR FP_DeblockSlice = DeblockSlice;
					if (IMGPAR MbaffFrameFlag)
						IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, currSlice->read_mb_nr);
					else
						IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, currSlice->read_mb_nr);
				}
			}

#if defined(_SHOW_THREAD_TIME_)
			QueryPerformanceCounter(&(stream_global->t_ip[3]));
#endif

			if(IMGPAR currentSlice->exit_flag || ISWARNINGLEVEL_3(ret))
			{
				if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C)	//In Intel Mode C, decoder doesn't execute iDCT, so IMGPAR cof_array should be reset.
					memset(IMGPAR cof_array, 0, IMGPAR current_mb_nr_d*768);  //We could need to add this memset to IPRD_Merge, if IPRD_Merge is used, and Reference B.
				if(active_pps.num_slice_groups_minus1 > 0)
				{
#if defined(_HW_ACCEL_)
					if(de_blocking_flag && g_DXVAVer==IviNotDxva && (IMGPAR error_mb_nr < 0) && (decode_buffer_pivot == 0) )	// No Deblocking while error detected
#else
					if(de_blocking_flag && (IMGPAR error_mb_nr < 0) && (decode_buffer_pivot == 0) )
#endif
					{
						IMGPAR FP_DeblockSlice = DeblockSlice;
						if (IMGPAR MbaffFrameFlag)
							IMGPAR FP_DeblockSlice ARGS3(dec_picture, 0, dec_picture->decoded_mb_number);
						else
							IMGPAR FP_DeblockSlice ARGS3(dec_picture, 0, dec_picture->decoded_mb_number);
					}
				}
				WaitForSingleObject(stream_global->m_event_for_field_ip, INFINITE);
#if defined(_HW_ACCEL_)
				if(g_DXVAVer && IMGPAR Hybrid_Decoding)
				{
					if(IMGPAR Hybrid_Decoding == 5 || (IMGPAR Hybrid_Decoding == 2 && IMGPAR type != B_SLICE) || IMGPAR type == I_SLICE)
						DMA_Transfer ARGS0();
				}
#endif
				ret = exit_picture ARGS0();
				if (FAILED(ret)) {		// Error or WarningLevel_1

					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;

					}
					break;
					
				}
			}
		}

		//IMGPAR current_slice_nr++;
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
	}
	ResetEvent(stream_global->m_event_for_field_ip);

	//if(dec_picture)
	//	exit_picture ARGS0();

	if (FAILED(ret) || (IMGPAR bReadCompleted == FALSE)) {

		if (IMGPAR structure == FRAME) {

			if(dec_picture)
			{
				if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->frame != dec_picture) {
					release_storable_picture ARGS2(dec_picture, 1);
				}
				dec_picture = NULL;
			}
			img->m_dec_picture_top = NULL;
			img->m_dec_picture_bottom = NULL;


		} else {

			if (img->m_dec_picture_top) {
				
					if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->top_field != img->m_dec_picture_top) {
						
						release_storable_picture ARGS2(img->m_dec_picture_top, 1);
					}
					img->m_dec_picture_top = NULL;
			}

			if (img->m_dec_picture_bottom) {

				if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->bottom_field != img->m_dec_picture_bottom) {

					release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
				}
				img->m_dec_picture_bottom = NULL;
			}

			dec_picture = NULL;

		}	


	}


	DEBUG_SHOW_SW_INFO_DETAIL("--01_Start Release Slices in This Frame--");

	IMGPAR currentSlice = IMGPAR firstSlice;
	for (i=0; i< IMGPAR slice_number; i++)
	{
		//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
		memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
		free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
		free_new_slice(IMGPAR prevSlice);
	}
	IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
	IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
	IMGPAR current_slice_nr = 0;
	IMGPAR slice_number = 0;

	img->m_dec_picture_top = NULL;
	img->m_dec_picture_bottom = NULL;
	dec_picture = NULL;

	if (IMGPAR structure != FRAME)
	{
		IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
		IMGPAR cof_array = IMGPAR cof_array_ori;
	}
	DEBUG_SHOW_SW_INFO_DETAIL("--01_End Release Slices in This Frame--");
	DEBUG_SHOW_SW_INFO_DETAIL("--01_End Decode This Frame--");

	return CREL_OK;

}

unsigned __stdcall decode_picture_decode_ip(void *parameter)
{
	stream_par *stream_global = (stream_par *)parameter;
	img_par *img;// = (img_par *)parameter;
	CREL_RETURN ret;

	while(1)
	{		
		WaitForSingleObject(stream_global->m_event_decode_start_ip, INFINITE);
		ResetEvent(stream_global->m_event_decode_start_ip);
		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_picture_decode_ip");

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&(stream_global->t_ip[2]));
#endif

		
		/*
		if ( img->number == 188 ) {
		img->number = img->number;
		}
		*/

		img = gImgIP_d;

		if ( stream_global->m_bSeekIDR == FALSE ) {	//Future Improvment


#if defined (IP_RD_MERGE)		
			ret = fnDoDecodePictureReadAndDecodeIP ARGS1(0);
#else
			ret = fnDoDecodePictureDecodeIP ARGS1(0);
#endif						
		}								//Future Improvement

		SetEvent(stream_global->m_event_decode_finish_ip);

		if (ISERROR(ret)) {
			break;
		}

		DEBUG_SHOW_SW_INFO("End decode_picture_decode_ip");
	}
	SetEvent(stream_global->m_event_decode_finish_ip);
	_endthreadex(0);

	return(0);
}

CREL_RETURN fnDoDecodePictureDecodeB01 PARGS1(int nImgID)
{
	Slice *currSlice;
	int i;
	int decode_buffer_pivot = 0;
	int list_offset;
	int MbaffFrameFlag;
	int prev_structure;
	imgpel *pImgY, *pImgUV;
	bool de_blocking_flag;
	CREL_RETURN ret;

	StreamParameters *stream_global = IMGPAR stream_global;

	//IMGPAR start_mb_x = 100000; //set a big number
	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;
	prev_structure = IMGPAR currentSlice->structure;

	DEBUG_SHOW_SW_INFO_DETAIL("--23_Start Decode This Frame--");
	/*
	if (!IMGPAR SkipThisFrame && !IMGPAR bReadCompleted)
	{
	DEBUG_SHOW_ERROR_INFO("ERROR!! [DecodeB01]This Frame doesn't read complete!!");
	IMGPAR SkipThisFrame = 1;
	}
	*/
	for(i=0; (!IMGPAR SkipThisFrame) && (IMGPAR bReadCompleted) && (i<IMGPAR slice_number); i++)
	{
		currSlice = IMGPAR currentSlice;
		IMGPAR start_mb_x = 100000; //set a big number

#if defined(_HW_ACCEL_)
		if(IMGPAR slice_number > 1 && currSlice->header == SOP)
			IMGPAR m_bFirstMB = TRUE;
#endif

		DEBUG_SHOW_SW_INFO_DETAIL("--23_Copy Slice Parameter--");

		CopySliceParameter ARGS0();
		//IMGPAR m_bFirstMB = TRUE;
		//FmoInit ARGS2(&active_pps, &active_sps);
		AssignQuantParam ARGS2(&active_pps, &active_sps);

		if (IMGPAR structure == TOP_FIELD)
		{
			dec_picture = IMGPAR m_dec_picture_top;
			IMGPAR mb_decdata = IMGPAR mb_decdata_top;
			IMGPAR cof_array = IMGPAR cof_array_top;

			IMGPAR toppoc = dec_picture->top_poc;
			IMGPAR bottompoc = dec_picture->bottom_poc;
			IMGPAR framepoc = dec_picture->frame_poc;
			IMGPAR PreviousPOC = currSlice->PreviousPOC;
			IMGPAR ThisPOC = currSlice->ThisPOC;
		} 
		else if (IMGPAR structure == BOTTOM_FIELD)
		{
			dec_picture = IMGPAR m_dec_picture_bottom;
			IMGPAR mb_decdata = IMGPAR mb_decdata_bottom;
			IMGPAR cof_array = IMGPAR cof_array_bottom;

			IMGPAR toppoc = dec_picture->top_poc;
			IMGPAR bottompoc = dec_picture->bottom_poc;
			IMGPAR framepoc = dec_picture->frame_poc;
			IMGPAR PreviousPOC = currSlice->PreviousPOC;
			IMGPAR ThisPOC = currSlice->ThisPOC;
		}

		if (IMGPAR MbaffFrameFlag)
			IMGPAR current_mb_nr_d = currSlice->start_mb_nr << 1;
		else
			IMGPAR current_mb_nr_d = currSlice->start_mb_nr;

		if (currSlice->header == SOP)
		{
			IMGPAR FP_BeginDecodeFrame ARGS0();

			ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
			if (FAILED(ret)) {
				return ret;
			}
			ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
			if (FAILED(ret)) {
				return ret;
			}

			free_ref_pic_list_reordering_buffer(currSlice);

			ret = check_lists ARGS0();
			if (FAILED(ret)) {
				return ret;
			}

			if (IMGPAR structure==FRAME)
				init_mbaff_lists ARGS0();
		}

		if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
			fill_wp_params ARGS0();	

		decode_buffer_pivot = currSlice->read_mb_nr;

		currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
		currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];

		MbaffFrameFlag = currSlice->MbaffFrameFlag;

		if (!(IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) || (prev_structure == IMGPAR structure))
		{
			DEBUG_SHOW_SW_INFO_DETAIL("--23_Start Decode MBs in One Frame--");

			while (decode_buffer_pivot)
			{
#if !defined(ONE_COF)
				IMGPAR cof_d = (IMGPAR cof_array + IMGPAR current_mb_nr_d*384);
#endif
				int remainder = IMGPAR current_mb_nr_d;
				if (MbaffFrameFlag)
				{
					IMGPAR mb_x_d = mb_pos_table[remainder>>1].x;
					IMGPAR mb_y_d = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);
				}
				else
				{
					IMGPAR mb_x_d = (((int)mb_pos_table[remainder].x));   
					IMGPAR mb_y_d = (((int)mb_pos_table[remainder].y));
				}

				/* Define vertical positions */
				IMGPAR block_y_d = IMGPAR mb_y_d * BLOCK_SIZE;      /* luma block position */
				IMGPAR pix_y_d   = IMGPAR mb_y_d * MB_BLOCK_SIZE;   /* luma macroblock position */
				IMGPAR pix_c_y_d = IMGPAR mb_y_d * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

				/* Define horizontal positions */
				IMGPAR block_x_d = IMGPAR mb_x_d * BLOCK_SIZE;      /* luma block position */
				IMGPAR pix_x_d   = IMGPAR mb_x_d * MB_BLOCK_SIZE;   /* luma pixel position */
				IMGPAR pix_c_x_d = IMGPAR mb_x_d * IMGPAR mb_cr_size_x; /* chroma pixel position */

				if(IMGPAR mb_x_d < IMGPAR start_mb_x)
					IMGPAR start_mb_x = IMGPAR mb_x_d;

				if(IMGPAR MbaffFrameFlag)
				{
					IMGPAR pix_y_rows = IMGPAR pix_y_d&63;		
					IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&31;
				}
				else
				{
					IMGPAR pix_y_rows = IMGPAR pix_y_d&31;		
					IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&15;
				}

				list_offset = (MbaffFrameFlag&&(currMB_s_d->mb_field))? (decode_buffer_pivot&1) ? 4 : 2 : 0;				

				if (currMB_s_d->do_record == 1) {
					ret = record_reference_picIds ARGS2(list_offset, currMB_s_d);
					if (FAILED(ret)) {		// Error or WarningLevel_1

						if (ISWARNINGLEVEL_1(ret)) {
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;
							break;
						} else {
							return ret;
						}
					}
				}

				if (IS_COPY(currMB_d))
					memset(&(currMB_s_d->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);

				if (!currMB_d->luma_transform_size_8x8_flag)
					IMGPAR FP_MB_itrans_Luma = MB_itrans4x4_Luma;
				else
					IMGPAR FP_MB_itrans_Luma = MB_itrans8x8_Luma;

				if(IMGPAR type == I_SLICE)
					ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
				else if(IMGPAR type == P_SLICE)
				{
					if(IS_INTRA(currMB_d))
						ret = IMGPAR FP_decode_one_macroblock_P_Intra ARGS0();
					else 
						ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
				}
				else
				{
					if(IS_INTRA(currMB_d))
						ret = IMGPAR FP_decode_one_macroblock_B_Intra ARGS0();
					else
						ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
				}

				if (FAILED(ret)) {
					break;
				}

				decode_buffer_pivot--;
				//IMGPAR current_mb_nr_d = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_d));
				IMGPAR current_mb_nr_d++;
				currMB_d++;
				currMB_s_d++;
#ifdef _SLEEP_FOR_AUDIO_
				if (IMGPAR current_mb_nr_d % 2040 == 0)
					Sleep(0);
#endif
			}

			if ( (IMGPAR mb_x_d+ 1) != IMGPAR PicWidthInMbs ) {
				IMGPAR FP_TransferData_at_SliceEnd ARGS0();
			}

			DEBUG_SHOW_SW_INFO_DETAIL("--23_End Decode MBs in One Frame--");
		}
		else
		{
			DEBUG_SHOW_SW_INFO_DETAIL("--23_Process Skip B--");

			for(i=0; i<IMGPAR PicSizeInMbs; i++)
			{
				memset(currMB_s_d->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
				memset(currMB_s_d->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/4);
				//memset(currMB_s_d->pred_info.ref_idx, -1, 2*BLOCK_SIZE*BLOCK_SIZE/4);	
				//memset(currMB_s_d->pred_info.ref_pic_id, 0xff, 2*BLOCK_SIZE*BLOCK_SIZE/4);
				IMGPAR current_mb_nr_d++;
				currMB_d++;
				currMB_s_d++;
			}

			if(IMGPAR structure == TOP_FIELD)
			{
				pImgY = IMGPAR m_dec_picture_bottom->imgY;
				pImgUV = IMGPAR m_dec_picture_bottom->imgUV;
			}
			else//(IMGPAR structure == BOTTOM_FIELD)
			{
				pImgY = IMGPAR m_dec_picture_top->imgY;
				pImgUV = IMGPAR m_dec_picture_top->imgUV;
			}
#if defined(_HW_ACCEL_)
			if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding==5)
#endif
			{
				for (i=0; i<dec_picture->size_y; i++)
					memcpy(dec_picture->imgY+(i*dec_picture->Y_stride), pImgY+(i*dec_picture->Y_stride), dec_picture->Y_stride>>1);
				for (i=0; i<dec_picture->size_y_cr; i++)
				{
					memcpy(dec_picture->imgUV+(i*dec_picture->UV_stride), pImgUV+(i*dec_picture->UV_stride), dec_picture->UV_stride>>1);
				}
			}
		}

		prev_structure = IMGPAR structure;		
		exit_slice ARGS0();

		if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
			de_blocking_flag = TRUE;
		else
			de_blocking_flag = FALSE;

		DEBUG_SHOW_SW_INFO_DETAIL("--23_Deblocking This Slice--");		

#if defined(_HW_ACCEL_)
		if(de_blocking_flag && g_DXVAVer==IviNotDxva && ( IMGPAR error_mb_nr < 0 ) && (decode_buffer_pivot == 0) )
#else
		if(de_blocking_flag && ( IMGPAR error_mb_nr < 0 ) && (decode_buffer_pivot == 0) )
#endif
		{
			IMGPAR FP_DeblockSlice = DeblockSlice;

			if (IMGPAR MbaffFrameFlag)
				IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, currSlice->read_mb_nr);
			else
				IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, currSlice->read_mb_nr);
		}

		if (nImgID == 0)
		{
			if(IMGPAR currentSlice->exit_flag) {
				ret = exit_picture ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			}
		}

		//IMGPAR current_slice_nr++;
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
	}

	if (nImgID == 1)
	{
		if (stream_global->two_B_Flag)
		{
#if defined(_SHOW_THREAD_TIME_)
			QueryPerformanceCounter(&(stream_global->t_b[7]));
#endif
			WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
		}

		if(!IMGPAR SkipThisFrame)
		{
			IMGPAR currentSlice = IMGPAR firstSlice;

			if (IMGPAR structure == FRAME) {
				ret = exit_picture ARGS0();
				if (FAILED(ret)) {
					return ret;
				}
			} else {
				while (IMGPAR currentSlice)
				{
					if (IMGPAR currentSlice->exit_flag)
					{
						if (IMGPAR currentSlice->structure == TOP_FIELD)
						{
							dec_picture = IMGPAR m_dec_picture_top;
							IMGPAR toppoc = dec_picture->top_poc;
							IMGPAR bottompoc = dec_picture->bottom_poc;
							IMGPAR framepoc = dec_picture->frame_poc;
							IMGPAR PreviousPOC = currSlice->PreviousPOC;
							IMGPAR ThisPOC = currSlice->ThisPOC;
							ret = exit_picture ARGS0();
							if (FAILED(ret)) {
								return ret;
							}
						}
						else
						{
							dec_picture = IMGPAR m_dec_picture_bottom;
							IMGPAR toppoc = dec_picture->top_poc;
							IMGPAR bottompoc = dec_picture->bottom_poc;
							IMGPAR framepoc = dec_picture->frame_poc;
							IMGPAR PreviousPOC = currSlice->PreviousPOC;
							IMGPAR ThisPOC = currSlice->ThisPOC;
							ret = exit_picture ARGS0();
							if (FAILED(ret)) {
								return ret;
							}
						}
					}
					IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
				}
			}
		}
	}

	DEBUG_SHOW_SW_INFO_DETAIL("--23_Start Release Slices in This Frame--");

	IMGPAR currentSlice = IMGPAR firstSlice;
	for (i=0; i< IMGPAR slice_number; i++)
	{
		//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
		memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
		free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
		free_new_slice(IMGPAR prevSlice);
	}
	IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
	IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
	IMGPAR current_slice_nr = 0;
	IMGPAR slice_number = 0;

	if (IMGPAR structure != FRAME)
	{
		IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
		IMGPAR cof_array = IMGPAR cof_array_ori;
	}

	DEBUG_SHOW_SW_INFO_DETAIL("--23_End Release Slices in This Frame--");
	DEBUG_SHOW_SW_INFO_DETAIL("--23_End Decode This Frame--");

	return CREL_OK;
}

unsigned __stdcall decode_picture_decode_b0(void *parameter)
{
	stream_par *stream_global = (stream_par *)parameter;
	img_par *img;// = (img_par *)parameter;
	CREL_RETURN ret;

	while(1)
	{		
		WaitForSingleObject(stream_global->m_event_decode_start_b[0], INFINITE);
		ResetEvent(stream_global->m_event_decode_start_b[0]);
		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_picture_decode_b0");

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&(stream_global->t_b[4]));
#endif

		img = gImgB_d0;

		ret = fnDoDecodePictureDecodeB01 ARGS1(0);

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&(stream_global->t_b[5]));
#endif

		SetEvent(stream_global->m_event_decode_finish_b[0]);

		DEBUG_SHOW_SW_INFO("End decode_picture_decode_b0");
	}
	SetEvent(stream_global->m_event_decode_finish_b[0]);
	_endthreadex(0);

	return(0);
}

unsigned __stdcall decode_picture_decode_b1(void *parameter)
{
	stream_par *stream_global = (stream_par *)parameter;
	img_par *img;// = (img_par *)parameter;
	CREL_RETURN ret;

	while(1)
	{		
		WaitForSingleObject(stream_global->m_event_decode_start_b[1], INFINITE);
		ResetEvent(stream_global->m_event_decode_start_b[1]);
		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_picture_decode_b1");

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&(stream_global->t_b[6]));
#endif

		img = gImgB_d1;

		ret = fnDoDecodePictureDecodeB01 ARGS1(1);

		SetEvent(stream_global->m_event_decode_finish_b[1]);

		DEBUG_SHOW_SW_INFO("End decode_picture_decode_b1");
	}
	SetEvent(stream_global->m_event_decode_finish_b[1]);
	_endthreadex(0);

	return(0);
}
#endif

/*!
************************************************************************
* \brief
*    decodes one slice
************************************************************************
*/
#if !defined(_COLLECT_PIC_)
CREL_RETURN read_one_slice PARGS0()
{
	BOOL end_of_slice = FALSE, bottom_mbaff_flag = FALSE;
	imgpel *imgTopY, *imgTopUV, *imgBotY, *imgBotUV;
	int read_flag, stride_Y, stride_UV, i;
	IMGPAR cod_counter=-1;
	IMGPAR start_mb_x = 100000; //set a big number
	CREL_RETURN ret;

	if(IMGPAR type==B_SLICE)
		SelectB8Mode_B ARGS0();
	//v2b8pd = b_v2b8pd;
	else if(IMGPAR type==P_SLICE)
		SelectB8Mode_P ARGS0();
	//v2b8pd = p_v2b8pd;
#ifdef DO_REF_PIC_NUM
	set_ref_pic_num();
#endif

	if(IMGPAR type==B_SLICE&&IMGPAR direct_spatial_mv_pred_flag==0)
		calc_mvscale ARGS0();
	while (end_of_slice == FALSE) // loop over macroblocks
	{
		// YC: This was previously performed inside getNeighbour
		if (IMGPAR current_mb_nr_r<0)
			DEBUG_SHOW_ERROR_INFO ("[ERROR]decode_one_slice: invalid macroblock number", 100);
		// Initializes the current macroblock
		ret = start_macroblock ARGS0();
		// Get the syntax elements from the NAL
		if(!(IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) || !(IMGPAR type==B_SLICE && IMGPAR structure==BOTTOM_FIELD))
			read_flag = read_one_macroblock ARGS0();

		//currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
		//currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];

		if (!currMB_d->luma_transform_size_8x8_flag)
			IMGPAR FP_MB_itrans_Luma = MB_itrans4x4_Luma;
		else
			IMGPAR FP_MB_itrans_Luma = MB_itrans8x8_Luma;

		int remainder = IMGPAR current_mb_nr_d;
		if (IMGPAR MbaffFrameFlag)
		{
			IMGPAR mb_x_d = mb_pos_table[remainder>>1].x;
			IMGPAR mb_y_d = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);
		}
		else
		{
			IMGPAR mb_x_d = (((int)mb_pos_table[remainder].x));   
			IMGPAR mb_y_d = (((int)mb_pos_table[remainder].y));
		}
#if !defined(ONE_COF)
		IMGPAR cof_d = (IMGPAR cof_array + IMGPAR current_mb_nr_d*384);
#endif

		/* Define vertical positions */
		IMGPAR block_y_d = IMGPAR mb_y_d * BLOCK_SIZE;      /* luma block position */
		IMGPAR pix_y_d   = IMGPAR mb_y_d * MB_BLOCK_SIZE;   /* luma macroblock position */
		IMGPAR pix_c_y_d = IMGPAR mb_y_d * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

		/* Define horizontal positions */
		IMGPAR block_x_d = IMGPAR mb_x_d * BLOCK_SIZE;      /* luma block position */
		IMGPAR pix_x_d   = IMGPAR mb_x_d * MB_BLOCK_SIZE;   /* luma pixel position */
		IMGPAR pix_c_x_d = IMGPAR mb_x_d * (MB_BLOCK_SIZE/2); /* chroma pixel position */

		if(IMGPAR mb_x_d < IMGPAR start_mb_x)
			IMGPAR start_mb_x = IMGPAR mb_x_d;

		if(IMGPAR MbaffFrameFlag)
		{
			IMGPAR pix_y_rows = IMGPAR pix_y_d&63;		
			IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&31;
		}
		else
		{
			IMGPAR pix_y_rows = IMGPAR pix_y_d&31;		
			IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&15;
		}


		if(!(IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B))
		{
			if(IMGPAR Hybrid_Decoding != 1)
			{
				if(IS_INTRA(currMB_d))
					ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
				else if(IMGPAR type == P_SLICE)
					ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
				else 
					ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
			}
			else
			{
				if(IMGPAR type == I_SLICE)
					ret = decode_one_macroblock_I ARGS0();
				else if(IS_INTRA(currMB_d))
					ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
				else if(IMGPAR type == P_SLICE)
					ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
				else 
					ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
			}

		}
		else
		{	// FRAME => MbaffFrameFlag == 0 or 1; TOP_FIELD or BOTTOM_FIELD => MbaffFrameFlag == 0
			bottom_mbaff_flag = (IMGPAR MbaffFrameFlag && currMB_s_d->mb_field && (IMGPAR current_mb_nr_d&1));

			if(IS_INTRA(currMB_d) && !(IMGPAR type==B_SLICE && IMGPAR structure==BOTTOM_FIELD))
				ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
			else if(IMGPAR type == P_SLICE)
				ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
			else if((IMGPAR structure != BOTTOM_FIELD) && (!bottom_mbaff_flag))
				ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
			else
			{
#if defined(ONE_COF)
				memset(IMGPAR cof, 0, (4+IMGPAR num_blk8x8_uv)*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE*sizeof(short));
#else
				memset(IMGPAR cof_d, 0, (4+IMGPAR num_blk8x8_uv)*BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE);
#endif

				if ( bottom_mbaff_flag )
				{
					stride_Y  = dec_picture->Y_stride;
					stride_UV = dec_picture->UV_stride;

					imgTopY = dec_picture->imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_d-16)*stride_Y);
					imgBotY = dec_picture->imgY + IMGPAR pix_x_d + ((IMGPAR pix_y_d-15)*stride_Y);
					stride_Y <<= 1;

					for (i=0; i<16; i++)
						memcpy(imgBotY+(i*stride_Y), imgTopY+(i*stride_Y), 16);

					imgTopUV = dec_picture->imgUV + ((IMGPAR pix_c_y_d-8)*stride_UV) + (IMGPAR pix_c_x_d<<1); 					
					imgBotUV = dec_picture->imgUV + ((IMGPAR pix_c_y_d-7)*stride_UV) + (IMGPAR pix_c_x_d<<1); 					
					stride_UV <<= 1;

					for (i=0; i<8; i++)
					{
						memcpy(imgBotUV+(i*stride_UV), imgTopUV+(i*stride_UV), 16);	
					}

					calc_chroma_vector_adjustment ARGS2(2, 1);

					if(IMGPAR do_co_located)
					{
						if ((IS_DIRECT (currMB_d) || 
							(IS_P8x8(currMB_d) && !(currMB_d->b8mode[0] && currMB_d->b8mode[1] && currMB_d->b8mode[2] && currMB_d->b8mode[3]))))
						{
							if(IMGPAR direct_spatial_mv_pred_flag)
								ret = IMGPAR FP_ReadMV_Direct_Spatial ARGS3(IMGPAR current_mb_nr_d, currMB_d, currMB_s_d);
							else
								ret = IMGPAR FP_ReadMV_Direct_Temproal ARGS3(IMGPAR current_mb_nr_d, currMB_d, currMB_s_d);

							record_reference_picIds ARGS2(2, currMB_s_d);
						}
					}
				}//(bottom_mbaff_flag)
			}
		}

		if (FAILED(ret)) {
			break;
		}

		if(IMGPAR MbaffFrameFlag && currMB_d->mb_field)
		{
			IMGPAR num_ref_idx_l0_active >>= 1;
			IMGPAR num_ref_idx_l1_active >>= 1;
		}

#ifdef ENABLE_ERROR_RESILIENCE
		ercWriteMBMODEandMV(img,inp);
#endif

		ret = exit_macroblock ARGS2((!IMGPAR MbaffFrameFlag||(IMGPAR current_mb_nr_r&1)), &end_of_slice);
		if (FAILED(ret)) {
			return ret;
		}

		//write back to picture

	}

	if ( (IMGPAR mb_x_d+ 1) != IMGPAR PicWidthInMbs ) 
		StoreImgRowToImgPic ARGS2(IMGPAR start_mb_x, IMGPAR mb_x_d);

	exit_slice ARGS0();

	return CREL_OK;
}

#else //#if !defined(_COLLECT_PIC_)

CREL_RETURN read_one_slice PARGS0()
{
	BOOL end_of_slice = FALSE, bottom_mbaff_flag = FALSE;
	int nImgType, nEntropyMode;
	Slice *currSlice = IMGPAR currentSlice;
	CREL_RETURN ret;

	IMGPAR cod_counter=-1;

	if(IMGPAR type==B_SLICE)
		SelectB8Mode_B ARGS0();
	else if(IMGPAR type==P_SLICE)
		SelectB8Mode_P ARGS0();

#ifdef DO_REF_PIC_NUM
	set_ref_pic_num();
#endif

	if(IMGPAR structure == TOP_FIELD)	  
	{
		dec_picture = IMGPAR m_dec_picture_top;
		IMGPAR mb_decdata = IMGPAR mb_decdata_top;
		IMGPAR cof_array = IMGPAR cof_array_top;		  
	}
	else if(IMGPAR structure == BOTTOM_FIELD)	  
	{
		dec_picture = IMGPAR m_dec_picture_bottom;
		IMGPAR mb_decdata = IMGPAR mb_decdata_bottom;
		IMGPAR cof_array = IMGPAR cof_array_bottom;
	}
	currSlice->read_mb_nr = 0;

	DEBUG_SHOW_SW_INFO_DETAIL("--Start Read MBs in One Slice--");

	nImgType = IMGPAR type;
	nEntropyMode = active_pps.entropy_coding_mode_flag;

	while (end_of_slice == FALSE) // loop over macroblocks
	{
		// YC: This was previously performed inside getNeighbour
		if (IMGPAR current_mb_nr_r<0) {
			DEBUG_SHOW_ERROR_INFO ("[ERROR]decode_one_slice: invalid macroblock number", 100);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
		// Initializes the current macroblock
		ret = start_macroblock ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		// Get the syntax elements from the NAL
		/*
		if ( (img->mb_x_r ==5) && (img->mb_y_r ==0) && (img->framepoc == -4) ){		//Debug P frame 
		img->number = img->number;
		}
		*/
		ret = pf_read_one_macroblock[nEntropyMode][nImgType][IMGPAR current_mb_nr_r&1] ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
		{
			IMGPAR num_ref_idx_l0_active >>= 1;
			IMGPAR num_ref_idx_l1_active >>= 1;
		}

#ifdef ENABLE_ERROR_RESILIENCE
		ercWriteMBMODEandMV(img,inp);
#endif

		currSlice->read_mb_nr++;
		ret =exit_macroblock ARGS2((!IMGPAR MbaffFrameFlag||(IMGPAR current_mb_nr_r&1)), &end_of_slice);
		if (FAILED(ret)) {
			return ret;
		}
#ifdef _SLEEP_FOR_AUDIO_
		if (IMGPAR current_mb_nr_r % 2040 == 0)
			Sleep(0);
#endif
	}
	DEBUG_SHOW_SW_INFO_DETAIL("--End Read MBs in One Slice--");
	return CREL_OK;
}

CREL_RETURN read_one_slice_FMO PARGS0()
{
	BOOL end_of_slice = FALSE, bottom_mbaff_flag = FALSE;
	int nImgType, nEntropyMode;
	Slice *currSlice = IMGPAR currentSlice;
	CREL_RETURN ret;

	IMGPAR cod_counter=-1;

	if(IMGPAR type==B_SLICE)
		SelectB8Mode_B ARGS0();
	else if(IMGPAR type==P_SLICE)
		SelectB8Mode_P ARGS0();

#ifdef DO_REF_PIC_NUM
	set_ref_pic_num();
#endif

	if(IMGPAR structure == TOP_FIELD)	  
	{
		dec_picture = IMGPAR m_dec_picture_top;
		IMGPAR mb_decdata = IMGPAR mb_decdata_top;
		IMGPAR cof_array = IMGPAR cof_array_top;		  
	}
	else if(IMGPAR structure == BOTTOM_FIELD)	  
	{
		dec_picture = IMGPAR m_dec_picture_bottom;
		IMGPAR mb_decdata = IMGPAR mb_decdata_bottom;
		IMGPAR cof_array = IMGPAR cof_array_bottom;
	}
	currSlice->read_mb_nr = 0;

	DEBUG_SHOW_SW_INFO_DETAIL("--Start Read MBs in One Slice--");

	nImgType = IMGPAR type;
	nEntropyMode = active_pps.entropy_coding_mode_flag;

#ifdef COMPRESS_IDCT
	IMGPAR cof_r = (IMGPAR cof_array + currSlice->start_mb_nr*384);
#endif
	/*	//Debugging
	if ((img->cof_array_top != img->cof_array_ori) && (img->cof_array_top != img->cof_array_ori + 675*768/2) ) {
	img->number = img->number;
	}
	*/
	memset(IMGPAR decoded_flag, 0, IMGPAR FrameSizeInMbs);
	while (end_of_slice == FALSE) // loop over macroblocks
	{
		// YC: This was previously performed inside getNeighbour
		if (IMGPAR current_mb_nr_r<0) {
			DEBUG_SHOW_ERROR_INFO ("read_one_slice: invalid macroblock number", 100);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
		}
		// Initializes the current macroblock
		ret = start_macroblock ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		// Get the syntax elements from the NAL
		/*
		if ( (img->mb_x_r ==5) && (img->mb_y_r ==0) && (img->framepoc == -4) ){		//Debug P frame 
		img->number = img->number;
		}
		*/
		ret = pf_read_one_macroblock[nEntropyMode][nImgType][IMGPAR current_mb_nr_r&1] ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
		{
			IMGPAR num_ref_idx_l0_active >>= 1;
			IMGPAR num_ref_idx_l1_active >>= 1;
		}

#ifdef ENABLE_ERROR_RESILIENCE
		ercWriteMBMODEandMV(img,inp);
#endif

		currSlice->read_mb_nr++;
		IMGPAR decoded_flag[IMGPAR current_mb_nr_r] = 1;
		ret = exit_macroblock_FMO ARGS2((!IMGPAR MbaffFrameFlag||(IMGPAR current_mb_nr_r&1)), &end_of_slice);
		if (FAILED(ret)) {
			return ret;
		}
#ifdef _SLEEP_FOR_AUDIO_
		if (IMGPAR current_mb_nr_r % 2040 == 0)
			Sleep(0);
#endif
	}
	DEBUG_SHOW_SW_INFO_DETAIL("--End Read MBs in One Slice--");
	return CREL_OK;
}
#endif

CREL_RETURN read_slice PARGS1(int header)
{
	Slice *currSlice = IMGPAR currentSlice;
	CREL_RETURN ret = CREL_OK;

	if (active_pps.entropy_coding_mode_flag)
	{
		init_contexts ARGS0();
		cabac_new_slice ARGS0();
	}

#if !defined(_COLLECT_PIC_)
	if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
		fill_wp_params ARGS0();
#endif

	// decode main slice information
	if ((header == SOP || header == SOS) && currSlice->ei_flag == 0) {
		if(active_pps.num_slice_groups_minus1 == 0)
			ret = read_one_slice ARGS0();		
		else
			ret = read_one_slice_FMO ARGS0();		
	}

	return ret;
}


#ifdef _COLLECT_PIC_
CREL_RETURN initial_image PARGS0()
{
	Slice *currSlice = IMGPAR currentSlice;
	DecodingEnvironment *dep;
	int ret;
	StreamParameters *stream_global = IMGPAR stream_global;

	if (currSlice == IMGPAR firstSlice) {

		if (stream_global->m_is_MTMS && stream_global->m_bIsSingleThreadMode == FALSE) {

			for (int k = 0; k < 4; k++) {
				stream_global->m_img[k]->m_dec_picture = NULL;
				stream_global->m_img[k]->m_dec_picture_top = NULL;
				stream_global->m_img[k]->m_dec_picture_bottom = NULL;
			}

		} else {
			IMGPAR m_dec_picture = NULL;
			IMGPAR m_dec_picture_top = NULL;
			IMGPAR m_dec_picture_bottom = NULL;
		}
	}

	//ParseHeader
	ret = UseSliceParameter ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if(active_pps.num_slice_groups_minus1 > 0)
		FmoInit ARGS2(&active_pps, &active_sps);

	AssignQuantParam ARGS2(&active_pps, &active_sps);


	if(currSlice->header == SOP)
	{
		ret = init_picture ARGS0();
		if(FAILED(ret))
		{
			DEBUG_SHOW_ERROR_INFO("[ERROR]ERROR: init_picture FAIL!!");
			return ret;
		}
		//check zero_byte if it is also the first NAL unit in the access unit
		CheckZeroByteVCL ARGS1(nalu);
		// IoK: for each new picture, reset first_mb_in_slice
		g_prev_start_mb_nr = (-1);
	}
	else if (stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode == TRUE)
	{
		//		if (IMGPAR frame_num != IMGPAR pre_frame_num && IMGPAR frame_num != (IMGPAR pre_frame_num + 1) % IMGPAR MaxFrameNum) 
		//		{
		//			if (active_sps.gaps_in_frame_num_value_allowed_flag == 0)
		//			{
		//				/* Advanced Error Concealment would be called here to combat unintentional loss of pictures. */
		//				DEBUG_SHOW_ERROR_INFO("[ERROR]An unintentional loss of pictures occurs! Exit\n", 100);
		//			}
		//			// IoK: This commenting-out is necessary to make cases SPEED_2 and SPEED_3 work
		//			// IoK: Uncommented to fix crash in bitstreams with gap in frame number...
		//			nSeekFlag = g_framemgr->GetDisplayCount() + dpb.used_size;
		//			// Terry: For seeking case (nSeekFlag==0), the first frame should be I frame.
		//			if((nSeekFlag == 0) && (IMGPAR type != I_SLICE))
		//				return -2;
		//			// Terry: For seeking to non_closed GOP, we should skip the first B frames between I and P frames.
		//			if((nSeekFlag == 0) && (IMGPAR idr_flag == 0))
		//			{
		//				IMGPAR bSkipBinOpenGOP = 1;
		//#if !defined(_GLOBAL_IMG_)
		//				stream_global->bSeekToOpenGOP = 1;
		//#endif
		//			}
		//			// Terry: For seeking case (nSeekFlag==0), we should skip this function.
		//			if( !((IMGPAR smart_dec==2) || (IMGPAR smart_dec==3)) && nSeekFlag && IMGPAR dwFillFrameNumGap)
		//				fill_frame_num_gap ARGS0();
		//		}

		if (IMGPAR structure == TOP_FIELD)
		{
			dec_picture = IMGPAR m_dec_picture_top;
			//IMGPAR cof_array_top = IMGPAR cof_array;
			IMGPAR cof_array_top = IMGPAR cof_array_ori;	//By Haihua
			IMGPAR mb_decdata_top = IMGPAR mb_decdata_ori;
		}
		else if (IMGPAR structure == BOTTOM_FIELD)
		{
			dec_picture = IMGPAR m_dec_picture_bottom;
			//IMGPAR cof_array_bottom = IMGPAR cof_array;
			IMGPAR cof_array_bottom = IMGPAR cof_array_ori + IMGPAR PicSizeInMbs * 384;	//By Haihua
			IMGPAR mb_decdata_bottom = IMGPAR mb_decdata_ori + IMGPAR PicSizeInMbs;
		}

		IMGPAR pre_frame_num = IMGPAR frame_num;

		//calculate POC
		ret = decode_poc ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		switch (IMGPAR structure )
		{
		case TOP_FIELD:
			{
				IMGPAR number *= 2;
				break;
			}
		case BOTTOM_FIELD:
			{
				IMGPAR number++;
				break;
			}
		case FRAME:
			{
				break;
			}
		default:
			DEBUG_SHOW_ERROR_INFO("[ERROR]IMGPAR structure not initialized", 235);
		}

		// CAVLC init
		if(active_pps.entropy_coding_mode_flag == UVLC)
			memset(IMGPAR nz_coeff, 0xFF, (int)IMGPAR PicSizeInMbs*sizeof(IMGPAR nz_coeff[0]));

		if(active_pps.constrained_intra_pred_flag)
		{
			for (int i=0; i<(int)IMGPAR PicSizeInMbs; i++)
			{
				IMGPAR mb_decdata[i].intra_block = 1;	//Could be moved to row on signle slice case
			}
		}

		IMGPAR mb_y_r = IMGPAR mb_x_r = IMGPAR mb_left_x_r_FMO = 0;
		IMGPAR mb_y_d = IMGPAR mb_x_d = 0;
		//IMGPAR block_y_r = IMGPAR pix_y_r = IMGPAR pix_c_y_r = 0; // define vertical positions
		//IMGPAR block_x_r = IMGPAR pix_x_r = IMGPAR pix_c_x_r = 0; // define horizontal positions
		IMGPAR block_y_d = IMGPAR pix_y_d = IMGPAR pix_c_y_d = 0; // define vertical positions
		IMGPAR block_x_d = IMGPAR pix_x_d = IMGPAR pix_c_x_d = 0; // define horizontal positions

		DecRefPicMarking_t *tmp_drpm;
		while (IMGPAR dec_ref_pic_marking_buffer)
		{ 
			tmp_drpm=IMGPAR dec_ref_pic_marking_buffer;

			IMGPAR dec_ref_pic_marking_buffer=tmp_drpm->Next;
			_aligned_free (tmp_drpm);
		} 
		IMGPAR dec_ref_pic_marking_buffer = NULL;

#ifdef __MB_POS_TABLE_LOOKUP__
		if(pic_width_in_mbs!=dec_picture->PicWidthInMbs || mb_pos_table_size<dec_picture->PicSizeInMbs)
		{
			int i, x, y;
			mb_pos_table_size = dec_picture->PicSizeInMbs;
			pic_width_in_mbs = dec_picture->PicWidthInMbs;
			x = y = 0;
			for (i=0;i<mb_pos_table_size;i++)
			{
				if (x == pic_width_in_mbs)
				{
					x = 0;
					y++;
				}
				mb_pos_table[i].x = x;
				mb_pos_table[i].y = y;
				x++;
			}
		}
#endif
	}

	// IoK: start_mb_nr can not be smaller than previously decoded start_mb_nr for HD-profile
	//if(currSlice->start_mb_nr < g_prev_start_mb_nr)
	//{
	//	//g_HDProfileFault = HD_ProfileFault_SLICE_first_mb_in_slice;
	//	return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
	//}
	//g_prev_start_mb_nr = currSlice->start_mb_nr;

	if (IMGPAR structure==FRAME)
	{
		if (IMGPAR direct_spatial_mv_pred_flag)
			compute_colocated_SUBMB = compute_colocated_SUBMB_frame_spatial;
		else
			compute_colocated_SUBMB = compute_colocated_SUBMB_frame_temporal;
	}
	else
	{
		if (IMGPAR direct_spatial_mv_pred_flag)
			compute_colocated_SUBMB = compute_colocated_SUBMB_field_spatial;
		else
			compute_colocated_SUBMB = compute_colocated_SUBMB_field_temporal;
	}

	// From here on, active_sps, active_pps and the slice header are valid
	if (IMGPAR MbaffFrameFlag)
		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r = currSlice->start_mb_nr << 1;
	else
		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r = currSlice->start_mb_nr;

	IMGPAR m_BitOffsetToSliceData = ((IMGPAR g_dep.Dcodestrm - IMGPAR g_dep.Dbasestrm) << 3);// - ((IMGPAR g_dep.Dbits_to_go >> 3) << 3);

	if (active_pps.entropy_coding_mode_flag)
	{
		IMGPAR m_BitOffsetToSliceData -= ((IMGPAR g_dep.Dbits_to_go >> 3) << 3); //CABAC, byte align
		dep = &(IMGPAR g_dep);
		// Byte-align
		dep->Dcodestrm -= dep->Dbits_to_go>>3;
		arideco_start_decoding ARGS0();
	}
	else
		IMGPAR m_BitOffsetToSliceData -= IMGPAR g_dep.Dbits_to_go; //CAVLC

	return CREL_OK;
}

CREL_RETURN fnDoDecodePictureReadIP PARGS0()
{
	Slice *currSlice;
	int i, j;
	int bPass;
	StreamParameters *stream_global = IMGPAR stream_global;
	CREL_RETURN ret = CREL_OK;

	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;
	IMGPAR m_start_mb_nr = IMGPAR currentSlice->start_mb_nr;
	IMGPAR error_mb_nr = -4711;
	bPass = 0;
	IMGPAR bReadCompleted = TRUE;

	for(i=0; (!IMGPAR SkipThisFrame) && (i<IMGPAR slice_number); i++)
	{

		currSlice = IMGPAR currentSlice;
		if (currSlice->picture_type == B_SLICE && (currSlice->structure!=FRAME && currSlice->m_pic_combine_status==FRAME) && currSlice->header == SOP) //put bottom field read and decode on decoding thread
		{
			//Wait for previous frame decode finished!!
			if (stream_global->m_bRefBWaitB == TRUE)
				WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);

			stream_global->m_bRefBWaitB = TRUE;
			SetEvent(stream_global->m_event_read_finish_ip);

			WaitForSingleObject(event_RB_1stfield_decode_complete, INFINITE);
			ResetEvent(event_RB_1stfield_decode_complete);


			if (stream_global->m_bSeekIDR) {
				IMGPAR bReadCompleted = FALSE;
				break;
			}

			IMGPAR currentSlice = IMGPAR firstSlice;
			for(j=0; j<i; j++)
			{
				IMGPAR prevSlice = IMGPAR currentSlice;
				IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
			}
			bPass = 1;

		}

		ret = initial_image ARGS0();
		if (FAILED(ret)) {	//Only 2 options: Error or Level1 Warning
			
			IMGPAR bReadCompleted = FALSE;
			IMGPAR error_mb_nr = IMGPAR currentSlice->start_mb_nr;

			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;		
			}
			
			break;			
		}


		DEBUG_SHOW_SW_INFO("Picture Type: %d  Structure: %d  MBAff: %d  Direct Spital Mode: %d", currSlice->picture_type, currSlice->structure, currSlice->MbaffFrameFlag, currSlice->direct_spatial_mv_pred_flag);

		if (currSlice->picture_type == B_SLICE)
		{
			if (stream_global->nNeedToDecodeIP && bPass == 0)
			{
				stream_global->m_bRefBWaitP = TRUE;
				WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
				if (stream_global->m_bSeekIDR) {
					//return CREL_OK;
					IMGPAR bReadCompleted = FALSE;
					break;
				}

				bPass = 1;
			}

			if (currSlice->header == SOP)
			{
				ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
				if (FAILED(ret)) {
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}
					break;
				}
				ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
				if (FAILED(ret)) {
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}
					break;
				}

				ret = check_lists ARGS0();
				if (FAILED(ret)) {
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}
					break;
				}

				if (IMGPAR structure == FRAME)
					init_mbaff_lists ARGS0();

				if(currSlice->direct_spatial_mv_pred_flag == 0)
					calc_mvscale ARGS0();

				IMGPAR bReadCompleted = FALSE;
			}
		}


		ret = read_slice ARGS1(currSlice->header);
		if (FAILED(ret)) {	//Only 2 options: Error or Level1 Warning
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				IMGPAR bReadCompleted = FALSE;
				break;
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;				
			} else {	
				return ret;
			}
		}


		//IMGPAR current_slice_nr++;
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
	}

	if (ISWARNINGLEVEL_1(ret) || (stream_global->m_bSeekIDR)) {
		
		IMGPAR currentSlice = IMGPAR firstSlice;
		for (i=0; i< IMGPAR slice_number; i++)
		{
			//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
			memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
			IMGPAR prevSlice = IMGPAR currentSlice;
			IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
			free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
			free_new_slice(IMGPAR prevSlice);
		}
		IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
		IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
		IMGPAR error_mb_nr = -4712;
		IMGPAR current_slice_nr = 0;
		IMGPAR slice_number = 0;

		ResetEvent(stream_global->m_event_read_start_ip);

		if (stream_global->m_bRefBWaitB) {
			SetEvent(event_RB_wait_clear);			//Future Improvement
		}
		
		return ret;

	}

	return CREL_OK;
}

CREL_RETURN fnDoDecodePictureReadB PARGS0()
{
	Slice *currSlice;
	int i;
	int prev_structure;
	StreamParameters *stream_global = IMGPAR stream_global;
	CREL_RETURN ret;

	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;
	IMGPAR m_start_mb_nr = IMGPAR currentSlice->start_mb_nr;

	prev_structure = IMGPAR currentSlice->structure;
	IMGPAR bReadCompleted = TRUE;
	for(i=0; (!IMGPAR SkipThisFrame) && (i<IMGPAR slice_number); i++)
	{
		currSlice = IMGPAR currentSlice;
		//Copy Imageparameter from Slice
		ret = initial_image ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		DEBUG_SHOW_SW_INFO("Picture Type: %d  Structure: %d  MBAff: %d  Direct Spital Mode: %d", currSlice->picture_type, currSlice->structure, currSlice->MbaffFrameFlag, currSlice->direct_spatial_mv_pred_flag);

		if (currSlice->header == SOP)
		{
			ret = init_lists ARGS2(IMGPAR type, currSlice->structure);
			if (FAILED(ret)) {
				return ret;
			}
			ret = reorder_lists ARGS2((IMGPAR type), (currSlice));
			if (FAILED(ret)) {
				return ret;
			}

			ret = check_lists ARGS0();
			if (FAILED(ret)) {
				return ret;
			}

			if (IMGPAR structure == FRAME)
				init_mbaff_lists ARGS0();

			if(currSlice->direct_spatial_mv_pred_flag == 0)
				calc_mvscale ARGS0();

			IMGPAR bReadCompleted = FALSE;
		}

		if (!((IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) && prev_structure != currSlice->structure))	
		{
			ret = read_slice ARGS1(currSlice->header);		
			if (FAILED(ret)) {
				return ret;
			}
		} else {
			return CREL_OK;
		}

		prev_structure = currSlice->structure;

		//IMGPAR current_slice_nr++;
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
	}
	return CREL_OK;
}

CREL_RETURN fnDoDecodePictureReadAndDecodeB PARGS1(int nImgID)
{
	Slice *currSlice;
	int i;
	int prev_structure;
	StreamParameters *stream_global = IMGPAR stream_global;
	CREL_RETURN ret = CREL_OK;

	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;
	IMGPAR m_start_mb_nr = IMGPAR currentSlice->start_mb_nr;
	unsigned int view_index = (IMGPAR firstSlice) ? (IMGPAR firstSlice->viewIndex) : 0;

	prev_structure = IMGPAR currentSlice->structure;
	for(i=0; (!IMGPAR SkipThisFrame) && (i<IMGPAR slice_number); i++)
	{
		currSlice = IMGPAR currentSlice;
		//Copy Imageparameter from Slice
		ret = initial_image ARGS0();
		if (FAILED(ret)) {	
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = currSlice->start_mb_nr;
			}
			break;
		}

		DEBUG_SHOW_SW_INFO( "Picture Type: %d  Structure: %d  MBAff: %d  Direct Spital Mode: %d", currSlice->picture_type, currSlice->structure, currSlice->MbaffFrameFlag, currSlice->direct_spatial_mv_pred_flag);

		if ((IMGPAR smart_dec & SMART_DEC_INT_PEL_Y) && (IMGPAR currentSlice->picture_type == B_SLICE) && !IMGPAR nal_reference_idc)
		{
			get_block_4xh  = get_block_4xh_int;
			get_block_8xh  = get_block_8xh_int;
			get_block_16xh = get_block_16xh_int;
		}
		else
		{
			get_block_4xh  = get_block_4xh_full;
			get_block_8xh  = get_block_8xh_full;
			get_block_16xh = get_block_16xh_full;
		}
		set_4xH_mc_function_ptr ARGS0();

		if ((IMGPAR smart_dec & SMART_DEC_INT_PEL_UV) && (IMGPAR currentSlice->picture_type == B_SLICE) && !IMGPAR nal_reference_idc)
		{
			mb_chroma_2xH = mb_chroma_2xH_int;
			mb_chroma_4xH = mb_chroma_4xH_int;
			mb_chroma_8xH = mb_chroma_8xH_int;
		}
		else
		{
			mb_chroma_2xH = mb_chroma_2xH_full;
			mb_chroma_4xH = mb_chroma_4xH_full;
			mb_chroma_8xH = mb_chroma_8xH_full;
		}
		set_2xH_mc_function_ptr ARGS0();

		if (currSlice->header == SOP)
			IMGPAR FP_BeginDecodeFrame ARGS0();

		if ((IMGPAR smart_dec & SMART_DEC_SKIP_FIL_B) && IMGPAR currentSlice->picture_type == B_SLICE && IMGPAR currentSlice->structure != FRAME && IMGPAR currentSlice->m_pic_combine_status == 0 && !IMGPAR nal_reference_idc)
		{
			imgpel *pImgY, *pImgUV;

			if (IMGPAR MbaffFrameFlag)
				IMGPAR current_mb_nr_d = currSlice->start_mb_nr << 1;
			else
				IMGPAR current_mb_nr_d = currSlice->start_mb_nr;

			currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
			currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];

			for(i=0; i<IMGPAR PicSizeInMbs; i++)
			{
				memset(currMB_s_d->pred_info.mv, 0, 2*BLOCK_SIZE*BLOCK_SIZE*2*sizeof(short));
				memset(currMB_s_d->pred_info.ref_idx, -1, 2*2*BLOCK_SIZE*BLOCK_SIZE/4);
				//memset(currMB_s_d->pred_info.ref_idx, -1, 2*BLOCK_SIZE*BLOCK_SIZE/4);	
				//memset(currMB_s_d->pred_info.ref_pic_id, 0xff, 2*BLOCK_SIZE*BLOCK_SIZE/4);
				IMGPAR current_mb_nr_d++;
				currMB_d++;
				currMB_s_d++;
			}

			if(IMGPAR structure == TOP_FIELD)
			{
				pImgY = IMGPAR m_dec_picture_bottom->imgY;
				pImgUV = IMGPAR m_dec_picture_bottom->imgUV;
			}
			else//(IMGPAR structure == BOTTOM_FIELD)
			{
				pImgY = IMGPAR m_dec_picture_top->imgY;
				pImgUV = IMGPAR m_dec_picture_top->imgUV;
			}
#if defined(_HW_ACCEL_)
			if(g_DXVAVer == IviNotDxva || IMGPAR Hybrid_Decoding == 5 )
#endif
			{
				for (i=0; i<dec_picture->size_y; i++)
					memcpy(dec_picture->imgY+(i*dec_picture->Y_stride), pImgY+(i*dec_picture->Y_stride), dec_picture->Y_stride>>1);
				for (i=0; i<dec_picture->size_y_cr; i++)
					memcpy(dec_picture->imgUV+(i*dec_picture->UV_stride), pImgUV+(i*dec_picture->UV_stride), dec_picture->UV_stride>>1);
			}

			if (nImgID == 0 || (g_DXVAVer && (IviDxva1!=g_DXVAVer || g_DXVAMode!=E_H264_DXVA_NVIDIA_PROPRIETARY_A)))
			{
#if defined(_HW_ACCEL_)
				if(g_DXVAVer && IMGPAR Hybrid_Decoding ==5)
					DMA_Transfer ARGS0();
#endif
				ret = exit_picture ARGS0();
				if (FAILED(ret)) {	
					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}
					break;
					
				}
			}
			break;
		}
		/*
		if ( img->framepoc == 598 ) {
		img->framepoc = img->framepoc;
		}
		*/
		if (IMGPAR error_mb_nr < 0) {
			ret = fnDoReadAndDecodeOneSlice ARGS0();

			if (FAILED(ret)) {	
				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;
					break;
				} else if (ISWARNINGLEVEL_3(ret)) {
					IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;
					break;
				} else {
					return ret;	//Error
				}
			}


			prev_structure = currSlice->structure;

			exit_slice ARGS0();
		}

#if defined(_HW_ACCEL_)
		if((!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) )) && g_DXVAVer==IviNotDxva)
#else
		if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
#endif
		{
			if ( ( IMGPAR error_mb_nr < 0 ) && (SUCCEEDED(ret)) ){
				if (IMGPAR MbaffFrameFlag)
					IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, currSlice->read_mb_nr );
				else
					IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, currSlice->read_mb_nr );
			}
		}

		if (nImgID == 0)
		{
			if(IMGPAR currentSlice->exit_flag || ISWARNINGLEVEL_3(ret))
			{
#if defined(_HW_ACCEL_)
				if(g_DXVAVer && IMGPAR Hybrid_Decoding == 5)
					DMA_Transfer ARGS0();
#endif
				ret = exit_picture ARGS0();
				if (FAILED(ret)) {	

					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}

					break;
					
				}
			}
		}
#if defined (_HW_ACCEL_)
		else if (g_DXVAVer && (IviDxva1!=g_DXVAVer || g_DXVAMode!=E_H264_DXVA_NVIDIA_PROPRIETARY_A)) //nImgID==1
		{
			if(IMGPAR currentSlice->exit_flag || ISWARNINGLEVEL_3(ret))
			{
				WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);

#if defined(_HW_ACCEL_)
				if(g_DXVAVer && IMGPAR Hybrid_Decoding == 5)
					DMA_Transfer ARGS0();						
#endif
				ret = exit_picture ARGS0();
				if (FAILED(ret)) {	

					if (ISWARNINGLEVEL_1(ret)) {
						stream_global->m_bSeekIDR = TRUE;
						nalu->buf = IMGPAR ori_nalu_buf;
					}
					break;
					
				}
			}
		}
#endif 

		//IMGPAR current_slice_nr++;
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
	}

#if defined (_HW_ACCEL_)
	if (g_DXVAVer==IviNotDxva || (IviDxva1==g_DXVAVer && g_DXVAMode==E_H264_DXVA_NVIDIA_PROPRIETARY_A))
	{
#endif
		if (nImgID == 1 && dec_picture)
		{
			if (stream_global->two_B_Flag)
			{
#if defined(_SHOW_THREAD_TIME_)
				QueryPerformanceCounter(&(stream_global->t_b[7]));
#endif
				WaitForSingleObject(stream_global->m_event_decode_finish_b[0], INFINITE);
			}

			if(!IMGPAR SkipThisFrame)
			{
				IMGPAR currentSlice = IMGPAR firstSlice;

				if (IMGPAR structure == FRAME)
				{
					if (IMGPAR Hybrid_Decoding == 5)
						DMA_Transfer ARGS0();	
					ret = exit_picture ARGS0();
					if (FAILED(ret)) {	
						if (ISWARNINGLEVEL_1(ret)) {
							stream_global->m_bSeekIDR = TRUE;
							nalu->buf = IMGPAR ori_nalu_buf;						

						}
					
					}
				}
				else
				{
					while (IMGPAR currentSlice)
					{
						if (IMGPAR currentSlice->exit_flag)
						{
							if (IMGPAR currentSlice->structure == TOP_FIELD)
							{
								dec_picture = IMGPAR m_dec_picture_top;
								IMGPAR toppoc = dec_picture->top_poc;
								IMGPAR bottompoc = dec_picture->bottom_poc;
								IMGPAR framepoc = dec_picture->frame_poc;
								IMGPAR PreviousPOC = currSlice->PreviousPOC;
								IMGPAR ThisPOC = currSlice->ThisPOC;
								if (IMGPAR Hybrid_Decoding == 5)
									DMA_Transfer ARGS0();	
								ret = exit_picture ARGS0();
								if (FAILED(ret)) {	
									if (ISWARNINGLEVEL_1(ret)) {
										stream_global->m_bSeekIDR = TRUE;
										nalu->buf = IMGPAR ori_nalu_buf;
									}

									break;
								
								}
							}
							else
							{
								dec_picture = IMGPAR m_dec_picture_bottom;
								IMGPAR toppoc = dec_picture->top_poc;
								IMGPAR bottompoc = dec_picture->bottom_poc;
								IMGPAR framepoc = dec_picture->frame_poc;
								IMGPAR PreviousPOC = currSlice->PreviousPOC;
								IMGPAR ThisPOC = currSlice->ThisPOC;
								if (IMGPAR Hybrid_Decoding == 5)
									DMA_Transfer ARGS0();	
								ret = exit_picture ARGS0();
								if (FAILED(ret)) {	
									if (ISWARNINGLEVEL_1(ret)) {
										stream_global->m_bSeekIDR = TRUE;
										nalu->buf = IMGPAR ori_nalu_buf;
									}
									break;
								
								}
							}
						}
						IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
					}
				}
			}
		}
#if defined (_HW_ACCEL_) && defined (_USE_QUEUE_FOR_DXVA2_)
	}
#endif

	if (FAILED(ret)) {

		if (IMGPAR structure == FRAME) {

			if(dec_picture)
			{
				if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->frame != dec_picture) {
					release_storable_picture ARGS2(dec_picture, 1);
				}
				dec_picture = NULL;
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;

			}		


		} else {

			if (img->m_dec_picture_top && (img->m_dec_picture_top->top_field_first)) {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->bottom_field != img->m_dec_picture_bottom) {

						for (unsigned int k = 0; k < dpb.used_size_on_view[view_index]; k++){
							if ((dpb.fs_on_view[view_index][k]->bottom_field == img->m_dec_picture_bottom) ||
								(dpb.fs_on_view[view_index][k]->top_field == img->m_dec_picture_bottom) ||
								(dpb.fs_on_view[view_index][k]->frame == img->m_dec_picture_bottom)){
									stream_global = stream_global;
							}
						}

						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
					}
					
				} else {

					if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->top_field != img->m_dec_picture_top) {
						for (unsigned int k = 0; k < dpb.used_size_on_view[view_index]; k++){
							if ((dpb.fs_on_view[view_index][k]->bottom_field == img->m_dec_picture_top) ||
								(dpb.fs_on_view[view_index][k]->top_field == img->m_dec_picture_top) ||
								(dpb.fs_on_view[view_index][k]->frame == img->m_dec_picture_top)){
									stream_global = stream_global;
							}
						}
						release_storable_picture ARGS2(img->m_dec_picture_top, 1);
					}					
					
				}
				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			} else {

				if(img->m_dec_picture_top && img->m_dec_picture_bottom)
				{
					if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->top_field != img->m_dec_picture_top) {
						for (unsigned int k = 0; k < dpb.used_size_on_view[view_index]; k++){
							if ((dpb.fs_on_view[view_index][k]->bottom_field == img->m_dec_picture_top) ||
								(dpb.fs_on_view[view_index][k]->top_field == img->m_dec_picture_top) ||
								(dpb.fs_on_view[view_index][k]->frame == img->m_dec_picture_top)){
									stream_global = stream_global;
							}
						}
						release_storable_picture ARGS2(img->m_dec_picture_top, 1);
					}
					
				} else {

					if ((dpb.used_size_on_view[view_index] == 0) ||dpb.fs_on_view[view_index][dpb.used_size_on_view[view_index] - 1]->bottom_field != img->m_dec_picture_bottom) {
						for (unsigned int k = 0; k < dpb.used_size_on_view[view_index]; k++){
							if ((dpb.fs_on_view[view_index][k]->bottom_field == img->m_dec_picture_bottom) ||
								(dpb.fs_on_view[view_index][k]->top_field == img->m_dec_picture_bottom) ||
								(dpb.fs_on_view[view_index][k]->frame == img->m_dec_picture_bottom)){
									stream_global = stream_global;
							}
						}
						release_storable_picture ARGS2(img->m_dec_picture_bottom, 1);
					}
					
				}

				img->m_dec_picture_top = NULL;
				img->m_dec_picture_bottom = NULL;


			}		


			dec_picture = NULL;

		}	


	}

	IMGPAR currentSlice = IMGPAR firstSlice;
	for (i=0; i< IMGPAR slice_number; i++)
	{
		//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
		memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
		free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
		free_new_slice(IMGPAR prevSlice);
	}
	IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
	IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
	IMGPAR error_mb_nr = -4712;
	IMGPAR current_slice_nr = 0;
	IMGPAR slice_number = 0;
	img->m_dec_picture_top = NULL;
	img->m_dec_picture_bottom = NULL;
	dec_picture = NULL;

	

	if (IMGPAR structure != FRAME)
	{
		IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
		IMGPAR cof_array = IMGPAR cof_array_ori;
	}

	return CREL_OK;
}

CREL_RETURN fnDoDecodePictureReadAndDecodeIP PARGS1(int nImgID)
{
	Slice *currSlice;
	int i;
	int prev_structure;
	StreamParameters *stream_global = IMGPAR stream_global;
	CREL_RETURN ret;

	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;
	IMGPAR m_start_mb_nr = IMGPAR currentSlice->start_mb_nr;

	prev_structure = IMGPAR currentSlice->structure;
	for(i=0; (!IMGPAR SkipThisFrame) && (i<IMGPAR slice_number); i++)
	{
		currSlice = IMGPAR currentSlice;
		//Copy Imageparameter from Slice
		ret = initial_image ARGS0();
		if (FAILED(ret)) {
			return ret;
		}


		if (IMGPAR structure == TOP_FIELD)
		{
			dec_picture = IMGPAR m_dec_picture_top;
			IMGPAR mb_decdata = IMGPAR mb_decdata_top;
			IMGPAR cof_array = IMGPAR cof_array_top;

			IMGPAR toppoc = dec_picture->top_poc;
			IMGPAR bottompoc = dec_picture->bottom_poc;
			IMGPAR framepoc = dec_picture->frame_poc;
			IMGPAR PreviousPOC = currSlice->PreviousPOC;
			IMGPAR ThisPOC = currSlice->ThisPOC;
		}
		else if (IMGPAR structure == BOTTOM_FIELD)
		{
			dec_picture = IMGPAR m_dec_picture_bottom;
			IMGPAR mb_decdata = IMGPAR mb_decdata_bottom;				
			IMGPAR cof_array = IMGPAR cof_array_bottom;

			IMGPAR toppoc = dec_picture->top_poc;
			IMGPAR bottompoc = dec_picture->bottom_poc;
			IMGPAR framepoc = dec_picture->frame_poc;
			IMGPAR PreviousPOC = currSlice->PreviousPOC;
			IMGPAR ThisPOC = currSlice->ThisPOC;

		}

		DEBUG_SHOW_SW_INFO( "Picture Type: %d  Structure: %d  MBAff: %d  Direct Spital Mode: %d", currSlice->picture_type, currSlice->structure, currSlice->MbaffFrameFlag, currSlice->direct_spatial_mv_pred_flag);

		if (currSlice->header == SOP)
			IMGPAR FP_BeginDecodeFrame ARGS0();

		ret = fnDoReadAndDecodeOneSlice ARGS0();

		prev_structure = currSlice->structure;

		exit_slice ARGS0();

#if defined(_HW_ACCEL_)
		if((!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) )) && g_DXVAVer==IviNotDxva)
#else
		if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (currSlice->picture_type == B_SLICE)) ))
#endif
		{
			if (IMGPAR MbaffFrameFlag)
				IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr<<1, currSlice->read_mb_nr );
			else
				IMGPAR FP_DeblockSlice ARGS3(dec_picture, currSlice->start_mb_nr, currSlice->read_mb_nr );
		}

		if(IMGPAR currentSlice->exit_flag)
		{
			WaitForSingleObject(stream_global->m_event_for_field_ip, INFINITE);
#if defined(_HW_ACCEL_)
			if(g_DXVAVer && IMGPAR Hybrid_Decoding)
			{
				if(IMGPAR Hybrid_Decoding == 5 || (IMGPAR Hybrid_Decoding == 2 && IMGPAR type != B_SLICE) || IMGPAR type == I_SLICE)
					DMA_Transfer ARGS0();
			}
#endif
			ret = exit_picture ARGS0();
			if (FAILED(ret)) {
				return ret;
			}
		}		

		//IMGPAR current_slice_nr++;
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
	}
	ResetEvent(stream_global->m_event_for_field_ip);

	IMGPAR currentSlice = IMGPAR firstSlice;
	for (i=0; i< IMGPAR slice_number; i++)
	{
		//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
		memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
		free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
		free_new_slice(IMGPAR prevSlice);
	}
	IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
	IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
	IMGPAR error_mb_nr = -4712;
	IMGPAR current_slice_nr = 0;
	IMGPAR slice_number = 0;

	if (IMGPAR structure != FRAME)
	{
		IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
		IMGPAR cof_array = IMGPAR cof_array_ori;
	}

	return ret;
}

unsigned __stdcall decode_picture_read_ip (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img;// = (img_par *)parameters;
	CREL_RETURN ret = CREL_OK;

	while (1)
	{
		WaitForSingleObject(stream_global->m_event_read_start_ip, INFINITE);
		ResetEvent(stream_global->m_event_read_start_ip);
		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_picture_read_ip");

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&t_ip[0]);
#endif

		img = gImgIP_r;
#if !defined (IP_RD_MERGE)
		if ((stream_global->m_bRefBWaitB == FALSE) &&(stream_global->m_bSeekIDR == FALSE)){		//Future Improvment
		//if (stream_global->m_bRefBWaitB == FALSE) {
			//img = gImgIP_r;		//Future Improvment
			ret = fnDoDecodePictureReadIP ARGS0();
		}
#endif		

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&t_ip[1]);
#endif

		if (stream_global->m_bRefBWaitBRead==TRUE && stream_global->m_bRefBWaitB==TRUE)
		{
			SetEvent(event_RB_2ndfield_read_complete);
			stream_global->m_bRefBWaitBRead = FALSE;
			stream_global->m_bRefBWaitB = FALSE;
			WaitForSingleObject(stream_global->m_event_decode_finish_ip, INFINITE);
		}
		else
			SetEvent(stream_global->m_event_read_finish_ip);

		stream_global->m_bRefBWaitP = FALSE;

		if (ISERROR(ret)) {
			break;
		}

		DEBUG_SHOW_SW_INFO("End decode_picture_read_ip");
	}//end of while(1)

	SetEvent(stream_global->m_event_read_finish_ip);
	_endthreadex(0);

	return(1);
}
unsigned __stdcall decode_picture_read_b0 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img;// = (img_par *)parameters;
	CREL_RETURN ret = CREL_OK;

	while (1)
	{
		WaitForSingleObject(stream_global->m_event_read_start_b[0], INFINITE);
		ResetEvent(stream_global->m_event_read_start_b[0]);
		if (g_event_exit_flag)		
			break;

		DEBUG_SHOW_SW_INFO("Start decode_picture_read_b0");

		/*
		if ( stream_global->b_Count == 0 ) {
		stream_global->b_Count = stream_global->b_Count;			
		}
		*/
#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&t_b[0]);
#endif

		img = gImgB_r0;

		/*
		if ( img->currentSlice == 0 ) {
		stream_global->b_Count = stream_global->b_Count;			
		}
		*/
		if (stream_global->m_bSeekIDR == FALSE) {
#if 0
			ret = fnDoDecodePictureReadB ARGS0();

			//gImgB_d0 = img;
			//SetEvent(stream_global->m_event_decode_start_b[0]);
			ret = fnDoDecodePictureDecodeB01 ARGS1(0);
#else
			ret = fnDoDecodePictureReadAndDecodeB ARGS1(0);
#endif
		}

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&t_b[1]);
#endif

		SetEvent(stream_global->m_event_decode_finish_b[0]);

		if (ISERROR(ret)) {
			break;
		}

		DEBUG_SHOW_SW_INFO("End decode_picture_read_b0");
	}//end of while(1)

	SetEvent(stream_global->m_event_read_finish_b[0]);
	_endthreadex(0);
	return(1);
}

unsigned __stdcall decode_picture_read_b1 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img;// = (img_par *)parameters;
	CREL_RETURN ret = CREL_OK;

	while (1)
	{
		WaitForSingleObject(stream_global->m_event_read_start_b[1], INFINITE);
		ResetEvent(stream_global->m_event_read_start_b[1]);
		/*
		if ( stream_global->b_Count == 0 ) {
		stream_global->b_Count = stream_global->b_Count;			
		}
		*/
		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_picture_read_b1");

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&t_b[2]);
#endif

		img = gImgB_r1;

		if (stream_global->m_bSeekIDR == FALSE) {
/*
			if ( img->currentSlice == 0 ) {
				stream_global->b_Count = stream_global->b_Count;			
			}
*/
#if 0
			ret = fnDoDecodePictureReadB ARGS0();

			//gImgB_d1 = img;
			//SetEvent(stream_global->m_event_decode_start_b[1]);
			ret = fnDoDecodePictureDecodeB01 ARGS1(1);
#else
/*
			if (img->framepoc == 10710 ) {
				img->number = img->number;
			}

			if (img->stream_global->number == 5209 ) {
				img->number = img->number;
			}
*/
			ret = fnDoDecodePictureReadAndDecodeB ARGS1(1);
#endif

		}

#if defined(_SHOW_THREAD_TIME_)
		QueryPerformanceCounter(&t_b[3]);
#endif

		SetEvent(stream_global->m_event_decode_finish_b[1]);

		if (ISERROR(ret)) {
			break;
		}

		DEBUG_SHOW_SW_INFO("End decode_picture_read_b1");
	}//end of while(1)

	SetEvent(stream_global->m_event_read_finish_b[1]);
	_endthreadex(0);
	return(1);
}
#endif

#if defined (_COLLECT_PIC_)
// Below functions are used for Reference-B
unsigned __stdcall decode_thread(void *parameter)
{
	img_par *img;// = (img_par *)parameter;
	stream_par *stream_global = (stream_par *)parameter;
	int nDecodedMB, nWillDecodeMB, MbaffFrameFlag, list_offset;
	bool de_blocking_flag;
	CREL_RETURN ret = CREL_OK;

	while(1)
	{		
		WaitForSingleObject(stream_global->m_event_begin_decode_thread, INFINITE);
		ResetEvent(stream_global->m_event_begin_decode_thread);
		if (g_event_exit_flag)
			break;

		img = img_array[6];

		IMGPAR start_mb_x = 100000; //set a big number
		MbaffFrameFlag = IMGPAR currentSlice->MbaffFrameFlag;

		if (MbaffFrameFlag)
			IMGPAR current_mb_nr_d = IMGPAR currentSlice->start_mb_nr << 1;
		else
			IMGPAR current_mb_nr_d = IMGPAR currentSlice->start_mb_nr;

		currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
		currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];

		nDecodedMB = 0;
		WaitForSingleObject(stream_global->m_event_decode_thread, INFINITE);
		ResetEvent(stream_global->m_event_decode_thread);
		while(1)
		{
			if ((nDecodedMB+IMGPAR PicWidthInMbs) <= IMGPAR currentSlice->read_mb_nr)
			{
				nWillDecodeMB = IMGPAR PicWidthInMbs;

				while(nWillDecodeMB)
				{
#if !defined(ONE_COF)
					IMGPAR cof_d = (IMGPAR cof_array + IMGPAR current_mb_nr_d*384);
#endif
					int remainder = IMGPAR current_mb_nr_d;
					if (MbaffFrameFlag)
					{
						IMGPAR mb_x_d = mb_pos_table[remainder>>1].x;
						IMGPAR mb_y_d = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);
					}
					else
					{
						IMGPAR mb_x_d = (((int)mb_pos_table[remainder].x));   
						IMGPAR mb_y_d = (((int)mb_pos_table[remainder].y));
					}					

					/* Define vertical positions */
					IMGPAR block_y_d = IMGPAR mb_y_d * BLOCK_SIZE;      /* luma block position */
					IMGPAR pix_y_d   = IMGPAR mb_y_d * MB_BLOCK_SIZE;   /* luma macroblock position */
					IMGPAR pix_c_y_d = IMGPAR mb_y_d * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

					/* Define horizontal positions */
					IMGPAR block_x_d = IMGPAR mb_x_d * BLOCK_SIZE;      /* luma block position */
					IMGPAR pix_x_d   = IMGPAR mb_x_d * MB_BLOCK_SIZE;   /* luma pixel position */
					IMGPAR pix_c_x_d = IMGPAR mb_x_d * (MB_BLOCK_SIZE/2); /* chroma pixel position */

					if(IMGPAR mb_x_d < IMGPAR start_mb_x)
						IMGPAR start_mb_x = IMGPAR mb_x_d;

					if(IMGPAR MbaffFrameFlag)
					{
						IMGPAR pix_y_rows = IMGPAR pix_y_d&63;		
						IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&31;
					}
					else
					{
						IMGPAR pix_y_rows = IMGPAR pix_y_d&31;		
						IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&15;
					}

					list_offset = (MbaffFrameFlag&&(currMB_s_d->mb_field))? (nDecodedMB&1) ? 4 : 2 : 0;

					if (currMB_s_d->do_record == 1) {
						ret = record_reference_picIds ARGS2(list_offset, currMB_s_d);
						if (FAILED(ret)) {		// Error or WarningLevel_1

							if (ISWARNINGLEVEL_1(ret)) {
								stream_global->m_bSeekIDR = TRUE;
								nalu->buf = IMGPAR ori_nalu_buf;
								break;
							} else {
								return ret;
							}
						}
					}

					if (IS_COPY(currMB_d))
						memset(&(currMB_s_d->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);

					if (!currMB_d->luma_transform_size_8x8_flag)
						IMGPAR FP_MB_itrans_Luma = MB_itrans4x4_Luma;
					else
						IMGPAR FP_MB_itrans_Luma = MB_itrans8x8_Luma;

					if(IMGPAR type == I_SLICE)
						ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
					else if(IMGPAR type == P_SLICE)
					{
						if(IS_INTRA(currMB_d))
							ret = IMGPAR FP_decode_one_macroblock_P_Intra ARGS0();
						else 
							ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
					}
					else
					{
						if(IS_INTRA(currMB_d))
							ret = IMGPAR FP_decode_one_macroblock_B_Intra ARGS0();
						else
							ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
					}

					if (FAILED(ret)) {
						break;
					}

					nDecodedMB++;
					nWillDecodeMB--;
					//IMGPAR current_mb_nr_d = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_d));
					IMGPAR current_mb_nr_d++;
					currMB_d++;
					currMB_s_d++;
				}

				if ( (IMGPAR mb_x_d+ 1) != IMGPAR PicWidthInMbs ) {
					IMGPAR FP_TransferData_at_SliceEnd ARGS0();
				}
			}
			else
			{
				if ((nDecodedMB == IMGPAR currentSlice->read_mb_nr) && 
					((IMGPAR currentSlice->start_mb_nr+nDecodedMB) == IMGPAR PicSizeInMbs))
				{
					ResetEvent(stream_global->m_event_decode_thread);
					break;
				}

				WaitForSingleObject(stream_global->m_event_decode_thread, INFINITE);
				ResetEvent(stream_global->m_event_decode_thread);
			}
		}

		exit_slice ARGS0();

		if(!( (IMGPAR smart_dec & SMART_DEC_NODEBLK_ALL) || ((IMGPAR smart_dec & SMART_DEC_NODEBLK_B) && (IMGPAR currentSlice->picture_type == B_SLICE)) ))
			de_blocking_flag = TRUE;
		else
			de_blocking_flag = FALSE;
#if defined(_HW_ACCEL_)
		if(de_blocking_flag && g_DXVAVer==IviNotDxva && ( IMGPAR error_mb_nr < 0 ) && (nWillDecodeMB == 0) )
#else
		if(de_blocking_flag && ( IMGPAR error_mb_nr < 0 ) && (nWillDecodeMB == 0))
#endif
		{
			IMGPAR FP_DeblockSlice = DeblockSlice;

			if (IMGPAR MbaffFrameFlag)
				IMGPAR FP_DeblockSlice ARGS3(dec_picture, IMGPAR currentSlice->start_mb_nr<<1, IMGPAR currentSlice->read_mb_nr);
			else
				IMGPAR FP_DeblockSlice ARGS3(dec_picture, IMGPAR currentSlice->start_mb_nr, IMGPAR currentSlice->read_mb_nr);
		}

		if(IMGPAR currentSlice->exit_flag)
		{
#if defined(_HW_ACCEL_)
			if(g_DXVAVer && IMGPAR Hybrid_Decoding)
			{
				if(IMGPAR currentSlice->m_pic_combine_status == 0)
				{
					if(IMGPAR Hybrid_Decoding == 5 || (IMGPAR Hybrid_Decoding == 2 && IMGPAR type != B_SLICE) || IMGPAR type == I_SLICE)
						DMA_Transfer ARGS0();
				}
			}
#endif
			ret = exit_picture ARGS0();			
		}

		SetEvent(stream_global->m_event_finish_decode_thread);
	}
	SetEvent(stream_global->m_event_finish_decode_thread);
	_endthreadex(0);

	return 0;
}

CREL_RETURN decode_slice_for_RB PARGS1(int header)
{
	BOOL end_of_slice = FALSE;
	int nImgType, nEntropyMode;
	Slice *currSlice = IMGPAR currentSlice;
	CREL_RETURN ret;
	StreamParameters *stream_global = IMGPAR stream_global;

	if (active_pps.entropy_coding_mode_flag)
	{
		init_contexts ARGS0();
		cabac_new_slice ARGS0();
	}

	if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
		fill_wp_params ARGS0();

	// decode main slice information
	if ((header == SOP || header == SOS) && currSlice->ei_flag == 0)
	{
		IMGPAR cod_counter=-1;

		if(IMGPAR type==B_SLICE)
			SelectB8Mode_B ARGS0();
		else if(IMGPAR type==P_SLICE)
			SelectB8Mode_P ARGS0();

#ifdef DO_REF_PIC_NUM
		set_ref_pic_num();
#endif
		currSlice->read_mb_nr = 0;


		SetEvent(stream_global->m_event_begin_decode_thread);
		nImgType = IMGPAR type;
		nEntropyMode = active_pps.entropy_coding_mode_flag;
		while (end_of_slice == FALSE) // loop over macroblocks
		{
			// YC: This was previously performed inside getNeighbour
			if (IMGPAR current_mb_nr_r<0) {
				DEBUG_SHOW_ERROR_INFO ("[ERROR]decode_one_slice: invalid macroblock number", 100);
				return CREL_WARNING_H264_STREAMERROR_LEVEL_1;
			}
			// Initializes the current macroblock
			ret = start_macroblock ARGS0();
			if (FAILED(ret)) {
				return ret;
			}

			// Get the syntax elements from the NAL
			//read_flag = read_one_macroblock ARGS0();
			ret = pf_read_one_macroblock[nEntropyMode][nImgType][IMGPAR current_mb_nr_r&1] ARGS0();

			if(IMGPAR MbaffFrameFlag && currMB_r->mb_field)
			{
				IMGPAR num_ref_idx_l0_active >>= 1;
				IMGPAR num_ref_idx_l1_active >>= 1;
			}

#ifdef ENABLE_ERROR_RESILIENCE
			ercWriteMBMODEandMV(img,inp);
#endif

			currSlice->read_mb_nr++;
			ret = exit_macroblock ARGS2((!IMGPAR MbaffFrameFlag||(IMGPAR current_mb_nr_r&1)), &end_of_slice);
			if (FAILED(ret)) {
				return ret;
			}
#ifdef _SLEEP_FOR_AUDIO_
			if (IMGPAR current_mb_nr_r % 2040 == 0)
				Sleep(0);
#endif

			if ((currSlice->read_mb_nr % IMGPAR PicWidthInMbs) == 0)
				SetEvent(stream_global->m_event_decode_thread);
		}
	}

	return CREL_OK;
}

CREL_RETURN fnDoDecodePictureReferenceB PARGS0()
{
	Slice *currSlice;
	int i;
	CREL_RETURN ret;
	StreamParameters *stream_global = IMGPAR stream_global;


	IMGPAR currentSlice = IMGPAR firstSlice;
	IMGPAR prevSlice    = NULL;
	IMGPAR current_slice_nr = 0;
	IMGPAR m_start_mb_nr = IMGPAR currentSlice->start_mb_nr;
	for(i=0; (!IMGPAR SkipThisFrame) && (i<IMGPAR slice_number); i++)
	{
		currSlice = IMGPAR currentSlice;
		//Copy Imageparameter from Slice
		ret = initial_image ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		if (currSlice->header == SOP)
		{
			IMGPAR FP_BeginDecodeFrame ARGS0();

			ret = init_lists ARGS2(IMGPAR type, currSlice->structure);
			if (FAILED(ret)) {
				return ret;
			}
			ret = reorder_lists ARGS2((IMGPAR type), (currSlice));
			if (FAILED(ret)) {
				return ret;
			}
			free_ref_pic_list_reordering_buffer(currSlice);

			ret = check_lists ARGS0();
			if (FAILED(ret)) {
				return ret;
			}

			if (IMGPAR structure == FRAME)
				init_mbaff_lists ARGS0();

			if(currSlice->direct_spatial_mv_pred_flag == 0)
				calc_mvscale ARGS0();
		}

		ret = decode_slice_for_RB ARGS1(currSlice->header);		

		WaitForSingleObject(stream_global->m_event_finish_decode_thread, INFINITE);
		ResetEvent(stream_global->m_event_finish_decode_thread);

		//IMGPAR current_slice_nr++;
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) (IMGPAR currentSlice->nextSlice);
	}

	IMGPAR currentSlice = IMGPAR firstSlice;
	for (i=0; i< IMGPAR slice_number; i++)
	{
		//active_pps = &PicParSet[IMGPAR currentSlice->pic_parameter_set_id];
		memcpy(&active_pps, &PicParSet[IMGPAR currentSlice->pic_parameter_set_id], sizeof(pic_parameter_set_rbsp_t));
		IMGPAR prevSlice = IMGPAR currentSlice;
		IMGPAR currentSlice = (Slice*) IMGPAR currentSlice->nextSlice;
		free_ref_pic_list_reordering_buffer(IMGPAR prevSlice);
		free_new_slice(IMGPAR prevSlice);
	}
	IMGPAR firstSlice = IMGPAR currentSlice = IMGPAR prevSlice = NULL;
	IMGPAR current_mb_nr_r = IMGPAR current_mb_nr_d = -4712;   // impossible value for debugging, StW
	IMGPAR current_slice_nr = 0;
	IMGPAR slice_number = 0;

	if (IMGPAR structure != FRAME)
	{
		IMGPAR mb_decdata = IMGPAR mb_decdata_ori;
		IMGPAR cof_array = IMGPAR cof_array_ori;
	}
	return CREL_OK;
}

unsigned __stdcall decode_picture_RB (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img = img_array[6];// = (img_par *)parameters;
	CREL_RETURN ret;

	while (1)
	{
		WaitForSingleObject(stream_global->m_event_start_pic_RB, INFINITE);
		ResetEvent(stream_global->m_event_start_pic_RB);
		if (g_event_exit_flag)
			break;

		ret = fnDoDecodePictureReferenceB ARGS0();

		SetEvent(stream_global->m_event_finish_pic_RB);
	}//end of while(1)

	SetEvent(stream_global->m_event_finish_pic_RB);
	_endthreadex(0);
	return(1);
}

CREL_RETURN read_decode_one_slice PARGS0()		//Used in MTMS = 1 or ( MTMS = 0 && B_SLICE )
{
	BOOL end_of_slice = FALSE, bottom_mbaff_flag = FALSE;
	int nImgType, nEntropyMode, MbaffFrameFlag, list_offset;
	CREL_RETURN ret;
	Slice *currSlice = IMGPAR currentSlice;
	Slice *nextSlice = (Slice*)IMGPAR currentSlice->nextSlice;
	StreamParameters *stream_global = IMGPAR stream_global;
	IMGPAR start_mb_x = 100000; //set a big number

	IMGPAR cod_counter=-1;
	
	if(IMGPAR type==B_SLICE)
		SelectB8Mode_B ARGS0();
	else if(IMGPAR type==P_SLICE)
		SelectB8Mode_P ARGS0();

#ifdef DO_REF_PIC_NUM
	set_ref_pic_num();
#endif

	MbaffFrameFlag = IMGPAR MbaffFrameFlag;

	IMGPAR num_dec_mb = 0;

	if(MbaffFrameFlag)
	{
		if(nextSlice != NULL && currSlice->structure == nextSlice->structure)
			currSlice->read_mb_nr = (nextSlice->start_mb_nr - currSlice->start_mb_nr)<<1;
		else
			currSlice->read_mb_nr = ((IMGPAR FrameSizeInMbs>>1) - currSlice->start_mb_nr)<<1;			
	}
	else
	{
		if(nextSlice != NULL && currSlice->structure == nextSlice->structure)
			currSlice->read_mb_nr = nextSlice->start_mb_nr - currSlice->start_mb_nr;
		else
			currSlice->read_mb_nr = (IMGPAR structure ==0 ? IMGPAR FrameSizeInMbs : (IMGPAR FrameSizeInMbs>>1)) - currSlice->start_mb_nr;
	}
	nImgType = IMGPAR type;
	nEntropyMode = active_pps.entropy_coding_mode_flag;

#ifdef ANALYZE_CODE_WRITE_DATA_OUT

	static int count = 0;  
	static unsigned int total_intra_bits = 0;
	static unsigned int total_inter_bits = 0;
	static unsigned int total_skip_bits = 0;

	FILE *outf;
	outf = fopen("inter_intra_analysis.txt","a");

	if(nImgType == P_SLICE)
	{
		fprintf(outf, "\n\nFramenumber%d", IMGPAR number);
		fprintf(outf, "{");
	}

#endif

#ifdef ANALYZE_CODE_READ_DATA_IN
	FILE *outf;
	outf = fopen("inter_intra_analysis.txt","r");
	fseek(outf, 0, SEEK_END);
	long FileSize = ftell(outf);
	char *buf_txt = (char*)malloc(FileSize + 1);
	fseek(outf, 0, SEEK_SET);
	fread(buf_txt, 1, FileSize, outf);
	fclose(outf);

	//FILE *result;
	//result = fopen("analyze_result.txt","a");

	static int count=0;
	int n;        // total mb numbers to test in a frame, n = -1 if there is no mb to test
	char tmp_string[512];
	
	int test_mb[8160];
	char frame_num[512] = "Framenumber";
	char a[16];
	static unsigned int total_intra_bits = 0;
	static unsigned int total_inter_bits = 0;
	static unsigned int total_skip_bits = 0;

	memset(test_mb, -1, (IMGPAR FrameSizeInMbs)*sizeof(int));
	
	
	itoa((IMGPAR number), a, 10);
	strcat(frame_num, a);
	if(currSlice->start_mb_nr == 0)
		n = get_mb_bits_data(buf_txt, frame_num, tmp_string, test_mb);
	//if(n != -1 && currSlice->start_mb_nr == 0)
	//	fprintf(result, "Frame Number: %d, Type: %d\n", IMGPAR number, IMGPAR type);

#endif

	while (end_of_slice == FALSE) // loop over macroblocks
	{
		ret = start_macroblock ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
	
#ifdef ANALYZE_CODE_WRITE_DATA_OUT
		unsigned int current_pointer = (unsigned int)IMGPAR g_dep.Dcodestrm;
		unsigned int current_bits_to_go = IMGPAR g_dep.Dbits_to_go;
				
						
		unsigned int next_pointer;
		unsigned int next_bits_to_go;
		int diff;
		
#endif
#ifdef ANALYZE_CODE_READ_DATA_IN
		unsigned int current_pointer = (unsigned int)IMGPAR g_dep.Dcodestrm;
		unsigned int current_bits_to_go = IMGPAR g_dep.Dbits_to_go;
				
						
		unsigned int next_pointer;
		unsigned int next_bits_to_go;
		int diff;
		int choose_flag=0;
		int end_flag = 1;
#endif

		
		ret = pf_read_one_macroblock[nEntropyMode][nImgType][IMGPAR current_mb_nr_r&1] ARGS0();
		if (FAILED(ret)) {
			return ret;
		}


#ifdef ANALYZE_CODE_WRITE_DATA_OUT

		next_pointer = (unsigned int)IMGPAR g_dep.Dcodestrm;
		next_bits_to_go = IMGPAR g_dep.Dbits_to_go;

		diff = (int)next_pointer - current_pointer;
		diff = (int)((diff << 3) - next_bits_to_go + current_bits_to_go);
		
		/********************************************/
		/* Choose desired picture type to test here*/
		/*******************************************/
		if(nImgType == P_SLICE)
		{
			if(currMB_r->mb_type == I4MB || currMB_r->mb_type == I8MB || currMB_r->mb_type == I16MB || currMB_r->mb_type == IPCM)  // intra block
			{
				total_intra_bits += diff;
				//fprintf(outf, "%d,", (int)(IMGPAR current_mb_nr_r));
			}
			else if(currMB_r->mb_type == P_SKIP || currMB_r->mb_type == B_DIRECT)  // skip/direct block
			{
				total_skip_bits += diff;
			}
			else // inter block
			{
				total_inter_bits += diff;
				//fprintf(outf, "%d,", (int)(IMGPAR current_mb_nr_r));
			}
			
		}

#endif
#ifdef ANALYZE_CODE_READ_DATA_IN
		next_pointer = (unsigned int)IMGPAR g_dep.Dcodestrm;
		next_bits_to_go = IMGPAR g_dep.Dbits_to_go;

		diff = (int)next_pointer - current_pointer;
		diff = (int)((diff << 3) - next_bits_to_go + current_bits_to_go);
		
		int i = 0;
		while(end_flag && i<(IMGPAR FrameSizeInMbs))
		{
			if(test_mb[i] == (IMGPAR current_mb_nr_d))
			{
				choose_flag = 1;
				end_flag = 0;
			}
			i++;
		}
		if(choose_flag && (nImgType == P_SLICE))
		{
			if(currMB_r->mb_type == I4MB || currMB_r->mb_type == I8MB || currMB_r->mb_type == I16MB || currMB_r->mb_type == IPCM)  // intra block
			{
				total_intra_bits += diff;
			}
			else if(currMB_r->mb_type == P_SKIP || currMB_r->mb_type == B_DIRECT)  // skip/direct block
			{
				total_skip_bits += diff;
			}
			else // inter block
			{
				total_inter_bits += diff;
			}
			
		}

#endif

		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r;

#if !defined(ONE_COF)
		//IMGPAR cof_d = (IMGPAR cof_array + IMGPAR current_mb_nr_d*384);
#endif
		int remainder = IMGPAR current_mb_nr_d;
		if (MbaffFrameFlag)
		{
			IMGPAR mb_x_d = mb_pos_table[remainder>>1].x;
			IMGPAR mb_y_d = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);
		}
		else
		{
			IMGPAR mb_x_d = (((int)mb_pos_table[remainder].x));   
			IMGPAR mb_y_d = (((int)mb_pos_table[remainder].y));
		}

		/* Define vertical positions */
		IMGPAR block_y_d = IMGPAR mb_y_d * BLOCK_SIZE;      /* luma block position */
		IMGPAR pix_y_d   = IMGPAR mb_y_d * MB_BLOCK_SIZE;   /* luma macroblock position */
		IMGPAR pix_c_y_d = IMGPAR mb_y_d * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

		/* Define horizontal positions */
		IMGPAR block_x_d = IMGPAR mb_x_d * BLOCK_SIZE;      /* luma block position */
		IMGPAR pix_x_d   = IMGPAR mb_x_d * MB_BLOCK_SIZE;   /* luma pixel position */
		IMGPAR pix_c_x_d = IMGPAR mb_x_d * (MB_BLOCK_SIZE/2); /* chroma pixel position */

		if(IMGPAR mb_x_d < IMGPAR start_mb_x)
			IMGPAR start_mb_x = IMGPAR mb_x_d;

		if(IMGPAR MbaffFrameFlag)
		{
			IMGPAR pix_y_rows = IMGPAR pix_y_d&63;		
			IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&31;
		}
		else
		{
			IMGPAR pix_y_rows = IMGPAR pix_y_d&31;		
			IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&15;
		}

		list_offset = (MbaffFrameFlag&&(currMB_s_d->mb_field))? (IMGPAR current_mb_nr_d&1) ? 4 : 2 : 0;

		if (currMB_s_d->do_record == 1) {
			ret = record_reference_picIds ARGS2(list_offset, currMB_s_d);
			if (FAILED(ret)) {		// Error or WarningLevel_1

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;					
				} 

				return ret;

			}
		}

		if (IS_COPY(currMB_d))
			memset(&(currMB_s_d->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);

		if (!currMB_d->luma_transform_size_8x8_flag)
			IMGPAR FP_MB_itrans_Luma = MB_itrans4x4_Luma;
		else
			IMGPAR FP_MB_itrans_Luma = MB_itrans8x8_Luma;

		if(IMGPAR type == I_SLICE)
			ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
		else if(IMGPAR type == P_SLICE)
		{
			if(IS_INTRA(currMB_d))
				ret = IMGPAR FP_decode_one_macroblock_P_Intra ARGS0();
			else 
				ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
		}
		else
		{
			if(IS_INTRA(currMB_d))
				ret = IMGPAR FP_decode_one_macroblock_B_Intra ARGS0();
			else
				ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
		}

		if (FAILED(ret)) {
			return ret;
		}

		if(MbaffFrameFlag && currMB_d->mb_field)
		{
			IMGPAR num_ref_idx_l0_active >>= 1;
			IMGPAR num_ref_idx_l1_active >>= 1;
		}

#ifdef ENABLE_ERROR_RESILIENCE
		ercWriteMBMODEandMV(img,inp);
#endif

		ret = exit_macroblock ARGS2((!MbaffFrameFlag||(IMGPAR current_mb_nr_r&1)), &end_of_slice);
		if((IMGPAR stream_global)->uiH264DXVAMode==E_H264_DXVA_MODE_C)  //In Intel Mode C, decoder doesn't execute iDCT, so IMGPAR cof_array should be reset.
			memset(IMGPAR cof_r, 0, 768);  //We could need to add this memset to IPRD_Merge, if IPRD_Merge is used, and Reference B.		
		if (FAILED(ret)) {
			return ret;
		}

		IMGPAR current_mb_nr_d++;
	}

#ifdef ANALYZE_CODE_WRITE_DATA_OUT
		count++; 
		if(nImgType == P_SLICE)
			fprintf(outf, "-1}");
		fclose(outf);
		printf("total intra: %d\n", total_intra_bits);
		printf("total inter: %d\n", total_inter_bits);
		printf("total skip: %d\n", total_skip_bits);

#endif
#ifdef ANALYZE_CODE_READ_DATA_IN
		count++;
		if(n != -1 && currSlice->start_mb_nr == 0)
		{
			printf("Total intra block bits: %d \n",total_intra_bits);
			printf("Total inter block bits: %d \n",total_inter_bits);
			printf("Total skip block bits: %d \n",total_skip_bits);
			//IMGPAR total_bits_intra = 0;
		}
		//fclose(result);
#endif

	if ( (IMGPAR mb_x_d+ 1) != IMGPAR PicWidthInMbs ) {
		IMGPAR FP_TransferData_at_SliceEnd ARGS0();
	}

	return CREL_OK;
}

CREL_RETURN read_decode_one_slice_FMO PARGS0()		//Used in MTMS = 1 or ( MTMS = 0 && B_SLICE )
{
	BOOL end_of_slice = FALSE, bottom_mbaff_flag = FALSE;
	int nImgType, nEntropyMode, MbaffFrameFlag, list_offset;
	CREL_RETURN ret;
	Slice *currSlice = IMGPAR currentSlice;
	Slice *nextSlice = (Slice*)IMGPAR currentSlice->nextSlice;
	StreamParameters *stream_global = IMGPAR stream_global;
	IMGPAR start_mb_x = 100000; //set a big number

	IMGPAR cod_counter=-1;

	if(IMGPAR type==B_SLICE)
		SelectB8Mode_B ARGS0();
	else if(IMGPAR type==P_SLICE)
		SelectB8Mode_P ARGS0();

#ifdef DO_REF_PIC_NUM
	set_ref_pic_num();
#endif

	MbaffFrameFlag = IMGPAR MbaffFrameFlag;

	IMGPAR num_dec_mb = 0;

	if(MbaffFrameFlag)
	{
		if(nextSlice != NULL && currSlice->structure == nextSlice->structure)
			currSlice->read_mb_nr = (nextSlice->start_mb_nr - currSlice->start_mb_nr)<<1;
		else
			currSlice->read_mb_nr = ((IMGPAR FrameSizeInMbs>>1) - currSlice->start_mb_nr)<<1;			
	}
	else
	{
		//if(nextSlice != NULL && currSlice->structure == nextSlice->structure)
		//	currSlice->read_mb_nr = nextSlice->start_mb_nr - currSlice->start_mb_nr;
		//else
		//	currSlice->read_mb_nr = (IMGPAR structure ==0 ? IMGPAR FrameSizeInMbs : (IMGPAR FrameSizeInMbs>>1)) - currSlice->start_mb_nr;

		currSlice->read_mb_nr = 0;
	}
	nImgType = IMGPAR type;
	nEntropyMode = active_pps.entropy_coding_mode_flag;

#ifdef COMPRESS_IDCT
	IMGPAR cof_r = (IMGPAR cof_array + currSlice->start_mb_nr*384);
#endif	

	memset(IMGPAR decoded_flag, 0, IMGPAR FrameSizeInMbs);

	while (end_of_slice == FALSE) // loop over macroblocks
	{
		ret = start_macroblock ARGS0();
		if (FAILED(ret)) {
			return ret;
		}
		/*
		if (( img->framepoc == 184 ) && (img->mb_x_r == 0) && (img->mb_y_r == 0)) {
		img->framepoc = img->framepoc;
		}
		*/
		ret = pf_read_one_macroblock[nEntropyMode][nImgType][IMGPAR current_mb_nr_r&1] ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

		IMGPAR current_mb_nr_d = IMGPAR current_mb_nr_r;

		if ( (img->type != B_SLICE) && ((IMGPAR stream_global->m_is_MTMS == 0 && stream_global->m_bIsSingleThreadMode != TRUE))){	// Top Field B, Bottom Field P, Handle P field in B branch -By Haihua
			currMB_d = &IMGPAR mb_decdata[IMGPAR current_mb_nr_d];
			currMB_s_d = &dec_picture->mb_data[IMGPAR current_mb_nr_d];
		}


#if !defined(ONE_COF)
		//IMGPAR cof_d = (IMGPAR cof_array + IMGPAR current_mb_nr_d*384);
#endif
		int remainder = IMGPAR current_mb_nr_d;
		if (MbaffFrameFlag)
		{
			IMGPAR mb_x_d = mb_pos_table[remainder>>1].x;
			IMGPAR mb_y_d = ((mb_pos_table[remainder>>1].y)<<1) + (remainder & 0x01);
		}
		else
		{
			IMGPAR mb_x_d = (((int)mb_pos_table[remainder].x));   
			IMGPAR mb_y_d = (((int)mb_pos_table[remainder].y));
		}

		/* Define vertical positions */
		IMGPAR block_y_d = IMGPAR mb_y_d * BLOCK_SIZE;      /* luma block position */
		IMGPAR pix_y_d   = IMGPAR mb_y_d * MB_BLOCK_SIZE;   /* luma macroblock position */
		IMGPAR pix_c_y_d = IMGPAR mb_y_d * (MB_BLOCK_SIZE/2); /* chroma macroblock position */

		/* Define horizontal positions */
		IMGPAR block_x_d = IMGPAR mb_x_d * BLOCK_SIZE;      /* luma block position */
		IMGPAR pix_x_d   = IMGPAR mb_x_d * MB_BLOCK_SIZE;   /* luma pixel position */
		IMGPAR pix_c_x_d = IMGPAR mb_x_d * (MB_BLOCK_SIZE/2); /* chroma pixel position */

		if(IMGPAR mb_x_d < IMGPAR start_mb_x)
			IMGPAR start_mb_x = IMGPAR mb_x_d;

		if(IMGPAR MbaffFrameFlag)
		{
			IMGPAR pix_y_rows = IMGPAR pix_y_d&63;		
			IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&31;
		}
		else
		{
			IMGPAR pix_y_rows = IMGPAR pix_y_d&31;		
			IMGPAR pix_c_y_rows = IMGPAR pix_c_y_d&15;
		}

		list_offset = (MbaffFrameFlag&&(currMB_s_d->mb_field))? (IMGPAR current_mb_nr_d&1) ? 4 : 2 : 0;

		if (currMB_s_d->do_record == 1) {
			ret = record_reference_picIds ARGS2(list_offset, currMB_s_d);
			if (FAILED(ret)) {		// Error or WarningLevel_1

				if (ISWARNINGLEVEL_1(ret)) {
					stream_global->m_bSeekIDR = TRUE;
					nalu->buf = IMGPAR ori_nalu_buf;					
				} 

				return ret;

			}
		}

		if (IS_COPY(currMB_d))
			memset(&(currMB_s_d->pred_info.ref_pic_id[0][0]),(char) listX[LIST_0+list_offset][0]->unique_id,4);

		if (!currMB_d->luma_transform_size_8x8_flag)
			IMGPAR FP_MB_itrans_Luma = MB_itrans4x4_Luma;
		else
			IMGPAR FP_MB_itrans_Luma = MB_itrans8x8_Luma;

		if(IMGPAR type == I_SLICE)
			ret = IMGPAR FP_decode_one_macroblock_I ARGS0();
		else if(IMGPAR type == P_SLICE)
		{
			if(IS_INTRA(currMB_d))
				ret = IMGPAR FP_decode_one_macroblock_P_Intra ARGS0();
			else 
				ret = IMGPAR FP_decode_one_macroblock_P ARGS0();
		}
		else
		{
			if(IS_INTRA(currMB_d))
				ret = IMGPAR FP_decode_one_macroblock_B_Intra ARGS0();
			else
				ret = IMGPAR FP_decode_one_macroblock_B ARGS0();
		}

		if (FAILED(ret)) {
			return ret;
		}

		if(MbaffFrameFlag && currMB_d->mb_field)
		{
			IMGPAR num_ref_idx_l0_active >>= 1;
			IMGPAR num_ref_idx_l1_active >>= 1;
		}

#ifdef ENABLE_ERROR_RESILIENCE
		ercWriteMBMODEandMV(img,inp);
#endif

		/*
		if ( img->framepoc == 598 && img->mb_x_d == 27 && img->mb_y_d == 1 ) {
		img->framepoc = img->framepoc;
		}
		*/

		ret = exit_macroblock_FMO ARGS2((!MbaffFrameFlag||(IMGPAR current_mb_nr_r&1)), &end_of_slice);

		if (FAILED(ret)) {
			return ret;
		}

		currSlice->read_mb_nr++;
		IMGPAR decoded_flag[IMGPAR current_mb_nr_d] = 1;
		IMGPAR current_mb_nr_d = FmoGetNextMBNr ARGS1((IMGPAR current_mb_nr_d));

		//IMGPAR current_mb_nr_d++;
	}

	//if ( (IMGPAR mb_x_d+ 1) != IMGPAR PicWidthInMbs ) {
	//	IMGPAR FP_TransferData_at_SliceEnd ARGS0();
	//}

	return CREL_OK;
}

CREL_RETURN fnDoReadAndDecodeOneSlice PARGS0()
{
	CREL_RETURN ret;
	DEBUG_SHOW_SW_INFO( "Picture Type: %d  Structure: %d  MBAff: %d  Direct Spital Mode: %d", IMGPAR currentSlice->picture_type, IMGPAR currentSlice->structure, IMGPAR currentSlice->MbaffFrameFlag, IMGPAR currentSlice->direct_spatial_mv_pred_flag);
	StreamParameters *stream_global = IMGPAR stream_global;
	
	ret = init_lists ARGS2(IMGPAR type, IMGPAR currentSlice->structure);
	if (FAILED(ret)) {
		return ret;
	}
	ret = reorder_lists ARGS2((IMGPAR type), (IMGPAR currentSlice));
	if (FAILED(ret)) {
		return ret;
	}
	free_ref_pic_list_reordering_buffer(IMGPAR currentSlice);

	ret = check_lists ARGS0();
	if (FAILED(ret)) {
		return ret;
	}

	if (IMGPAR structure == FRAME)
		init_mbaff_lists ARGS0();

	if(IMGPAR type == B_SLICE && IMGPAR currentSlice->direct_spatial_mv_pred_flag == 0)
		calc_mvscale ARGS0();

	if ((active_pps.weighted_bipred_idc > 0 && IMGPAR type == B_SLICE) || (active_pps.weighted_pred_flag && IMGPAR type == P_SLICE))
		fill_wp_params ARGS0();

	if (active_pps.entropy_coding_mode_flag)
	{
		init_contexts ARGS0();
		cabac_new_slice ARGS0();
	}

	// decode main slice information
#if defined(_HW_ACCEL_)
	if (imgDXVAVer && (IviDxva1 == g_DXVAVer) && ((stream_global->m_is_MTMS || stream_global->m_bIsSingleThreadMode == TRUE) || (IMGPAR currentSlice->header == SOP)))
		IMGPAR m_bFirstMB = TRUE;
#endif
	
	IMGPAR m_start_mb_nr = IMGPAR currentSlice->start_mb_nr;
	if ((IMGPAR currentSlice->header == SOP || IMGPAR currentSlice->header == SOS) && IMGPAR currentSlice->ei_flag == 0)
	{
		if(active_pps.num_slice_groups_minus1 == 0)  //less than 0, return?
			ret = read_decode_one_slice ARGS0();
		else
			ret = read_decode_one_slice_FMO ARGS0();
		if (FAILED(ret)) {
			return ret;
		}

#if defined(_HW_ACCEL_)
		if(imgDXVAVer && (IviDxva1==g_DXVAVer && g_DXVAMode==E_H264_DXVA_NVIDIA_PROPRIETARY_A) && !((IMGPAR Hybrid_Decoding ==1 && IMGPAR currentSlice->picture_type == I_SLICE) || (IMGPAR Hybrid_Decoding ==2 && IMGPAR currentSlice->picture_type != B_SLICE)))
			EXECUTE ARGS0();
#endif
	}

	return CREL_OK;
}
#endif

unsigned __stdcall decode_slice_0 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img = img_array[0];// = (img_par *)parameters;
	CREL_RETURN ret;

	while(1)
	{
		WaitForSingleObject(stream_global->m_event_start_slice[0], INFINITE);
		ResetEvent(stream_global->m_event_start_slice[0]);

		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_slice_0");

		//img = img_array[0];

		ret = fnDoReadAndDecodeOneSlice ARGS0();


		if (FAILED(ret)) {	
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;				
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;
			} else {
				SetEvent(stream_global->m_event_end_slice[0]);
				break;	//Error				
			}
		}		

		SetEvent(stream_global->m_event_end_slice[0]);

		DEBUG_SHOW_SW_INFO("End decode_slice_0");
	}

	SetEvent(stream_global->m_event_end_slice[0]);
	_endthreadex(0);
	return(1);
}

unsigned __stdcall decode_slice_1 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img =  img_array[1];// = (img_par *)parameters;
	CREL_RETURN ret;

	while(1)
	{
		WaitForSingleObject(stream_global->m_event_start_slice[1], INFINITE);
		ResetEvent(stream_global->m_event_start_slice[1]);

		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_slice_1");

		//img = img_array[1];

		ret = fnDoReadAndDecodeOneSlice ARGS0();

		if (FAILED(ret)) {	
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;				
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;
			} else {
				SetEvent(stream_global->m_event_end_slice[1]);
				break;	//Error
			}
		}

		SetEvent(stream_global->m_event_end_slice[1]);

		DEBUG_SHOW_SW_INFO("End decode_slice_1");
	}

	SetEvent(stream_global->m_event_end_slice[1]);
	_endthreadex(0);
	return(1);
}

unsigned __stdcall decode_slice_2 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img = img_array[2];// = (img_par *)parameters;
	CREL_RETURN ret;

	while(1)
	{
		WaitForSingleObject(stream_global->m_event_start_slice[2], INFINITE);
		ResetEvent(stream_global->m_event_start_slice[2]);

		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_slice_2");
		//img = img_array[2];

		ret = fnDoReadAndDecodeOneSlice ARGS0();

		if (FAILED(ret)) {	
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;				
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;
			} else {
				SetEvent(stream_global->m_event_end_slice[2]);
				break;	//Error
			}
		}

		SetEvent(stream_global->m_event_end_slice[2]);

		DEBUG_SHOW_SW_INFO("End decode_slice_2");
	}

	SetEvent(stream_global->m_event_end_slice[2]);
	_endthreadex(0);
	return(1);
}

unsigned __stdcall decode_slice_3 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img = img_array[3];// = (img_par *)parameters;
	CREL_RETURN ret;

	while(1)
	{
		WaitForSingleObject(stream_global->m_event_start_slice[3], INFINITE);
		ResetEvent(stream_global->m_event_start_slice[3]);

		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_slice_3");

		/*		
		if (img->framepoc == 205) {
		img->number = img->number;
		}
		*/
		ret = fnDoReadAndDecodeOneSlice ARGS0();


		if (FAILED(ret)) {	
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;				
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;
			} else {
				SetEvent(stream_global->m_event_end_slice[3]);
				break;	//Error
			}
		}

		SetEvent(stream_global->m_event_end_slice[3]);

		DEBUG_SHOW_SW_INFO("End decode_slice_3");
	}

	SetEvent(stream_global->m_event_end_slice[3]);
	_endthreadex(0);
	return(1);
}

unsigned __stdcall decode_slice_4 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img = img_array[4];// = (img_par *)parameters;
	CREL_RETURN ret;

	while(1)
	{
		WaitForSingleObject(stream_global->m_event_start_slice[4], INFINITE);
		ResetEvent(stream_global->m_event_start_slice[4]);

		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_slice_4");

		/*		
		if (img->framepoc == 205) {
		img->number = img->number;
		}
		*/
		ret = fnDoReadAndDecodeOneSlice ARGS0();


		if (FAILED(ret)) {	
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;				
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;
			} else {
				SetEvent(stream_global->m_event_end_slice[4]);
				break;	//Error
			}
		}

		SetEvent(stream_global->m_event_end_slice[4]);

		DEBUG_SHOW_SW_INFO("End decode_slice_4");
	}

	SetEvent(stream_global->m_event_end_slice[4]);
	_endthreadex(0);
	return(1);
}

unsigned __stdcall decode_slice_5 (void *parameters)
{
	stream_par *stream_global = (stream_par *)parameters;
	img_par *img = img_array[5];// = (img_par *)parameters;
	CREL_RETURN ret;

	while(1)
	{
		WaitForSingleObject(stream_global->m_event_start_slice[5], INFINITE);
		ResetEvent(stream_global->m_event_start_slice[5]);

		if (g_event_exit_flag)
			break;

		DEBUG_SHOW_SW_INFO("Start decode_slice_5");

		/*		
		if (img->framepoc == 205) {
		img->number = img->number;
		}
		*/
		ret = fnDoReadAndDecodeOneSlice ARGS0();


		if (FAILED(ret)) {	
			if (ISWARNINGLEVEL_1(ret)) {
				stream_global->m_bSeekIDR = TRUE;
				nalu->buf = IMGPAR ori_nalu_buf;				
			} else if (ISWARNINGLEVEL_3(ret)) {
				IMGPAR error_mb_nr = IMGPAR current_mb_nr_r;
			} else {
				SetEvent(stream_global->m_event_end_slice[5]);
				break;	//Error
			}
		}

		SetEvent(stream_global->m_event_end_slice[5]);

		DEBUG_SHOW_SW_INFO("End decode_slice_5");
	}

	SetEvent(stream_global->m_event_end_slice[5]);
	_endthreadex(0);
	return(1);
}

void reset_wp_params PARGS0()
{
	int i,comp;
	int log_weight_denom;

	for (i=0; i<MAX_REFERENCE_PICTURES; i++)
	{
		for (comp=0; comp<3; comp++)
		{
			log_weight_denom = (comp == 0) ? IMGPAR luma_log2_weight_denom : IMGPAR chroma_log2_weight_denom;
			*(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i)*3+comp) = 1<<log_weight_denom;
			*(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+i)*3+comp) = 1<<log_weight_denom;
		}
	}
}


void fill_wp_params PARGS0()
{
	int i, j, k;
	int comp;
	int log_weight_denom;
	int tb, td;
	int bframe = (IMGPAR type==B_SLICE);
	int max_bwd_ref, max_fwd_ref;
	int tx,DistScaleFactor;

#if !defined(_COLLECT_PIC_)
	max_fwd_ref = IMGPAR num_ref_idx_l0_active;
	max_bwd_ref = IMGPAR num_ref_idx_l1_active;
#else
	max_fwd_ref = IMGPAR currentSlice->num_ref_idx_l0_active;
	max_bwd_ref = IMGPAR currentSlice->num_ref_idx_l1_active;
#endif

	if (active_pps.weighted_bipred_idc == 2 && bframe)
	{
		IMGPAR luma_log2_weight_denom = 5;
		IMGPAR chroma_log2_weight_denom = 5;
		IMGPAR wp_round_luma = 16;
		IMGPAR wp_round_chroma = 16;

		for (i=0; i<MAX_REFERENCE_PICTURES; i++)
		{
			for (comp=0; comp<3; comp++)
			{
				log_weight_denom = (comp == 0) ? IMGPAR luma_log2_weight_denom : IMGPAR chroma_log2_weight_denom;
				*(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i)*3+comp) = 1<<log_weight_denom;
				*(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+i)*3+comp) = 1<<log_weight_denom;
				*(IMGPAR wp_offset+(0*MAX_REFERENCE_PICTURES+i)*3+comp) = 0;
				*(IMGPAR wp_offset+(1*MAX_REFERENCE_PICTURES+i)*3+comp) = 0;
			}
		}
	}

	if (bframe)
	{
		for (i=0; i<max_fwd_ref; i++)
		{
			for (j=0; j<max_bwd_ref; j++)
			{
				for (comp = 0; comp<3; comp++)
				{
					log_weight_denom = (comp == 0) ? IMGPAR luma_log2_weight_denom : IMGPAR chroma_log2_weight_denom;
					if (active_pps.weighted_bipred_idc == 1)
					{
						*(IMGPAR wbp_weight+((0*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =  *(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i)*3+comp);
						*(IMGPAR wbp_weight+((1*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =  *(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+j)*3+comp);
					}
					else if (active_pps.weighted_bipred_idc == 2)
					{
						td = Clip3(-128,127,listX[LIST_1][j]->poc - listX[LIST_0][i]->poc);
						if (td == 0 || listX[LIST_1][j]->is_long_term || listX[LIST_0][i]->is_long_term)
						{
							*(IMGPAR wbp_weight+((0*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =   32;
							*(IMGPAR wbp_weight+((1*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =   32;
						}
						else
						{
							tb = Clip3(-128,127,IMGPAR ThisPOC - listX[LIST_0][i]->poc);

							tx = (16384 + abs(td/2))/td;
							DistScaleFactor = Clip3(-1024, 1023, (tx*tb + 32 )>>6);

							*(IMGPAR wbp_weight+((1*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = DistScaleFactor >> 2;
							*(IMGPAR wbp_weight+((0*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = 64 - 
								(*(IMGPAR wbp_weight+((1*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp));
							if ((*(IMGPAR wbp_weight+((1*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp)) < -64 ||
								(*(IMGPAR wbp_weight+((1*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp)) > 128)
							{
								*(IMGPAR wbp_weight+((0*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = 32;
								*(IMGPAR wbp_weight+((1*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = 32;
								*(IMGPAR wp_offset+(0*MAX_REFERENCE_PICTURES+i)*3+comp) = 0;
								*(IMGPAR wp_offset+(1*MAX_REFERENCE_PICTURES+i)*3+comp) = 0;
							}
						}
					}
				}
			}
		}
	}

	if (bframe && IMGPAR MbaffFrameFlag)
	{
		for (i=0; i<2*max_fwd_ref; i++)
		{
			for (j=0; j<2*max_bwd_ref; j++)
			{
				for (comp = 0; comp<3; comp++)
				{
					for (k=2; k<6; k+=2)
					{
						*(IMGPAR wp_offset+((k+0)*MAX_REFERENCE_PICTURES+i)*3+comp) = *(IMGPAR wp_offset+(0*MAX_REFERENCE_PICTURES+i/2)*3+comp);
						*(IMGPAR wp_offset+((k+1)*MAX_REFERENCE_PICTURES+i)*3+comp) = *(IMGPAR wp_offset+(1*MAX_REFERENCE_PICTURES+i/2)*3+comp);

						log_weight_denom = (comp == 0) ? IMGPAR luma_log2_weight_denom : IMGPAR chroma_log2_weight_denom;
						if (active_pps.weighted_bipred_idc == 1)
						{
							*(IMGPAR wbp_weight+(((k+0)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =  *(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i/2)*3+comp);
							*(IMGPAR wbp_weight+(((k+1)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =  *(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+j/2)*3+comp);
						}
						else if (active_pps.weighted_bipred_idc == 2)
						{
							td = Clip3(-128,127,listX[k+LIST_1][j]->poc - listX[k+LIST_0][i]->poc);
							if (td == 0 || listX[k+LIST_1][j]->is_long_term || listX[k+LIST_0][i]->is_long_term)
							{
								*(IMGPAR wbp_weight+(((k+0)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =   32;
								*(IMGPAR wbp_weight+(((k+1)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) =   32;
							}
							else
							{
								tb = Clip3(-128,127,((k==2)?IMGPAR toppoc:IMGPAR bottompoc) - listX[k+LIST_0][i]->poc);

								tx = (16384 + abs(td/2))/td;
								DistScaleFactor = Clip3(-1024, 1023, (tx*tb + 32 )>>6);

								*(IMGPAR wbp_weight+(((k+1)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = DistScaleFactor >> 2;
								*(IMGPAR wbp_weight+(((k+0)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = 64 - 
									(*(IMGPAR wbp_weight+(((k+1)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp));
								if ((*(IMGPAR wbp_weight+(((k+1)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp)) < -64 ||
									(*(IMGPAR wbp_weight+(((k+1)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp)) > 128)
								{
									*(IMGPAR wbp_weight+(((k+1)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = 32;
									*(IMGPAR wbp_weight+(((k+0)*MAX_REFERENCE_PICTURES+i)*MAX_REFERENCE_PICTURES+j)*3+comp) = 32;
									*(IMGPAR wp_offset+((k+0)*MAX_REFERENCE_PICTURES+i)*3+comp) = 0;
									*(IMGPAR wp_offset+((k+1)*MAX_REFERENCE_PICTURES+i)*3+comp) = 0;
								}
							}
						}
					}
				}
			}
		}
	}
}

void SetH264CCCode PARGS0()
{
	StreamParameters *stream_global = IMGPAR stream_global;

	long lCClen=0;
	unsigned char* pCCBuf;
	g_pCCDec->GetCCData(&lCClen, &pCCBuf, 0);

	if (lCClen>0 && pCCBuf[20]!=0)
	{
		memcpy(IMGPAR m_CCCode.ccbuf, pCCBuf, lCClen);
		IMGPAR m_CCCode.cclen = lCClen;
		IMGPAR m_CCCode.ccnum = pCCBuf[20];
	}
}

#ifdef ANALYZE_CODE_READ_DATA_IN
int get_mb_bits_data(char *buf, char *token, char *tmp_string, int *test_mb)
{
	char * pch, *pequal, *pend1, *pend2;
	int count = 0;
	int n;
	int stop_flag = 1;
	char compare_string[512];
	char compare_string2[512];

	pch	   = strstr(buf, token);	
	
	if(pch == NULL)	//token doesn't exist
		return -1;
	
	pequal = strstr(pch, "{")+1;
	pend2  = strstr(pequal, "}");
	pend1  = strstr(pequal, ",");

	n = strlen(pch) - strlen(pequal-1);
	strncpy(compare_string, pch, n*sizeof(char));
	compare_string[n] = '\0';
	
	

	while(stop_flag)
	{
		n = strlen(pequal) - strlen(pend1);	
		strncpy(tmp_string,pequal,n*sizeof(char));	
		tmp_string[n]='\0';

		pequal = pend1 + 1;
		pend1 = strstr(pequal, ",");
		test_mb[count] = atoi(tmp_string);
		
		if(atoi(tmp_string) == -1)
		{
			if(pend1 == NULL)  // end of txt file
				return count;
			else
				pch = strstr(pend1, token);	
			if(pch != NULL)
			{
				pequal = strstr(pch, "{");
				n = strlen(pch) - strlen(pequal);
				strncpy(compare_string2, pch, n*sizeof(char));
				compare_string2[n] = '\0';

				if(strcmp(compare_string2,compare_string) != 0)
					return count;

				pequal = strstr(pch, "{")+1;
				pend2  = strstr(pequal, "}");
				pend1  = strstr(pequal, ",");
			}
			else	//token doesn't exist
				stop_flag = 0;
			
		}
		else
			count++;
		
	}
	//printf("Total mb number: %d\n", count);
		
	return count;
}

#endif
