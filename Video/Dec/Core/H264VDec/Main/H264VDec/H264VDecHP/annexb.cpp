
/*!
*************************************************************************************
* \file annexb.c
*
* \brief
*    Annex B Byte Stream format
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Stephan Wenger                  <stewe@cs.tu-berlin.de>
*************************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "global.h"
#include "annexb.h"
#include "memalloc.h"
#if defined (_USE_SCRAMBLE_DATA_)
#include "DataEncryption.h"
#endif

#ifdef _COLLECT_PIC_
#define		stream_global		IMGPAR stream_global
#endif

static int FindStartCode (unsigned char *Buf, int zeros_in_startcode);
/*!
************************************************************************
* \brief
*    Returns the size of the NALU (bits between start codes in case of
*    Annex B.  nalu->buf and nalu->len are filled.  Other field in
*    nalu-> remain uninitialized (will be taken care of by NALUtoRBSP.
*
* \return
*     0 if there is nothing any more to read (EOF)
*    -1 in case of any error
*
*  \note Side-effect: Returns length of start-code in bytes. 
*
* \note
*   GetAnnexbNALU expects start codes at byte aligned positions in the file
*
************************************************************************
*/
static void ResetAnnexbNALUDecoding PARGS0()
{
	memset(&stNAL, 0x0, sizeof(stNAL));
	buf_begin = buf_end = 0;
}

void BeginAnnexbNALUDecoding PARGS0()
{
	ResetAnnexbNALUDecoding ARGS0();
}

void EndAnnexbNALUDecoding PARGS0()
{
	if(stNAL.buf)
		_aligned_free(stNAL.buf);
	ResetAnnexbNALUDecoding ARGS0();
}

static CREL_RETURN AllocNALUDecodingBuffer PARGS1(NALU_t *one_nalu)
{
	int pos = stNAL.pos;

	if(one_nalu->max_size <= stNAL.buf_size)
		return CREL_OK;

	if(pos > 0)
	{
		memcpy(one_nalu->buf,stNAL.buf+stNAL.begin,stNAL.pos-stNAL.begin);
	}
	if(stNAL.buf)
		_aligned_free(stNAL.buf);
	stNAL.buf = (char*) _aligned_malloc(one_nalu->max_size, 16);
	if (stNAL.buf == NULL) {
		return CREL_ERROR_H264_NOMEMORY;
	}
	stNAL.buf_size = one_nalu->max_size;
	if(pos > 0)
	{
		int size = stNAL.pos-stNAL.begin;
		memcpy(stNAL.buf, one_nalu->buf, size);
		stNAL.begin = 0;
		stNAL.pos = size;
	}
	else
	{
		stNAL.begin = 0;
		stNAL.pos = 0;
	}

	return CREL_OK;
}

CREL_RETURN Fill_Buffer PARGS0()
{
	unsigned long n;
	int has_pts = false;
	H264_TS pts;
	CREL_RETURN ret = 0;

	ret = (*get_data_fcn)(H264_pvDataContext,&buf_begin,(DWORD *)&n,&has_pts, &pts);
	if(FAILED(ret) || n==0)
	{
		if (n)
		{
			// PERFORM_FRAME_POINTER_DESCRAMBLING
			DWORD dwDeScramblingPt = (DWORD)buf_begin;
			buf_begin = (unsigned char *)INLINE_DESCRAMBLE(dwDeScramblingPt,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);
		}
		return ret;
	}	
	if(has_pts)
	{
		g_pts = pts;
		g_has_pts = true;
		streamptscounter++;
	}

	// PERFORM_FRAME_POINTER_DESCRAMBLING
	DWORD dwDeScramblingPt = (DWORD)buf_begin;
	buf_begin = (unsigned char *)INLINE_DESCRAMBLE(dwDeScramblingPt,SCRAMBLE_PARAM_1,SCRAMBLE_PARAM_2,SCRAMBLE_PARAM_3);
	if(ret == CREL_OK)
	{
#if defined (_USE_SCRAMBLE_DATA_)
		// PERFORM_BUFFER_DESCRAMBLING
		DataDecryption((unsigned char *)buf_begin, n < H264_SCRAMBLE_DATA_LEN ? n : H264_SCRAMBLE_DATA_LEN);
#endif
	}
	buf_end = buf_begin + n;

	return CREL_OK;
}

// eat data until we see start code(eat start code too.).
static CREL_RETURN ConsumeAnnexbNALStartCode PARGS1(NALU_t *one_nalu)
{
	int zero_count;

	for(zero_count=0;;)
	{
		if(buf_begin==buf_end || !buf_begin)
		{
			if(FAILED(Fill_Buffer ARGS0()) || buf_begin==buf_end)
				return CREL_ERROR_H264_SYNCNOTFOUND;
		}
		if(*buf_begin==0)
			zero_count++;
		else
		{
			if(*buf_begin==1 && zero_count>1)
			{
				buf_begin++;
				return CREL_OK;
			}
			zero_count = 0;
		}
		buf_begin++;
	}

}

CREL_RETURN GetAnnexbNALU PARGS2(NALU_t *one_nalu, int* output_pos)
{
	int pos, zero_count, nalu_type;
	CREL_RETURN ret;

	ret = AllocNALUDecodingBuffer ARGS1(one_nalu);
	if (FAILED(ret)) {
		return ret;
	}

	if(!stNAL.bStartCodeFound)
	{
		ret = ConsumeAnnexbNALStartCode ARGS1(one_nalu);

		if (FAILED(ret))
		{
			DEBUG_SHOW_SW_INFO("ERROR! Failed to find start code");
			return ret;		// failed to find start code.
		}

		stNAL.bStartCodeFound = TRUE;

		WRITE_BITSTREAM(0, 0, 1);
	}

	pos = zero_count = 0;

	if(buf_begin==buf_end || !buf_begin)
	{
		if(FAILED(Fill_Buffer ARGS0()) || buf_begin==buf_end)
		{
			if (g_bNormalSpeed && buf_begin)
			{	
				DEBUG_SHOW_SW_INFO("buf_begin: %d  buf_end: %d", buf_begin, buf_end);
				DEBUG_SHOW_SW_INFO("No More Data --> EOS");

				*output_pos = 0;
				return CREL_OK;	// no more data, should trigger EOS
			}
			else
			{
				DEBUG_SHOW_SW_INFO("No More Data on FF or FR --> EOS");
				return CREL_ERROR_H264_SYNCNOTFOUND;	// no more data and not on normal speed -> skip this frame!
			}
		}
		if(*buf_begin==0)
			zero_count++;
	}

	WRITE_BITSTREAM(buf_begin, 1, 0);

	memset(&one_nalu->pts, 0x0, sizeof(H264_TS));
	one_nalu->buf[pos++] = *buf_begin++;
	nalu_type = one_nalu->buf[0] & 0x1f;

	if( g_has_pts && ((nalu_type == NALU_TYPE_SLICE) || (nalu_type == NALU_TYPE_IDR)) )
	{
		g_has_pts = 0;
		one_nalu->pts = g_pts;
		DEBUG_SHOW_SW_INFO("This NALU has TS = %I64d\n", one_nalu->pts.ts);
	}
	else if (g_bReceivedFirst && (nalu_type == NALU_TYPE_EOSEQ || nalu_type == NALU_TYPE_EOSTREAM))
	{
		DEBUG_SHOW_SW_INFO("Read EOS[%d] NALU!!", nalu_type);
		*output_pos = 0;
		buf_begin--;
		return 0;	//trigger EOS
	}

	for(;;)
	{
		if(buf_begin==buf_end)
		{
			ret = Fill_Buffer ARGS0();
			if(FAILED(ret) || buf_begin==buf_end)
			{
				DEBUG_SHOW_SW_INFO("buf_begin: %d  buf_end: %d", buf_begin, buf_end);
				one_nalu->buf[pos++] = 0x00;
				one_nalu->buf[pos++] = 0x00;
				one_nalu->buf[pos++] = 0x01;
				one_nalu->buf[pos] = 0x0b;

				if (g_bNormalSpeed && buf_begin && (ret!=E_H264_DATA_STATUS_DATA_DISCONTINUITY) && (ret!=E_H264_DATA_STATUS_READ_ERROR))//Terry: remove stop_indicator, it is used to be the flag of flush_dpb once in ::Stop.
				{
					DEBUG_SHOW_SW_INFO("Exist More Data");

					WRITE_BITSTREAM(0, 0, 1);

					pos -= 2;	//pos - zero_count;

					one_nalu->len = pos;
					one_nalu->forbidden_bit		= (one_nalu->buf[0]>>7) & 1;
					one_nalu->nal_reference_idc	= (one_nalu->buf[0]>>5) & 3;
					one_nalu->nal_unit_type		= (one_nalu->buf[0]) & 0x1f;

					// IoK: This is constrained to be different than 2,3,4 for HD-DVD streams
					if(one_nalu->nal_unit_type==NALU_TYPE_DPA || one_nalu->nal_unit_type==NALU_TYPE_DPB || one_nalu->nal_unit_type==NALU_TYPE_DPC) {
						return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
					}

					*output_pos = pos;

					return CREL_OK;
				}
				else
				{
					DEBUG_SHOW_SW_INFO("No More Data --> EOS");
					return CREL_ERROR_H264_SYNCNOTFOUND;	// no more data and not on normal speed -> skip this frame!
				}
			}
		}
		if(*buf_begin==0)
			zero_count++;
		else
		{
			if(zero_count>1)
			{
				if(zero_count==ZEROBYTES_SHORTSTARTCODE && *buf_begin==0x03)
				{
					WRITE_BITSTREAM(buf_begin, 1, 0);
					zero_count = 0;
					buf_begin++;
					continue;
				}
				else if(*buf_begin==1)
				{					
					WRITE_BITSTREAM(buf_begin, 1, 0);
					one_nalu->buf[pos] = *buf_begin++;
					break;
				}
			}
			zero_count = 0;
		}
		WRITE_BITSTREAM(buf_begin, 1, 0);
		one_nalu->buf[pos++] = *buf_begin++;
	}
	// Here the leading zeros(if any), Start code, the complete NALU, trailing zeros(if any)
	// and the next start code is in the stNAL.buf.
	// The size of stNAL.buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-TrailingZero8Bits
	// is the size of the NALU.

	one_nalu->len = pos - zero_count;
	one_nalu->forbidden_bit		= (one_nalu->buf[0]>>7) & 1;
	one_nalu->nal_reference_idc	= (one_nalu->buf[0]>>5) & 3;
	one_nalu->nal_unit_type		= (one_nalu->buf[0]) & 0x1f;
	// IoK: This is constrained to be different than 2,3,4 for HD-DVD streams
	if(one_nalu->nal_unit_type==NALU_TYPE_DPA || one_nalu->nal_unit_type==NALU_TYPE_DPB || one_nalu->nal_unit_type==NALU_TYPE_DPC) {
		return CREL_ERROR_H264_NOTSUPPORTED|CREL_ERROR_H264_HD_ProfileFault;
	}
	*output_pos = zero_count>2 ? pos-3 : pos-2;
	return CREL_OK;
}

/*!
************************************************************************
* \brief
*    returns if new start code is found at byte aligned position buf.
*    new-startcode is of form N 0x00 bytes, followed by a 0x01 byte.
*
*  \return
*     1 if start-code is found or                      \n
*     0, indicating that there is no start code
*
*  \param Buf
*     pointer to byte-stream
*  \param zeros_in_startcode
*     indicates number of 0x00 bytes in start-code.
************************************************************************
*/
static int FindStartCode (unsigned char *Buf, int zeros_in_startcode)
{
	int info;
	int i;

	info = 1;
	for (i = 0; i < zeros_in_startcode; i++)
		if(Buf[i] != 0)
			info = 0;

	if(Buf[i] != 1)
		info = 0;
	return info;
}

void CheckZeroByteNonVCL PARGS1(NALU_t *one_nalu)
{
	int CheckZeroByte=0;

	//This function deals only with non-VCL NAL units
	if(one_nalu->nal_unit_type>=1&&one_nalu->nal_unit_type<=5)
		return;

	//for SPS and PPS, zero_byte shall exist
	if(one_nalu->nal_unit_type==NALU_TYPE_SPS || one_nalu->nal_unit_type==NALU_TYPE_PPS)
		CheckZeroByte=1;
	//check the possibility of the current NALU to be the start of a new access unit, according to 7.4.1.2.3
	if(one_nalu->nal_unit_type==NALU_TYPE_AUD  || one_nalu->nal_unit_type==NALU_TYPE_SPS ||
		one_nalu->nal_unit_type==NALU_TYPE_PPS || one_nalu->nal_unit_type==NALU_TYPE_SEI ||
		(one_nalu->nal_unit_type>=13 && one_nalu->nal_unit_type<=18))
	{
		if(LastAccessUnitExists)
		{
			LastAccessUnitExists=0;		//deliver the last access unit to decoder
			NALUCount=0;
		}
	}
	NALUCount++;
	//for the first NAL unit in an access unit, zero_byte shall exists
	if(NALUCount==1)
		CheckZeroByte=1;
	if(CheckZeroByte && one_nalu->startcodeprefix_len==3)
	{
		DEBUG_SHOW_ERROR_INFO("warning: zero_byte shall exist\n");
		//because it is not a very serious problem, we may not indicate an error by setting ret to -1
		//*ret=-1;
	}
}

void CheckZeroByteVCL PARGS1(NALU_t *one_nalu)
{
	int CheckZeroByte=0;

	//This function deals only with VCL NAL units
	if(!(one_nalu->nal_unit_type>=1&&one_nalu->nal_unit_type<=5))
		return;

	if(LastAccessUnitExists)
	{
		NALUCount=0;
	}
	NALUCount++;
	//the first VCL NAL unit that is the first NAL unit after last VCL NAL unit indicates 
	//the start of a new access unit and hence the first NAL unit of the new access unit.						(sounds like a tongue twister :-)
	if(NALUCount==1)
		CheckZeroByte=1;
	LastAccessUnitExists=1;
	if(CheckZeroByte && one_nalu->startcodeprefix_len==3)
	{
		DEBUG_SHOW_ERROR_INFO("warning: zero_byte shall exist\n");
		//because it is not a very serious problem, we may not indicate an error by setting ret to -1
		//*ret=-1;
	}
}
