#pragma warning ( disable : 4995 )
/*!
************************************************************************
* \file output.c
*
* \brief
*    Output an image and Trance support
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*    - Karsten Suehring               <suehring@hhi.de>
************************************************************************
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#include <stdarg.h>
#endif

#include "global.h"
#include "mbuffer.h"
#include "image.h"
#include "memalloc.h"
#include "h264dxvabase.h"

void write_out_picture PARGS2(StorablePicture *p, int out);

#ifdef _COLLECT_PIC_
#define		stream_global		IMGPAR stream_global
#endif

/*!
************************************************************************
* \brief
*    Convert image plane to temporary buffer for file writing
* \param imgX
*    Pointer to image plane
* \param buf
*    Buffer for file output
* \param size_x
*    horizontal size
* \param size_y
*    vertical size
* \param crop_left
*    pixels to crop from left
* \param crop_right
*    pixels to crop from right
* \param crop_top
*    pixels to crop from top
* \param crop_bottom
*    pixels to crop from bottom
************************************************************************
*/
static void img2buf (imgpel* imgX, unsigned char* buf, int size_x, int size_y, size_t symbol_size_in_bytes, int crop_left, int crop_right, int crop_top, int crop_bottom, int stride)
{
	int i,j;

	int twidth  = size_x - crop_left - crop_right;
	int theight = size_y - crop_top - crop_bottom;

	int size = 0;

#ifdef USE_BIGENDIAN
	unsigned char  ui8;
	unsigned short tmp16, ui16;
	unsigned long  tmp32, ui32;
#endif

	if (( sizeof(char) == sizeof (imgpel)) && ( sizeof(char) == symbol_size_in_bytes))
	{
		// imgpel == pixel_in_file == 1 byte -> simple copy
		for(i=0;i<theight;i++)
			memcpy(buf+crop_left+(i*twidth),imgX+(i+crop_top)*stride+crop_left, twidth);

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
				for(i=crop_top;i<size_y-crop_bottom;i++)
					for(j=crop_left;j<size_x-crop_right;j++)
					{
						ui8 = (unsigned char) *(imgX+i*stride+j);
						buf[(j-crop_left+((i-crop_top)*(twidth)))] = ui8;
					}
					break;
			}
		case 2:
			{
				for(i=crop_top;i<size_y-crop_bottom;i++)
					for(j=crop_left;j<size_x-crop_right;j++)
					{
						tmp16 = (unsigned short) *(imgX+i*stride+j);
						ui16  = (tmp16 >> 8) | ((tmp16&0xFF)<<8);
						memcpy(buf+((j-crop_left+((i-crop_top)*(twidth)))*2),&(ui16), 2);
					}
					break;
			}
		case 4:
			{
				for(i=crop_top;i<size_y-crop_bottom;i++)
					for(j=crop_left;j<size_x-crop_right;j++)
					{
						tmp32 = (unsigned long) *(imgX+i*stride+j);
						ui32  = ((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24);
						memcpy(buf+((j-crop_left+((i-crop_top)*(twidth)))*4),&(ui32), 4);
					}
					break;
			}
		default:
			{
				DEBUG_SHOW_ERROR_INFO ("[ERROR]writing only to formats of 8, 16 or 32 bit allowed on big endian architecture", 500);
				break;
			}
		}

#else
		// little endian
		if (sizeof (imgpel) < symbol_size_in_bytes)
		{
			// this should not happen. we should not have smaller imgpel than our source material.
			size = sizeof (imgpel);
			// clear buffer
			memset (buf, 0, (twidth*theight*symbol_size_in_bytes));
		}
		else
		{
			size = symbol_size_in_bytes;
		}

		for(i=crop_top;i<size_y-crop_bottom;i++)
			for(j=crop_left;j<size_x-crop_right;j++)
			{
				memcpy(buf+((j-crop_left+((i-crop_top)*(twidth)))*symbol_size_in_bytes),imgX+i*stride+j, size);
			}
#endif
	}
}


#ifdef PAIR_FIELDS_IN_OUTPUT

void clear_picture(StorablePicture *p);

/*!
************************************************************************
* \brief
*    output the pending frame buffer
* \param out
*    Output file
************************************************************************
*/
void flush_pending_output(int out)
{
	if (pending_output_state!=FRAME)
	{
		write_out_picture ARGS2(pending_output, out);
	}

	if (pending_output->FrameBuffer)
	{
		_aligned_free (pending_output->FrameBuffer);
		pending_output->FrameBuffer=NULL;
	}

	pending_output_state = FRAME;
}


/*!
************************************************************************
* \brief
*    Writes out a storable picture 
*    If the picture is a field, the output buffers the picture and tries 
*    to pair it with the next field.
* \param p
*    Picture to be written
* \param out
*    Output file
************************************************************************
*/
void write_picture PARGS3(StorablePicture *p, int out, int real_structure)
{
	int i, add;

	if (real_structure==FRAME)
	{
		flush_pending_output(out);
		write_out_picture ARGS2(p, out);
		return;
	}
	if (real_structure==pending_output_state)
	{
		flush_pending_output(out);
		write_picture ARGS3(p, out, real_structure);
		return;
	}

	if (pending_output_state == FRAME)
	{
		pending_output->size_x = p->size_x;
		pending_output->size_y = p->size_y;
		pending_output->size_x_cr = p->size_x_cr;
		pending_output->size_y_cr = p->size_y_cr;
		pending_output->chroma_format_idc = p->chroma_format_idc;

		pending_output->frame_mbs_only_flag = p->frame_mbs_only_flag;
		pending_output->frame_cropping_flag = p->frame_cropping_flag;
		if (pending_output->frame_cropping_flag)
		{
			pending_output->frame_cropping_rect_left_offset = p->frame_cropping_rect_left_offset;
			pending_output->frame_cropping_rect_right_offset = p->frame_cropping_rect_right_offset;
			pending_output->frame_cropping_rect_top_offset = p->frame_cropping_rect_top_offset;
			pending_output->frame_cropping_rect_bottom_offset = p->frame_cropping_rect_bottom_offset;
		}

		pending_output->FrameBuffer = (imgpel *) _aligned_malloc(pending_output->size_y*pending_output->size_x+
			2*pending_output->size_y_cr*pending_output->size_x_cr*sizeof(imgpel), 16);
		pending_output->imgY     = pending_output->FrameBuffer;
		pending_output->imgUV    = pending_output->imgY + pending_output->size_y*pending_output->size_x;

		if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
			clear_picture(pending_output);

		// copy first field
		if (real_structure == TOP_FIELD)
		{
			add = 0;
		}
		else
		{
			add = 1;
		}

		for (i=0; i<pending_output->size_y; i+=2)
		{
			memcpy(pending_output->imgY+(i+add)*pending_output->Y_stride, p->imgY+(i+add)*p->Y_stride, p->size_x * sizeof(imgpel));
		}
		for (i=0; i<pending_output->size_y_cr; i+=2)
		{
			memcpy(pending_output->imgUV+(i+add)*pending_output->UV_stride, p->imgUV+(i+add)*p->UV_stride, p->size_x_cr * 2 * sizeof(imgpel));
		}
		pending_output_state = real_structure;
	}
	else
	{
		if (  (pending_output->size_x!=p->size_x) || (pending_output->size_y!= p->size_y) 
			|| (pending_output->frame_mbs_only_flag != p->frame_mbs_only_flag)
			|| (pending_output->frame_cropping_flag != p->frame_cropping_flag)
			|| ( pending_output->frame_cropping_flag &&
			(  (pending_output->frame_cropping_rect_left_offset   != p->frame_cropping_rect_left_offset)
			||(pending_output->frame_cropping_rect_right_offset  != p->frame_cropping_rect_right_offset)
			||(pending_output->frame_cropping_rect_top_offset    != p->frame_cropping_rect_top_offset)
			||(pending_output->frame_cropping_rect_bottom_offset != p->frame_cropping_rect_bottom_offset)
			)
			)
			)
		{
			flush_pending_output(out);
			write_picture ARGS3(p, out, real_structure);
			return;
		}
		// copy second field
		if (real_structure == TOP_FIELD)
		{
			add = 0;
		}
		else
		{
			add = 1;
		}

		for (i=0; i<pending_output->size_y; i+=2)
		{
			memcpy(pending_output->imgY+(i+add)*pending_output->Y_stride, p->imgY+(i+add)*p->Y_stride, p->size_x * sizeof(imgpel));
		}
		for (i=0; i<pending_output->size_y_cr; i+=2)
		{
			memcpy(pending_output->imgUV+(i+add)*pending_output->UV_stride, p->imgUV+(i+add)*p->UV_stride, p->size_x_cr * 2 * sizeof(imgpel));
		}

		flush_pending_output(out);
	}
}

#else

/*!
************************************************************************
* \brief
*    Writes out a storable picture without doing any output modifications
* \param p
*    Picture to be written
* \param out
*    Output file
* \param real_structure
*    real picture structure
************************************************************************
*/
void write_picture PARGS3(StorablePicture *p, int out, int real_structure)
{
	if (g_bNormalSpeed || g_bDisplayed==FALSE)
	{
		write_out_picture ARGS2(p, out);
	}
	else if (p->pic_store_idx != -1)
	{
		CH264DXVABase *pDXVABase = (CH264DXVABase*)g_pH264DXVA;
		pDXVABase->m_pUncompBufQueue->PutItem(p->pic_store_idx);
		p->pic_store_idx = -1;
	}
}


#endif

/*!
************************************************************************
* \brief
*    Writes out a storable picture
* \param p
*    Picture to be written
* \param out
*    Output file
************************************************************************
*/
void write_out_picture PARGS2(StorablePicture *p, int out)
{
	extern void dump_dpb();

	extern int write_out_picture_h264_interface PARGS1(StorablePicture *p);
	write_out_picture_h264_interface ARGS1(p);
	if(out<=0)
		return;

	int SubWidthC  [2]= { 1, 2 };
	int SubHeightC [2]= { 1, 2 };
	int crop_left, crop_right, crop_top, crop_bottom;
	size_t symbol_size_in_bytes = 1; // HP restriction
	BOOL rgb_output = (active_sps.vui_seq_parameters.matrix_coefficients==0);
	unsigned char *buf;


	if (p->non_existing)
		return;

	if (p->frame_cropping_flag)
	{
		crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_left_offset;
		crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_right_offset;
		crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
		crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
	}
	else
	{
		crop_left = crop_right = crop_top = crop_bottom = 0;
	}

	DEBUG_SHOW_SW_INFO ("write frame size: %dx%d\n", p->size_x-crop_left-crop_right,p->size_y-crop_top-crop_bottom );

	// KS: this buffer should actually be allocated only once, but this is still much faster than the previous version
	buf = (unsigned char *) _aligned_malloc (p->size_x*p->size_y*symbol_size_in_bytes, 16);
	if (NULL==buf)
	{
		no_mem_exit("write_out_picture: buf");
	}

	if(rgb_output)
	{
		crop_left   = p->frame_cropping_rect_left_offset;
		crop_right  = p->frame_cropping_rect_right_offset;
		crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
		crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;

		img2buf (p->imgUV, buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, p->UV_stride);
		write(out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);

		if (active_sps.frame_cropping_flag)
		{
			crop_left   = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_left_offset;
			crop_right  = SubWidthC[p->chroma_format_idc] * p->frame_cropping_rect_right_offset;
			crop_top    = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
			crop_bottom = SubHeightC[p->chroma_format_idc]*( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;
		}
		else
		{
			crop_left = crop_right = crop_top = crop_bottom = 0;
		}
	}

	img2buf (p->imgY, buf, p->size_x, p->size_y, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, p->Y_stride);
	write(out, buf, (p->size_y-crop_bottom-crop_top)*(p->size_x-crop_right-crop_left)*symbol_size_in_bytes);

#ifdef __SUPPORT_YUV400__
	if (p->chroma_format_idc!=YUV400)
	{
#endif
		crop_left   = p->frame_cropping_rect_left_offset;
		crop_right  = p->frame_cropping_rect_right_offset;
		crop_top    = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_top_offset;
		crop_bottom = ( 2 - p->frame_mbs_only_flag ) * p->frame_cropping_rect_bottom_offset;

		img2buf (p->imgUV, buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, p->UV_stride);
		write(out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)* symbol_size_in_bytes);

		if (!rgb_output)
		{
			img2buf (p->imgUV, buf, p->size_x_cr, p->size_y_cr, symbol_size_in_bytes, crop_left, crop_right, crop_top, crop_bottom, p->UV_stride);
			write(out, buf, (p->size_y_cr-crop_bottom-crop_top)*(p->size_x_cr-crop_right-crop_left)*symbol_size_in_bytes);
		}
#ifdef __SUPPORT_YUV400__
	}
	else
	{
		int i,j;
		imgpel cr_val = 128; // HP restriction

		p->imgUV = (imgpel *) _aligned_malloc((p->size_y/2) * (p->size_x) * sizeof(imgpel), 16);
		for (j=0; j<p->size_y/2; j++)
			for (i=0; i<p->size_x; i++)
				*(p->imgUV+j*p->size_x/2+i)=cr_val;

		// fake out U=V=128 to make a YUV 4:2:0 stream
		img2buf (p->imgUV, buf, p->size_x, p->size_y/2, symbol_size_in_bytes, crop_left, crop_right, crop_top/2, crop_bottom/2, p->UV_stride);

		write(out, buf, (p->size_y-crop_bottom-crop_top)/2 * (p->size_x-crop_right-crop_left));

		_aligned_free(p->imgUV);

		p->imgUV = NULL;
	}
#endif

	_aligned_free(buf);

	//  fsync(out);
}

/*!
************************************************************************
* \brief
*    Initialize output buffer for direct output
************************************************************************
*/
void init_out_buffer PARGS0()
{
	out_buffer = alloc_frame_store();
#ifdef PAIR_FIELDS_IN_OUTPUT
	pending_output = (StorablePicture *) _aligned_malloc (sizeof(StorablePicture), 16);
	if (NULL==pending_output) no_mem_exit("init_out_buffer");
	pending_output->imgUV = NULL;
	pending_output->imgY  = NULL;
#endif
}

/*!
************************************************************************
* \brief
*    Uninitialize output buffer for direct output
************************************************************************
*/
void uninit_out_buffer PARGS0()
{
	free_frame_store ARGS2(out_buffer, -1);
	out_buffer=NULL;
#ifdef PAIR_FIELDS_IN_OUTPUT
	flush_pending_output(p_out);
	_aligned_free (pending_output);
#endif
}

/*!
************************************************************************
* \brief
*    Initialize picture memory with (Y:0,U:128,V:128)
************************************************************************
*/
void clear_picture(StorablePicture *p)
{
	int i;

	for(i=0;i<p->size_y;i++)
		memset(p->imgY+i*p->Y_stride, 128, p->size_x*sizeof(imgpel)); // HP restriction
	for(i=0;i<p->size_y_cr;i++)
		memset(p->imgUV+i*p->UV_stride, 128, p->size_x*sizeof(imgpel)); // HP restriction
}

/*!
************************************************************************
* \brief
*    Write out not paired direct output fields. A second empty field is generated
*    and combined into the frame buffer.
* \param fs
*    FrameStore that contains a single field
* \param out
*    Output file
************************************************************************
*/
void write_unpaired_field PARGS2(FrameStore* fs, int out)
{
	StorablePicture *p;
	//assert (fs->is_used<3);

	//return;	//Try to output unpaired field to display queue

	if(fs->is_used &TOP_FIELD)
	{
		// we have a top field
		// construct an empty bottom field
		p = fs->top_field;
		fs->bottom_field = get_storable_picture ARGS7(BOTTOM_FIELD, p->size_x, 2*p->size_y, p->size_x_cr, 2*p->size_y_cr, 0, 0);
		fs->bottom_field->imgY        = fs->top_field->imgY     + (fs->top_field->Y_stride>>1);
		fs->bottom_field->imgUV       = fs->top_field->imgUV    + (fs->top_field->UV_stride>>1);
		fs->bottom_field->pic_store_idx = fs->top_field->pic_store_idx;
		//pic_combine_status = 0;	// Frame is full
		fs->bottom_field->chroma_format_idc = p->chroma_format_idc;
		fs->bottom_field->framerate1000 = fs->top_field->framerate1000;
		fs->bottom_field->poc = fs->bottom_field->bottom_poc = fs->top_field->poc + 1;
#ifdef _HW_ACCEL_
		if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
#endif
			clear_picture(fs->bottom_field);
		dpb_combine_field_yuv ARGS1(fs);	
		write_picture ARGS3(fs->frame, out, TOP_FIELD);
	}

	if(fs->is_used &BOTTOM_FIELD)
	{
		// we have a bottom field
		// construct an empty top field
		p = fs->bottom_field;
		fs->top_field = get_storable_picture ARGS7(TOP_FIELD, p->size_x, 2*p->size_y, p->size_x_cr, 2*p->size_y_cr, 0, 0);
		fs->top_field->imgY        = fs->bottom_field->imgY     + (fs->bottom_field->Y_stride>>1);
		fs->top_field->imgUV       = fs->bottom_field->imgUV    + (fs->bottom_field->UV_stride>>1);
		//pic_combine_status = 0;	// Frame is full
		fs->top_field->pic_store_idx = fs->bottom_field->pic_store_idx;
		fs->top_field->framerate1000 = fs->bottom_field->framerate1000;
		fs->top_field->poc = fs->top_field->top_poc = fs->bottom_field->poc - 1;

#ifdef _HW_ACCEL_
		if(g_DXVAVer==IviNotDxva || IMGPAR Hybrid_Decoding)
#endif
			clear_picture(fs->top_field);
		fs ->top_field->frame_cropping_flag = fs->bottom_field->frame_cropping_flag;
		if(fs ->top_field->frame_cropping_flag) 
		{
			fs ->top_field->frame_cropping_rect_top_offset = fs->bottom_field->frame_cropping_rect_top_offset;
			fs ->top_field->frame_cropping_rect_bottom_offset = fs->bottom_field->frame_cropping_rect_bottom_offset;
			fs ->top_field->frame_cropping_rect_left_offset = fs->bottom_field->frame_cropping_rect_left_offset;
			fs ->top_field->frame_cropping_rect_right_offset = fs->bottom_field->frame_cropping_rect_right_offset;
		}
		dpb_combine_field_yuv ARGS1(fs);
		write_picture ARGS3(fs->frame, out, BOTTOM_FIELD);
	}

	fs->is_used=(TOP_FIELD|BOTTOM_FIELD);
}

/*!
************************************************************************
* \brief
*    Write out unpaired fields from output buffer.
* \param out
*    Output file
************************************************************************
*/
void flush_direct_output PARGS1(int out)
{
	if (out_buffer->is_used<(TOP_FIELD|BOTTOM_FIELD)) {
		write_unpaired_field ARGS2(out_buffer, out);
	}
	
	release_storable_picture ARGS2(out_buffer->frame,1);
	out_buffer->frame = NULL;
	release_storable_picture ARGS2(out_buffer->top_field,0);
	out_buffer->top_field = NULL;
	release_storable_picture ARGS2(out_buffer->bottom_field,0);
	out_buffer->bottom_field = NULL;
	out_buffer->is_used = 0;
}


/*!
************************************************************************
* \brief
*    Write a frame (from FrameStore)
* \param fs
*    FrameStore containing the frame
* \param out
*    Output file
************************************************************************
*/
void write_stored_frame PARGS2( FrameStore *fs,int out)
{
	// make sure no direct output field is pending
	flush_direct_output ARGS1(out);

	if (fs->is_used<(TOP_FIELD|BOTTOM_FIELD))
	{
		write_unpaired_field ARGS2(fs, out);
	}
	else
	{
		write_picture ARGS3(fs->frame, out, FRAME);
	}

	fs->is_output = 1;
}

/*!
************************************************************************
* \brief
*    Directly output a picture without storing it in the DPB. Fields 
*    are buffered before they are written to the file.
* \param p
*    Picture for output
* \param out
*    Output file
************************************************************************
*/
CREL_RETURN direct_output PARGS2(StorablePicture *p, int out)
{
	CREL_RETURN ret;
	if (p->structure==FRAME)
	{
		// we have a frame (or complementary field pair)
		// so output it directly
		flush_direct_output ARGS1(out);
		p->repeat_first_field = (IMGPAR firstSlice->m_nDispPicStructure==5 || IMGPAR firstSlice->m_nDispPicStructure==6);
		p->top_field_first = (IMGPAR firstSlice->m_nDispPicStructure>2 && IMGPAR firstSlice->m_nDispPicStructure<7)?(IMGPAR firstSlice->m_nDispPicStructure&1):1;
		write_picture ARGS3(p, out, FRAME);
		release_storable_picture ARGS2(p,1);
	}
	else
	{
		if (p->structure == TOP_FIELD)
		{
			if ((out_buffer->is_used &TOP_FIELD)||((out_buffer->is_used &BOTTOM_FIELD)&&(out_buffer->bottom_field->frame_num != p->frame_num)))
				flush_direct_output ARGS1(out);
			out_buffer->top_field = p;
			out_buffer->frame_num = p->frame_num;
			out_buffer->is_used |= TOP_FIELD;
			if(out_buffer->bottom_field == NULL)
			{
				p->repeat_first_field = 0;
				p->top_field_first = 1;
			}
		}
		else //BOTTOM_FIELD
		{
			if ((out_buffer->is_used &BOTTOM_FIELD)||((out_buffer->is_used &TOP_FIELD)&&(out_buffer->top_field->frame_num != p->frame_num)))
				flush_direct_output ARGS1(out);
			out_buffer->bottom_field = p;
			out_buffer->frame_num = p->frame_num;
			out_buffer->is_used |= BOTTOM_FIELD;
		}

		if (out_buffer->is_used == (TOP_FIELD|BOTTOM_FIELD))
		{
			// we have both fields, so output them
			ret = dpb_combine_field_yuv ARGS1(out_buffer);
			if (FAILED(ret)) {
				return ret;
			}

			out_buffer->frame->top_field_first = (p->structure == TOP_FIELD ? 0 : 1);
			memcpy(&(out_buffer->frame->m_CCCode), out_buffer->frame->top_field_first ? (&(out_buffer->top_field->m_CCCode)):(&(out_buffer->bottom_field->m_CCCode)), sizeof(H264_CC));

			out_buffer->frame->repeat_first_field = 0;
			out_buffer->frame->top_field_first = (p->structure == TOP_FIELD ? 0 : 1);
			write_picture ARGS3(out_buffer->frame, out, FRAME);
			release_storable_picture ARGS2(out_buffer->frame,out_buffer->is_output?0:1);
			out_buffer->frame = NULL;
			release_storable_picture ARGS2(out_buffer->top_field,0);
			out_buffer->top_field = NULL;
			release_storable_picture ARGS2(out_buffer->bottom_field,0);
			out_buffer->bottom_field = NULL;
			out_buffer->is_used = 0;
			out_buffer->frame_num = -1;
		}
	}

	return CREL_OK;
}

void __cdecl H264Debug(const char *szFormat, ...)
{
	char szBuffer[256]; 
	va_list vl; 
	va_start(vl, szFormat);

	_vsnprintf(szBuffer,255,szFormat,vl);
	szBuffer[255]=0;
#ifdef _WIN32_WCE
	{
		WCHAR szBufferW[256];

		MultiByteToWideChar(CP_ACP,0,szBuffer,-1,szBufferW,255);
		szBufferW[255]=0;
		OutputDebugString(szBufferW);
	}
#else
#if defined(__linux__)
	fprintf(stderr,"%s",szBuffer); fflush(stderr);
#elif !defined(__SYMBIAN32__)
	OutputDebugStringA(szBuffer);
#endif
#endif
	va_end(vl);
}
