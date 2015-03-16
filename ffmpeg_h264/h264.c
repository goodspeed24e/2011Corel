
 
/**
 * @file h264.c
 * H.264 / AVC / MPEG4 part10 codec.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */
 
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "dsputil.h"
#include "avcodec.h"
#include "mpegvideo.h"
#include "h264data.h"
#include "golomb.h"
#include "h264context.h"
#include "fill_func.h"
#include "cavlc.h"
#include "pred_mb.h"
#include "filter_mb.h"

#undef NDEBUG
#include <assert.h>

/**
 * gets the directionally predicted 16x8 MV.
 * @param n the block index
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */

/**
 * Decodes a network abstraction layer unit.
 * @param consumed is the number of bytes used as input
 * @param length is the length of the array
 * @param dst_length is the number of decoded bytes FIXME here or a decode rbsp ttailing?
 * @returns decoded bytes, might be src+1 if no escapes 
 */
static uint8_t *decode_nal(H264Context *h, uint8_t *src, int *dst_length, int *consumed, int length){
    int i, si, di;
    uint8_t *dst;
//    src[0]&0x80;		//forbidden bit
    h->nal_ref_idc= src[0]>>5;
    h->nal_unit_type= src[0]&0x1F;
    src++; length--;
#if 0    
    for(i=0; i<length; i++)
        printf("%2X ", src[i]);
#endif
    for(i=0; i+1<length; i+=2){
        if(src[i]) continue;
        if(i>0 && src[i-1]==0) i--;
        if(i+2<length && src[i+1]==0 && src[i+2]<=3){
            if(src[i+2]!=3){
                /* startcode, so we must be past the end */
                length=i;
            }
            break;
        }
    }
    if(i>=length-1){ //no escaped 0
        *dst_length= length;
        *consumed= length+1; //+1 for the header
        return src; 
    }
    h->rbsp_buffer= av_fast_realloc(h->rbsp_buffer, &h->rbsp_buffer_size, length);
    dst= h->rbsp_buffer;
//printf("deoding esc\n");
    si=di=0;
    while(si<length){ 
        //remove escapes (very rare 1:2^22)
        if(si+2<length && src[si]==0 && src[si+1]==0 && src[si+2]<=3){
            if(src[si+2]==3){ //escape
                dst[di++]= 0;
                dst[di++]= 0;
                si+=3;
                continue;
            }else //next start code
                break;
        }
        dst[di++]= src[si++];
    }
    *dst_length= di;
    *consumed= si + 1;//+1 for the header
//FIXME store exact number of bits in the getbitcontext (its needed for decoding)
    return dst;
}
//#endif	
/**
 * identifies the exact end of the bitstream
 * @return the length of the trailing, or 0 if damaged
 */
static int decode_rbsp_trailing(uint8_t *src){
    int v= *src;
    int r;
    tprintf("rbsp trailing %X\n", v);
    for(r=1; r<9; r++){
        if(v&1) return r;
        v>>=1;
    }
    return 0;
}
/**
 * idct tranforms the 16 dc values and dequantize them.
 * @param qp quantization parameter
 */
static void h264_luma_dc_dequant_idct_c(DCTELEM *block, int qp){
    const int qmul= dequant_coeff[qp][0];
#define stride 16
    int i;
    int temp[16]; //FIXME check if this is a good idea
    static const int x_offset[4]={0, 1*stride, 4* stride,  5*stride};
    static const int y_offset[4]={0, 2*stride, 8* stride, 10*stride};
//memset(block, 64, 2*256);
//return;
    for(i=0; i<4; i++){
        const int offset= y_offset[i];
        const int z0= block[offset+stride*0] + block[offset+stride*4];
        const int z1= block[offset+stride*0] - block[offset+stride*4];
        const int z2= block[offset+stride*1] - block[offset+stride*5];
        const int z3= block[offset+stride*1] + block[offset+stride*5];
        temp[4*i+0]= z0+z3;
        temp[4*i+1]= z1+z2;
        temp[4*i+2]= z1-z2;
        temp[4*i+3]= z0-z3;
    }
    for(i=0; i<4; i++){
        const int offset= x_offset[i];
        const int z0= temp[4*0+i] + temp[4*2+i];
        const int z1= temp[4*0+i] - temp[4*2+i];
        const int z2= temp[4*1+i] - temp[4*3+i];
        const int z3= temp[4*1+i] + temp[4*3+i];
        block[stride*0 +offset]= ((z0 + z3)*qmul + 2)>>2; //FIXME think about merging this into decode_resdual
        block[stride*2 +offset]= ((z1 + z2)*qmul + 2)>>2;
        block[stride*8 +offset]= ((z1 - z2)*qmul + 2)>>2;
        block[stride*10+offset]= ((z0 - z3)*qmul + 2)>>2;
    }
}

#undef xStride
#undef stride
static void chroma_dc_dequant_idct_c(DCTELEM *block, int qp){
    const int qmul= dequant_coeff[qp][0];
    const int stride= 16*2;
    const int xStride= 16;
    int a,b,c,d,e;
    a= block[stride*0 + xStride*0];
    b= block[stride*0 + xStride*1];
    c= block[stride*1 + xStride*0];
    d= block[stride*1 + xStride*1];
    e= a-b;
    a= a+b;
    b= c-d;
    c= c+d;
    block[stride*0 + xStride*0]= ((a+c)*qmul + 0)>>1;
    block[stride*0 + xStride*1]= ((e+b)*qmul + 0)>>1;
    block[stride*1 + xStride*0]= ((a-c)*qmul + 0)>>1;
    block[stride*1 + xStride*1]= ((e-b)*qmul + 0)>>1;
}

/**
 * gets the chroma qp.
 */


/**
 *
 */
static void h264_add_idct_c(uint8_t *dst, DCTELEM *block, int stride){
    int i;
    uint8_t *cm = cropTbl + MAX_NEG_CROP;
    block[0] += 32;
    for(i=0; i<4; i++){
        const int z0=  block[0 + 4*i]     +  block[2 + 4*i];
        const int z1=  block[0 + 4*i]     -  block[2 + 4*i];
        const int z2= (block[1 + 4*i]>>1) -  block[3 + 4*i];
        const int z3=  block[1 + 4*i]     + (block[3 + 4*i]>>1);
        block[0 + 4*i]= z0 + z3;
        block[1 + 4*i]= z1 + z2;
        block[2 + 4*i]= z1 - z2;
        block[3 + 4*i]= z0 - z3;
    }
    for(i=0; i<4; i++){
        const int z0=  block[i + 4*0]     +  block[i + 4*2];
        const int z1=  block[i + 4*0]     -  block[i + 4*2];
        const int z2= (block[i + 4*1]>>1) -  block[i + 4*3];
        const int z3=  block[i + 4*1]     + (block[i + 4*3]>>1);
        dst[i + 0*stride]= cm[ dst[i + 0*stride] + ((z0 + z3) >> 6) ];
        dst[i + 1*stride]= cm[ dst[i + 1*stride] + ((z1 + z2) >> 6) ];
        dst[i + 2*stride]= cm[ dst[i + 2*stride] + ((z1 - z2) >> 6) ];
        dst[i + 3*stride]= cm[ dst[i + 3*stride] + ((z0 - z3) >> 6) ];
    }
}


static inline void mc_dir_part(H264Context *h, Picture *pic, int n, int square, int chroma_height, int delta, int list,
                           uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                           int src_x_offset, int src_y_offset,
                           qpel_mc_func *qpix_op, h264_chroma_mc_func chroma_op){
    MpegEncContext * const s = &h->s;
    const int mx= h->mv_cache[list][ scan8[n] ][0] + src_x_offset*8;
    const int my= h->mv_cache[list][ scan8[n] ][1] + src_y_offset*8;
    const int luma_xy= (mx&3) + ((my&3)<<2);
    uint8_t * src_y = pic->data[0] + (mx>>2) + (my>>2)*s->linesize;
    uint8_t * src_cb= pic->data[1] + (mx>>3) + (my>>3)*s->uvlinesize;
    uint8_t * src_cr= pic->data[2] + (mx>>3) + (my>>3)*s->uvlinesize;
    int extra_width= (s->flags&CODEC_FLAG_EMU_EDGE) ? 0 : 16; //FIXME increase edge?, IMHO not worth it
    int extra_height= extra_width;
    int emu=0;
    const int full_mx= mx>>2;
    const int full_my= my>>2;
    
    assert(pic->data[0]);
    
    if(mx&7) extra_width -= 3;
    if(my&7) extra_height -= 3;
    
    if(   full_mx < 0-extra_width 
       || full_my < 0-extra_height 
       || full_mx + 16/*FIXME*/ > s->width + extra_width 
       || full_my + 16/*FIXME*/ > s->height + extra_height){
        ff_emulated_edge_mc(s->edge_emu_buffer, src_y - 2 - 2*s->linesize, s->linesize, 16+5, 16+5/*FIXME*/, full_mx-2, full_my-2, s->width, s->height);
            src_y= s->edge_emu_buffer + 2 + 2*s->linesize;
        emu=1;
    }
    
    qpix_op[luma_xy](dest_y, src_y, s->linesize); //FIXME try variable height perhaps?
    if(!square){
        qpix_op[luma_xy](dest_y + delta, src_y + delta, s->linesize);
    }
    
    if(s->flags&CODEC_FLAG_GRAY) return;
    
    if(emu){
        ff_emulated_edge_mc(s->edge_emu_buffer, src_cb, s->uvlinesize, 9, 9/*FIXME*/, (mx>>3), (my>>3), s->width>>1, s->height>>1);
            src_cb= s->edge_emu_buffer;
    }
    chroma_op(dest_cb, src_cb, s->uvlinesize, chroma_height, mx&7, my&7);
    if(emu){
        ff_emulated_edge_mc(s->edge_emu_buffer, src_cr, s->uvlinesize, 9, 9/*FIXME*/, (mx>>3), (my>>3), s->width>>1, s->height>>1);
            src_cr= s->edge_emu_buffer;
    }
    chroma_op(dest_cr, src_cr, s->uvlinesize, chroma_height, mx&7, my&7);
}
static inline void mc_part(H264Context *h, int n, int square, int chroma_height, int delta,
                           uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                           int x_offset, int y_offset,
                           qpel_mc_func *qpix_put, h264_chroma_mc_func chroma_put,
                           qpel_mc_func *qpix_avg, h264_chroma_mc_func chroma_avg,
                           int list0, int list1){
    MpegEncContext * const s = &h->s;
    qpel_mc_func *qpix_op=  qpix_put;
    h264_chroma_mc_func chroma_op= chroma_put;
    
    dest_y  += 2*x_offset + 2*y_offset*s->linesize;
    dest_cb +=   x_offset +   y_offset*s->uvlinesize;
    dest_cr +=   x_offset +   y_offset*s->uvlinesize;
    x_offset += 8*s->mb_x;
    y_offset += 8*s->mb_y;
    
    if(list0){
        Picture *ref= &h->ref_list[0][ h->ref_cache[0][ scan8[n] ] ];
        mc_dir_part(h, ref, n, square, chroma_height, delta, 0,
                           dest_y, dest_cb, dest_cr, x_offset, y_offset,
                           qpix_op, chroma_op);
        qpix_op=  qpix_avg;
        chroma_op= chroma_avg;
    }
    if(list1){
        Picture *ref= &h->ref_list[1][ h->ref_cache[1][ scan8[n] ] ];
        mc_dir_part(h, ref, n, square, chroma_height, delta, 1,
                           dest_y, dest_cb, dest_cr, x_offset, y_offset,
                           qpix_op, chroma_op);
    }
}

/*Inter块的运动补偿*/
static void hl_motion(H264Context *h, uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                      qpel_mc_func (*qpix_put)[16], h264_chroma_mc_func (*chroma_put),
                      qpel_mc_func (*qpix_avg)[16], h264_chroma_mc_func (*chroma_avg)){
    MpegEncContext * const s = &h->s;
    const int mb_xy= s->mb_x + s->mb_y*s->mb_stride;
    const int mb_type= s->current_picture.mb_type[mb_xy];
    
    assert(IS_INTER(mb_type));
    
    if(IS_16X16(mb_type)){
        mc_part(h, 0, 1, 8, 0, dest_y, dest_cb, dest_cr, 0, 0,
                qpix_put[0], chroma_put[0], qpix_avg[0], chroma_avg[0],
                IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1));
    }else if(IS_16X8(mb_type)){
        mc_part(h, 0, 0, 4, 8, dest_y, dest_cb, dest_cr, 0, 0,
                qpix_put[1], chroma_put[0], qpix_avg[1], chroma_avg[0],
                IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1));
        mc_part(h, 8, 0, 4, 8, dest_y, dest_cb, dest_cr, 0, 4,
                qpix_put[1], chroma_put[0], qpix_avg[1], chroma_avg[0],
                IS_DIR(mb_type, 1, 0), IS_DIR(mb_type, 1, 1));
    }else if(IS_8X16(mb_type)){
        mc_part(h, 0, 0, 8, 8*s->linesize, dest_y, dest_cb, dest_cr, 0, 0,
                qpix_put[1], chroma_put[1], qpix_avg[1], chroma_avg[1],
                IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1));
        mc_part(h, 4, 0, 8, 8*s->linesize, dest_y, dest_cb, dest_cr, 4, 0,
                qpix_put[1], chroma_put[1], qpix_avg[1], chroma_avg[1],
                IS_DIR(mb_type, 1, 0), IS_DIR(mb_type, 1, 1));
    }else{
        int i;
        
        assert(IS_8X8(mb_type));
        for(i=0; i<4; i++){
            const int sub_mb_type= h->sub_mb_type[i];
            const int n= 4*i;
            int x_offset= (i&1)<<2;
            int y_offset= (i&2)<<1;
            if(IS_SUB_8X8(sub_mb_type)){
                mc_part(h, n, 1, 4, 0, dest_y, dest_cb, dest_cr, x_offset, y_offset,
                    qpix_put[1], chroma_put[1], qpix_avg[1], chroma_avg[1],
                    IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
            }else if(IS_SUB_8X4(sub_mb_type)){
                mc_part(h, n  , 0, 2, 4, dest_y, dest_cb, dest_cr, x_offset, y_offset,
                    qpix_put[2], chroma_put[1], qpix_avg[2], chroma_avg[1],
                    IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
                mc_part(h, n+2, 0, 2, 4, dest_y, dest_cb, dest_cr, x_offset, y_offset+2,
                    qpix_put[2], chroma_put[1], qpix_avg[2], chroma_avg[1],
                    IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
            }else if(IS_SUB_4X8(sub_mb_type)){
                mc_part(h, n  , 0, 4, 4*s->linesize, dest_y, dest_cb, dest_cr, x_offset, y_offset,
                    qpix_put[2], chroma_put[2], qpix_avg[2], chroma_avg[2],
                    IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
                mc_part(h, n+1, 0, 4, 4*s->linesize, dest_y, dest_cb, dest_cr, x_offset+2, y_offset,
                    qpix_put[2], chroma_put[2], qpix_avg[2], chroma_avg[2],
                    IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
            }else{
                int j;
                assert(IS_SUB_4X4(sub_mb_type));
                for(j=0; j<4; j++){
                    int sub_x_offset= x_offset + 2*(j&1);
                    int sub_y_offset= y_offset +   (j&2);
                    mc_part(h, n+j, 1, 2, 0, dest_y, dest_cb, dest_cr, sub_x_offset, sub_y_offset,
                        qpix_put[2], chroma_put[2], qpix_avg[2], chroma_avg[2],
                        IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
                }
            }
        }
    }
}


static void free_tables(H264Context *h){
    av_freep(&h->intra4x4_pred_mode);
    av_freep(&h->chroma_pred_mode_table);
    av_freep(&h->cbp_table);
    av_freep(&h->mvd_table[0]);
    av_freep(&h->mvd_table[1]);
    av_freep(&h->non_zero_count);
    av_freep(&h->slice_table_base);
    av_freep(&h->top_border);
    h->slice_table= NULL;
    av_freep(&h->mb2b_xy);
    av_freep(&h->mb2b8_xy);
}
/**
 * allocates tables.
 * needs widzh/height
 */
static int alloc_tables(H264Context *h){
    MpegEncContext * const s = &h->s;
    const int big_mb_num= s->mb_stride * (s->mb_height+1);
    int x,y;
    CHECKED_ALLOCZ(h->intra4x4_pred_mode, big_mb_num * 8  * sizeof(uint8_t))
    CHECKED_ALLOCZ(h->non_zero_count    , big_mb_num * 16 * sizeof(uint8_t))
    CHECKED_ALLOCZ(h->slice_table_base  , big_mb_num * sizeof(uint8_t))
    CHECKED_ALLOCZ(h->top_border       , s->mb_width * (16+8+8) * sizeof(uint8_t))
    if( h->pps.cabac ) {
        CHECKED_ALLOCZ(h->chroma_pred_mode_table, big_mb_num * sizeof(uint8_t))
        CHECKED_ALLOCZ(h->cbp_table, big_mb_num * sizeof(uint16_t))
        CHECKED_ALLOCZ(h->mvd_table[0], 32*big_mb_num * sizeof(uint16_t));
        CHECKED_ALLOCZ(h->mvd_table[1], 32*big_mb_num * sizeof(uint16_t));
    }
    memset(h->slice_table_base, -1, big_mb_num  * sizeof(uint8_t));
    h->slice_table= h->slice_table_base + s->mb_stride + 1;
    CHECKED_ALLOCZ(h->mb2b_xy  , big_mb_num * sizeof(uint16_t));
    CHECKED_ALLOCZ(h->mb2b8_xy , big_mb_num * sizeof(uint16_t));
    for(y=0; y<s->mb_height; y++){
        for(x=0; x<s->mb_width; x++){
            const int mb_xy= x + y*s->mb_stride;
            const int b_xy = 4*x + 4*y*h->b_stride;
            const int b8_xy= 2*x + 2*y*h->b8_stride;
        
            h->mb2b_xy [mb_xy]= b_xy;
            h->mb2b8_xy[mb_xy]= b8_xy;
        }
    }
    
    return 0;
fail:
    free_tables(h);
    return -1;
}
static void common_init(H264Context *h){
    MpegEncContext * const s = &h->s;
    s->width = s->avctx->width;
    s->height = s->avctx->height;
    s->codec_id= s->avctx->codec->id;
    
    init_pred_ptrs(h);
    s->unrestricted_mv=1;
    s->decode=1; //FIXME
}

/*
  H.264解码器初始化
*/
static int decode_init(AVCodecContext *avctx){
	/*获取解码器上下文指针*/
	H264Context *h= avctx->priv_data;
	/*获取MpegEncContext指针*/
	MpegEncContext * const s = &h->s;
	/*设置默认的参数*/
	MPV_decode_defaults(s);
	/*赋值解码器上下文*/
	s->avctx = avctx;
	/*初始化图像宽和高，初始化I帧预测函数*/
	common_init(h);
	s->out_format = FMT_H264;
	s->workaround_bugs= avctx->workaround_bugs;

	s->low_delay= 1;
	/*像素格式为YUV420 Plane*/
	avctx->pix_fmt= PIX_FMT_YUV420P;
	// added this
	//avctx->flags = CODEC_FLAG_EMU_EDGE;
	
	/*初始化VLC表*/
	decode_init_vlc(h);

	return 0;
}


static void frame_start(H264Context *h){
    MpegEncContext * const s = &h->s;
    int i;
    MPV_frame_start(s, s->avctx);
    ff_er_frame_start(s);
    h->mmco_index=0;
    assert(s->linesize && s->uvlinesize);
    for(i=0; i<16; i++){
        h->block_offset[i]= 4*((scan8[i] - scan8[0])&7) + 4*s->linesize*((scan8[i] - scan8[0])>>3);
        h->chroma_subblock_offset[i]= 2*((scan8[i] - scan8[0])&7) + 2*s->uvlinesize*((scan8[i] - scan8[0])>>3);
    }
    for(i=0; i<4; i++){
        h->block_offset[16+i]=
        h->block_offset[20+i]= 4*((scan8[i] - scan8[0])&7) + 4*s->uvlinesize*((scan8[i] - scan8[0])>>3);
    }
//    s->decode= (s->flags&CODEC_FLAG_PSNR) || !s->encoding || s->current_picture.reference /*|| h->contains_intra*/ || 1;
}
static inline void backup_mb_border(H264Context *h, uint8_t *src_y, uint8_t *src_cb, uint8_t *src_cr, int linesize, int uvlinesize){
    MpegEncContext * const s = &h->s;
    int i;
    
    src_y  -=   linesize;
    src_cb -= uvlinesize;
    src_cr -= uvlinesize;
    h->left_border[0]= h->top_border[s->mb_x][15];
    for(i=1; i<17; i++){
        h->left_border[i]= src_y[15+i*  linesize];
    }
    
    *(uint64_t*)(h->top_border[s->mb_x]+0)= *(uint64_t*)(src_y +  16*linesize);
    *(uint64_t*)(h->top_border[s->mb_x]+8)= *(uint64_t*)(src_y +8+16*linesize);
    if(!(s->flags&CODEC_FLAG_GRAY)){
        h->left_border[17  ]= h->top_border[s->mb_x][16+7];
        h->left_border[17+9]= h->top_border[s->mb_x][24+7];
        for(i=1; i<9; i++){
            h->left_border[i+17  ]= src_cb[7+i*uvlinesize];
            h->left_border[i+17+9]= src_cr[7+i*uvlinesize];
        }
        *(uint64_t*)(h->top_border[s->mb_x]+16)= *(uint64_t*)(src_cb+8*uvlinesize);
        *(uint64_t*)(h->top_border[s->mb_x]+24)= *(uint64_t*)(src_cr+8*uvlinesize);
    }
}
static inline void xchg_mb_border(H264Context *h, uint8_t *src_y, uint8_t *src_cb, uint8_t *src_cr, int linesize, int uvlinesize, int xchg){
    MpegEncContext * const s = &h->s;
    int temp8, i;
    uint64_t temp64;
    src_y  -=   linesize + 1;
    src_cb -= uvlinesize + 1;
    src_cr -= uvlinesize + 1;
#define XCHG(a,b,t,xchg)\
t= a;\
if(xchg)\
    a= b;\
b= t;
    
    for(i=0; i<17; i++){
        XCHG(h->left_border[i     ], src_y [i*  linesize], temp8, xchg);
    }
    
    XCHG(*(uint64_t*)(h->top_border[s->mb_x]+0), *(uint64_t*)(src_y +1), temp64, xchg);
    XCHG(*(uint64_t*)(h->top_border[s->mb_x]+8), *(uint64_t*)(src_y +9), temp64, 1);
    if(!(s->flags&CODEC_FLAG_GRAY)){
        for(i=0; i<9; i++){
            XCHG(h->left_border[i+17  ], src_cb[i*uvlinesize], temp8, xchg);
            XCHG(h->left_border[i+17+9], src_cr[i*uvlinesize], temp8, xchg);
        }
        XCHG(*(uint64_t*)(h->top_border[s->mb_x]+16), *(uint64_t*)(src_cb+1), temp64, 1);
        XCHG(*(uint64_t*)(h->top_border[s->mb_x]+24), *(uint64_t*)(src_cr+1), temp64, 1);
    }
}

/*宏块解码*/
static void hl_decode_mb(H264Context *h){
	MpegEncContext * const s = &h->s;
	const int mb_x= s->mb_x;			//宏块宽度
	const int mb_y= s->mb_y;			//宏块高度
	const int mb_xy= mb_x + mb_y*s->mb_stride;	 	   //宏块序号
	const int mb_type= s->current_picture.mb_type[mb_xy];//宏块模式
	uint8_t  *dest_y, *dest_cb, *dest_cr;
	int linesize, uvlinesize /*dct_offset*/;
	int i;
	if(!s->decode)
		return;
	/*指向当前的图像空间，以保存解码图像*/
	dest_y  = s->current_picture.data[0] + (mb_y * 16* s->linesize  ) + mb_x * 16;
	dest_cb = s->current_picture.data[1] + (mb_y * 8 * s->uvlinesize) + mb_x * 8;
	dest_cr = s->current_picture.data[2] + (mb_y * 8 * s->uvlinesize) + mb_x * 8;
	/*图像场解码*/
	if (h->mb_field_decoding_flag) {	/*交织编码标志*/
		linesize = s->linesize * 2;
		uvlinesize = s->uvlinesize * 2;
		if(mb_y&1){
			dest_y -= s->linesize*15;
			dest_cb-= s->linesize*7;
			dest_cr-= s->linesize*7;
		}
	} else {
		linesize = s->linesize;
		uvlinesize = s->uvlinesize;
	}
	 /************************************ Intra块 *********************************/
	if(IS_INTRA(mb_type)){
		/*环路滤波启用，改变宏块的边界*/
		if(h->deblocking_filter)
			xchg_mb_border(h, dest_y, dest_cb, dest_cr, linesize, uvlinesize, 1);
        if(!(s->flags&CODEC_FLAG_GRAY)){	/*不是灰度图像*/
		/*两个色度分量预测*/
		h->pred8x8[ h->chroma_pred_mode ](dest_cb, uvlinesize);
		h->pred8x8[ h->chroma_pred_mode ](dest_cr, uvlinesize);
        }
		/*4x4的intra模式*/
        if(IS_INTRA4x4(mb_type)){
            if(!s->encoding){
                for(i=0; i<16; i++){
                    uint8_t * const ptr= dest_y + h->block_offset[i];
                    uint8_t *topright= ptr + 4 - linesize;
                    const int topright_avail= (h->topright_samples_available<<i)&0x8000;
                    const int dir= h->intra4x4_pred_mode_cache[ scan8[i] ];
                    int tr;
			/*右上块*/ 
                    if(!topright_avail){
                        tr= ptr[3 - linesize]*0x01010101;
                        topright= (uint8_t*) &tr;
                    }else if(i==5 && h->deblocking_filter){
                        tr= *(uint32_t*)h->top_border[mb_x+1];
                        topright= (uint8_t*) &tr;
                    }
			/*4x4预测*/
                    h->pred4x4[ dir ](ptr, topright, linesize);
			/*具有非零系数*/
                    if(h->non_zero_count_cache[ scan8[i] ]){
			/*反DCT变换，并补偿*/
                        if(s->codec_id == CODEC_ID_H264)
                            h264_add_idct_c(ptr, h->mb + i*16, linesize);
                        else
                            ;//svq3_add_idct_c(ptr, h->mb + i*16, linesize, s->qscale, 0);
                    }
                }
            }
        }else{/*非4x4模式，16x16模式*/
           /*16x16的亮度预测*/
            h->pred16x16[ h->intra16x16_pred_mode ](dest_y , linesize);
			/*16个直流分量DC反变换、反量化*/
            if(s->codec_id == CODEC_ID_H264)
                h264_luma_dc_dequant_idct_c(h->mb, s->qscale);
            else
                ;//svq3_luma_dc_dequant_idct_c(h->mb, s->qscale);
        }
		 /*存在deblock，处理块边界*/
        if(h->deblocking_filter)
            xchg_mb_border(h, dest_y, dest_cb, dest_cr, linesize, uvlinesize, 0);
    }else {
       /*************************** Inter块运动补偿***************************/
        hl_motion(h, dest_y, dest_cb, dest_cr,
                  s->dsp.put_h264_qpel_pixels_tab, s->dsp.put_h264_chroma_pixels_tab, 
                  s->dsp.avg_h264_qpel_pixels_tab, s->dsp.avg_h264_chroma_pixels_tab);
    }

    if(!IS_INTRA4x4(mb_type)){/*非4x4 的Intra模式*/
        for(i=0; i<16; i++){
            if(h->non_zero_count_cache[ scan8[i] ] || h->mb[i*16]){ //FIXME benchmark weird rule, & below
                uint8_t * const ptr= dest_y + h->block_offset[i];
		     /*反变换，并补偿*/
                h264_add_idct_c(ptr, h->mb + i*16, linesize);
            }
        }
    }

	/*色度IDCT、IQUANT*/
	if(!(s->flags&CODEC_FLAG_GRAY)){  /*不是灰度图像*/
	/*色度的直流分量IDCT/IQUANT*/
        chroma_dc_dequant_idct_c(h->mb + 16*16, h->chroma_qp);
        chroma_dc_dequant_idct_c(h->mb + 16*16+4*16, h->chroma_qp);
	/*色度的交流分量IDCT*/
        if(s->codec_id == CODEC_ID_H264){
		 /*色度U的反变换，并补偿*/
		for(i=16; i<16+4; i++){
                if(h->non_zero_count_cache[ scan8[i] ] || h->mb[i*16]){
                    uint8_t * const ptr= dest_cb + h->block_offset[i];
                    h264_add_idct_c(ptr, h->mb + i*16, uvlinesize);
                }
            }
		 /*色度V的反变换，并补偿*/
            for(i=20; i<20+4; i++){
                if(h->non_zero_count_cache[ scan8[i] ] || h->mb[i*16]){
                    uint8_t * const ptr= dest_cr + h->block_offset[i];
                    h264_add_idct_c(ptr, h->mb + i*16, uvlinesize);
                }
            }
        }else{

        }
    }
	/*deblock 的后处理，备份宏块的边界*/
	if(h->deblocking_filter) {
		backup_mb_border(h, dest_y, dest_cb, dest_cr, linesize, uvlinesize);
		/*环路滤波*/
		filter_mb(h, mb_x, mb_y, dest_y, dest_cb, dest_cr);
    }
}



/**
 * fills the default_ref_list.
 */
static int fill_default_ref_list(H264Context *h){
    MpegEncContext * const s = &h->s;
    int i;
    Picture sorted_short_ref[16];
    
    if(h->slice_type==B_TYPE){
        int out_i;
        int limit= -1;
        for(out_i=0; out_i<h->short_ref_count; out_i++){
            int best_i=-1;
            int best_poc=-1;
            for(i=0; i<h->short_ref_count; i++){
                const int poc= h->short_ref[i]->poc;
                if(poc > limit && poc < best_poc){
                    best_poc= poc;
                    best_i= i;
                }
            }
            
            assert(best_i != -1);
            
            limit= best_poc;
            sorted_short_ref[out_i]= *h->short_ref[best_i];
        }
    }
    if(s->picture_structure == PICT_FRAME){
        if(h->slice_type==B_TYPE){
            const int current_poc= s->current_picture_ptr->poc;
            int list;
            for(list=0; list<2; list++){
                int index=0;
                for(i=0; i<h->short_ref_count && index < h->ref_count[list]; i++){
                    const int i2= list ? h->short_ref_count - i - 1 : i;
                    const int poc= sorted_short_ref[i2].poc;
                    
                    if(sorted_short_ref[i2].reference != 3) continue; //FIXME refernce field shit
                    if((list==1 && poc > current_poc) || (list==0 && poc < current_poc)){
                        h->default_ref_list[list][index  ]= sorted_short_ref[i2];
                        h->default_ref_list[list][index++].pic_id= sorted_short_ref[i2].frame_num;
                    }
                }
                for(i=0; i<h->long_ref_count && index < h->ref_count[ list ]; i++){
                    if(h->long_ref[i]->reference != 3) continue;
                    h->default_ref_list[ list ][index  ]= *h->long_ref[i];
                    h->default_ref_list[ list ][index++].pic_id= i;;
                }
                
                if(h->long_ref_count > 1 && h->short_ref_count==0){
                    Picture temp= h->default_ref_list[1][0];
                    h->default_ref_list[1][0] = h->default_ref_list[1][1];
                    h->default_ref_list[1][0] = temp;
                }
                if(index < h->ref_count[ list ])
                    memset(&h->default_ref_list[list][index], 0, sizeof(Picture)*(h->ref_count[ list ] - index));
            }
        }else{
            int index=0;
            for(i=0; i<h->short_ref_count && index < h->ref_count[0]; i++){
                if(h->short_ref[i]->reference != 3) continue; //FIXME refernce field shit
                h->default_ref_list[0][index  ]= *h->short_ref[i];
                h->default_ref_list[0][index++].pic_id= h->short_ref[i]->frame_num;
            }
            for(i=0; i<h->long_ref_count && index < h->ref_count[0]; i++){
                if(h->long_ref[i]->reference != 3) continue;
                h->default_ref_list[0][index  ]= *h->long_ref[i];
                h->default_ref_list[0][index++].pic_id= i;;
            }
            if(index < h->ref_count[0])
                memset(&h->default_ref_list[0][index], 0, sizeof(Picture)*(h->ref_count[0] - index));
        }
    }else{ //FIELD
        if(h->slice_type==B_TYPE){
        }else{
            //FIXME second field balh
        }
    }
    return 0;
}
static int decode_ref_pic_list_reordering(H264Context *h){
    MpegEncContext * const s = &h->s;
    int list;
    
    if(h->slice_type==I_TYPE || h->slice_type==SI_TYPE) return 0; //FIXME move beofre func
    
    for(list=0; list<2; list++){
        memcpy(h->ref_list[list], h->default_ref_list[list], sizeof(Picture)*h->ref_count[list]);
        if(get_bits1(&s->gb)){
            int pred= h->curr_pic_num;
            int index;
            for(index=0; ; index++){
                int reordering_of_pic_nums_idc= get_ue_golomb(&s->gb);
                int pic_id;
                int i;
                
                
                if(index >= h->ref_count[list]){
                    av_log(h->s.avctx, AV_LOG_ERROR, "reference count overflow\n");
                    return -1;
                }
                
                if(reordering_of_pic_nums_idc<3){
                    if(reordering_of_pic_nums_idc<2){
                        const int abs_diff_pic_num= get_ue_golomb(&s->gb) + 1;
                        if(abs_diff_pic_num >= h->max_pic_num){
                            av_log(h->s.avctx, AV_LOG_ERROR, "abs_diff_pic_num overflow\n");
                            return -1;
                        }
                        if(reordering_of_pic_nums_idc == 0) pred-= abs_diff_pic_num;
                        else                                pred+= abs_diff_pic_num;
                        pred &= h->max_pic_num - 1;
                    
                        for(i= h->ref_count[list]-1; i>=index; i--){
                            if(h->ref_list[list][i].pic_id == pred && h->ref_list[list][i].long_ref==0)
                                break;
                        }
                    }else{
                        pic_id= get_ue_golomb(&s->gb); //long_term_pic_idx
                        for(i= h->ref_count[list]-1; i>=index; i--){
                            if(h->ref_list[list][i].pic_id == pic_id && h->ref_list[list][i].long_ref==1)
                                break;
                        }
                    }
                    if(i < index){
                        av_log(h->s.avctx, AV_LOG_ERROR, "reference picture missing during reorder\n");
                        memset(&h->ref_list[list][index], 0, sizeof(Picture)); //FIXME
                    }else if(i > index){
                        Picture tmp= h->ref_list[list][i];
                        for(; i>index; i--){
                            h->ref_list[list][i]= h->ref_list[list][i-1];
                        }
                        h->ref_list[list][index]= tmp;
                    }
                }else if(reordering_of_pic_nums_idc==3) 
                    break;
                else{
                    av_log(h->s.avctx, AV_LOG_ERROR, "illegal reordering_of_pic_nums_idc\n");
                    return -1;
                }
            }
        }
        if(h->slice_type!=B_TYPE) break;
    }
    return 0;    
}
static int pred_weight_table(H264Context *h){
    MpegEncContext * const s = &h->s;
    int list, i;
    
    h->luma_log2_weight_denom= get_ue_golomb(&s->gb);
    h->chroma_log2_weight_denom= get_ue_golomb(&s->gb);
    for(list=0; list<2; list++){
        for(i=0; i<h->ref_count[list]; i++){
            int luma_weight_flag, chroma_weight_flag;
            
            luma_weight_flag= get_bits1(&s->gb);
            if(luma_weight_flag){
                h->luma_weight[list][i]= get_se_golomb(&s->gb);
                h->luma_offset[list][i]= get_se_golomb(&s->gb);
            }
            chroma_weight_flag= get_bits1(&s->gb);
            if(chroma_weight_flag){
                int j;
                for(j=0; j<2; j++){
                    h->chroma_weight[list][i][j]= get_se_golomb(&s->gb);
                    h->chroma_offset[list][i][j]= get_se_golomb(&s->gb);
                }
            }
        }
        if(h->slice_type != B_TYPE) break;
    }
    return 0;
}
/**
 * instantaneos decoder refresh.
 */
static void idr(H264Context *h){
    int i;
    for(i=0; i<h->long_ref_count; i++){
        h->long_ref[i]->reference=0;
        h->long_ref[i]= NULL;
    }
    h->long_ref_count=0;
    for(i=0; i<h->short_ref_count; i++){
        h->short_ref[i]->reference=0;
        h->short_ref[i]= NULL;
    }
    h->short_ref_count=0;
}
/**
 *
 * @return the removed picture or NULL if an error occures
 */
static Picture * remove_short(H264Context *h, int frame_num){
    MpegEncContext * const s = &h->s;
    int i;
    
    if(s->avctx->debug&FF_DEBUG_MMCO)
        av_log(h->s.avctx, AV_LOG_DEBUG, "remove short %d count %d\n", frame_num, h->short_ref_count);
    
    for(i=0; i<h->short_ref_count; i++){
        Picture *pic= h->short_ref[i];
        if(s->avctx->debug&FF_DEBUG_MMCO)
            av_log(h->s.avctx, AV_LOG_DEBUG, "%d %d %p\n", i, pic->frame_num, pic);
        if(pic->frame_num == frame_num){
            h->short_ref[i]= NULL;
            memmove(&h->short_ref[i], &h->short_ref[i+1], (h->short_ref_count - i - 1)*sizeof(Picture*));
            h->short_ref_count--;
            return pic;
        }
    }
    return NULL;
}
/**
 *
 * @return the removed picture or NULL if an error occures
 */
static Picture * remove_long(H264Context *h, int i){
    Picture *pic;
    if(i >= h->long_ref_count) return NULL;
    pic= h->long_ref[i];
    if(pic==NULL) return NULL;
    
    h->long_ref[i]= NULL;
    memmove(&h->long_ref[i], &h->long_ref[i+1], (h->long_ref_count - i - 1)*sizeof(Picture*));
    h->long_ref_count--;
    return pic;
}
/**
 * Executes the reference picture marking (memory management control operations).
 */
static int execute_ref_pic_marking(H264Context *h, MMCO *mmco, int mmco_count){
    MpegEncContext * const s = &h->s;
    int i;
    int current_is_long=0;
    Picture *pic;
    
    if((s->avctx->debug&FF_DEBUG_MMCO) && mmco_count==0)
        av_log(h->s.avctx, AV_LOG_DEBUG, "no mmco here\n");
        
    for(i=0; i<mmco_count; i++){
        if(s->avctx->debug&FF_DEBUG_MMCO)
            av_log(h->s.avctx, AV_LOG_DEBUG, "mmco:%d %d %d\n", h->mmco[i].opcode, h->mmco[i].short_frame_num, h->mmco[i].long_index);
        switch(mmco[i].opcode){
        case MMCO_SHORT2UNUSED:
            pic= remove_short(h, mmco[i].short_frame_num);
            if(pic==NULL) return -1;
            pic->reference= 0;
            break;
        case MMCO_SHORT2LONG:
            pic= remove_long(h, mmco[i].long_index);
            if(pic) pic->reference=0;
            
            h->long_ref[ mmco[i].long_index ]= remove_short(h, mmco[i].short_frame_num);
            h->long_ref[ mmco[i].long_index ]->long_ref=1;
            break;
        case MMCO_LONG2UNUSED:
            pic= remove_long(h, mmco[i].long_index);
            if(pic==NULL) return -1;
            pic->reference= 0;
            break;
        case MMCO_LONG:
            pic= remove_long(h, mmco[i].long_index);
            if(pic) pic->reference=0;
            
            h->long_ref[ mmco[i].long_index ]= s->current_picture_ptr;
            h->long_ref[ mmco[i].long_index ]->long_ref=1;
            h->long_ref_count++;
            
            current_is_long=1;
            break;
        case MMCO_SET_MAX_LONG:
            assert(mmco[i].long_index <= 16);
            while(mmco[i].long_index < h->long_ref_count){
                pic= remove_long(h, mmco[i].long_index);
                pic->reference=0;
            }
            while(mmco[i].long_index > h->long_ref_count){
                h->long_ref[ h->long_ref_count++ ]= NULL;
            }
            break;
        case MMCO_RESET:
            while(h->short_ref_count){
                pic= remove_short(h, h->short_ref[0]->frame_num);
                pic->reference=0;
            }
            while(h->long_ref_count){
                pic= remove_long(h, h->long_ref_count-1);
                pic->reference=0;
            }
            break;
        default: assert(0);
        }
    }
    
    if(!current_is_long){
        pic= remove_short(h, s->current_picture_ptr->frame_num);
        if(pic){
            pic->reference=0;
            av_log(h->s.avctx, AV_LOG_ERROR, "illegal short term buffer state detected\n");
        }
        
        if(h->short_ref_count)
            memmove(&h->short_ref[1], &h->short_ref[0], h->short_ref_count*sizeof(Picture*));
        h->short_ref[0]= s->current_picture_ptr;
        h->short_ref[0]->long_ref=0;
        h->short_ref_count++;
    }
    
    return 0; 
}
static int decode_ref_pic_marking(H264Context *h){
    MpegEncContext * const s = &h->s;
    int i;
    
    if(h->nal_unit_type == NAL_IDR_SLICE){ //FIXME fields
        s->broken_link= get_bits1(&s->gb) -1;
        h->mmco[0].long_index= get_bits1(&s->gb) - 1; // current_long_term_idx
        if(h->mmco[0].long_index == -1)
            h->mmco_index= 0;
        else{
            h->mmco[0].opcode= MMCO_LONG;
            h->mmco_index= 1;
        } 
    }else{
        if(get_bits1(&s->gb)){ // adaptive_ref_pic_marking_mode_flag
            for(i= h->mmco_index; i<MAX_MMCO_COUNT; i++) { 
                MMCOOpcode opcode= get_ue_golomb(&s->gb);;
                h->mmco[i].opcode= opcode;
                if(opcode==MMCO_SHORT2UNUSED || opcode==MMCO_SHORT2LONG){
                    h->mmco[i].short_frame_num= (h->frame_num - get_ue_golomb(&s->gb) - 1) & ((1<<h->sps.log2_max_frame_num)-1); //FIXME fields
/*                    if(h->mmco[i].short_frame_num >= h->short_ref_count || h->short_ref[ h->mmco[i].short_frame_num ] == NULL){
                        fprintf(stderr, "illegal short ref in memory management control operation %d\n", mmco);
                        return -1;
                    }*/
                }
                if(opcode==MMCO_SHORT2LONG || opcode==MMCO_LONG2UNUSED || opcode==MMCO_LONG || opcode==MMCO_SET_MAX_LONG){
                    h->mmco[i].long_index= get_ue_golomb(&s->gb);
                    if(/*h->mmco[i].long_index >= h->long_ref_count || h->long_ref[ h->mmco[i].long_index ] == NULL*/ h->mmco[i].long_index >= 16){
                        av_log(h->s.avctx, AV_LOG_ERROR, "illegal long ref in memory management control operation %d\n", opcode);
                        return -1;
                    }
                }
                    
                if(opcode > MMCO_LONG){
                    av_log(h->s.avctx, AV_LOG_ERROR, "illegal memory management control operation %d\n", opcode);
                    return -1;
                }
            }
            h->mmco_index= i;
        }else{
            assert(h->long_ref_count + h->short_ref_count <= h->sps.ref_frame_count);
            if(h->long_ref_count + h->short_ref_count == h->sps.ref_frame_count){ //FIXME fields
                h->mmco[0].opcode= MMCO_SHORT2UNUSED;
                h->mmco[0].short_frame_num= h->short_ref[ h->short_ref_count - 1 ]->frame_num;
                h->mmco_index= 1;
            }else
                h->mmco_index= 0;
        }
    }
    
    return 0; 
}
static int init_poc(H264Context *h){
    MpegEncContext * const s = &h->s;
    const int max_frame_num= 1<<h->sps.log2_max_frame_num;
    int field_poc[2];
    if(h->nal_unit_type == NAL_IDR_SLICE){
        h->frame_num_offset= 0;
    }else{
        if(h->frame_num < h->prev_frame_num)
            h->frame_num_offset= h->prev_frame_num_offset + max_frame_num;
        else
            h->frame_num_offset= h->prev_frame_num_offset;
    }
    if(h->sps.poc_type==0){
        const int max_poc_lsb= 1<<h->sps.log2_max_poc_lsb;
        if     (h->poc_lsb < h->prev_poc_lsb && h->prev_poc_lsb - h->poc_lsb >= max_poc_lsb/2)
            h->poc_msb = h->prev_poc_msb + max_poc_lsb;
        else if(h->poc_lsb > h->prev_poc_lsb && h->prev_poc_lsb - h->poc_lsb < -max_poc_lsb/2)
            h->poc_msb = h->prev_poc_msb - max_poc_lsb;
        else
            h->poc_msb = h->prev_poc_msb;
//printf("poc: %d %d\n", h->poc_msb, h->poc_lsb);
        field_poc[0] = 
        field_poc[1] = h->poc_msb + h->poc_lsb;
        if(s->picture_structure == PICT_FRAME) 
            field_poc[1] += h->delta_poc_bottom;
    }else if(h->sps.poc_type==1){
        int abs_frame_num, expected_delta_per_poc_cycle, expectedpoc;
        int i;
        if(h->sps.poc_cycle_length != 0)
            abs_frame_num = h->frame_num_offset + h->frame_num;
        else
            abs_frame_num = 0;
        if(h->nal_ref_idc==0 && abs_frame_num > 0)
            abs_frame_num--;
            
        expected_delta_per_poc_cycle = 0;
        for(i=0; i < h->sps.poc_cycle_length; i++)
            expected_delta_per_poc_cycle += h->sps.offset_for_ref_frame[ i ]; //FIXME integrate during sps parse
        if(abs_frame_num > 0){
            int poc_cycle_cnt          = (abs_frame_num - 1) / h->sps.poc_cycle_length;
            int frame_num_in_poc_cycle = (abs_frame_num - 1) % h->sps.poc_cycle_length;
            expectedpoc = poc_cycle_cnt * expected_delta_per_poc_cycle;
            for(i = 0; i <= frame_num_in_poc_cycle; i++)
                expectedpoc = expectedpoc + h->sps.offset_for_ref_frame[ i ];
        } else
            expectedpoc = 0;
        if(h->nal_ref_idc == 0) 
            expectedpoc = expectedpoc + h->sps.offset_for_non_ref_pic;
        
        field_poc[0] = expectedpoc + h->delta_poc[0];
        field_poc[1] = field_poc[0] + h->sps.offset_for_top_to_bottom_field;
        if(s->picture_structure == PICT_FRAME)
            field_poc[1] += h->delta_poc[1];
    }else{
        int poc;
        if(h->nal_unit_type == NAL_IDR_SLICE){
            poc= 0;
        }else{
            if(h->nal_ref_idc) poc= 2*(h->frame_num_offset + h->frame_num);
            else               poc= 2*(h->frame_num_offset + h->frame_num) - 1;
        }
        field_poc[0]= poc;
        field_poc[1]= poc;
    }
    
    if(s->picture_structure != PICT_BOTTOM_FIELD)
        s->current_picture_ptr->field_poc[0]= field_poc[0];
    if(s->picture_structure != PICT_TOP_FIELD)
        s->current_picture_ptr->field_poc[1]= field_poc[1];
    if(s->picture_structure == PICT_FRAME) // FIXME field pix?
        s->current_picture_ptr->poc= FFMIN(field_poc[0], field_poc[1]);
    return 0;
}
/**
 * 解码片头
 * 该函数内也调用 MPV_common_init() 和 frame_start()
 */
static int decode_slice_header(H264Context *h){
    MpegEncContext * const s = &h->s;
    int first_mb_in_slice, pps_id;
    int num_ref_idx_active_override_flag;
    static const uint8_t slice_type_map[5]= {P_TYPE, B_TYPE, I_TYPE, SP_TYPE, SI_TYPE};
    s->current_picture.reference= h->nal_ref_idc != 0;
    first_mb_in_slice= get_ue_golomb(&s->gb);
	/*获得片类型*/	
    h->slice_type= get_ue_golomb(&s->gb);
	if(h->slice_type > 9){
        av_log(h->s.avctx, AV_LOG_ERROR, "slice type too large (%d) at %d %d\n", h->slice_type, s->mb_x, s->mb_y);
    }
    if(h->slice_type > 4){
        h->slice_type -= 5;
        h->slice_type_fixed=1;
    }else
        h->slice_type_fixed=0;
    h->slice_type= slice_type_map[ h->slice_type ];

	/*记录最后的的片类型*/
    s->pict_type= h->slice_type; // to make a few old func happy, its wrong though
    /*获得PPS数目*/    
    pps_id= get_ue_golomb(&s->gb);
    if(pps_id>255){
        av_log(h->s.avctx, AV_LOG_ERROR, "pps_id out of range\n");
        return -1;
    }
	/*获取PPS空间*/
    h->pps= h->pps_buffer[pps_id];
    if(h->pps.slice_group_count == 0){
        av_log(h->s.avctx, AV_LOG_ERROR, "non existing PPS referenced\n");
        return -1;
    }
	/*获取SPS空间*/
    h->sps= h->sps_buffer[ h->pps.sps_id ];
    if(h->sps.log2_max_frame_num == 0){
        av_log(h->s.avctx, AV_LOG_ERROR, "non existing SPS referenced\n");
        return -1;
    }
    /*记录图像大小*/
    s->mb_width= h->sps.mb_width;
    s->mb_height= h->sps.mb_height;
    
    h->b_stride=  s->mb_width*4;
    h->b8_stride= s->mb_width*2;
    s->mb_x = first_mb_in_slice % s->mb_width;
    s->mb_y = first_mb_in_slice / s->mb_width; //FIXME AFFW
    
    s->width = 16*s->mb_width - 2*(h->sps.crop_left + h->sps.crop_right );
    if(h->sps.frame_mbs_only_flag)
        s->height= 16*s->mb_height - 2*(h->sps.crop_top  + h->sps.crop_bottom);
    else
        s->height= 16*s->mb_height - 4*(h->sps.crop_top  + h->sps.crop_bottom); //FIXME recheck
    /*图像大小不一致，释放先前申请的空间*/
    if (s->context_initialized 
        && (   s->width != s->avctx->width || s->height != s->avctx->height)) {
        free_tables(h);
        MPV_common_end(s);
    }
	/*初始化，申请空间*/
    if (!s->context_initialized) {
        if (MPV_common_init(s) < 0)
            return -1;
        alloc_tables(h);
        s->avctx->width = s->width;
        s->avctx->height = s->height;
        s->avctx->sample_aspect_ratio= h->sps.sar;
    }
	/*图像片中的第一个宏块*/
    if(first_mb_in_slice == 0){
        frame_start(h);
    }
	/*获取帧序号*/
    s->current_picture_ptr->frame_num=
    h->frame_num= get_bits(&s->gb, h->sps.log2_max_frame_num);
	/*只有宏块*/
	if(h->sps.frame_mbs_only_flag){
        s->picture_structure= PICT_FRAME;
    }else{
    /*图像场编码*/
        if(get_bits1(&s->gb)) //field_pic_flag
            s->picture_structure= PICT_TOP_FIELD + get_bits1(&s->gb); //bottom_field_flag
        else
            s->picture_structure= PICT_FRAME;
    }
	/*计算图像帧序号*/	
    if(s->picture_structure==PICT_FRAME){
        h->curr_pic_num=   h->frame_num;
        h->max_pic_num= 1<< h->sps.log2_max_frame_num;
    }else{
        h->curr_pic_num= 2*h->frame_num;
        h->max_pic_num= 1<<(h->sps.log2_max_frame_num + 1);
    }
    /*IDR:Instantaneous Decoding Refresh，即时解码刷新片*/    
    if(h->nal_unit_type == NAL_IDR_SLICE){
        get_ue_golomb(&s->gb); /* idr_pic_id */
    }
   /*读取POC，0*/
    if(h->sps.poc_type==0){
        h->poc_lsb= get_bits(&s->gb, h->sps.log2_max_poc_lsb);
        
        if(h->pps.pic_order_present==1 && s->picture_structure==PICT_FRAME){
            h->delta_poc_bottom= get_se_golomb(&s->gb);
        }
    }
    /*读取POC，1*/
    if(h->sps.poc_type==1 && !h->sps.delta_pic_order_always_zero_flag){
        h->delta_poc[0]= get_se_golomb(&s->gb);
        
        if(h->pps.pic_order_present==1 && s->picture_structure==PICT_FRAME)
            h->delta_poc[1]= get_se_golomb(&s->gb);
    }

	/*初始化图像顺序号POC*/
    init_poc(h);

	/*冗余图像数目*/
    if(h->pps.redundant_pic_cnt_present){
        h->redundant_pic_count= get_ue_golomb(&s->gb);
    }
    /*设置缺省值，后面可能会覆盖本设置*/
    h->ref_count[0]= h->pps.ref_count[0];
    h->ref_count[1]= h->pps.ref_count[1];
    if(h->slice_type == P_TYPE || h->slice_type == SP_TYPE || h->slice_type == B_TYPE){
        if(h->slice_type == B_TYPE){
            h->direct_spatial_mv_pred= get_bits1(&s->gb);
        }
        num_ref_idx_active_override_flag= get_bits1(&s->gb);
    
        if(num_ref_idx_active_override_flag){
            h->ref_count[0]= get_ue_golomb(&s->gb) + 1;
            if(h->slice_type==B_TYPE)
                h->ref_count[1]= get_ue_golomb(&s->gb) + 1;
            if(h->ref_count[0] > 32 || h->ref_count[1] > 32){
                av_log(h->s.avctx, AV_LOG_ERROR, "reference overflow\n");
                return -1;
            }
        }
    }
	/*填充缺省的参考帧列表*/
    if(first_mb_in_slice == 0){
        fill_default_ref_list(h);
    }
	/*重排参考帧列表*/		
    decode_ref_pic_list_reordering(h);
	/*加权预测*/
	if(   (h->pps.weighted_pred          && (h->slice_type == P_TYPE || h->slice_type == SP_TYPE )) 
       || (h->pps.weighted_bipred_idc==1 && h->slice_type==B_TYPE ) )
        pred_weight_table(h);
	/*标记参考帧*/    
    if(s->current_picture.reference)
        decode_ref_pic_marking(h);
	/*CABAC标识*/		
    if( h->slice_type != I_TYPE && h->slice_type != SI_TYPE && h->pps.cabac )
        h->cabac_init_idc = get_ue_golomb(&s->gb);
	/*读取量化步长*/
	h->last_qscale_diff = 0;
    s->qscale = h->pps.init_qp + get_se_golomb(&s->gb);
	/*量化步长合理性检查*/
	if(s->qscale<0 || s->qscale>51){
        av_log(s->avctx, AV_LOG_ERROR, "QP %d out of range\n", s->qscale);
        return -1;
    }

	/*场景切换*/
    if(h->slice_type == SP_TYPE){
        get_bits1(&s->gb); /* sp_for_switch_flag */
    }
	/*量化尺度*/
    if(h->slice_type==SP_TYPE || h->slice_type == SI_TYPE){
        get_se_golomb(&s->gb); /* slice_qs_delta */
    }
	/*去块效应参数赋值*/
    h->deblocking_filter = 1;
    h->slice_alpha_c0_offset = 0;
    h->slice_beta_offset = 0;
	/*读取环路滤波/去除块效应的参数*/	
    if( h->pps.deblocking_filter_parameters_present ) {
        h->deblocking_filter= get_ue_golomb(&s->gb);
        if(h->deblocking_filter < 2) 
            h->deblocking_filter^= 1; // 1<->0
        if( h->deblocking_filter ) {
            h->slice_alpha_c0_offset = get_se_golomb(&s->gb) << 1;
            h->slice_beta_offset = get_se_golomb(&s->gb) << 1;
        }
    }

#if 0 //FMO
    if( h->pps.num_slice_groups > 1  && h->pps.mb_slice_group_map_type >= 3 && h->pps.mb_slice_group_map_type <= 5)
        slice_group_change_cycle= get_bits(&s->gb, ?);
#endif
	/*输出图像片的格式信息*/
    if(s->avctx->debug&FF_DEBUG_PICT_INFO){
        av_log(h->s.avctx, AV_LOG_DEBUG, "mb:%d %c pps:%d frame:%d poc:%d/%d ref:%d/%d qp:%d loop:%d\n", 
               first_mb_in_slice, 
               av_get_pict_type_char(h->slice_type),
               pps_id, h->frame_num,
               s->current_picture_ptr->field_poc[0], s->current_picture_ptr->field_poc[1],
               h->ref_count[0], h->ref_count[1],
               s->qscale,
               h->deblocking_filter
               );
    }
    return 0;
}

/*解码图像片*/
static int decode_slice(H264Context *h){
    MpegEncContext * const s = &h->s;
    const int part_mask= s->partitioned_frame ? (AC_END|AC_ERROR) : 0x7F;
    s->mb_skip_run= -1;
	/*CABAC解码*/
	if( h->pps.cabac ) {
        int i;
        /* 规整码流 */
        align_get_bits( &s->gb );
        /* 初始化CABAC 的状态*/
        ff_init_cabac_states( &h->cabac, ff_h264_lps_range, ff_h264_mps_state, ff_h264_lps_state, 64 );
		/*填充码流解码结构*/
		ff_init_cabac_decoder( &h->cabac,
                               s->gb.buffer + get_bits_count(&s->gb)/8,
                               ( s->gb.size_in_bits - get_bits_count(&s->gb) + 7)/8);
        /* 计算固定400的初始状态 */
        for( i= 0; i < 399; i++ ) {
            int pre;
            if( h->slice_type == I_TYPE )
                pre = clip( ((cabac_context_init_I[i][0] * s->qscale) >>4 ) + cabac_context_init_I[i][1], 1, 126 );
            else
                pre = clip( ((cabac_context_init_PB[h->cabac_init_idc][i][0] * s->qscale) >>4 ) + cabac_context_init_PB[h->cabac_init_idc][i][1], 1, 126 );
            if( pre <= 63 )
                h->cabac_state[i] = 2 * ( 63 - pre ) + 0;
            else
                h->cabac_state[i] = 2 * ( pre - 64 ) + 1;
        }
        for(;;){
			 /*CABAC 宏块熵解码、反量化*/		
            int ret = decode_mb_cabac(h);
			 /*返回片的结尾标记*/
            int eos = get_cabac_terminate( &h->cabac ); /* End of Slice flag */
			/*反变换、运动补偿等*/
			 hl_decode_mb(h);
			
            /* 帧或场自适应切换 */
            if( ret >= 0 && h->sps.mb_aff ) { //FIXME optimal? or let mb_decode decode 16x32 ?
                s->mb_y++;
                ret = decode_mb_cabac(h);
                eos = get_cabac_terminate( &h->cabac );
                hl_decode_mb(h);
                s->mb_y--;
            }
			 /*解码出错或已经解码到结尾*/
            if( ret < 0 || h->cabac.bytestream > h->cabac.bytestream_end + 1) {
                av_log(h->s.avctx, AV_LOG_ERROR, "error while decoding MB %d %d\n", s->mb_x, s->mb_y);
                ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y, (AC_ERROR|DC_ERROR|MV_ERROR)&part_mask);
                return -1;
            }
			 /*一行宏块解码结束，为提高Cache效率，保留水平方向内容*/
            if( ++s->mb_x >= s->mb_width ) {
                s->mb_x = 0;
                ff_draw_horiz_band(s, 16*s->mb_y, 16);
				 /*根据宏块数目，解码结束*/
                if( ++s->mb_y >= s->mb_height ) {
                    tprintf("slice end %d %d\n", get_bits_count(&s->gb), s->gb.size_in_bits);
                }
            }
			 /*解码完毕，添加图像片*/
            if( eos || s->mb_y >= s->mb_height ) {
                ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, (AC_END|DC_END|MV_END)&part_mask);
                return 0;
            }

        }
    } 
	else{
   /*CAVLC解码*/
        for(;;){
			 /*一个宏块熵解码、反量化*/
            int ret = decode_mb_cavlc(h);
			 /*反变换、运动补偿等*/
            hl_decode_mb(h);
			 /*宏块自适应帧、场编码*/
            if(ret>=0 && h->sps.mb_aff){ //FIXME optimal? or let mb_decode decode 16x32 ?
                s->mb_y++;
                ret = decode_mb_cavlc(h);
                hl_decode_mb(h);
                s->mb_y--;
            }
			/*解码出错*/
            if(ret<0){
                av_log(h->s.avctx, AV_LOG_ERROR, "error while decoding MB %d %d\n", s->mb_x, s->mb_y);
                ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y, (AC_ERROR|DC_ERROR|MV_ERROR)&part_mask);
                return -1;
            }
			/*一行宏块解码结束，为提高Cache效率，保留水平区*/
            if(++s->mb_x >= s->mb_width){
                s->mb_x=0;
                ff_draw_horiz_band(s, 16*s->mb_y, 16);
				 /*根据宏块数目，解码完毕，添加图像片*/
                if(++s->mb_y >= s->mb_height){
                    tprintf("slice end %d %d\n", get_bits_count(&s->gb), s->gb.size_in_bits);
                    if(get_bits_count(&s->gb) == s->gb.size_in_bits ) {
                        ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, (AC_END|DC_END|MV_END)&part_mask);
                        return 0;
                    }else{
                        ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y, (AC_END|DC_END|MV_END)&part_mask);
                        return -1;
                    }
                }
            }
			/*码流消耗完毕，添加图像片*/
            if(get_bits_count(&s->gb) >= s->gb.size_in_bits && s->mb_skip_run<=0){
                if(get_bits_count(&s->gb) == s->gb.size_in_bits ){
                    ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, (AC_END|DC_END|MV_END)&part_mask);
                    return 0;
                }else{
                    ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y, (AC_ERROR|DC_ERROR|MV_ERROR)&part_mask);
                    return -1;
                }
            }
        }
    }

    return -1; //正常情况下不会达到这里
}
static inline int decode_vui_parameters(H264Context *h, SPS *sps){
    MpegEncContext * const s = &h->s;
    int aspect_ratio_info_present_flag, aspect_ratio_idc;
    aspect_ratio_info_present_flag= get_bits1(&s->gb);
    
    if( aspect_ratio_info_present_flag ) {
        aspect_ratio_idc= get_bits(&s->gb, 8);
        if( aspect_ratio_idc == EXTENDED_SAR ) {
            sps->sar.num= get_bits(&s->gb, 16);
            sps->sar.den= get_bits(&s->gb, 16);
        }else if(aspect_ratio_idc < 16){
            sps->sar=  pixel_aspect[aspect_ratio_idc];
        }else{
            av_log(h->s.avctx, AV_LOG_ERROR, "illegal aspect ratio\n");
            return -1;
        }
    }else{
        sps->sar.num= 
        sps->sar.den= 0;
    }

    return 0;
}
static inline int decode_seq_parameter_set(H264Context *h){
    MpegEncContext * const s = &h->s;
    int profile_idc, level_idc;
    int sps_id, i;
    SPS *sps;
    
    profile_idc= get_bits(&s->gb, 8);
    get_bits1(&s->gb);   //constraint_set0_flag
    get_bits1(&s->gb);   //constraint_set1_flag
    get_bits1(&s->gb);   //constraint_set2_flag
    get_bits(&s->gb, 5); // reserved
    level_idc= get_bits(&s->gb, 8);
    sps_id= get_ue_golomb(&s->gb);
    
    sps= &h->sps_buffer[ sps_id ];
    sps->profile_idc= profile_idc;
    sps->level_idc= level_idc;
    
    sps->log2_max_frame_num= get_ue_golomb(&s->gb) + 4;
    sps->poc_type= get_ue_golomb(&s->gb);
    
    if(sps->poc_type == 0){ //FIXME #define
        sps->log2_max_poc_lsb= get_ue_golomb(&s->gb) + 4;
    } else if(sps->poc_type == 1){//FIXME #define
        sps->delta_pic_order_always_zero_flag= get_bits1(&s->gb);
        sps->offset_for_non_ref_pic= get_se_golomb(&s->gb);
        sps->offset_for_top_to_bottom_field= get_se_golomb(&s->gb);
        sps->poc_cycle_length= get_ue_golomb(&s->gb);
        
        for(i=0; i<sps->poc_cycle_length; i++)
            sps->offset_for_ref_frame[i]= get_se_golomb(&s->gb);
    }
    if(sps->poc_type > 2){
        av_log(h->s.avctx, AV_LOG_ERROR, "illegal POC type %d\n", sps->poc_type);
        return -1;
    }
    sps->ref_frame_count= get_ue_golomb(&s->gb);
    sps->gaps_in_frame_num_allowed_flag= get_bits1(&s->gb);
    sps->mb_width= get_ue_golomb(&s->gb) + 1;
    sps->mb_height= get_ue_golomb(&s->gb) + 1;
    sps->frame_mbs_only_flag= get_bits1(&s->gb);
    if(!sps->frame_mbs_only_flag)
        sps->mb_aff= get_bits1(&s->gb);
    else
        sps->mb_aff= 0;
    sps->direct_8x8_inference_flag= get_bits1(&s->gb);
    sps->crop= get_bits1(&s->gb);
    if(sps->crop){
        sps->crop_left  = get_ue_golomb(&s->gb);
        sps->crop_right = get_ue_golomb(&s->gb);
        sps->crop_top   = get_ue_golomb(&s->gb);
        sps->crop_bottom= get_ue_golomb(&s->gb);
        if(sps->crop_left || sps->crop_top){
            av_log(h->s.avctx, AV_LOG_ERROR, "insane cropping not completly supported, this could look slightly wrong ...\n");
        }
    }else{
        sps->crop_left  = 
        sps->crop_right = 
        sps->crop_top   = 
        sps->crop_bottom= 0;
    }
    sps->vui_parameters_present_flag= get_bits1(&s->gb);
    if( sps->vui_parameters_present_flag )
        decode_vui_parameters(h, sps);
    
    if(s->avctx->debug&FF_DEBUG_PICT_INFO){
        av_log(h->s.avctx, AV_LOG_DEBUG, "sps:%d profile:%d/%d poc:%d ref:%d %dx%d %s %s crop:%d/%d/%d/%d %s\n", 
               sps_id, sps->profile_idc, sps->level_idc,
               sps->poc_type,
               sps->ref_frame_count,
               sps->mb_width, sps->mb_height,
               sps->frame_mbs_only_flag ? "FRM" : (sps->mb_aff ? "MB-AFF" : "PIC-AFF"),
               sps->direct_8x8_inference_flag ? "8B8" : "",
               sps->crop_left, sps->crop_right, 
               sps->crop_top, sps->crop_bottom, 
               sps->vui_parameters_present_flag ? "VUI" : ""
               );
    }
    return 0;
}
static inline int decode_picture_parameter_set(H264Context *h){
    MpegEncContext * const s = &h->s;
    int pps_id= get_ue_golomb(&s->gb);
    PPS *pps= &h->pps_buffer[pps_id];
    
    pps->sps_id= get_ue_golomb(&s->gb);
    pps->cabac= get_bits1(&s->gb);
    pps->pic_order_present= get_bits1(&s->gb);
    pps->slice_group_count= get_ue_golomb(&s->gb) + 1;
    if(pps->slice_group_count > 1 ){
        pps->mb_slice_group_map_type= get_ue_golomb(&s->gb);
        av_log(h->s.avctx, AV_LOG_ERROR, "FMO not supported\n");
        switch(pps->mb_slice_group_map_type){
        case 0:
            break;
        case 2:
            break;
        case 3:
        case 4:
        case 5:
            break;
        case 6:

            break;
        }
    }
    pps->ref_count[0]= get_ue_golomb(&s->gb) + 1;
    pps->ref_count[1]= get_ue_golomb(&s->gb) + 1;
    if(pps->ref_count[0] > 32 || pps->ref_count[1] > 32){
        av_log(h->s.avctx, AV_LOG_ERROR, "reference overflow (pps)\n");
        return -1;
    }
    
    pps->weighted_pred= get_bits1(&s->gb);
    pps->weighted_bipred_idc= get_bits(&s->gb, 2);
    pps->init_qp= get_se_golomb(&s->gb) + 26;
    pps->init_qs= get_se_golomb(&s->gb) + 26;
    pps->chroma_qp_index_offset= get_se_golomb(&s->gb);
    pps->deblocking_filter_parameters_present= get_bits1(&s->gb);
    pps->constrained_intra_pred= get_bits1(&s->gb);
    pps->redundant_pic_cnt_present = get_bits1(&s->gb);
    
    if(s->avctx->debug&FF_DEBUG_PICT_INFO){
        av_log(h->s.avctx, AV_LOG_DEBUG, "pps:%d sps:%d %s slice_groups:%d ref:%d/%d %s qp:%d/%d/%d %s %s %s\n", 
               pps_id, pps->sps_id,
               pps->cabac ? "CABAC" : "CAVLC",
               pps->slice_group_count,
               pps->ref_count[0], pps->ref_count[1],
               pps->weighted_pred ? "weighted" : "",
               pps->init_qp, pps->init_qs, pps->chroma_qp_index_offset,
               pps->deblocking_filter_parameters_present ? "LPAR" : "",
               pps->constrained_intra_pred ? "CONSTR" : "",
               pps->redundant_pic_cnt_present ? "REDU" : ""
               );
    }
    
    return 0;
}
/**
 * finds the end of the current frame in the bitstream.
 * @return the position of the first byte of the next frame, or -1
 */
static int find_frame_end(ParseContext *pc, const uint8_t *buf, int buf_size){
    int i;
    uint32_t state;
//printf("first %02X%02X%02X%02X\n", buf[0], buf[1],buf[2],buf[3]);
//    mb_addr= pc->mb_addr - 1;
    state= pc->state;
    //FIXME this will fail with slices
    for(i=0; i<buf_size; i++){
        state= (state<<8) | buf[i];
        if((state&0xFFFFFF1F) == 0x101 || (state&0xFFFFFF1F) == 0x102 || (state&0xFFFFFF1F) == 0x105){
            if(pc->frame_start_found){
                pc->state=-1; 
                pc->frame_start_found= 0;
                return i-3;
            }
            pc->frame_start_found= 1;
        }
    }
    
    pc->state= state;
    return END_NOT_FOUND;
}

static int h264_parse(AVCodecParserContext *s,
                      AVCodecContext *avctx,
                      uint8_t **poutbuf, int *poutbuf_size, 
                      const uint8_t *buf, int buf_size)
{
    ParseContext *pc = s->priv_data;
    int next;
    
    next= find_frame_end(pc, buf, buf_size);
    if (ff_combine_frame(pc, next, (uint8_t **)&buf, &buf_size) < 0) {
        *poutbuf = NULL;
        *poutbuf_size = 0;
        return buf_size;
    }
    *poutbuf = (uint8_t *)buf;
    *poutbuf_size = buf_size;
    return next;
}

/*NAL单元解码*/
static int decode_nal_units(H264Context *h, 	//解码器句柄
							uint8_t *buf,		//码流空间
							int buf_size){	//码流长度
	MpegEncContext * const s = &h->s;
	AVCodecContext * const avctx= s->avctx;
	int buf_index=0;

	/*循环解码*/
	for(;;){
		int consumed;	//消耗的长度
		int dst_length;	//目标长度
		int bit_length;	//比特长度
		uint8_t *ptr;	//临时指针

		/*搜索前缀开始码:0x 00 00 01*/
		for(; buf_index + 3 < buf_size; buf_index++){
		/*在第一次搜索时，必须成功，否则认为码流出错*/
			if(buf[buf_index] == 0 && buf[buf_index+1] == 0 && buf[buf_index+2] == 1)
				break;
		}
		/*后续仍然有数据*/
		if(buf_index+3 >= buf_size) break;
		/*索引后移*/
		buf_index+=3;
		/*网络抽象层NAL解包*/
		ptr= decode_nal(h, buf + buf_index, &dst_length, &consumed, buf_size - buf_index);
		if(ptr[dst_length - 1] == 0) dst_length--;
		/*确定码流的精确的结束位置*/
		bit_length= 8*dst_length - decode_rbsp_trailing(ptr + dst_length - 1);
		if(s->avctx->debug&FF_DEBUG_STARTCODE){
			av_log(h->s.avctx, AV_LOG_DEBUG, "NAL %d at %d length %d\n", h->nal_unit_type, buf_index, dst_length);
		}
		/*索引后移*/
		buf_index += consumed;
		if( s->hurry_up == 1 && h->nal_ref_idc  == 0 )
			continue;

		switch(h->nal_unit_type){	/*根据NAL类型执行解码*/
		case NAL_IDR_SLICE:	//IDR片
			idr(h); 			//
		case NAL_SLICE:		//图像片SLICE解码
			/*初始化码流指针*/
			init_get_bits(&s->gb, ptr, bit_length);
			h->intra_gb_ptr=
			h->inter_gb_ptr= &s->gb;
			s->data_partitioning = 0;
			/*解码图像片的头*/
			if(decode_slice_header(h) < 0) return -1;
			if(h->redundant_pic_count==0 && s->hurry_up < 5 )
			/*****************图像片数据解码*/
			decode_slice(h);
			/*****************图像片数据解码*/
			break;
		case NAL_DPA:		//数据分区A
			init_get_bits(&s->gb, ptr, bit_length);
			h->intra_gb_ptr=
			h->inter_gb_ptr= NULL;
			s->data_partitioning = 1;
			/*解码图像片的头*/
			if(decode_slice_header(h) < 0) return -1;
			break;
		case NAL_DPB:		//数据分区B
			init_get_bits(&h->intra_gb, ptr, bit_length);
			h->intra_gb_ptr= &h->intra_gb;
			break;
		case NAL_DPC:		//数据分区C
			init_get_bits(&h->inter_gb, ptr, bit_length);
			h->inter_gb_ptr= &h->inter_gb;
			if(h->redundant_pic_count==0 && h->intra_gb_ptr && s->data_partitioning && s->hurry_up < 5 )
			decode_slice(h);
			break;
		case NAL_SEI:		//补充增强信息
			break;
		case NAL_SPS:		//序列参数集SPS
			/*初始化码流*/
			init_get_bits(&s->gb, ptr, bit_length);
			/*SPS解码*/
			decode_seq_parameter_set(h);
			if(s->flags& CODEC_FLAG_LOW_DELAY)
			s->low_delay=1;
			avctx->has_b_frames= !s->low_delay;
			printf("decode SPS\n");
			break;
		case NAL_PPS:		//图像参数集PPS
			/*初始化码流*/
			init_get_bits(&s->gb, ptr, bit_length);
			/*PPS解码*/
			decode_picture_parameter_set(h);
			printf("decode PPS\n");
			break;
		case NAL_PICTURE_DELIMITER:
			break;
		case NAL_FILTER_DATA:
			break;
		default:
			av_log(avctx, AV_LOG_ERROR, "Unknown NAL code: %d\n", h->nal_unit_type);
		}        
		/*图像帧类型*/
		s->current_picture.pict_type= s->pict_type;
		/*关键帧类型初始化*/
		s->current_picture.key_frame= s->pict_type == I_TYPE;
	}

	/*没有解码输出图像*/
	if(!s->current_picture_ptr) return buf_index; //no frame

	/*修改图像序列POC*/
	h->prev_frame_num_offset= h->frame_num_offset;
	h->prev_frame_num= h->frame_num;
	if(s->current_picture_ptr->reference){
	    h->prev_poc_msb= h->poc_msb;
	    h->prev_poc_lsb= h->poc_lsb;
	}
	/*标记参考帧*/
	if(s->current_picture_ptr->reference)
	    execute_ref_pic_marking(h, h->mmco, h->mmco_index);
	else
	    assert(h->mmco_index==0);
	/*码流容错结束*/
	ff_er_frame_end(s);
	/*解码一帧完毕，扩展图像*/
	MPV_frame_end(s);
	return buf_index;
}
/**
 * retunrs the number of bytes consumed for building the current frame
 */
static int get_consumed_bytes(MpegEncContext *s, int pos, int buf_size){
    if(s->flags&CODEC_FLAG_TRUNCATED){
        pos -= s->parse_context.last_index;
        if(pos<0) pos=0; // FIXME remove (uneeded?)
        
        return pos;
    }else{
        if(pos==0) pos=1; //avoid infinite loops (i doubt thats needed but ...)
        if(pos+10>buf_size) pos=buf_size; // oops ;)
        return pos;
    }
}
static int decode_frame(AVCodecContext *avctx,	/*解码器上下文*/
                             void *data, 						/*解码图像的结构*/
                             int *data_size,					/*结构大小*/
                             uint8_t *buf,						/*码流空间*/
                             int buf_size)						/*码流长度*/
{
	H264Context *h = avctx->priv_data;			/*解码器上下文*/
	MpegEncContext *s = &h->s;				/*MpegEncContext指针*/
	AVFrame *pict = data; 					/*图像空间*/
	int buf_index;							/*当前的码流位置*/

	s->flags= avctx->flags;					/*CODEC的标志*/
	s->flags2= avctx->flags2;					/*CODEC的标志2*/
	if (buf_size == 0) {						/*码流不为空*/
	    return 0;
	}
	/*截断的码流解码*/
	if(s->flags&CODEC_FLAG_TRUNCATED){
		int next= find_frame_end(&s->parse_context, buf, buf_size);
		if( ff_combine_frame(&s->parse_context, next, &buf, &buf_size) < 0 )
			return buf_size;
	}
	/*码流中特别数据的解码*/
	if(s->avctx->extradata_size && s->picture_number==0){
	    if(0 < decode_nal_units(h, s->avctx->extradata, s->avctx->extradata_size) ) 
	        return -1;
	}
	/*正常的解码*/
	buf_index=decode_nal_units(h, buf, buf_size);
	if(buf_index < 0) 
	    return -1;

#if 0 /*B帧*/
	if(s->pict_type==B_TYPE || s->low_delay){
	    *pict= *(AVFrame*)&s->current_picture;
	} else {
	    *pict= *(AVFrame*)&s->last_picture;
	}
#endif
	/*没有解码输出图像，解码头数据*/
	if(!s->current_picture_ptr){
	    av_log(h->s.avctx, AV_LOG_DEBUG, "error, NO frame\n");
	    return -1;
	}
	/*有解码图像输出*/
	*pict= *(AVFrame*)&s->current_picture; 
	assert(pict->data[0]);
	/*表明解码输出*/
	*data_size = sizeof(AVFrame);
	/*返回当前消耗的码流*/
	return get_consumed_bytes(s, buf_index, buf_size);
}


int FindStartCode (unsigned char *Buf, int zeros_in_startcode)
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

int getNextNal(FILE* inpf, unsigned char* Buf)
{
	int pos = 0;
	int StartCodeFound = 0;
	int info2 = 0;
	int info3 = 0;

	while(!feof(inpf) && (Buf[pos++]=fgetc(inpf))==0);

	while (!StartCodeFound)
	{
		if (feof (inpf))
		{
			return pos-1;
		}
		Buf[pos++] = fgetc (inpf);
		info3 = FindStartCode(&Buf[pos-4], 3);
		if(info3 != 1)
			info2 = FindStartCode(&Buf[pos-3], 2);
		StartCodeFound = (info2 == 1 || info3 == 1);
	}
	fseek (inpf, -4, SEEK_CUR);
	return pos - 4;
}

void pgm_save(unsigned char *buf,int wrap, int xsize,int ysize,char *filename)    
{   
    FILE *f;   
    int i;   
   
    f=fopen(filename,"w");   
    fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);   
    for(i=0;i<ysize;i++)   
        fwrite(buf + i * wrap,1,xsize,f);   
    fclose(f);   
}   

int main(){
	
	FILE * inp_file;
	FILE * out_file;

	int i;
	int nalLen;			/*NAL 长度*/
	unsigned char* Buf;	/*H.264码流*/
	int got_picture;		/*是否解码一帧图像*/
	int consumed_bytes; /*解码器消耗的码流长度*/
	int cnt=0;

	AVCodec *codec;			  /* 编解码CODEC*/
	AVCodecContext *c;		  /* 编解码CODEC context*/
	AVFrame *picture;		  /* 解码后的图像*/	


	/*输出和输出的文件*/
	inp_file = fopen("./data/test.264", "rb");
	out_file = fopen("./data/dsp_dec.yuv", "wb");

	if (inp_file) {
        printf("檔案開啟成功...\n");
    }
    else {
        printf("檔案開啟失敗...\n");
    }

	nalLen = 0;
	/*分配内存，并初始化为0*/
	Buf = (unsigned char*)calloc ( 1000000, sizeof(char));

	/*CODEC的初始化，初始化一些常量表*/
	avcodec_init(); 
	
	/*注册CODEC*/
	avcodec_register_all(); 

	/*查找 H264 CODEC*/
	codec = avcodec_find_decoder(CODEC_ID_H264);

	if (!codec)  return 0; 
	
	/*初始化CODEC的默认参数*/
	c = avcodec_alloc_context(); 
	
	if(!c)  return 0;
	
	/*1. 打开CODEC，这里初始化H.264解码器，调用decode_init本地函数*/
	if (avcodec_open(c, codec) < 0) 	return 0;  
	
	/*为AVFrame申请空间，并清零*/
    picture   = avcodec_alloc_frame();
	if(!picture) 	return 0;
	
	/*循环解码*/
	while(!feof(inp_file)) 	{
		/*从码流中获得一个NAL包*/
		nalLen = getNextNal(inp_file, Buf);

		/*2. NAL解码,调用decode_frame本地函数*/
		consumed_bytes= avcodec_decode_video(c, picture, &got_picture, Buf, nalLen);
		if (got_picture)                              
           c->frame_number++;   

		cnt++;
		/*输出当前的解码信息*/
		printf("No:=%4d, length=%4d\n",cnt,consumed_bytes);
		
		/*返回<0 表示解码数据头，返回>0，表示解码一帧图像*/
		if(consumed_bytes > 0)
		{
			/*从二维空间中提取解码后的图像*/
			for(i=0; i<c->height; i++)
				fwrite(picture->data[0] + i * picture->linesize[0], 1, c->width, out_file);
			for(i=0; i<c->height/2; i++)
				fwrite(picture->data[1] + i * picture->linesize[1], 1, c->width/2, out_file);
			for(i=0; i<c->height/2; i++)
				fwrite(picture->data[2] + i * picture->linesize[2], 1, c->width/2, out_file);
		}

	}

	/*关闭文件*/
	if(inp_file) 	fclose(inp_file);
	if(out_file)	fclose(out_file);

	/*3. 关闭CODEC，释放资源,调用decode_end本地函数*/
	if(c) {
		avcodec_close(c); 
		av_free(c);
		c = NULL;
	} 
	/*释放AVFrame空间*/
	if(picture) {
		av_free(picture);
		picture = NULL;
	}
	/*释放内存*/
	if(Buf) {
		free(Buf);
		Buf = NULL;
	}    

    return 0;
}

static int decode_end(AVCodecContext *avctx)
{
	/*获得解码器上下文指针*/
	H264Context *h = avctx->priv_data;
	/*获得MpegEncContext指针*/
	MpegEncContext *s = &h->s;
	/*释放码表空间*/
	free_tables(h); //FIXME cleanup init stuff perhaps
	/*释放缺省设置的空间*/
	MPV_common_end(s);

	return 0;
}


AVCodec h264_decoder = {
    "h264",				//解码器名称
    CODEC_TYPE_VIDEO,	//解码器类型，视频
    CODEC_ID_H264,		//解码器ID
    sizeof(H264Context),		//解码器上下文大小
    decode_init,			//解码器初始化
    NULL,					//编码器，禁用
    decode_end,			//销毁解码器
    decode_frame,		//解码
    0,						//CODEC兼容性，禁用
};


