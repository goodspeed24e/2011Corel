
/*!
*************************************************************************************
* \file header.c
*
* \brief
*    H.264 Slice headers
*
*************************************************************************************
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "global.h"
#include "elements.h"
#include "defines.h"
#include "fmo.h"
#include "vlc.h"
#include "cabac.h"
#include "mbuffer.h"
#include "header.h"
#include "image.h"
#include "h264dxvabase.h"

#include "ctx_tables.h"

#ifdef _COLLECT_PIC_
#include "parset.h"
#endif
//extern StorablePicture *dec_picture;

static CREL_RETURN ref_pic_list_reordering PARGS0();
static void pred_weight_table PARGS0();

/*!
************************************************************************
* \brief
*    calculate Ceil(Log2(uiVal))
************************************************************************
*/
unsigned CeilLog2( unsigned uiVal)
{
	unsigned uiTmp = uiVal-1;
	unsigned uiRet = 0;

	while( uiTmp != 0 )
	{
		uiTmp >>= 1;
		uiRet++;
	}
	return uiRet;
}

/*
*/
static void reset_read_functions PARGS0()
{
	read_functions_t *read_functions = &(IMGPAR currentSlice->readSyntax);
	//	int is_b = (codec_context->current_slice.slice_type == B_SLICE);
	if (active_pps.entropy_coding_mode_flag)
	{
		read_functions->raw_ipred4x4_mode_luma = readIntraPredMode_CABAC;
		read_functions->raw_ipred4x4_mode_chroma = readCIPredMode_CABAC;
		read_functions->readMVD = readMVD_CABAC;

#if !defined (_COLLECT_PIC_)
		if (IMGPAR type == I_SLICE)
			read_one_macroblock = read_one_macroblock_CABAC_I;
		else if (IMGPAR type == P_SLICE)
			read_one_macroblock = read_one_macroblock_CABAC_P;
		else
			read_one_macroblock = read_one_macroblock_CABAC_B;		
#endif

		readRefInfo = readRefInfo_CABAC;
	}
	else
	{
		read_functions->raw_ipred4x4_mode_luma = readSyntaxElement_Intra4x4PredictionMode;
		read_functions->raw_ipred4x4_mode_chroma = read_raw_mb_uvlc; //same as mb_type
		read_functions->readMVD = readMVD_uvlc;

#if !defined (_COLLECT_PIC_)
		if (IMGPAR type == I_SLICE)
			read_one_macroblock = read_one_macroblock_UVLC_I;
		else if (IMGPAR type == P_SLICE)
			read_one_macroblock = read_one_macroblock_UVLC_P;
		else
			read_one_macroblock = read_one_macroblock_UVLC_B;
#endif

		readRefInfo = readRefInfo_UVLC;
	}
	if(IMGPAR MbaffFrameFlag)
	{
		read_functions->read_ipred_modes = read_ipred_modes_MbAFF;

#if defined (_COLLECT_PIC_)
#if defined (IP_RD_MERGE)
		if(!(IMGPAR type == B_SLICE && IMGPAR nal_reference_idc ))
#else		
		if ( (IMGPAR stream_global->m_is_MTMS == 1 || IMGPAR stream_global->m_bIsSingleThreadMode == TRUE) || (IMGPAR type == B_SLICE && (!IMGPAR nal_reference_idc || imgDXVAVer)) )
#endif		

			CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_MBAff_Row;
		else
			CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_MBAff_Plane;
#else
		CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_MBAff_Row;
#endif

#if defined(_COLLECT_PIC_)
		start_macroblock									= start_macroblock_MBAff;
#endif
	}
	else
	{
		read_functions->read_ipred_modes = read_ipred_modes_NonMbAFF;

#if defined (_COLLECT_PIC_)
#if defined (IP_RD_MERGE)
		if(!(IMGPAR type == B_SLICE && IMGPAR nal_reference_idc ))
#else		
		if ( (IMGPAR stream_global->m_is_MTMS == 1 || IMGPAR stream_global->m_bIsSingleThreadMode == TRUE) || (IMGPAR type == B_SLICE && (!IMGPAR nal_reference_idc || imgDXVAVer)) )
#endif		
		{
			if(active_pps.num_slice_groups_minus1 == 0)
				CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_NonMBAff_Row;
			else
				CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_NonMBAff_Row_FMO;
		}
		else
		{
			if(active_pps.num_slice_groups_minus1 == 0)
				CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_NonMBAff_Plane;
			else
				CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_NonMBAff_Plane_FMO;
		}
#else
		CheckAvailabilityOfNeighbors     = CheckAvailabilityOfNeighbors_NonMBAff_Row;
#endif

#if defined(_COLLECT_PIC_)
		start_macroblock									= start_macroblock_NonMBAff;
#endif
	}
	if(active_sps.chroma_format_idc)
		NCBP = (unsigned char *)&_NCBP[1][0][0];
	else
		NCBP = (unsigned char *)&_NCBP[0][0][0];
}

#if !defined (_COLLECT_PIC_)
/*!
************************************************************************
* \brief
*    read the first part of the header (only the pic_parameter_set_id)
* \return
*    Length of the first part of the slice header (in bits)
************************************************************************
*/
int FirstPartOfSliceHeader PARGS0()
{
	Slice *currSlice = IMGPAR currentSlice;
	int tmp;

	// Get first_mb_in_slice
	currSlice->start_mb_nr = ue_v ("SH: first_mb_in_slice");

	tmp = ue_v ("SH: slice_type");

	if (tmp>4) tmp -=5;

	IMGPAR type = currSlice->picture_type = (SliceType) tmp;
	// IoK: This can only be I, P or B for HD-profile
	if(currSlice->picture_type!=P_SLICE && currSlice->picture_type!=I_SLICE && currSlice->picture_type!=B_SLICE)
	{
		return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
	}

	currSlice->pic_parameter_set_id = ue_v ("SH: pic_parameter_set_id");

	return 0;
}

/*!
************************************************************************
* \brief
*    read the scond part of the header (without the pic_parameter_set_id 
* \return
*    Length of the second part of the Slice header in bits
************************************************************************
*/
int RestOfSliceHeader PARGS0()
{
	Slice *currSlice = IMGPAR currentSlice;
	CREL_RETURN ret;
	int val, len;

	IMGPAR frame_num = u_v ((active_sps.log2_max_frame_num_minus4 + 4), "SH: frame_num");

	/* Tian Dong: frame_num gap processing, if found */
	if (IMGPAR idr_flag)
	{
		IMGPAR pre_frame_num = IMGPAR frame_num;
		assert(IMGPAR frame_num == 0);
	}

	if (active_sps.frame_mbs_only_flag)
	{
		IMGPAR structure = FRAME;
		IMGPAR field_pic_flag=0;
	}
	else
	{
		// field_pic_flag   u(1)
		IMGPAR field_pic_flag = u_1 ("SH: field_pic_flag");
		if (IMGPAR field_pic_flag)
		{
			// bottom_field_flag  u(1)
			IMGPAR bottom_field_flag = u_1 ("SH: bottom_field_flag");

			IMGPAR structure = IMGPAR bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;
		}
		else
		{
			IMGPAR structure = FRAME;
			IMGPAR bottom_field_flag=0;
		}
	}

	currSlice->structure = (PictureStructure)IMGPAR structure;

	IMGPAR MbaffFrameFlag=(active_sps.mb_adaptive_frame_field_flag && (IMGPAR field_pic_flag==0));
	IMGPAR mvd_pairs_mask = (IMGPAR MbaffFrameFlag<<1)+1;

	if (IMGPAR structure == FRAME       ) assert (IMGPAR field_pic_flag == 0);
	if (IMGPAR structure == TOP_FIELD   ) assert (IMGPAR field_pic_flag == 1 && IMGPAR bottom_field_flag == 0);
	if (IMGPAR structure == BOTTOM_FIELD) assert (IMGPAR field_pic_flag == 1 && IMGPAR bottom_field_flag == 1);

	if (IMGPAR idr_flag)
	{
		IMGPAR idr_pic_id = ue_v ("SH: idr_pic_id");
	}

	if (active_sps.pic_order_cnt_type == 0)
	{
		IMGPAR pic_order_cnt_lsb = u_v ((active_sps.log2_max_pic_order_cnt_lsb_minus4 + 4), "SH: pic_order_cnt_lsb");
		if( active_pps.pic_order_present_flag  ==  1 &&  !IMGPAR field_pic_flag )
			IMGPAR delta_pic_order_cnt_bottom = se_v ("SH: delta_pic_order_cnt_bottom");
		else
			IMGPAR delta_pic_order_cnt_bottom = 0;  
	}
	if( active_sps.pic_order_cnt_type == 1 && !active_sps.delta_pic_order_always_zero_flag ) 
	{
		IMGPAR delta_pic_order_cnt[ 0 ] = se_v ("SH: delta_pic_order_cnt[0]");
		if( active_pps.pic_order_present_flag  ==  1  &&  !IMGPAR field_pic_flag )
			IMGPAR delta_pic_order_cnt[ 1 ] = se_v ("SH: delta_pic_order_cnt[1]");
	}else
	{
		if (active_sps.pic_order_cnt_type == 1)
		{
			IMGPAR delta_pic_order_cnt[ 0 ] = 0;
			IMGPAR delta_pic_order_cnt[ 1 ] = 0;
		}
	}

	if(IMGPAR type==B_SLICE)
	{
		IMGPAR direct_spatial_mv_pred_flag = u_1 ("SH: direct_spatial_mv_pred_flag");
	}

	IMGPAR num_ref_idx_l0_active = active_pps.num_ref_idx_l0_active_minus1 + 1;
	IMGPAR num_ref_idx_l1_active = active_pps.num_ref_idx_l1_active_minus1 + 1;

	if(IMGPAR type==P_SLICE || IMGPAR type==B_SLICE)
	{
		val = u_1 ("SH: num_ref_idx_override_flag");
		if (val)
		{
			IMGPAR num_ref_idx_l0_active = 1 + ue_v ("SH: num_ref_idx_l0_active_minus1");

			if(IMGPAR type==B_SLICE)
			{
				IMGPAR num_ref_idx_l1_active = 1 + ue_v ("SH: num_ref_idx_l1_active_minus1");
			}
		}
	}
	if (IMGPAR type!=B_SLICE)
	{
		IMGPAR num_ref_idx_l1_active = 0;
	}

	ret = ref_pic_list_reordering ARGS0();

	IMGPAR apply_weights = ( active_pps.weighted_pred_flag && (currSlice->picture_type == P_SLICE) )
		|| ((active_pps.weighted_bipred_idc > 0 ) && (currSlice->picture_type == B_SLICE) );


#if defined(_HW_ACCEL_)
	if(g_DXVAVer == IviNotDxva || IMGPAR Hybrid_Decoding == 5)
#endif
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
		pred_weight_table ARGS0();
	}

	if (IMGPAR nal_reference_idc) {
		dec_ref_pic_marking ARGS0();
	}

	if (active_pps.entropy_coding_mode_flag && IMGPAR type!=I_SLICE)
	{
		IMGPAR model_number = ue_v ("SH: cabac_init_idc");
	}
	else 
	{
		IMGPAR model_number = 0;
	}

	val = se_v ("SH: slice_qp_delta");
	currSlice->qp = IMGPAR qp = 26 + active_pps.pic_init_qp_minus26 + val;


	currSlice->slice_qp_delta = val;  

	if (active_pps.deblocking_filter_control_present_flag)
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
	}

	if (active_pps.num_slice_groups_minus1>0 && active_pps.slice_group_map_type>=3 &&
		active_pps.slice_group_map_type<=5)
	{
		len = (active_sps.pic_height_in_map_units_minus1+1)*(active_sps.pic_width_in_mbs_minus1+1)/ 
			(active_pps.slice_group_change_rate_minus1+1);
		if (((active_sps.pic_height_in_map_units_minus1+1)*(active_sps.pic_width_in_mbs_minus1+1))% 
			(active_pps.slice_group_change_rate_minus1+1))
			len +=1;

		len = CeilLog2(len+1);

		IMGPAR slice_group_change_cycle = u_v (len, "SH: slice_group_change_cycle");
	}
	IMGPAR PicHeightInMbs = IMGPAR FrameHeightInMbs>>IMGPAR field_pic_flag;
	IMGPAR PicSizeInMbs   = IMGPAR PicWidthInMbs * IMGPAR PicHeightInMbs;
	IMGPAR FrameSizeInMbs = IMGPAR PicWidthInMbs * IMGPAR FrameHeightInMbs;

	reset_read_functions ARGS0();

	return 0;
}

#else

#if 0
void	RearrangeCCCode (bool isBSlice)
{
	if (g_nCCBufLen == 0)return;

	int nMarkPos = g_nCCBufLen-1;
	while((g_CCBuf[nMarkPos]!=0xff)&&(nMarkPos>21))
		nMarkPos--;
	while((g_CCBuf[nMarkPos]==0xff)&&(nMarkPos>21))	//eat multi 0xff;
		nMarkPos--;
	g_nCCBufLen = nMarkPos + 2;

	if ( g_CCBuf[LINE21BUF_SIZE-1]!=1 && g_CCBuf[LINE21BUF_SIZE-1]!=2 ) //has rearranged or invalide cccode skipped.
		return;

	g_CCBuf[LINE21BUF_SIZE-1] = 0;

	if ( isBSlice )
	{
		//move B CC code forward upto before IP cc code.
		nMarkPos = g_nCCBufLen-1;
		if (g_nCCBufLen ==0)
			return;

		int nBPos=0, nIPPos=0;
		if (g_CCBuf[LINE21BUF_SIZE-3]==0) return;
		for (int ii=0; ii<g_CCBuf[LINE21BUF_SIZE-3]; ii++)
		{
			while(g_CCBuf[nMarkPos]!=0xfc && g_CCBuf[nMarkPos]!=0xfd && nMarkPos>21)
				nMarkPos--;
			nBPos = nMarkPos--;
		}

		if (g_CCBuf[LINE21BUF_SIZE-4]==0) return;
		for (int jj=0; jj<g_CCBuf[LINE21BUF_SIZE-4]; jj++)
		{
			while(g_CCBuf[nMarkPos]!=0xfc && g_CCBuf[nMarkPos]!=0xfd && nMarkPos>21)
				nMarkPos--;
			nIPPos = nMarkPos--;
		}

		//if ((g_nCCBufLen>0)&&(nMarkPos>21)&&(nBPos>nIPPos)&&(g_nCCBufLen>nIPPos))
		if ( (nMarkPos>=21) )
		{
			memcpy( g_CCBuf + g_nCCBufLen-1, g_CCBuf +nIPPos, nBPos-nIPPos);
			memcpy( g_CCBuf + nIPPos, g_CCBuf + nBPos, g_nCCBufLen-nIPPos );
			g_CCBuf[g_nCCBufLen-1] = 0xff;
		}
	}
	else
	{
		//move old cc code to cc2, it's rearranged already. but we must keep 1 IP cccode, for maybe that's b cccode need to insert before them.
		int nCC1Count = g_CCBuf[20]; //&0x1f ;

		g_CCBuf[LINE21BUF_SIZE-4] = g_CCBuf[LINE21BUF_SIZE-3];  //p units.
		g_CCBuf[LINE21BUF_SIZE-3] = 0;

		if ( (nCC1Count>1)&&(g_CCBuf[LINE21BUF_SIZE-4]!=0) )
#if 0
		{  
			int nCC1keepPos;
			nMarkPos = m_cc1len - 2;
			//send out which has been arranged already.
			while(m_cc1buf[nMarkPos]!=0xfc && m_cc1buf[nMarkPos]!=0xfd && nMarkPos>21)
				nMarkPos--;
			m_cc1buf[LINE21BUF_SIZE-2]=2;	//code available.
		}
#else
		{
			//nMarkPos = m_cc1len + 1 - m_cc1buf[LINE21BUF_SIZE-4]*3;
			//check if it's a complete sentence now. it can only be the 2nd end for maybe there is more b cc code.
			//while(m_cc1buf[nMarkPos]!=0xfc && m_cc1buf[nMarkPos]!=0xfd && nMarkPos>21)
			//	nMarkPos--;
			//nMarkPos--;
			//while( (m_cc1buf[nMarkPos]!=0x2f) && (m_cc1buf[nMarkPos]!=0x2c) && (nMarkPos>21) )
			//	nMarkPos--;
			//if ( (m_cc1buf[nMarkPos]==0x2f ||m_cc1buf[nMarkPos]==0x2c) && ( m_cc1buf[nMarkPos-2]==0xfc || m_cc1buf[nMarkPos-2]==0xfd ) )
			//{
			//	m_cc1buf[LINE21BUF_SIZE-2]=nMarkPos;	//code available.
			//} 
			g_CCBuf[LINE21BUF_SIZE-2] = g_nCCBufLen + 1-g_CCBuf[LINE21BUF_SIZE-4]*3;

		}
#endif

	}
}
#endif

CREL_RETURN CopyNaluExtToSlice PARGS1 (Slice* currSlice) {

	StreamParameters *stream_global = IMGPAR stream_global;
	nalu_header_mvc_extension_t* pNaluMvcExt = &(stream_global->nalu_mvc_extension);

	//if (currSlice->idr_flag != pNaluMvcExt->idrFlag) {
		//return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	//}

	currSlice->idr_flag = pNaluMvcExt->idrFlag;		
	//Dependent View idr_flag is not from NAL_UNIT_TYPE, it is from NALU EXTENSION non_idr_flag instead
	
	currSlice->viewId			= pNaluMvcExt->viewId;
	currSlice->temporalId		= pNaluMvcExt->temporalId;
	currSlice->anchorPicFlag	= pNaluMvcExt->anchorPicFlag;
	currSlice->interViewFlag	= pNaluMvcExt->interViewFlag;
	currSlice->priorityId		= pNaluMvcExt->priorityId;

	return CREL_OK;
}

CREL_RETURN ParseSliceHeader PARGS0()
{
	Slice *currSlice = IMGPAR currentSlice;
	const int dP_nr = 0;	
	int tmp;
	seq_parameter_set_rbsp_t *sps;
	pic_parameter_set_rbsp_t *pps;
	int						 PicParsetId;
	int val;
	StreamParameters *stream_global = IMGPAR stream_global;

	//IMGPAR g_dep = currSlice->g_dep;
	IMGPAR g_dep.Dei_flag    = currSlice->g_dep->Dei_flag;
	IMGPAR g_dep.Dbits_to_go = currSlice->g_dep->Dbits_to_go;
	IMGPAR g_dep.Dbuffer     = currSlice->g_dep->Dbuffer;
	//memcpy(IMGPAR g_dep.Dbasestrm, currSlice->g_dep->Dbasestrm, currSlice->g_dep->Dstrmlength);
	IMGPAR g_dep.Dbasestrm = currSlice->g_dep->Dbasestrm;
	IMGPAR g_dep.Dstrmlength = currSlice->g_dep->Dstrmlength;
	IMGPAR g_dep.Dcodestrm   = currSlice->g_dep->Dcodestrm;

	// Get first_mb_in_slice
	currSlice->start_mb_nr = ue_v ("SH: first_mb_in_slice");
	/*
	if ( (IMGPAR PicWidthInMbs <= 0) || (currSlice->start_mb_nr == 0) ) {
	currSlice->start_mb_x = currSlice->start_mb_nr;
	} else {
	currSlice->start_mb_x = currSlice->start_mb_nr % (IMGPAR PicWidthInMbs << 1);
	}
	*/
	tmp = ue_v ("SH: slice_type");

	if (tmp>4) tmp -=5;

	IMGPAR type = currSlice->picture_type = (SliceType) tmp;

	//RearrangeCCCode(0/*IMGPAR type==B_SLICE*/);

	
	if ( stream_global->m_bSeekIDR ) {		

		if (currSlice->picture_type == I_SLICE) {

			stream_global->m_bSeekIDR = FALSE;

		} else {

			return CREL_WARNING_H264_STREAMERROR_LEVEL_3;

		}		

	}

	// IoK: This can only be I, P or B for HD-profile
	if(currSlice->picture_type<P_SLICE || currSlice->picture_type>RB_SLICE)
	{
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;	//Bitstream Error or possible un-supported type in HD-profile
	} else if ( currSlice->picture_type>I_SLICE ) {
		return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
	}

	if ((nalu->nal_unit_type == NALU_TYPE_IDR) && (currSlice->picture_type!=I_SLICE)) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

	PicParsetId = currSlice->pic_parameter_set_id = ue_v ("SH: pic_parameter_set_id");
	if (PicParsetId > MAXPPS - 1) {
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
	}

	if (PicParSet[PicParsetId].Valid != TRUE) {
		DEBUG_SHOW_ERROR_INFO ("Trying to use an invalid (uninitialized) Picture Parameter Set with ID %d, expect the unexpected...\n", PicParsetId);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
	}
	if ((currSlice->bIsBaseView && SeqParSet[PicParSet[PicParsetId].seq_parameter_set_id].Valid != TRUE) || (!currSlice->bIsBaseView && SeqParSubset[PicParSet[PicParsetId].seq_parameter_set_id].Valid != TRUE)) {
		DEBUG_SHOW_ERROR_INFO ("PicParset %d references an invalid (uninitialized) Sequence Parameter Set with ID %d, expect the unexpected...\n", PicParsetId, PicParSet[PicParsetId].seq_parameter_set_id);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
	}

	if (PicParSet[PicParsetId].slice_group_id == NULL)
	{
		if ((PicParSet[PicParsetId].slice_group_id = (unsigned int *) _aligned_malloc ((PicParSet[PicParsetId].num_slice_group_map_units_minus1+1)*sizeof(int), 16)) == NULL) {
			DEBUG_SHOW_ERROR_INFO ("MakePPSavailable: Cannot alloc slice_group_id");
			return CREL_ERROR_H264_NOMEMORY;
		}
	}

	sps =  (currSlice->bIsBaseView) ? &SeqParSet[PicParSet[PicParsetId].seq_parameter_set_id] :
		&SeqParSubset[PicParSet[PicParsetId].seq_parameter_set_id];
	pps =  &PicParSet[PicParsetId];

	if ((int) sps->pic_order_cnt_type < 0 || sps->pic_order_cnt_type > 2)  // != 1
	{
		DEBUG_SHOW_ERROR_INFO ("invalid sps->pic_order_cnt_type = %d\n", sps->pic_order_cnt_type);
		DEBUG_SHOW_ERROR_INFO ("pic_order_cnt_type != 1", -1000);
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
	}

	if (sps->pic_order_cnt_type == 1)
	{
		if(sps->num_ref_frames_in_pic_order_cnt_cycle >= MAXnum_ref_frames_in_pic_order_cnt_cycle)
		{
			DEBUG_SHOW_ERROR_INFO("num_ref_frames_in_pic_order_cnt_cycle too large",-1011);
			return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
		}
	}

	//Terry: error handle for no SEI frame.
	currSlice->NumClockTs = (sps->vui_seq_parameters.NumClockTs) ? sps->vui_seq_parameters.NumClockTs : 1;

	IMGPAR Transform8x8Mode = currSlice->Transform8x8Mode = pps->transform_8x8_mode_flag;
	IMGPAR frame_num = currSlice->frame_num = u_v ((sps->log2_max_frame_num_minus4 + 4), "SH: frame_num");

	if (currSlice->idr_flag || (IMGPAR firstSlice && IMGPAR firstSlice->idr_flag))
	{		
		currSlice->pre_frame_num = currSlice->frame_num;
	}
	else
	{
		if (IMGPAR last_has_mmco_5)
			currSlice->pre_frame_num = 0;
		else
			currSlice->pre_frame_num = stream_global->pre_frame_num;
	}

	if (sps->frame_mbs_only_flag)
	{
		IMGPAR structure = currSlice->structure = FRAME;
		IMGPAR field_pic_flag = currSlice->field_pic_flag = 0;
	}
	else
	{
		// field_pic_flag   u(1)
		IMGPAR field_pic_flag = currSlice->field_pic_flag = u_1 ("SH: field_pic_flag");
		if (IMGPAR field_pic_flag)
		{
			// bottom_field_flag  u(1)
			IMGPAR bottom_field_flag = currSlice->bottom_field_flag = u_1 ("SH: bottom_field_flag");
			IMGPAR structure = currSlice->structure =  currSlice->bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;
		}
		else
		{
			IMGPAR structure = currSlice->structure = FRAME;
			IMGPAR bottom_field_flag= currSlice->bottom_field_flag = 0;
		}
	}

	IMGPAR MbaffFrameFlag= currSlice->MbaffFrameFlag = (sps->mb_adaptive_frame_field_flag && (currSlice->field_pic_flag==0));	 
	IMGPAR mvd_pairs_mask = (IMGPAR MbaffFrameFlag<<1)+1;

	/*
	if (currSlice->structure == FRAME       ) assert (currSlice->field_pic_flag == 0);
	if (currSlice->structure == TOP_FIELD   ) assert (currSlice->field_pic_flag == 1 && currSlice->bottom_field_flag == 0);
	if (currSlice->structure == BOTTOM_FIELD) assert (currSlice->field_pic_flag == 1 && currSlice->bottom_field_flag == 1);
	*/

	if (currSlice->idr_flag)
	{	
		IMGPAR idr_pic_id = currSlice->idr_pic_id = ue_v ("SH: idr_pic_id");		
	}

	if (sps->pic_order_cnt_type == 0)
	{
		IMGPAR pic_order_cnt_lsb = currSlice->pic_order_cnt_lsb = u_v ((sps->log2_max_pic_order_cnt_lsb_minus4 + 4), "SH: pic_order_cnt_lsb");
		if( pps->pic_order_present_flag  ==  1 &&  !currSlice->field_pic_flag )
			IMGPAR delta_pic_order_cnt_bottom = currSlice->delta_pic_order_cnt_bottom = se_v ("SH: delta_pic_order_cnt_bottom");
		else
			IMGPAR delta_pic_order_cnt_bottom = currSlice->delta_pic_order_cnt_bottom = 0;  
	}

	if( sps->pic_order_cnt_type == 1 && !sps->delta_pic_order_always_zero_flag ) 
	{
		IMGPAR delta_pic_order_cnt[ 0 ] = currSlice->delta_pic_order_cnt[0] = se_v ("SH: delta_pic_order_cnt[0]");

		if( pps->pic_order_present_flag  ==  1  &&  !currSlice->field_pic_flag )
			IMGPAR delta_pic_order_cnt[ 1 ] = currSlice->delta_pic_order_cnt[1] = se_v ("SH: delta_pic_order_cnt[1]");
		else
			IMGPAR delta_pic_order_cnt[ 1 ] = currSlice->delta_pic_order_cnt[1] = 0;
	}
	else
	{
		if (sps->pic_order_cnt_type == 1)
		{
			IMGPAR delta_pic_order_cnt[ 0 ] = currSlice->delta_pic_order_cnt[0] = 0;
			IMGPAR delta_pic_order_cnt[ 1 ] = currSlice->delta_pic_order_cnt[1] = 0;
		}
	}

	if(currSlice->picture_type==B_SLICE)
	{
		IMGPAR direct_spatial_mv_pred_flag = currSlice->direct_spatial_mv_pred_flag = u_1 ("SH: direct_spatial_mv_pred_flag");
	}
	else
		IMGPAR direct_spatial_mv_pred_flag = currSlice->direct_spatial_mv_pred_flag = 0;

	IMGPAR num_ref_idx_l0_active = currSlice->num_ref_idx_l0_active = pps->num_ref_idx_l0_active_minus1 + 1;
	IMGPAR num_ref_idx_l1_active = currSlice->num_ref_idx_l1_active = pps->num_ref_idx_l1_active_minus1 + 1;

	if(currSlice->picture_type==P_SLICE || currSlice->picture_type==B_SLICE)
	{
		val = u_1 ("SH: num_ref_idx_override_flag");
		if (val)
		{
			IMGPAR num_ref_idx_l0_active = currSlice->num_ref_idx_l0_active = 1 + ue_v ("SH: num_ref_idx_l0_active_minus1");

			if ( IMGPAR field_pic_flag ) {
				if (currSlice->num_ref_idx_l0_active > 32){
					return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
				}
			} else {
				if (currSlice->num_ref_idx_l0_active > 16){
					return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
				}
			}

			

			if(currSlice->picture_type==B_SLICE)
			{
				IMGPAR num_ref_idx_l1_active = currSlice->num_ref_idx_l1_active = 1 + ue_v ("SH: num_ref_idx_l1_active_minus1");

				if ( IMGPAR field_pic_flag ) {
					if (currSlice->num_ref_idx_l1_active > 32){
						return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
					}
				} else {
					if (currSlice->num_ref_idx_l1_active > 16){
						return CREL_WARNING_H264_STREAMERROR_LEVEL_3|CREL_WARNING_H264_ERROR_SLICEHEAD;
					}
				}

			}
		}
	}
	if (currSlice->picture_type!=B_SLICE)
	{
		IMGPAR num_ref_idx_l1_active = currSlice->num_ref_idx_l1_active = 0;
	}

	//ref_pic_list_reordering ARGS0();

	IMGPAR apply_weights = currSlice->apply_weights = ( pps->weighted_pred_flag && (currSlice->picture_type == P_SLICE) )
		|| ((pps->weighted_bipred_idc > 0 ) && (currSlice->picture_type == B_SLICE));

	currSlice->g_dep->Dei_flag = IMGPAR g_dep.Dei_flag;
	currSlice->g_dep->Dbits_to_go = IMGPAR g_dep.Dbits_to_go;
	currSlice->g_dep->Dbuffer = IMGPAR g_dep.Dbuffer;
	//memcpy(currSlice->g_dep->Dbasestrm, IMGPAR g_dep.Dbasestrm, IMGPAR g_dep.Dstrmlength);
	currSlice->g_dep->Dstrmlength = IMGPAR g_dep.Dstrmlength;
	currSlice->g_dep->Dcodestrm = IMGPAR g_dep.Dcodestrm;

	return CREL_OK;
}
#endif

/*!
************************************************************************
* \brief
*    read the reference picture reordering information
************************************************************************
*/
static CREL_RETURN ref_pic_list_reordering PARGS0()
{
	Slice *currSlice = IMGPAR currentSlice;
	int i, val;  

	alloc_ref_pic_list_reordering_buffer ARGS1(currSlice);  

	if (IMGPAR type!=I_SLICE)
	{
		val = currSlice->ref_pic_list_reordering_flag_l0 = u_1 ("SH: ref_pic_list_reordering_flag_l0");

		if (val)
		{
			i=0;
			do
			{
				val = currSlice->remapping_of_pic_nums_idc_l0[i] = ue_v ("SH: remapping_of_pic_nums_idc_l0");
				if (val==0 || val==1)
				{
					currSlice->abs_diff_pic_num_minus1_l0[i] = ue_v ("SH: abs_diff_pic_num_minus1_l0");

				} else if (val==2) {

					currSlice->long_term_pic_idx_l0[i] = ue_v ("SH: long_term_pic_idx_l0");		

				} else if (val==4 || val==5) {

					currSlice->abs_diff_view_idx_minus1_l0[i] = ue_v ("SH: abs_diff_view_idx_minus1_l0");

				}

				if (i>IMGPAR num_ref_idx_l0_active) {
					return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
				}

				i++;

			} while (val != 3);

		}

		if (IMGPAR type==B_SLICE)
		{
			val = currSlice->ref_pic_list_reordering_flag_l1 = u_1 ("SH: ref_pic_list_reordering_flag_l1");

			if (val)
			{
				i=0;
				do
				{
					val = currSlice->remapping_of_pic_nums_idc_l1[i] = ue_v ("SH: remapping_of_pic_nums_idc_l1");
					if (val==0 || val==1) {

						currSlice->abs_diff_pic_num_minus1_l1[i] = ue_v ("SH: abs_diff_pic_num_minus1_l1");

					} else if (val==2) {

						currSlice->long_term_pic_idx_l1[i] = ue_v ("SH: long_term_pic_idx_l1");
					} else if (val==4 || val==5) {

						currSlice->abs_diff_view_idx_minus1_l1[i] = ue_v ("SH: abs_diff_view_idx_minus1_l1");

					}

					if (i>IMGPAR num_ref_idx_l1_active) {
						return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
					}

					i++;				

				} while (val != 3);

			}
		}

	}

	return CREL_OK;

}

/*!
************************************************************************
* \brief
*    read the weighted prediction tables
************************************************************************
*/
static void pred_weight_table PARGS0()
{
	int luma_weight_flag_l0, luma_weight_flag_l1, chroma_weight_flag_l0, chroma_weight_flag_l1;
	int i,j;

	Slice *currSlice = IMGPAR currentSlice;

	IMGPAR luma_log2_weight_denom = currSlice->luma_log2_weight_denom = ue_v ("SH: luma_log2_weight_denom");
	IMGPAR wp_round_luma = currSlice->wp_round_luma = IMGPAR luma_log2_weight_denom ? 1<<(IMGPAR luma_log2_weight_denom - 1): 0;

	IMGPAR chroma_log2_weight_denom = currSlice->chroma_log2_weight_denom = ue_v ("SH: chroma_log2_weight_denom");
	IMGPAR wp_round_chroma = currSlice->wp_round_chroma = IMGPAR chroma_log2_weight_denom ? 1<<(IMGPAR chroma_log2_weight_denom - 1): 0;

	IMGPAR wp_weight = currSlice->wp_weight;
	IMGPAR wp_offset = currSlice->wp_offset;
	IMGPAR wbp_weight = currSlice->wbp_weight;

	reset_wp_params ARGS0();

	for (i=0; i<IMGPAR num_ref_idx_l0_active; i++)
	{
		luma_weight_flag_l0 = u_1 ("SH: luma_weight_flag_l0");

		if (luma_weight_flag_l0)
		{
			*(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i)*3+0) = se_v ("SH: luma_weight_l0");
			*(IMGPAR wp_offset+(0*MAX_REFERENCE_PICTURES+i)*3+0) = se_v ("SH: luma_offset_l0");
		}
		else
		{
			*(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i)*3+0) = 1<<IMGPAR luma_log2_weight_denom;
			*(IMGPAR wp_offset+(0*MAX_REFERENCE_PICTURES+i)*3+0) = 0;
		}

		chroma_weight_flag_l0 = u_1 ("SH: chroma_weight_flag_l0");

		for (j=1; j<3; j++)
		{
			if (chroma_weight_flag_l0)
			{
				*(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i)*3+j) = se_v ("SH: chroma_weight_l0");
				*(IMGPAR wp_offset+(0*MAX_REFERENCE_PICTURES+i)*3+j) = se_v ("SH: chroma_offset_l0");
			}
			else
			{
				*(IMGPAR wp_weight+(0*MAX_REFERENCE_PICTURES+i)*3+j) = 1<<IMGPAR chroma_log2_weight_denom;
				*(IMGPAR wp_offset+(0*MAX_REFERENCE_PICTURES+i)*3+j) = 0;
			}
		}
	}
	if ((IMGPAR type == B_SLICE) && active_pps.weighted_bipred_idc == 1)
	{
		for (i=0; i<IMGPAR num_ref_idx_l1_active; i++)
		{
			luma_weight_flag_l1 = u_1 ("SH: luma_weight_flag_l1");

			if (luma_weight_flag_l1)
			{
				*(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+i)*3+0) = se_v ("SH: luma_weight_l1");
				*(IMGPAR wp_offset+(1*MAX_REFERENCE_PICTURES+i)*3+0) = se_v ("SH: luma_offset_l1");
			}
			else
			{
				*(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+i)*3+0) = 1<<IMGPAR luma_log2_weight_denom;
				*(IMGPAR wp_offset+(1*MAX_REFERENCE_PICTURES+i)*3+0) = 0;
			}

			chroma_weight_flag_l1 = u_1 ("SH: chroma_weight_flag_l1");

			for (j=1; j<3; j++)
			{
				if (chroma_weight_flag_l1)
				{
					*(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+i)*3+j) = se_v ("SH: chroma_weight_l1");
					*(IMGPAR wp_offset+(1*MAX_REFERENCE_PICTURES+i)*3+j) = se_v ("SH: chroma_offset_l1");
				}
				else
				{
					*(IMGPAR wp_weight+(1*MAX_REFERENCE_PICTURES+i)*3+j) = 1<<IMGPAR chroma_log2_weight_denom;
					*(IMGPAR wp_offset+(1*MAX_REFERENCE_PICTURES+i)*3+j) = 0;
				}
			}
		}
	}    
}


/*!
************************************************************************
* \brief
*    read the memory control operations
************************************************************************
*/
void dec_ref_pic_marking PARGS0()
{
	int val;

	DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

#ifdef  _COLLECT_PIC_
	StreamParameters *stream_global = IMGPAR stream_global;
#endif

	// free old buffer content
	while (IMGPAR dec_ref_pic_marking_buffer)
	{ 
		tmp_drpm=IMGPAR dec_ref_pic_marking_buffer;

		IMGPAR dec_ref_pic_marking_buffer=tmp_drpm->Next;
		_aligned_free (tmp_drpm);
	} 

	if (IMGPAR idr_flag)
	{
		IMGPAR no_output_of_prior_pics_flag = u_1 ("SH: no_output_of_prior_pics_flag");
		IMGPAR long_term_reference_flag = u_1 ("SH: long_term_reference_flag");
#ifdef _COLLECT_PIC_
		stream_global->no_output_of_prior_pics_flag = IMGPAR no_output_of_prior_pics_flag;
#endif
	}
	else
	{
		IMGPAR adaptive_ref_pic_buffering_flag = u_1 ("SH: adaptive_ref_pic_buffering_flag");
		if (IMGPAR adaptive_ref_pic_buffering_flag)
		{
			// read Memory Management Control Operation 
			do
			{
				tmp_drpm=(DecRefPicMarking_t*) _aligned_malloc (sizeof (DecRefPicMarking_t), 16);

				tmp_drpm->Next=NULL;

				val = tmp_drpm->memory_management_control_operation = ue_v ("SH: memory_management_control_operation");

				if ((val==1)||(val==3)) 
				{
					tmp_drpm->difference_of_pic_nums_minus1 = ue_v ("SH: difference_of_pic_nums_minus1");
				}
				if (val==2)
				{
					tmp_drpm->long_term_pic_num = ue_v ("SH: long_term_pic_num");
				}

				if ((val==3)||(val==6))
				{
					tmp_drpm->long_term_frame_idx = ue_v ("SH: long_term_frame_idx");
				}
				if (val==4)
				{
					tmp_drpm->max_long_term_frame_idx_plus1 = ue_v ("SH: max_long_term_pic_idx_plus1");
				}
#if defined(_COLLECT_PIC_)
				if(val == 5)
					stream_global->last_has_mmco_5 = 1;
#endif

				// add command
				if (IMGPAR dec_ref_pic_marking_buffer==NULL) 
				{
					IMGPAR dec_ref_pic_marking_buffer=tmp_drpm;
				}
				else
				{
					tmp_drpm2=IMGPAR dec_ref_pic_marking_buffer;
					while (tmp_drpm2->Next!=NULL) tmp_drpm2=tmp_drpm2->Next;
					tmp_drpm2->Next=tmp_drpm;
				}

			}while (val != 0);

		}
	}  
}

/*!
************************************************************************
* \brief
*    To calculate the poc values
*        based upon JVT-F100d2
*  POC200301: Until Jan 2003, this function will calculate the correct POC
*    values, but the management of POCs in buffered pictures may need more work.
* \return
*    none
************************************************************************
*/
CREL_RETURN decode_poc PARGS0()
{
	int i;
	// for POC mode 0:
#ifdef _COLLECT_PIC_
	StreamParameters *stream_global = IMGPAR stream_global;
#endif
	seq_parameter_set_rbsp_t *sps = (IMGPAR currentSlice->bIsBaseView) ? &SeqParSet[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id] :
		&SeqParSubset[PicParSet[IMGPAR currentSlice->pic_parameter_set_id].seq_parameter_set_id];
	unsigned int MaxPicOrderCntLsb = (1<<(sps->log2_max_pic_order_cnt_lsb_minus4+4));

	switch ( sps->pic_order_cnt_type )
	{
	case 0: // POC MODE 0
		// 1st
		if(IMGPAR idr_flag || IMGPAR firstSlice->idr_flag)
		{
			IMGPAR PrevPicOrderCntMsb = 0;
			IMGPAR PrevPicOrderCntLsb = 0;
		}
		else
		{
			if (IMGPAR last_has_mmco_5) 
			{
				if (IMGPAR last_pic_bottom_field)
				{
					IMGPAR PrevPicOrderCntMsb = 0;
					IMGPAR PrevPicOrderCntLsb = 0;
				}
				else
				{
					IMGPAR PrevPicOrderCntMsb = 0;
					IMGPAR PrevPicOrderCntLsb = IMGPAR toppoc;
				}
			}
		}
		// Calculate the MSBs of current picture
		if( IMGPAR pic_order_cnt_lsb  <  IMGPAR PrevPicOrderCntLsb  &&  
			( IMGPAR PrevPicOrderCntLsb - IMGPAR pic_order_cnt_lsb )  >=  ( MaxPicOrderCntLsb / 2 ) )
			IMGPAR PicOrderCntMsb = IMGPAR PrevPicOrderCntMsb + MaxPicOrderCntLsb;
		else if ( IMGPAR pic_order_cnt_lsb  >  IMGPAR PrevPicOrderCntLsb  &&
			( IMGPAR pic_order_cnt_lsb - IMGPAR PrevPicOrderCntLsb )  >  ( MaxPicOrderCntLsb / 2 ) )
			IMGPAR PicOrderCntMsb = IMGPAR PrevPicOrderCntMsb - MaxPicOrderCntLsb;
		else
			IMGPAR PicOrderCntMsb = IMGPAR PrevPicOrderCntMsb;

		// 2nd

		if(IMGPAR field_pic_flag==0)
		{           //frame pix
			IMGPAR toppoc = IMGPAR PicOrderCntMsb + IMGPAR pic_order_cnt_lsb;
			IMGPAR bottompoc = IMGPAR toppoc + IMGPAR delta_pic_order_cnt_bottom;
			IMGPAR PreviousPOC = IMGPAR ThisPOC;
			IMGPAR ThisPOC = IMGPAR framepoc = (IMGPAR toppoc < IMGPAR bottompoc)? IMGPAR toppoc : IMGPAR bottompoc; // POC200301
		}
		else if (IMGPAR bottom_field_flag==0)
		{  //top field 
			IMGPAR PreviousPOC = IMGPAR ThisPOC;
			IMGPAR ThisPOC= IMGPAR toppoc = IMGPAR PicOrderCntMsb + IMGPAR pic_order_cnt_lsb;
		}
		else
		{  //bottom field
			IMGPAR PreviousPOC = IMGPAR ThisPOC;
			IMGPAR ThisPOC= IMGPAR bottompoc = IMGPAR PicOrderCntMsb + IMGPAR pic_order_cnt_lsb;
		}
		IMGPAR framepoc=IMGPAR ThisPOC;

		if ( IMGPAR frame_num!=IMGPAR PreviousFrameNum)
			IMGPAR PreviousFrameNum=IMGPAR frame_num;

		if(!IMGPAR disposable_flag)
		{
			IMGPAR PrevPicOrderCntLsb = IMGPAR pic_order_cnt_lsb;
			IMGPAR PrevPicOrderCntMsb = IMGPAR PicOrderCntMsb;
		}

		break;

	case 1: // POC MODE 1
		// 1st
		if(IMGPAR idr_flag)
		{
			IMGPAR FrameNumOffset=0;     //  first pix of IDRGOP, 
			IMGPAR delta_pic_order_cnt[0]=0;                        //ignore first delta
			if(IMGPAR frame_num)  {
				DEBUG_SHOW_ERROR_INFO("[ERROR]frame_num != 0 in idr pix", -1020);
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
		}
		else 
		{
			if (IMGPAR last_has_mmco_5)
			{
				IMGPAR PreviousFrameNumOffset = 0;
				IMGPAR PreviousFrameNum = 0;
			}
			if (IMGPAR frame_num<IMGPAR PreviousFrameNum)
			{             //not first pix of IDRGOP
				IMGPAR FrameNumOffset = IMGPAR PreviousFrameNumOffset + IMGPAR MaxFrameNum;
			}
			else 
			{
				IMGPAR FrameNumOffset = IMGPAR PreviousFrameNumOffset;
			}
		}

		// 2nd
		if(sps->num_ref_frames_in_pic_order_cnt_cycle)
			IMGPAR AbsFrameNum = IMGPAR FrameNumOffset+IMGPAR frame_num;
		else 
			IMGPAR AbsFrameNum=0;
		if(IMGPAR disposable_flag && IMGPAR AbsFrameNum>0)
			IMGPAR AbsFrameNum--;

		// 3rd
		IMGPAR ExpectedDeltaPerPicOrderCntCycle=0;

		if(sps->num_ref_frames_in_pic_order_cnt_cycle)
			for(i=0;i<(int) sps->num_ref_frames_in_pic_order_cnt_cycle;i++)
				IMGPAR ExpectedDeltaPerPicOrderCntCycle += sps->offset_for_ref_frame[i];

		if(IMGPAR AbsFrameNum)
		{
			IMGPAR PicOrderCntCycleCnt = (IMGPAR AbsFrameNum-1)/sps->num_ref_frames_in_pic_order_cnt_cycle;
			IMGPAR FrameNumInPicOrderCntCycle = (IMGPAR AbsFrameNum-1)%sps->num_ref_frames_in_pic_order_cnt_cycle;
			IMGPAR ExpectedPicOrderCnt = IMGPAR PicOrderCntCycleCnt*IMGPAR ExpectedDeltaPerPicOrderCntCycle;
			for(i=0;i<=(int)IMGPAR FrameNumInPicOrderCntCycle;i++)
				IMGPAR ExpectedPicOrderCnt += sps->offset_for_ref_frame[i];
		}
		else 
			IMGPAR ExpectedPicOrderCnt=0;

		if(IMGPAR disposable_flag)
			IMGPAR ExpectedPicOrderCnt += sps->offset_for_non_ref_pic;

		if(IMGPAR field_pic_flag==0)
		{           //frame pix
			IMGPAR toppoc = IMGPAR ExpectedPicOrderCnt + IMGPAR delta_pic_order_cnt[0];
			IMGPAR bottompoc = IMGPAR toppoc + sps->offset_for_top_to_bottom_field + IMGPAR delta_pic_order_cnt[1];
			IMGPAR PreviousPOC = IMGPAR ThisPOC;
			IMGPAR ThisPOC = IMGPAR framepoc = (IMGPAR toppoc < IMGPAR bottompoc)? IMGPAR toppoc : IMGPAR bottompoc; // POC200301
		}
		else if (IMGPAR bottom_field_flag==0)
		{  //top field 
			IMGPAR PreviousPOC = IMGPAR ThisPOC;
			IMGPAR ThisPOC = IMGPAR toppoc = IMGPAR ExpectedPicOrderCnt + IMGPAR delta_pic_order_cnt[0];
		} 
		else
		{  //bottom field
			IMGPAR PreviousPOC = IMGPAR ThisPOC;
			IMGPAR ThisPOC = IMGPAR bottompoc = IMGPAR ExpectedPicOrderCnt + sps->offset_for_top_to_bottom_field + IMGPAR delta_pic_order_cnt[0];
		}
		IMGPAR framepoc=IMGPAR ThisPOC;

		IMGPAR PreviousFrameNum=IMGPAR frame_num;
		IMGPAR PreviousFrameNumOffset=IMGPAR FrameNumOffset;

		break;


	case 2: // POC MODE 2
		if(IMGPAR idr_flag) // IDR picture
		{
			IMGPAR FrameNumOffset=0;     //  first pix of IDRGOP, 
			IMGPAR PreviousPOC = IMGPAR ThisPOC;
			IMGPAR ThisPOC = IMGPAR framepoc = IMGPAR toppoc = IMGPAR bottompoc = 0;
			if(IMGPAR frame_num)  {
				DEBUG_SHOW_ERROR_INFO("[ERROR]frame_num != 0 in idr pix", -1020);
				return CREL_WARNING_H264_STREAMERROR_LEVEL_3;
			}
		}
		else
		{
			if (IMGPAR last_has_mmco_5)
			{
				IMGPAR PreviousFrameNum = 0;
				IMGPAR PreviousFrameNumOffset = 0;
			}

			if(IMGPAR MaxFrameNum == 0)
				IMGPAR MaxFrameNum = 1<<(sps->log2_max_frame_num_minus4+4);

			if (IMGPAR frame_num<IMGPAR PreviousFrameNum)
				IMGPAR FrameNumOffset = IMGPAR PreviousFrameNumOffset + IMGPAR MaxFrameNum;
			else 
				IMGPAR FrameNumOffset = IMGPAR PreviousFrameNumOffset;


			IMGPAR AbsFrameNum = IMGPAR FrameNumOffset+IMGPAR frame_num;
			if(IMGPAR disposable_flag)
			{
				IMGPAR PreviousPOC = IMGPAR ThisPOC;
				IMGPAR ThisPOC = (2*IMGPAR AbsFrameNum - 1);
			}
			else
			{
				IMGPAR PreviousPOC = IMGPAR ThisPOC;
				IMGPAR ThisPOC = (2*IMGPAR AbsFrameNum);
			}

			if (IMGPAR field_pic_flag==0)
				IMGPAR toppoc = IMGPAR bottompoc = IMGPAR framepoc = IMGPAR ThisPOC;
			else if (IMGPAR bottom_field_flag==0)
				IMGPAR toppoc = IMGPAR framepoc = IMGPAR ThisPOC;
			else IMGPAR bottompoc = IMGPAR framepoc = IMGPAR ThisPOC;
		}

		if (!IMGPAR disposable_flag)
			IMGPAR PreviousFrameNum=IMGPAR frame_num;
		IMGPAR PreviousFrameNumOffset=IMGPAR FrameNumOffset;
		break;


	default:
		//error must occurs
		//assert( 1==0 );
		return CREL_WARNING_H264_STREAMERROR_LEVEL_3;

	}

	return CREL_OK;
}

#if 0
/*!
************************************************************************
* \brief
*    A little helper for the debugging of POC code
* \return
*    none
************************************************************************
*/
int dumppoc() {
	DEBUG_INFO ("\nPOC locals...\n");
	DEBUG_INFO ("toppoc                                %d\n", IMGPAR toppoc);
	DEBUG_INFO ("bottompoc                             %d\n", IMGPAR bottompoc);
	DEBUG_INFO ("frame_num                             %d\n", IMGPAR frame_num);
	DEBUG_INFO ("field_pic_flag                        %d\n", IMGPAR field_pic_flag);
	DEBUG_INFO ("bottom_field_flag                     %d\n", IMGPAR bottom_field_flag);
	DEBUG_INFO ("POC SPS\n");
	DEBUG_INFO ("log2_max_frame_num_minus4             %d\n", active_sps.log2_max_frame_num_minus4);         // POC200301
	DEBUG_INFO ("log2_max_pic_order_cnt_lsb_minus4     %d\n", active_sps.log2_max_pic_order_cnt_lsb_minus4);
	DEBUG_INFO ("pic_order_cnt_type                    %d\n", active_sps.pic_order_cnt_type);
	DEBUG_INFO ("num_ref_frames_in_pic_order_cnt_cycle %d\n", active_sps.num_ref_frames_in_pic_order_cnt_cycle);
	DEBUG_INFO ("delta_pic_order_always_zero_flag      %d\n", active_sps.delta_pic_order_always_zero_flag);
	DEBUG_INFO ("offset_for_non_ref_pic                %d\n", active_sps.offset_for_non_ref_pic);
	DEBUG_INFO ("offset_for_top_to_bottom_field        %d\n", active_sps.offset_for_top_to_bottom_field);
	DEBUG_INFO ("offset_for_ref_frame[0]               %d\n", active_sps.offset_for_ref_frame[0]);
	DEBUG_INFO ("offset_for_ref_frame[1]               %d\n", active_sps.offset_for_ref_frame[1]);
	DEBUG_INFO ("POC in SLice Header\n");
	DEBUG_INFO ("pic_order_present_flag                %d\n", active_pps.pic_order_present_flag);
	DEBUG_INFO ("delta_pic_order_cnt[0]                %d\n", IMGPAR delta_pic_order_cnt[0]);
	DEBUG_INFO ("delta_pic_order_cnt[1]                %d\n", IMGPAR delta_pic_order_cnt[1]);
	DEBUG_INFO ("delta_pic_order_cnt[2]                %d\n", IMGPAR delta_pic_order_cnt[2]);
	DEBUG_INFO ("idr_flag                              %d\n", IMGPAR idr_flag);
	DEBUG_INFO ("MaxFrameNum                           %d\n", IMGPAR MaxFrameNum);

	return 0;
}
#endif

/*!
************************************************************************
* \brief
*    return the poc of img as per (8-1) JVT-F100d2
*  POC200301
************************************************************************
*/
int picture_order PARGS0()
{
	if (IMGPAR field_pic_flag==0) // is a frame
		return IMGPAR framepoc;
	else if (IMGPAR bottom_field_flag==0) // top field
		return IMGPAR toppoc;
	else // bottom field
		return IMGPAR bottompoc;
}

#if (!defined(__linux__)) && defined(_HW_ACCEL_)
void* SW_Open PARGS4(int nframe, int iVGAType, int iDXVAMode, ICPService *m_pIviCP)
#else
void* SW_Open PARGS3(int nframe, int iVGAType, int iDXVAMode)
#endif
{
	//	CnvH264dxva::Open(nframe);
	return NULL;
}

int SW_Close PARGS0()
{
	//	CnvH264dxva::Close();
	return 0;
}

int SW_BeginDecodeFrame PARGS0()
{
	//	CnvH264dxva::BeginDecodeFrame();
	return 0;
}

int SW_EndDecodeFrame PARGS0()
{
	//	CnvH264dxva::EndDecodeFrame();
	return 0;
}

int  SW_ReleaseDecodeFrame PARGS1(int frame_index)
{
	//	CnvH264dxva::ReleaseDecodeFrame(frame_index);
	return 0;
}

#ifdef _COLLECT_PIC_
CREL_RETURN UseSliceParameter PARGS0()	//Also parse some bits from slice head (ref_pic_list_reordering, pred_weight_table, dec_ref_pic_marking)
{	
	Slice *currSlice = IMGPAR currentSlice;
	const int dP_nr = 0;	
	int val, len;
	StreamParameters *stream_global = IMGPAR stream_global;
	CREL_RETURN ret;

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
	{
		IMGPAR pre_frame_num = currSlice->pre_frame_num;
	}
	IMGPAR structure = currSlice->structure;
	IMGPAR field_pic_flag = currSlice->field_pic_flag;
	IMGPAR bottom_field_flag = currSlice->bottom_field_flag;		
	IMGPAR MbaffFrameFlag= currSlice->MbaffFrameFlag;
	IMGPAR mvd_pairs_mask = (IMGPAR MbaffFrameFlag<<1)+1;
	IMGPAR idr_pic_id = currSlice->idr_pic_id;
	IMGPAR pic_order_cnt_lsb = currSlice->pic_order_cnt_lsb;
	IMGPAR delta_pic_order_cnt_bottom = currSlice->delta_pic_order_cnt_bottom;
	IMGPAR delta_pic_order_cnt_bottom = currSlice->delta_pic_order_cnt_bottom;	
	IMGPAR delta_pic_order_cnt[ 0 ] = currSlice->delta_pic_order_cnt[0];
	IMGPAR delta_pic_order_cnt[ 1 ] = currSlice->delta_pic_order_cnt[1];
	IMGPAR direct_spatial_mv_pred_flag = currSlice->direct_spatial_mv_pred_flag;
	IMGPAR num_ref_idx_l0_active = currSlice->num_ref_idx_l0_active;
	IMGPAR num_ref_idx_l1_active = currSlice->num_ref_idx_l1_active;
	IMGPAR framerate1000 = currSlice->framerate1000;
	*static_cast<H264_TS *>(&streampts) = currSlice->pts;
	IMGPAR has_pts = currSlice->has_pts;
	IMGPAR NumClockTs = currSlice->NumClockTs;
	ret = ref_pic_list_reordering ARGS0();
	if (FAILED(ret)) {
		return ret;
	}


	IMGPAR apply_weights = currSlice->apply_weights;

#if defined(_HW_ACCEL_)
	if(g_DXVAVer == IviNotDxva || IMGPAR Hybrid_Decoding == 5)
#endif
	{
		if(active_pps.num_slice_groups_minus1 == 0)  //less than 0, return?
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
		pred_weight_table ARGS0();
	}
	else if (IMGPAR apply_weights)
	{
		IMGPAR wp_weight = currSlice->wp_weight;
		IMGPAR wp_offset = currSlice->wp_offset;
		IMGPAR wbp_weight = currSlice->wbp_weight;
	}

	if (IMGPAR nal_reference_idc) {
		dec_ref_pic_marking ARGS0();		
	}

	if (active_pps.entropy_coding_mode_flag && IMGPAR type!=I_SLICE)
	{
		IMGPAR model_number = ue_v ("SH: cabac_init_idc");
	}
	else 
	{
		IMGPAR model_number = 0;
	}
	/*
	if ( img->number == 620 ) {
	img->number = img->number;
	}
	*/
	val = se_v ("SH: slice_qp_delta");
	currSlice->qp = IMGPAR qp = 26 + active_pps.pic_init_qp_minus26 + val;


	currSlice->slice_qp_delta = val;  

	if (active_pps.deblocking_filter_control_present_flag)
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
	}

	if (active_pps.num_slice_groups_minus1>0 && active_pps.slice_group_map_type>=3 &&
		active_pps.slice_group_map_type<=5)
	{
		len = (active_sps.pic_height_in_map_units_minus1+1)*(active_sps.pic_width_in_mbs_minus1+1)/ 
			(active_pps.slice_group_change_rate_minus1+1);
		if (((active_sps.pic_height_in_map_units_minus1+1)*(active_sps.pic_width_in_mbs_minus1+1))% 
			(active_pps.slice_group_change_rate_minus1+1))
			len +=1;

		len = CeilLog2(len+1);

		IMGPAR slice_group_change_cycle = u_v (len, "SH: slice_group_change_cycle");
	}

	IMGPAR PicHeightInMbs = IMGPAR FrameHeightInMbs / ( 1 + IMGPAR field_pic_flag );
	IMGPAR PicSizeInMbs   = IMGPAR PicWidthInMbs * IMGPAR PicHeightInMbs;
	IMGPAR FrameSizeInMbs = IMGPAR PicWidthInMbs * IMGPAR FrameHeightInMbs;

	reset_read_functions ARGS0(); 

	// currSlice->dp_mode is set by read_new_slice (NALU first byte available there)
	if (active_pps.entropy_coding_mode_flag == UVLC)
		nal_startcode_follows = uvlc_startcode_follows;
	else
		nal_startcode_follows = cabac_startcode_follows;

	return CREL_OK;

}
#endif
