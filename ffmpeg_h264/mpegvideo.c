
 
/**
 * @file mpegvideo.c
 * The simplest mpeg encoder (well, it was the simplest!).
 */ 
 
#include "avcodec.h"
#include "dsputil.h"
#include "mpegvideo.h"
//#include "faandct.h"
#include <limits.h>

#ifdef USE_FASTMEMCPY
#include "fastmemcpy.h"
#endif

//#undef NDEBUG
//#include <assert.h>

#ifdef CONFIG_ENCODERS
static void encode_picture(MpegEncContext *s, int picture_number);
#endif //CONFIG_ENCODERS
static void dct_unquantize_mpeg1_intra_c(MpegEncContext *s, 
                                   DCTELEM *block, int n, int qscale);
static void dct_unquantize_mpeg1_inter_c(MpegEncContext *s, 
                                   DCTELEM *block, int n, int qscale);
static void dct_unquantize_mpeg2_intra_c(MpegEncContext *s,
                                   DCTELEM *block, int n, int qscale);
static void dct_unquantize_mpeg2_inter_c(MpegEncContext *s,
                                   DCTELEM *block, int n, int qscale);
static void dct_unquantize_h263_intra_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale);
static void dct_unquantize_h263_inter_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale);
static void dct_unquantize_h261_intra_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale);
static void dct_unquantize_h261_inter_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale);
static void draw_edges_c(uint8_t *buf, int wrap, int width, int height, int w);


#ifdef HAVE_XVMC
extern int  XVMC_field_start(MpegEncContext*s, AVCodecContext *avctx);
extern void XVMC_field_end(MpegEncContext *s);
extern void XVMC_decode_mb(MpegEncContext *s);
#endif

void (*draw_edges)(uint8_t *buf, int wrap, int width, int height, int w)= draw_edges_c;


/* enable all paranoid tests for rounding, overflows, etc... */
//#define PARANOID

//#define DEBUG


/* for jpeg fast DCT */
#define CONST_BITS 14

static const uint16_t aanscales[64] = {
    /* precomputed values scaled up by 14 bits */
    16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
    22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
    21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
    19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
    16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
    12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
    8867 , 12299, 11585, 10426,  8867,  6967,  4799,  2446,
    4520 ,  6270,  5906,  5315,  4520,  3552,  2446,  1247
};

static const uint8_t h263_chroma_roundtab[16] = {
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
};

static const uint8_t ff_default_chroma_qscale_table[32]={
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
};



uint8_t ff_mpeg1_dc_scale_table[128]={
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};



void ff_init_scantable(uint8_t *permutation, ScanTable *st, const uint8_t *src_scantable){
    int i;
    int end;
    
    st->scantable= src_scantable;
    for(i=0; i<64; i++){
        int j;
        j = src_scantable[i];
        st->permutated[i] = permutation[j];
#ifdef ARCH_POWERPC
        st->inverse[j] = i;
#endif
    }
    
    end=-1;
    for(i=0; i<64; i++){
        int j;
        j = st->permutated[i];
        if(j>end) end=j;
        st->raster_end[i]= end;
    }
}
/* init common dct for both encoder and decoder */
int DCT_common_init(MpegEncContext *s)
{
    s->dct_unquantize_h263_intra = dct_unquantize_h263_intra_c;
    s->dct_unquantize_h263_inter = dct_unquantize_h263_inter_c;
    s->dct_unquantize_h261_intra = dct_unquantize_h261_intra_c;
    s->dct_unquantize_h261_inter = dct_unquantize_h261_inter_c;
    s->dct_unquantize_mpeg1_intra = dct_unquantize_mpeg1_intra_c;
    s->dct_unquantize_mpeg1_inter = dct_unquantize_mpeg1_inter_c;
    s->dct_unquantize_mpeg2_intra = dct_unquantize_mpeg2_intra_c;
    s->dct_unquantize_mpeg2_inter = dct_unquantize_mpeg2_inter_c;
        
#ifdef HAVE_MMX
    MPV_common_init_mmx(s);
#endif
#ifdef ARCH_ALPHA
    MPV_common_init_axp(s);
#endif
#ifdef HAVE_MLIB
    MPV_common_init_mlib(s);
#endif
#ifdef HAVE_MMI
    MPV_common_init_mmi(s);
#endif
#ifdef ARCH_ARMV4L
    MPV_common_init_armv4l(s);
#endif
#ifdef ARCH_POWERPC
    MPV_common_init_ppc(s);
#endif
    /* load & permutate scantables
       note: only wmv uses differnt ones 
    */
    if(s->alternate_scan){
        ff_init_scantable(s->dsp.idct_permutation, &s->inter_scantable  , ff_alternate_vertical_scan);
        ff_init_scantable(s->dsp.idct_permutation, &s->intra_scantable  , ff_alternate_vertical_scan);
    }else{
        ff_init_scantable(s->dsp.idct_permutation, &s->inter_scantable  , ff_zigzag_direct);
        ff_init_scantable(s->dsp.idct_permutation, &s->intra_scantable  , ff_zigzag_direct);
    }
    ff_init_scantable(s->dsp.idct_permutation, &s->intra_h_scantable, ff_alternate_horizontal_scan);
    ff_init_scantable(s->dsp.idct_permutation, &s->intra_v_scantable, ff_alternate_vertical_scan);
    return 0;
}
static void copy_picture(Picture *dst, Picture *src){
    *dst = *src;
    dst->type= FF_BUFFER_TYPE_COPY;
}
static void copy_picture_attributes(MpegEncContext *s, AVFrame *dst, AVFrame *src){
    int i;
    dst->pict_type              = src->pict_type;
    dst->quality                = src->quality;
    dst->coded_picture_number   = src->coded_picture_number;
    dst->display_picture_number = src->display_picture_number;
//    dst->reference              = src->reference;
    dst->pts                    = src->pts;
    dst->interlaced_frame       = src->interlaced_frame;
    dst->top_field_first        = src->top_field_first;
    if(s->avctx->me_threshold){
        if(!src->motion_val[0])
            av_log(s->avctx, AV_LOG_ERROR, "AVFrame.motion_val not set!\n");
        if(!src->mb_type)
            av_log(s->avctx, AV_LOG_ERROR, "AVFrame.mb_type not set!\n");
        if(!src->ref_index[0])
            av_log(s->avctx, AV_LOG_ERROR, "AVFrame.ref_index not set!\n");
        if(src->motion_subsample_log2 != dst->motion_subsample_log2)
            av_log(s->avctx, AV_LOG_ERROR, "AVFrame.motion_subsample_log2 doesnt match! (%d!=%d)\n",
            src->motion_subsample_log2, dst->motion_subsample_log2);
        memcpy(dst->mb_type, src->mb_type, s->mb_stride * s->mb_height * sizeof(dst->mb_type[0]));
        
        for(i=0; i<2; i++){
            int stride= ((16*s->mb_width )>>src->motion_subsample_log2) + 1;
            int height= ((16*s->mb_height)>>src->motion_subsample_log2);
            if(src->motion_val[i] && src->motion_val[i] != dst->motion_val[i]){
                memcpy(dst->motion_val[i], src->motion_val[i], 2*stride*height*sizeof(int16_t));
            }
            if(src->ref_index[i] && src->ref_index[i] != dst->ref_index[i]){
                memcpy(dst->ref_index[i], src->ref_index[i], s->b8_stride*2*s->mb_height*sizeof(int8_t));
            }
        }
    }
}
/**
 * allocates a Picture
 * The pixels are allocated/set by calling get_buffer() if shared=0
 */
static int alloc_picture(MpegEncContext *s, Picture *pic, int shared){
    const int big_mb_num= s->mb_stride*(s->mb_height+1) + 1; //the +1 is needed so memset(,,stride*height) doesnt sig11
    const int mb_array_size= s->mb_stride*s->mb_height;
    const int b8_array_size= s->b8_stride*s->mb_height*2;
    const int b4_array_size= s->b4_stride*s->mb_height*4;
    int i;
    
    if(shared){
        assert(pic->data[0]);
        assert(pic->type == 0 || pic->type == FF_BUFFER_TYPE_SHARED);
        pic->type= FF_BUFFER_TYPE_SHARED;
    }else{
        int r;
        
        assert(!pic->data[0]);
        
        r= s->avctx->get_buffer(s->avctx, (AVFrame*)pic);
        
        if(r<0 || !pic->age || !pic->type || !pic->data[0]){
	    av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed (%d %d %d %p)\n", r, pic->age, pic->type, pic->data[0]);
            return -1;
        }
        if(s->linesize && (s->linesize != pic->linesize[0] || s->uvlinesize != pic->linesize[1])){
            av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed (stride changed)\n");
            return -1;
        }
        if(pic->linesize[1] != pic->linesize[2]){
            av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed (uv stride missmatch)\n");
            return -1;
        }
        s->linesize  = pic->linesize[0];
        s->uvlinesize= pic->linesize[1];
    }
    
    if(pic->qscale_table==NULL){
        if (s->encoding) {        
            CHECKED_ALLOCZ(pic->mb_var   , mb_array_size * sizeof(int16_t))
            CHECKED_ALLOCZ(pic->mc_mb_var, mb_array_size * sizeof(int16_t))
            CHECKED_ALLOCZ(pic->mb_mean  , mb_array_size * sizeof(int8_t))
        }
        CHECKED_ALLOCZ(pic->mbskip_table , mb_array_size * sizeof(uint8_t)+2) //the +2 is for the slice end check
        CHECKED_ALLOCZ(pic->qscale_table , mb_array_size * sizeof(uint8_t))
        CHECKED_ALLOCZ(pic->mb_type_base , big_mb_num    * sizeof(uint32_t))
        pic->mb_type= pic->mb_type_base + s->mb_stride+1;
        if(s->out_format == FMT_H264){
            for(i=0; i<2; i++){
                CHECKED_ALLOCZ(pic->motion_val_base[i], 2 * (b4_array_size+2)  * sizeof(int16_t))
                pic->motion_val[i]= pic->motion_val_base[i]+2;
                CHECKED_ALLOCZ(pic->ref_index[i], b8_array_size * sizeof(uint8_t))
            }
            pic->motion_subsample_log2= 2;
        }else if(s->out_format == FMT_H263 || s->encoding || (s->avctx->debug&FF_DEBUG_MV) || (s->avctx->debug_mv)){
            for(i=0; i<2; i++){
                CHECKED_ALLOCZ(pic->motion_val_base[i], 2 * (b8_array_size+2) * sizeof(int16_t))
                pic->motion_val[i]= pic->motion_val_base[i]+2;
                CHECKED_ALLOCZ(pic->ref_index[i], b8_array_size * sizeof(uint8_t))
            }
            pic->motion_subsample_log2= 3;
        }
        if(s->avctx->debug&FF_DEBUG_DCT_COEFF) {
            CHECKED_ALLOCZ(pic->dct_coeff, 64 * mb_array_size * sizeof(DCTELEM)*6)
        }
        pic->qstride= s->mb_stride;
        CHECKED_ALLOCZ(pic->pan_scan , 1 * sizeof(AVPanScan))
    }
    //it might be nicer if the application would keep track of these but it would require a API change
    memmove(s->prev_pict_types+1, s->prev_pict_types, PREV_PICT_TYPES_BUFFER_SIZE-1);
    s->prev_pict_types[0]= s->pict_type;
    if(pic->age < PREV_PICT_TYPES_BUFFER_SIZE && s->prev_pict_types[pic->age] == B_TYPE)
        pic->age= INT_MAX; // skiped MBs in b frames are quite rare in mpeg1/2 and its a bit tricky to skip them anyway
    
    return 0;
fail: //for the CHECKED_ALLOCZ macro
    return -1;
}
/**
 * deallocates a picture
 */
static void free_picture(MpegEncContext *s, Picture *pic){
    int i;
    if(pic->data[0] && pic->type!=FF_BUFFER_TYPE_SHARED){
        s->avctx->release_buffer(s->avctx, (AVFrame*)pic);
    }
    av_freep(&pic->mb_var);
    av_freep(&pic->mc_mb_var);
    av_freep(&pic->mb_mean);
    av_freep(&pic->mbskip_table);
    av_freep(&pic->qscale_table);
    av_freep(&pic->mb_type_base);
    av_freep(&pic->dct_coeff);
    av_freep(&pic->pan_scan);
    pic->mb_type= NULL;
    for(i=0; i<2; i++){
        av_freep(&pic->motion_val_base[i]);
        av_freep(&pic->ref_index[i]);
    }
    
    if(pic->type == FF_BUFFER_TYPE_SHARED){
        for(i=0; i<4; i++){
            pic->base[i]=
            pic->data[i]= NULL;
        }
        pic->type= 0;        
    }
}
static int init_duplicate_context(MpegEncContext *s, MpegEncContext *base){
    int i;
    // edge emu needs blocksize + filter length - 1 (=17x17 for halfpel / 21x21 for h264) 
    CHECKED_ALLOCZ(s->allocated_edge_emu_buffer, (s->width+64)*2*17*2); //(width + edge + align)*interlaced*MBsize*tolerance
    s->edge_emu_buffer= s->allocated_edge_emu_buffer + (s->width+64)*2*17;
     //FIXME should be linesize instead of s->width*2 but that isnt known before get_buffer()
    CHECKED_ALLOCZ(s->me.scratchpad,  (s->width+64)*4*16*2*sizeof(uint8_t)) 
    s->rd_scratchpad=   s->me.scratchpad;
    s->b_scratchpad=    s->me.scratchpad;
    s->obmc_scratchpad= s->me.scratchpad + 16;
    if (s->encoding) {
        CHECKED_ALLOCZ(s->me.map      , ME_MAP_SIZE*sizeof(uint32_t))
        CHECKED_ALLOCZ(s->me.score_map, ME_MAP_SIZE*sizeof(uint32_t))
        if(s->avctx->noise_reduction){
            CHECKED_ALLOCZ(s->dct_error_sum, 2 * 64 * sizeof(int))
        }
    }   
    CHECKED_ALLOCZ(s->blocks, 64*12*2 * sizeof(DCTELEM))
    s->block= s->blocks[0];
    for(i=0;i<12;i++){
        s->pblocks[i] = (short *)(&s->block[i]);
    }
    return 0;
fail:
    return -1; //free() through MPV_common_end()
}
static void free_duplicate_context(MpegEncContext *s){
    if(s==NULL) return;
    av_freep(&s->allocated_edge_emu_buffer); s->edge_emu_buffer= NULL;
    av_freep(&s->me.scratchpad);
    s->rd_scratchpad=   
    s->b_scratchpad=    
    s->obmc_scratchpad= NULL;
    
    av_freep(&s->dct_error_sum);
    av_freep(&s->me.map);
    av_freep(&s->me.score_map);
    av_freep(&s->blocks);
    s->block= NULL;
}
static void backup_duplicate_context(MpegEncContext *bak, MpegEncContext *src){
#define COPY(a) bak->a= src->a
    COPY(allocated_edge_emu_buffer);
    COPY(edge_emu_buffer);
    COPY(me.scratchpad);
    COPY(rd_scratchpad);
    COPY(b_scratchpad);
    COPY(obmc_scratchpad);
    COPY(me.map);
    COPY(me.score_map);
    COPY(blocks);
    COPY(block);
    COPY(start_mb_y);
    COPY(end_mb_y);
    COPY(me.map_generation);
    COPY(pb);
    COPY(dct_error_sum);
    COPY(dct_count[0]);
    COPY(dct_count[1]);
#undef COPY
}
void ff_update_duplicate_context(MpegEncContext *dst, MpegEncContext *src){
    MpegEncContext bak;
    int i;
    //FIXME copy only needed parts
//START_TIMER
    backup_duplicate_context(&bak, dst);
    memcpy(dst, src, sizeof(MpegEncContext));
    backup_duplicate_context(dst, &bak);
    for(i=0;i<12;i++){
        dst->pblocks[i] = (short *)(&dst->block[i]);
    }
//STOP_TIMER("update_duplicate_context") //about 10k cycles / 0.01 sec for 1000frames on 1ghz with 2 threads
}
static void update_duplicate_context_after_me(MpegEncContext *dst, MpegEncContext *src){
#define COPY(a) dst->a= src->a
    COPY(pict_type);
    COPY(current_picture);
    COPY(f_code);
    COPY(b_code);
    COPY(qscale);
    COPY(lambda);
    COPY(lambda2);
    COPY(picture_in_gop_number);
    COPY(gop_picture_number);
    COPY(frame_pred_frame_dct); //FIXME dont set in encode_header
    COPY(progressive_frame); //FIXME dont set in encode_header
    COPY(partitioned_frame); //FIXME dont set in encode_header
#undef COPY
}
/**
 * sets the given MpegEncContext to common defaults (same for encoding and decoding).
 * the changed fields will not depend upon the prior state of the MpegEncContext.
 */
static void MPV_common_defaults(MpegEncContext *s){
    s->y_dc_scale_table=
    s->c_dc_scale_table= ff_mpeg1_dc_scale_table;
    s->chroma_qscale_table= ff_default_chroma_qscale_table;
    s->progressive_frame= 1;
    s->progressive_sequence= 1;
    s->picture_structure= PICT_FRAME;
    s->coded_picture_number = 0;
    s->picture_number = 0;
    s->input_picture_number = 0;
    s->picture_in_gop_number = 0;
    s->f_code = 1;
    s->b_code = 1;
}
/**
 * sets the given MpegEncContext to defaults for decoding.
 * the changed fields will not depend upon the prior state of the MpegEncContext.
 */
void MPV_decode_defaults(MpegEncContext *s){
    MPV_common_defaults(s);
}
/**
 * sets the given MpegEncContext to defaults for encoding.
 * the changed fields will not depend upon the prior state of the MpegEncContext.
 */
/** 
 * init common structure for both encoder and decoder.
 * this assumes that some variables like width/height are already set
 */
int MPV_common_init(MpegEncContext *s)
{
    int y_size, c_size, yc_size, i, mb_array_size, mv_table_size, x, y;
    if(s->avctx->thread_count > MAX_THREADS || (16*s->avctx->thread_count > s->height && s->height)){
        av_log(s->avctx, AV_LOG_ERROR, "too many threads\n");
        return -1;
    }
    dsputil_init(&s->dsp, s->avctx);
    //DCT_common_init(s);
    s->flags= s->avctx->flags;
    s->flags2= s->avctx->flags2;
    s->mb_width  = (s->width  + 15) / 16;
    s->mb_height = (s->height + 15) / 16;
    s->mb_stride = s->mb_width + 1;
    s->b8_stride = s->mb_width*2 + 1;
    s->b4_stride = s->mb_width*4 + 1;
    mb_array_size= s->mb_height * s->mb_stride;
    mv_table_size= (s->mb_height+2) * s->mb_stride + 1;
    /* set chroma shifts */
    avcodec_get_chroma_sub_sample(s->avctx->pix_fmt,&(s->chroma_x_shift),
                                                    &(s->chroma_y_shift) );
    /* set default edge pos, will be overriden in decode_header if needed */
    s->h_edge_pos= s->mb_width*16;
    s->v_edge_pos= s->mb_height*16;
    s->mb_num = s->mb_width * s->mb_height;
    
    s->block_wrap[0]=
    s->block_wrap[1]=
    s->block_wrap[2]=
    s->block_wrap[3]= s->b8_stride;
    s->block_wrap[4]=
    s->block_wrap[5]= s->mb_stride;
 
    y_size = s->b8_stride * (2 * s->mb_height + 1);
    c_size = s->mb_stride * (s->mb_height + 1);
    yc_size = y_size + 2 * c_size;
    
    /* convert fourcc to upper case */
    s->avctx->codec_tag=   toupper( s->avctx->codec_tag     &0xFF)          
                        + (toupper((s->avctx->codec_tag>>8 )&0xFF)<<8 )
                        + (toupper((s->avctx->codec_tag>>16)&0xFF)<<16) 
                        + (toupper((s->avctx->codec_tag>>24)&0xFF)<<24);
    s->avctx->stream_codec_tag=   toupper( s->avctx->stream_codec_tag     &0xFF)          
                               + (toupper((s->avctx->stream_codec_tag>>8 )&0xFF)<<8 )
                               + (toupper((s->avctx->stream_codec_tag>>16)&0xFF)<<16) 
                               + (toupper((s->avctx->stream_codec_tag>>24)&0xFF)<<24);
    s->avctx->coded_frame= (AVFrame*)&s->current_picture;
    CHECKED_ALLOCZ(s->mb_index2xy, (s->mb_num+1)*sizeof(int)) //error ressilience code looks cleaner with this
    for(y=0; y<s->mb_height; y++){
        for(x=0; x<s->mb_width; x++){
            s->mb_index2xy[ x + y*s->mb_width ] = x + y*s->mb_stride;
        }
    }
    s->mb_index2xy[ s->mb_height*s->mb_width ] = (s->mb_height-1)*s->mb_stride + s->mb_width; //FIXME really needed?

    CHECKED_ALLOCZ(s->picture, MAX_PICTURE_COUNT * sizeof(Picture))
    CHECKED_ALLOCZ(s->error_status_table, mb_array_size*sizeof(uint8_t))


    /* which mb is a intra block */
    CHECKED_ALLOCZ(s->mbintra_table, mb_array_size);
    memset(s->mbintra_table, 1, mb_array_size);
    
    /* init macroblock skip table */
    CHECKED_ALLOCZ(s->mbskip_table, mb_array_size+2);
    //Note the +1 is for a quicker mpeg4 slice_end detection
    CHECKED_ALLOCZ(s->prev_pict_types, PREV_PICT_TYPES_BUFFER_SIZE);
    
    s->parse_context.state= -1;
    if((s->avctx->debug&(FF_DEBUG_VIS_QP|FF_DEBUG_VIS_MB_TYPE)) || (s->avctx->debug_mv)){
       s->visualization_buffer[0] = av_malloc((s->mb_width*16 + 2*EDGE_WIDTH) * s->mb_height*16 + 2*EDGE_WIDTH);
       s->visualization_buffer[1] = av_malloc((s->mb_width*8 + EDGE_WIDTH) * s->mb_height*8 + EDGE_WIDTH);
       s->visualization_buffer[2] = av_malloc((s->mb_width*8 + EDGE_WIDTH) * s->mb_height*8 + EDGE_WIDTH);
    }
    s->context_initialized = 1;
    s->thread_context[0]= s;
    for(i=1; i<s->avctx->thread_count; i++){
        s->thread_context[i]= av_malloc(sizeof(MpegEncContext));
        memcpy(s->thread_context[i], s, sizeof(MpegEncContext));
    }
    for(i=0; i<s->avctx->thread_count; i++){
        if(init_duplicate_context(s->thread_context[i], s) < 0)
           goto fail;
        s->thread_context[i]->start_mb_y= (s->mb_height*(i  ) + s->avctx->thread_count/2) / s->avctx->thread_count;
        s->thread_context[i]->end_mb_y  = (s->mb_height*(i+1) + s->avctx->thread_count/2) / s->avctx->thread_count;
    }
    return 0;
 fail:
    MPV_common_end(s);
    return -1;
}
/* init common structure for both encoder and decoder */
void MPV_common_end(MpegEncContext *s)
{
    int i, j, k;
    for(i=0; i<s->avctx->thread_count; i++){
        free_duplicate_context(s->thread_context[i]);
    }
    for(i=1; i<s->avctx->thread_count; i++){
        av_freep(&s->thread_context[i]);
    }
    av_freep(&s->parse_context.buffer);
    s->parse_context.buffer_size=0;
    av_freep(&s->mb_type);
    av_freep(&s->p_mv_table_base);
    av_freep(&s->b_forw_mv_table_base);
    av_freep(&s->b_back_mv_table_base);
    av_freep(&s->b_bidir_forw_mv_table_base);
    av_freep(&s->b_bidir_back_mv_table_base);
    av_freep(&s->b_direct_mv_table_base);
    s->p_mv_table= NULL;
    s->b_forw_mv_table= NULL;
    s->b_back_mv_table= NULL;
    s->b_bidir_forw_mv_table= NULL;
    s->b_bidir_back_mv_table= NULL;
    s->b_direct_mv_table= NULL;
    for(i=0; i<2; i++){
        for(j=0; j<2; j++){
            for(k=0; k<2; k++){
                av_freep(&s->b_field_mv_table_base[i][j][k]);
                s->b_field_mv_table[i][j][k]=NULL;
            }
            av_freep(&s->b_field_select_table[i][j]);
            av_freep(&s->p_field_mv_table_base[i][j]);
            s->p_field_mv_table[i][j]=NULL;
        }
        av_freep(&s->p_field_select_table[i]);
    }
    
    av_freep(&s->dc_val_base);
    av_freep(&s->ac_val_base);
    av_freep(&s->coded_block_base);
    av_freep(&s->mbintra_table);
    av_freep(&s->cbp_table);
    av_freep(&s->pred_dir_table);
    
    av_freep(&s->mbskip_table);
    av_freep(&s->prev_pict_types);
    av_freep(&s->bitstream_buffer);
    av_freep(&s->avctx->stats_out);
    av_freep(&s->ac_stats);
    av_freep(&s->error_status_table);
    av_freep(&s->mb_index2xy);
    av_freep(&s->lambda_table);
    av_freep(&s->q_intra_matrix);
    av_freep(&s->q_inter_matrix);
    av_freep(&s->q_intra_matrix16);
    av_freep(&s->q_inter_matrix16);
    av_freep(&s->input_picture);
    av_freep(&s->reordered_input_picture);
    av_freep(&s->dct_offset);
    if(s->picture){
        for(i=0; i<MAX_PICTURE_COUNT; i++){
            free_picture(s, &s->picture[i]);
        }
    }
    av_freep(&s->picture);
    s->context_initialized = 0;
    s->last_picture_ptr=
    s->next_picture_ptr=
    s->current_picture_ptr= NULL;
    for(i=0; i<3; i++)
        av_freep(&s->visualization_buffer[i]);
}
void init_rl(RLTable *rl)
{
    int8_t max_level[MAX_RUN+1], max_run[MAX_LEVEL+1];
    uint8_t index_run[MAX_RUN+1];
    int last, run, level, start, end, i;
    /* compute max_level[], max_run[] and index_run[] */
    for(last=0;last<2;last++) {
        if (last == 0) {
            start = 0;
            end = rl->last;
        } else {
            start = rl->last;
            end = rl->n;
        }
        memset(max_level, 0, MAX_RUN + 1);
        memset(max_run, 0, MAX_LEVEL + 1);
        memset(index_run, rl->n, MAX_RUN + 1);
        for(i=start;i<end;i++) {
            run = rl->table_run[i];
            level = rl->table_level[i];
            if (index_run[run] == rl->n)
                index_run[run] = i;
            if (level > max_level[run])
                max_level[run] = level;
            if (run > max_run[level])
                max_run[level] = run;
        }
        rl->max_level[last] = av_malloc(MAX_RUN + 1);
        memcpy(rl->max_level[last], max_level, MAX_RUN + 1);
        rl->max_run[last] = av_malloc(MAX_LEVEL + 1);
        memcpy(rl->max_run[last], max_run, MAX_LEVEL + 1);
        rl->index_run[last] = av_malloc(MAX_RUN + 1);
        memcpy(rl->index_run[last], index_run, MAX_RUN + 1);
    }
}
/* draw the edges of width 'w' of an image of size width, height */
//FIXME check that this is ok for mpeg4 interlaced
static void draw_edges_c(uint8_t *buf, int wrap, int width, int height, int w)
{
    uint8_t *ptr, *last_line;
    int i;
    last_line = buf + (height - 1) * wrap;
    for(i=0;i<w;i++) {
        /* top and bottom */
        memcpy(buf - (i + 1) * wrap, buf, width);
        memcpy(last_line + (i + 1) * wrap, last_line, width);
    }
    /* left and right */
    ptr = buf;
    for(i=0;i<height;i++) {
        memset(ptr - w, ptr[0], w);
        memset(ptr + width, ptr[width-1], w);
        ptr += wrap;
    }
    /* corners */
    for(i=0;i<w;i++) {
        memset(buf - (i + 1) * wrap - w, buf[0], w); /* top left */
        memset(buf - (i + 1) * wrap + width, buf[width-1], w); /* top right */
        memset(last_line + (i + 1) * wrap - w, last_line[0], w); /* top left */
        memset(last_line + (i + 1) * wrap + width, last_line[width-1], w); /* top right */
    }
}
int ff_find_unused_picture(MpegEncContext *s, int shared){
    int i;
    
    if(shared){
        for(i=0; i<MAX_PICTURE_COUNT; i++){
            if(s->picture[i].data[0]==NULL && s->picture[i].type==0) return i;
        }
    }else{
        for(i=0; i<MAX_PICTURE_COUNT; i++){
            if(s->picture[i].data[0]==NULL && s->picture[i].type!=0) return i; //FIXME
        }
        for(i=0; i<MAX_PICTURE_COUNT; i++){
            if(s->picture[i].data[0]==NULL) return i;
        }
    }
    assert(0);
    return -1;
}
static void update_noise_reduction(MpegEncContext *s){
    int intra, i;
    for(intra=0; intra<2; intra++){
        if(s->dct_count[intra] > (1<<16)){
            for(i=0; i<64; i++){
                s->dct_error_sum[intra][i] >>=1;
            }
            s->dct_count[intra] >>= 1;
        }
        
        for(i=0; i<64; i++){
            s->dct_offset[intra][i]= (s->avctx->noise_reduction * s->dct_count[intra] + s->dct_error_sum[intra][i]/2) / (s->dct_error_sum[intra][i]+1);
        }
    }
}
/**
 * generic function for encode/decode called after coding/decoding the header and before a frame is coded/decoded
 */
int MPV_frame_start(MpegEncContext *s, AVCodecContext *avctx)
{
    int i;
    AVFrame *pic;
    s->mb_skiped = 0;
    assert(s->last_picture_ptr==NULL || s->out_format != FMT_H264 || s->codec_id == CODEC_ID_SVQ3);
    /* mark&release old frames */
    if (s->pict_type != B_TYPE && s->last_picture_ptr && s->last_picture_ptr != s->next_picture_ptr && s->last_picture_ptr->data[0]) {
        avctx->release_buffer(avctx, (AVFrame*)s->last_picture_ptr);
        /* release forgotten pictures */
        /* if(mpeg124/h263) */
        if(!s->encoding){
            for(i=0; i<MAX_PICTURE_COUNT; i++){
                if(s->picture[i].data[0] && &s->picture[i] != s->next_picture_ptr && s->picture[i].reference){
                    av_log(avctx, AV_LOG_ERROR, "releasing zombie picture\n");
                    avctx->release_buffer(avctx, (AVFrame*)&s->picture[i]);                
                }
            }
        }
    }
alloc:
    if(!s->encoding){
        /* release non refernce frames */
        for(i=0; i<MAX_PICTURE_COUNT; i++){
            if(s->picture[i].data[0] && !s->picture[i].reference /*&& s->picture[i].type!=FF_BUFFER_TYPE_SHARED*/){
                s->avctx->release_buffer(s->avctx, (AVFrame*)&s->picture[i]);
            }
        }
        if(s->current_picture_ptr && s->current_picture_ptr->data[0]==NULL)
            pic= (AVFrame*)s->current_picture_ptr; //we allready have a unused image (maybe it was set before reading the header)
        else{
            i= ff_find_unused_picture(s, 0);
            pic= (AVFrame*)&s->picture[i];
        }
        pic->reference= s->pict_type != B_TYPE && !s->dropable ? 3 : 0;
        pic->coded_picture_number= s->coded_picture_number++;
        
        if( alloc_picture(s, (Picture*)pic, 0) < 0)
            return -1;
        s->current_picture_ptr= (Picture*)pic;
        s->current_picture_ptr->top_field_first= s->top_field_first; //FIXME use only the vars from current_pic
        s->current_picture_ptr->interlaced_frame= !s->progressive_frame && !s->progressive_sequence;
    }
    s->current_picture_ptr->pict_type= s->pict_type;
//    if(s->flags && CODEC_FLAG_QSCALE) 
  //      s->current_picture_ptr->quality= s->new_picture_ptr->quality;
    s->current_picture_ptr->key_frame= s->pict_type == I_TYPE;
    copy_picture(&s->current_picture, s->current_picture_ptr);
  
  if(s->out_format != FMT_H264 || s->codec_id == CODEC_ID_SVQ3){
    if (s->pict_type != B_TYPE) {
        s->last_picture_ptr= s->next_picture_ptr;
        if(!s->dropable)
            s->next_picture_ptr= s->current_picture_ptr;
    }
/*    av_log(s->avctx, AV_LOG_DEBUG, "L%p N%p C%p L%p N%p C%p type:%d drop:%d\n", s->last_picture_ptr, s->next_picture_ptr,s->current_picture_ptr,
        s->last_picture_ptr    ? s->last_picture_ptr->data[0] : NULL, 
        s->next_picture_ptr    ? s->next_picture_ptr->data[0] : NULL, 
        s->current_picture_ptr ? s->current_picture_ptr->data[0] : NULL,
        s->pict_type, s->dropable);*/
    
    if(s->last_picture_ptr) copy_picture(&s->last_picture, s->last_picture_ptr);
    if(s->next_picture_ptr) copy_picture(&s->next_picture, s->next_picture_ptr);
    
    if(s->pict_type != I_TYPE && (s->last_picture_ptr==NULL || s->last_picture_ptr->data[0]==NULL)){
        av_log(avctx, AV_LOG_ERROR, "warning: first frame is no keyframe\n");
        assert(s->pict_type != B_TYPE); //these should have been dropped if we dont have a reference
        goto alloc;
    }
    assert(s->pict_type == I_TYPE || (s->last_picture_ptr && s->last_picture_ptr->data[0]));
    if(s->picture_structure!=PICT_FRAME){
        int i;
        for(i=0; i<4; i++){
            if(s->picture_structure == PICT_BOTTOM_FIELD){
                 s->current_picture.data[i] += s->current_picture.linesize[i];
            } 
            s->current_picture.linesize[i] *= 2;
            s->last_picture.linesize[i] *=2;
            s->next_picture.linesize[i] *=2;
        }
    }
  }
   
    s->hurry_up= s->avctx->hurry_up;
    s->error_resilience= avctx->error_resilience;
    /* set dequantizer, we cant do it during init as it might change for mpeg4
       and we cant do it in the header decode as init isnt called for mpeg4 there yet */
    if(s->mpeg_quant || s->codec_id == CODEC_ID_MPEG2VIDEO){
        s->dct_unquantize_intra = s->dct_unquantize_mpeg2_intra;
        s->dct_unquantize_inter = s->dct_unquantize_mpeg2_inter;
    }else if(s->out_format == FMT_H263){
        s->dct_unquantize_intra = s->dct_unquantize_h263_intra;
        s->dct_unquantize_inter = s->dct_unquantize_h263_inter;
    }else if(s->out_format == FMT_H261){
        s->dct_unquantize_intra = s->dct_unquantize_h261_intra;
        s->dct_unquantize_inter = s->dct_unquantize_h261_inter;
    }else{
        s->dct_unquantize_intra = s->dct_unquantize_mpeg1_intra;
        s->dct_unquantize_inter = s->dct_unquantize_mpeg1_inter;
    }
    if(s->dct_error_sum){
        assert(s->avctx->noise_reduction && s->encoding);
        update_noise_reduction(s);
    }
        
#ifdef HAVE_XVMC
    if(s->avctx->xvmc_acceleration)
        return XVMC_field_start(s, avctx);
#endif
    return 0;
}
/* generic function for encode/decode called after a frame has been coded/decoded */
void MPV_frame_end(MpegEncContext *s)
{
    int i;
    /* draw edge for correct motion prediction if outside */
#ifdef HAVE_XVMC
//just to make sure that all data is rendered.
    if(s->avctx->xvmc_acceleration){
        XVMC_field_end(s);
    }else
#endif
    if(s->unrestricted_mv && s->pict_type != B_TYPE && !s->intra_only && !(s->flags&CODEC_FLAG_EMU_EDGE)) {
            draw_edges(s->current_picture.data[0], s->linesize  , s->h_edge_pos   , s->v_edge_pos   , EDGE_WIDTH  );
            draw_edges(s->current_picture.data[1], s->uvlinesize, s->h_edge_pos>>1, s->v_edge_pos>>1, EDGE_WIDTH/2);
            draw_edges(s->current_picture.data[2], s->uvlinesize, s->h_edge_pos>>1, s->v_edge_pos>>1, EDGE_WIDTH/2);
    }
    emms_c();
    
    s->last_pict_type    = s->pict_type;
    if(s->pict_type!=B_TYPE){
        s->last_non_b_pict_type= s->pict_type;
    }


}
/**
 * draws an line from (ex, ey) -> (sx, sy).
 * @param w width of the image
 * @param h height of the image
 * @param stride stride/linesize of the image
 * @param color color of the arrow
 */
static void draw_line(uint8_t *buf, int sx, int sy, int ex, int ey, int w, int h, int stride, int color){
    int t, x, y, fr, f;
    
    sx= clip(sx, 0, w-1);
    sy= clip(sy, 0, h-1);
    ex= clip(ex, 0, w-1);
    ey= clip(ey, 0, h-1);
    
    buf[sy*stride + sx]+= color;
    
    if(ABS(ex - sx) > ABS(ey - sy)){
        if(sx > ex){
            t=sx; sx=ex; ex=t;
            t=sy; sy=ey; ey=t;
        }
        buf+= sx + sy*stride;
        ex-= sx;
        f= ((ey-sy)<<16)/ex;
        for(x= 0; x <= ex; x++){
            y = (x*f)>>16;
            fr= (x*f)&0xFFFF;
            buf[ y   *stride + x]+= (color*(0x10000-fr))>>16;
            buf[(y+1)*stride + x]+= (color*         fr )>>16;
        }
    }else{
        if(sy > ey){
            t=sx; sx=ex; ex=t;
            t=sy; sy=ey; ey=t;
        }
        buf+= sx + sy*stride;
        ey-= sy;
        if(ey) f= ((ex-sx)<<16)/ey;
        else   f= 0;
        for(y= 0; y <= ey; y++){
            x = (y*f)>>16;
            fr= (y*f)&0xFFFF;
            buf[y*stride + x  ]+= (color*(0x10000-fr))>>16;;
            buf[y*stride + x+1]+= (color*         fr )>>16;;
        }
    }
}
/**
 * draws an arrow from (ex, ey) -> (sx, sy).
 * @param w width of the image
 * @param h height of the image
 * @param stride stride/linesize of the image
 * @param color color of the arrow
 */
static void draw_arrow(uint8_t *buf, int sx, int sy, int ex, int ey, int w, int h, int stride, int color){ 
    int dx,dy;
    sx= clip(sx, -100, w+100);
    sy= clip(sy, -100, h+100);
    ex= clip(ex, -100, w+100);
    ey= clip(ey, -100, h+100);
    
    dx= ex - sx;
    dy= ey - sy;
    
    if(dx*dx + dy*dy > 3*3){
        int rx=  dx + dy;
        int ry= -dx + dy;
        int length= ff_sqrt((rx*rx + ry*ry)<<8);
        
        //FIXME subpixel accuracy
        rx= ROUNDED_DIV(rx*3<<4, length);
        ry= ROUNDED_DIV(ry*3<<4, length);
        
        draw_line(buf, sx, sy, sx + rx, sy + ry, w, h, stride, color);
        draw_line(buf, sx, sy, sx - ry, sy + rx, w, h, stride, color);
    }
    draw_line(buf, sx, sy, ex, ey, w, h, stride, color);
}

/**
 * Copies a rectangular area of samples to a temporary buffer and replicates the boarder samples.
 * @param buf destination buffer
 * @param src source buffer
 * @param linesize number of bytes between 2 vertically adjacent samples in both the source and destination buffers
 * @param block_w width of block
 * @param block_h height of block
 * @param src_x x coordinate of the top left sample of the block in the source buffer
 * @param src_y y coordinate of the top left sample of the block in the source buffer
 * @param w width of the source buffer
 * @param h height of the source buffer
 */
void ff_emulated_edge_mc(uint8_t *buf, uint8_t *src, int linesize, int block_w, int block_h, 
                                    int src_x, int src_y, int w, int h){
    int x, y;
    int start_y, start_x, end_y, end_x;
    if(src_y>= h){
        src+= (h-1-src_y)*linesize;
        src_y=h-1;
    }else if(src_y<=-block_h){
        src+= (1-block_h-src_y)*linesize;
        src_y=1-block_h;
    }
    if(src_x>= w){
        src+= (w-1-src_x);
        src_x=w-1;
    }else if(src_x<=-block_w){
        src+= (1-block_w-src_x);
        src_x=1-block_w;
    }
    start_y= FFMAX(0, -src_y);
    start_x= FFMAX(0, -src_x);
    end_y= FFMIN(block_h, h-src_y);
    end_x= FFMIN(block_w, w-src_x);
    // copy existing part
    for(y=start_y; y<end_y; y++){
        for(x=start_x; x<end_x; x++){
            buf[x + y*linesize]= src[x + y*linesize];
        }
    }
    //top
    for(y=0; y<start_y; y++){
        for(x=start_x; x<end_x; x++){
            buf[x + y*linesize]= buf[x + start_y*linesize];
        }
    }
    //bottom
    for(y=end_y; y<block_h; y++){
        for(x=start_x; x<end_x; x++){
            buf[x + y*linesize]= buf[x + (end_y-1)*linesize];
        }
    }
                                    
    for(y=0; y<block_h; y++){
       //left
        for(x=0; x<start_x; x++){
            buf[x + y*linesize]= buf[start_x + y*linesize];
        }
       
       //right
        for(x=end_x; x<block_w; x++){
            buf[x + y*linesize]= buf[end_x - 1 + y*linesize];
        }
    }
}
static inline int hpel_motion(MpegEncContext *s, 
                                  uint8_t *dest, uint8_t *src,
                                  int field_based, int field_select,
                                  int src_x, int src_y,
                                  int width, int height, int stride,
                                  int h_edge_pos, int v_edge_pos,
                                  int w, int h, op_pixels_func *pix_op,
                                  int motion_x, int motion_y)
{
    int dxy;
    int emu=0;
    dxy = ((motion_y & 1) << 1) | (motion_x & 1);
    src_x += motion_x >> 1;
    src_y += motion_y >> 1;
                
    /* WARNING: do no forget half pels */
    src_x = clip(src_x, -16, width); //FIXME unneeded for emu?
    if (src_x == width)
        dxy &= ~1;
    src_y = clip(src_y, -16, height);
    if (src_y == height)
        dxy &= ~2;
    src += src_y * stride + src_x;
    if(s->unrestricted_mv && (s->flags&CODEC_FLAG_EMU_EDGE)){
        if(   (unsigned)src_x > h_edge_pos - (motion_x&1) - w
           || (unsigned)src_y > v_edge_pos - (motion_y&1) - h){
            ff_emulated_edge_mc(s->edge_emu_buffer, src, s->linesize, w+1, (h+1)<<field_based,
                             src_x, src_y<<field_based, h_edge_pos, s->v_edge_pos);
            src= s->edge_emu_buffer;
            emu=1;
        }
    }
    if(field_select)
        src += s->linesize;
    pix_op[dxy](dest, src, stride, h);
    return emu;
}
/* apply one mpeg motion vector to the three components */
static always_inline void mpeg_motion(MpegEncContext *s,
                               uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                               int field_based, int bottom_field, int field_select,
                               uint8_t **ref_picture, op_pixels_func (*pix_op)[4],
                               int motion_x, int motion_y, int h)
{
    uint8_t *ptr_y, *ptr_cb, *ptr_cr;
    int dxy, uvdxy, mx, my, src_x, src_y, uvsrc_x, uvsrc_y, v_edge_pos, uvlinesize, linesize;
    
#if 0    
if(s->quarter_sample)
{
    motion_x>>=1;
    motion_y>>=1;
}
#endif
    v_edge_pos = s->v_edge_pos >> field_based;
    linesize   = s->current_picture.linesize[0] << field_based;
    uvlinesize = s->current_picture.linesize[1] << field_based;
    dxy = ((motion_y & 1) << 1) | (motion_x & 1);
    src_x = s->mb_x* 16               + (motion_x >> 1);
    src_y =(s->mb_y<<(4-field_based)) + (motion_y >> 1);
    if (s->out_format == FMT_H263) {
        if((s->workaround_bugs & FF_BUG_HPEL_CHROMA) && field_based){
            mx = (motion_x>>1)|(motion_x&1);
            my = motion_y >>1;
            uvdxy = ((my & 1) << 1) | (mx & 1);
            uvsrc_x = s->mb_x* 8               + (mx >> 1);
            uvsrc_y = (s->mb_y<<(3-field_based)) + (my >> 1);
        }else{
            uvdxy = dxy | (motion_y & 2) | ((motion_x & 2) >> 1);
            uvsrc_x = src_x>>1;
            uvsrc_y = src_y>>1;
        }
    }else if(s->out_format == FMT_H261){//even chroma mv's are full pel in H261
        mx = motion_x / 4;
        my = motion_y / 4;
        uvdxy = 0;
        uvsrc_x = s->mb_x*8 + mx;
        uvsrc_y = s->mb_y*8 + my;
    } else {
        if(s->chroma_y_shift){
            mx = motion_x / 2;
            my = motion_y / 2;
            uvdxy = ((my & 1) << 1) | (mx & 1);
            uvsrc_x = s->mb_x* 8               + (mx >> 1);
            uvsrc_y = (s->mb_y<<(3-field_based)) + (my >> 1);
        } else {
            if(s->chroma_x_shift){
            //Chroma422
                mx = motion_x / 2;
                uvdxy = ((motion_y & 1) << 1) | (mx & 1);
                uvsrc_x = s->mb_x* 8           + (mx >> 1);
                uvsrc_y = src_y;
            } else {
            //Chroma444
                uvdxy = dxy;
                uvsrc_x = src_x;
                uvsrc_y = src_y;
            }
        }
    }
    ptr_y  = ref_picture[0] + src_y * linesize + src_x;
    ptr_cb = ref_picture[1] + uvsrc_y * uvlinesize + uvsrc_x;
    ptr_cr = ref_picture[2] + uvsrc_y * uvlinesize + uvsrc_x;
    if(   (unsigned)src_x > s->h_edge_pos - (motion_x&1) - 16
       || (unsigned)src_y >    v_edge_pos - (motion_y&1) - h){
            if(s->codec_id == CODEC_ID_MPEG2VIDEO ||
               s->codec_id == CODEC_ID_MPEG1VIDEO){
                av_log(s->avctx,AV_LOG_DEBUG,"MPEG motion vector out of boundary\n");
                return ;
            }
            ff_emulated_edge_mc(s->edge_emu_buffer, ptr_y, s->linesize, 17, 17+field_based,
                             src_x, src_y<<field_based, s->h_edge_pos, s->v_edge_pos);
            ptr_y = s->edge_emu_buffer;
            if(!(s->flags&CODEC_FLAG_GRAY)){
                uint8_t *uvbuf= s->edge_emu_buffer+18*s->linesize;
                ff_emulated_edge_mc(uvbuf  , ptr_cb, s->uvlinesize, 9, 9+field_based, 
                                 uvsrc_x, uvsrc_y<<field_based, s->h_edge_pos>>1, s->v_edge_pos>>1);
                ff_emulated_edge_mc(uvbuf+16, ptr_cr, s->uvlinesize, 9, 9+field_based, 
                                 uvsrc_x, uvsrc_y<<field_based, s->h_edge_pos>>1, s->v_edge_pos>>1);
                ptr_cb= uvbuf;
                ptr_cr= uvbuf+16;
            }
    }
    if(bottom_field){ //FIXME use this for field pix too instead of the obnoxious hack which changes picture.data
        dest_y += s->linesize;
        dest_cb+= s->uvlinesize;
        dest_cr+= s->uvlinesize;
    }
    if(field_select){
        ptr_y += s->linesize;
        ptr_cb+= s->uvlinesize;
        ptr_cr+= s->uvlinesize;
    }
    pix_op[0][dxy](dest_y, ptr_y, linesize, h);
    
    if(!(s->flags&CODEC_FLAG_GRAY)){
        pix_op[s->chroma_x_shift][uvdxy](dest_cb, ptr_cb, uvlinesize, h >> s->chroma_y_shift);
        pix_op[s->chroma_x_shift][uvdxy](dest_cr, ptr_cr, uvlinesize, h >> s->chroma_y_shift);
    }
}
//FIXME move to dsputil, avg variant, 16x16 version
static inline void put_obmc(uint8_t *dst, uint8_t *src[5], int stride){
    int x;
    uint8_t * const top   = src[1];
    uint8_t * const left  = src[2];
    uint8_t * const mid   = src[0];
    uint8_t * const right = src[3];
    uint8_t * const bottom= src[4];
#define OBMC_FILTER(x, t, l, m, r, b)\
    dst[x]= (t*top[x] + l*left[x] + m*mid[x] + r*right[x] + b*bottom[x] + 4)>>3
#define OBMC_FILTER4(x, t, l, m, r, b)\
    OBMC_FILTER(x         , t, l, m, r, b);\
    OBMC_FILTER(x+1       , t, l, m, r, b);\
    OBMC_FILTER(x  +stride, t, l, m, r, b);\
    OBMC_FILTER(x+1+stride, t, l, m, r, b);
    
    x=0;
    OBMC_FILTER (x  , 2, 2, 4, 0, 0);
    OBMC_FILTER (x+1, 2, 1, 5, 0, 0);
    OBMC_FILTER4(x+2, 2, 1, 5, 0, 0);
    OBMC_FILTER4(x+4, 2, 0, 5, 1, 0);
    OBMC_FILTER (x+6, 2, 0, 5, 1, 0);
    OBMC_FILTER (x+7, 2, 0, 4, 2, 0);
    x+= stride;
    OBMC_FILTER (x  , 1, 2, 5, 0, 0);
    OBMC_FILTER (x+1, 1, 2, 5, 0, 0);
    OBMC_FILTER (x+6, 1, 0, 5, 2, 0);
    OBMC_FILTER (x+7, 1, 0, 5, 2, 0);
    x+= stride;
    OBMC_FILTER4(x  , 1, 2, 5, 0, 0);
    OBMC_FILTER4(x+2, 1, 1, 6, 0, 0);
    OBMC_FILTER4(x+4, 1, 0, 6, 1, 0);
    OBMC_FILTER4(x+6, 1, 0, 5, 2, 0);
    x+= 2*stride;
    OBMC_FILTER4(x  , 0, 2, 5, 0, 1);
    OBMC_FILTER4(x+2, 0, 1, 6, 0, 1);
    OBMC_FILTER4(x+4, 0, 0, 6, 1, 1);
    OBMC_FILTER4(x+6, 0, 0, 5, 2, 1);
    x+= 2*stride;
    OBMC_FILTER (x  , 0, 2, 5, 0, 1);
    OBMC_FILTER (x+1, 0, 2, 5, 0, 1);
    OBMC_FILTER4(x+2, 0, 1, 5, 0, 2);
    OBMC_FILTER4(x+4, 0, 0, 5, 1, 2);
    OBMC_FILTER (x+6, 0, 0, 5, 2, 1);
    OBMC_FILTER (x+7, 0, 0, 5, 2, 1);
    x+= stride;
    OBMC_FILTER (x  , 0, 2, 4, 0, 2);
    OBMC_FILTER (x+1, 0, 1, 5, 0, 2);
    OBMC_FILTER (x+6, 0, 0, 5, 1, 2);
    OBMC_FILTER (x+7, 0, 0, 4, 2, 2);
}
/* obmc for 1 8x8 luma block */
static inline void obmc_motion(MpegEncContext *s,
                               uint8_t *dest, uint8_t *src,
                               int src_x, int src_y,
                               op_pixels_func *pix_op,
                               int16_t mv[5][2]/* mid top left right bottom*/)
#define MID    0
{
    int i;
    uint8_t *ptr[5];
    
    assert(s->quarter_sample==0);
    
    for(i=0; i<5; i++){
        if(i && mv[i][0]==mv[MID][0] && mv[i][1]==mv[MID][1]){
            ptr[i]= ptr[MID];
        }else{
            ptr[i]= s->obmc_scratchpad + 8*(i&1) + s->linesize*8*(i>>1);
            hpel_motion(s, ptr[i], src, 0, 0,
                        src_x, src_y,
                        s->width, s->height, s->linesize,
                        s->h_edge_pos, s->v_edge_pos,
                        8, 8, pix_op,
                        mv[i][0], mv[i][1]);
        }
    }
    put_obmc(dest, ptr, s->linesize);                
}
static inline void qpel_motion(MpegEncContext *s,
                               uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                               int field_based, int bottom_field, int field_select,
                               uint8_t **ref_picture, op_pixels_func (*pix_op)[4],
                               qpel_mc_func (*qpix_op)[16],
                               int motion_x, int motion_y, int h)
{
    uint8_t *ptr_y, *ptr_cb, *ptr_cr;
    int dxy, uvdxy, mx, my, src_x, src_y, uvsrc_x, uvsrc_y, v_edge_pos, linesize, uvlinesize;
    dxy = ((motion_y & 3) << 2) | (motion_x & 3);
    src_x = s->mb_x *  16                 + (motion_x >> 2);
    src_y = s->mb_y * (16 >> field_based) + (motion_y >> 2);
    v_edge_pos = s->v_edge_pos >> field_based;
    linesize = s->linesize << field_based;
    uvlinesize = s->uvlinesize << field_based;
    
    if(field_based){
        mx= motion_x/2;
        my= motion_y>>1;
    }else if(s->workaround_bugs&FF_BUG_QPEL_CHROMA2){
        static const int rtab[8]= {0,0,1,1,0,0,0,1};
        mx= (motion_x>>1) + rtab[motion_x&7];
        my= (motion_y>>1) + rtab[motion_y&7];
    }else if(s->workaround_bugs&FF_BUG_QPEL_CHROMA){
        mx= (motion_x>>1)|(motion_x&1);
        my= (motion_y>>1)|(motion_y&1);
    }else{
        mx= motion_x/2;
        my= motion_y/2;
    }
    mx= (mx>>1)|(mx&1);
    my= (my>>1)|(my&1);
    uvdxy= (mx&1) | ((my&1)<<1);
    mx>>=1;
    my>>=1;
    uvsrc_x = s->mb_x *  8                 + mx;
    uvsrc_y = s->mb_y * (8 >> field_based) + my;
    ptr_y  = ref_picture[0] +   src_y *   linesize +   src_x;
    ptr_cb = ref_picture[1] + uvsrc_y * uvlinesize + uvsrc_x;
    ptr_cr = ref_picture[2] + uvsrc_y * uvlinesize + uvsrc_x;
    if(   (unsigned)src_x > s->h_edge_pos - (motion_x&3) - 16 
       || (unsigned)src_y >    v_edge_pos - (motion_y&3) - h  ){
        ff_emulated_edge_mc(s->edge_emu_buffer, ptr_y, s->linesize, 17, 17+field_based, 
                         src_x, src_y<<field_based, s->h_edge_pos, s->v_edge_pos);
        ptr_y= s->edge_emu_buffer;
        if(!(s->flags&CODEC_FLAG_GRAY)){
            uint8_t *uvbuf= s->edge_emu_buffer + 18*s->linesize;
            ff_emulated_edge_mc(uvbuf, ptr_cb, s->uvlinesize, 9, 9 + field_based, 
                             uvsrc_x, uvsrc_y<<field_based, s->h_edge_pos>>1, s->v_edge_pos>>1);
            ff_emulated_edge_mc(uvbuf + 16, ptr_cr, s->uvlinesize, 9, 9 + field_based, 
                             uvsrc_x, uvsrc_y<<field_based, s->h_edge_pos>>1, s->v_edge_pos>>1);
            ptr_cb= uvbuf;
            ptr_cr= uvbuf + 16;
        }
    }
    if(!field_based)
        qpix_op[0][dxy](dest_y, ptr_y, linesize);
    else{
        if(bottom_field){
            dest_y += s->linesize;
            dest_cb+= s->uvlinesize;
            dest_cr+= s->uvlinesize;
        }
        if(field_select){
            ptr_y  += s->linesize;
            ptr_cb += s->uvlinesize;
            ptr_cr += s->uvlinesize;
        }
        //damn interlaced mode
        //FIXME boundary mirroring is not exactly correct here
        qpix_op[1][dxy](dest_y  , ptr_y  , linesize);
        qpix_op[1][dxy](dest_y+8, ptr_y+8, linesize);
    }
    if(!(s->flags&CODEC_FLAG_GRAY)){
        pix_op[1][uvdxy](dest_cr, ptr_cr, uvlinesize, h >> 1);
        pix_op[1][uvdxy](dest_cb, ptr_cb, uvlinesize, h >> 1);
    }
}
inline int ff_h263_round_chroma(int x){
    if (x >= 0)
        return  (h263_chroma_roundtab[x & 0xf] + ((x >> 3) & ~1));
    else {
        x = -x;
        return -(h263_chroma_roundtab[x & 0xf] + ((x >> 3) & ~1));
    }
}
/**
 * h263 chorma 4mv motion compensation.
 */
static inline void chroma_4mv_motion(MpegEncContext *s,
                                     uint8_t *dest_cb, uint8_t *dest_cr,
                                     uint8_t **ref_picture,
                                     op_pixels_func *pix_op,
                                     int mx, int my){
    int dxy, emu=0, src_x, src_y, offset;
    uint8_t *ptr;
    
    /* In case of 8X8, we construct a single chroma motion vector
       with a special rounding */
    mx= ff_h263_round_chroma(mx);
    my= ff_h263_round_chroma(my);
    
    dxy = ((my & 1) << 1) | (mx & 1);
    mx >>= 1;
    my >>= 1;
    src_x = s->mb_x * 8 + mx;
    src_y = s->mb_y * 8 + my;
    src_x = clip(src_x, -8, s->width/2);
    if (src_x == s->width/2)
        dxy &= ~1;
    src_y = clip(src_y, -8, s->height/2);
    if (src_y == s->height/2)
        dxy &= ~2;
    
    offset = (src_y * (s->uvlinesize)) + src_x;
    ptr = ref_picture[1] + offset;
    if(s->flags&CODEC_FLAG_EMU_EDGE){
        if(   (unsigned)src_x > (s->h_edge_pos>>1) - (dxy &1) - 8
           || (unsigned)src_y > (s->v_edge_pos>>1) - (dxy>>1) - 8){
            ff_emulated_edge_mc(s->edge_emu_buffer, ptr, s->uvlinesize, 9, 9, src_x, src_y, s->h_edge_pos>>1, s->v_edge_pos>>1);
            ptr= s->edge_emu_buffer;
            emu=1;
        }
    }
    pix_op[dxy](dest_cb, ptr, s->uvlinesize, 8);
    ptr = ref_picture[2] + offset;
    if(emu){
        ff_emulated_edge_mc(s->edge_emu_buffer, ptr, s->uvlinesize, 9, 9, src_x, src_y, s->h_edge_pos>>1, s->v_edge_pos>>1);
        ptr= s->edge_emu_buffer;
    }
    pix_op[dxy](dest_cr, ptr, s->uvlinesize, 8);
}
/**
 * motion compesation of a single macroblock
 * @param s context
 * @param dest_y luma destination pointer
 * @param dest_cb chroma cb/u destination pointer
 * @param dest_cr chroma cr/v destination pointer
 * @param dir direction (0->forward, 1->backward)
 * @param ref_picture array[3] of pointers to the 3 planes of the reference picture
 * @param pic_op halfpel motion compensation function (average or put normally)
 * @param pic_op qpel motion compensation function (average or put normally)
 * the motion vectors are taken from s->mv and the MV type from s->mv_type
 */
static inline void MPV_motion(MpegEncContext *s, 
                              uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr,
                              int dir, uint8_t **ref_picture, 
                              op_pixels_func (*pix_op)[4], qpel_mc_func (*qpix_op)[16])
{
    int dxy, mx, my, src_x, src_y, motion_x, motion_y;
    int mb_x, mb_y, i;
    uint8_t *ptr, *dest;
    mb_x = s->mb_x;
    mb_y = s->mb_y;
    if(s->obmc && s->pict_type != B_TYPE){
        int16_t mv_cache[4][4][2];
        const int xy= s->mb_x + s->mb_y*s->mb_stride;
        const int mot_stride= s->b8_stride;
        const int mot_xy= mb_x*2 + mb_y*2*mot_stride;
        assert(!s->mb_skiped);
                
        memcpy(mv_cache[1][1], s->current_picture.motion_val[0][mot_xy           ], sizeof(int16_t)*4);
        memcpy(mv_cache[2][1], s->current_picture.motion_val[0][mot_xy+mot_stride], sizeof(int16_t)*4);
        memcpy(mv_cache[3][1], s->current_picture.motion_val[0][mot_xy+mot_stride], sizeof(int16_t)*4);
        if(mb_y==0 || IS_INTRA(s->current_picture.mb_type[xy-s->mb_stride])){
            memcpy(mv_cache[0][1], mv_cache[1][1], sizeof(int16_t)*4);
        }else{
            memcpy(mv_cache[0][1], s->current_picture.motion_val[0][mot_xy-mot_stride], sizeof(int16_t)*4);
        }
        if(mb_x==0 || IS_INTRA(s->current_picture.mb_type[xy-1])){
            *(int32_t*)mv_cache[1][0]= *(int32_t*)mv_cache[1][1];
            *(int32_t*)mv_cache[2][0]= *(int32_t*)mv_cache[2][1];
        }else{
            *(int32_t*)mv_cache[1][0]= *(int32_t*)s->current_picture.motion_val[0][mot_xy-1];
            *(int32_t*)mv_cache[2][0]= *(int32_t*)s->current_picture.motion_val[0][mot_xy-1+mot_stride];
        }
        if(mb_x+1>=s->mb_width || IS_INTRA(s->current_picture.mb_type[xy+1])){
            *(int32_t*)mv_cache[1][3]= *(int32_t*)mv_cache[1][2];
            *(int32_t*)mv_cache[2][3]= *(int32_t*)mv_cache[2][2];
        }else{
            *(int32_t*)mv_cache[1][3]= *(int32_t*)s->current_picture.motion_val[0][mot_xy+2];
            *(int32_t*)mv_cache[2][3]= *(int32_t*)s->current_picture.motion_val[0][mot_xy+2+mot_stride];
        }
        
        mx = 0;
        my = 0;
        for(i=0;i<4;i++) {
            const int x= (i&1)+1;
            const int y= (i>>1)+1;
            int16_t mv[5][2]= {
                {mv_cache[y][x  ][0], mv_cache[y][x  ][1]},
                {mv_cache[y-1][x][0], mv_cache[y-1][x][1]},
                {mv_cache[y][x-1][0], mv_cache[y][x-1][1]},
                {mv_cache[y][x+1][0], mv_cache[y][x+1][1]},
                {mv_cache[y+1][x][0], mv_cache[y+1][x][1]}};
            //FIXME cleanup
            obmc_motion(s, dest_y + ((i & 1) * 8) + (i >> 1) * 8 * s->linesize,
                        ref_picture[0],
                        mb_x * 16 + (i & 1) * 8, mb_y * 16 + (i >>1) * 8,
                        pix_op[1],
                        mv);
            mx += mv[0][0];
            my += mv[0][1];
        }
        if(!(s->flags&CODEC_FLAG_GRAY))
            chroma_4mv_motion(s, dest_cb, dest_cr, ref_picture, pix_op[1], mx, my);
        return;
    }
   
    switch(s->mv_type) {
    case MV_TYPE_16X16:
#ifdef CONFIG_RISKY
        if(s->mcsel){
            if(s->real_sprite_warping_points==1){
                gmc1_motion(s, dest_y, dest_cb, dest_cr,
                            ref_picture);
            }else{
                gmc_motion(s, dest_y, dest_cb, dest_cr,
                            ref_picture);
            }
        }else if(s->quarter_sample){
            qpel_motion(s, dest_y, dest_cb, dest_cr, 
                        0, 0, 0,
                        ref_picture, pix_op, qpix_op,
                        s->mv[dir][0][0], s->mv[dir][0][1], 16);
        }else if(s->mspel){
            ff_mspel_motion(s, dest_y, dest_cb, dest_cr,
                        ref_picture, pix_op,
                        s->mv[dir][0][0], s->mv[dir][0][1], 16);
        }else
#endif
        {
            mpeg_motion(s, dest_y, dest_cb, dest_cr, 
                        0, 0, 0,
                        ref_picture, pix_op,
                        s->mv[dir][0][0], s->mv[dir][0][1], 16);
        }           
        break;
    case MV_TYPE_8X8:
        mx = 0;
        my = 0;
        if(s->quarter_sample){
            for(i=0;i<4;i++) {
                motion_x = s->mv[dir][i][0];
                motion_y = s->mv[dir][i][1];
                dxy = ((motion_y & 3) << 2) | (motion_x & 3);
                src_x = mb_x * 16 + (motion_x >> 2) + (i & 1) * 8;
                src_y = mb_y * 16 + (motion_y >> 2) + (i >>1) * 8;
                    
                /* WARNING: do no forget half pels */
                src_x = clip(src_x, -16, s->width);
                if (src_x == s->width)
                    dxy &= ~3;
                src_y = clip(src_y, -16, s->height);
                if (src_y == s->height)
                    dxy &= ~12;
                    
                ptr = ref_picture[0] + (src_y * s->linesize) + (src_x);
                if(s->flags&CODEC_FLAG_EMU_EDGE){
                    if(   (unsigned)src_x > s->h_edge_pos - (motion_x&3) - 8 
                       || (unsigned)src_y > s->v_edge_pos - (motion_y&3) - 8 ){
                        ff_emulated_edge_mc(s->edge_emu_buffer, ptr, s->linesize, 9, 9, src_x, src_y, s->h_edge_pos, s->v_edge_pos);
                        ptr= s->edge_emu_buffer;
                    }
                }
                dest = dest_y + ((i & 1) * 8) + (i >> 1) * 8 * s->linesize;
                qpix_op[1][dxy](dest, ptr, s->linesize);
                mx += s->mv[dir][i][0]/2;
                my += s->mv[dir][i][1]/2;
            }
        }else{
            for(i=0;i<4;i++) {
                hpel_motion(s, dest_y + ((i & 1) * 8) + (i >> 1) * 8 * s->linesize,
                            ref_picture[0], 0, 0,
                            mb_x * 16 + (i & 1) * 8, mb_y * 16 + (i >>1) * 8,
                            s->width, s->height, s->linesize,
                            s->h_edge_pos, s->v_edge_pos,
                            8, 8, pix_op[1],
                            s->mv[dir][i][0], s->mv[dir][i][1]);
                mx += s->mv[dir][i][0];
                my += s->mv[dir][i][1];
            }
        }
        if(!(s->flags&CODEC_FLAG_GRAY))
            chroma_4mv_motion(s, dest_cb, dest_cr, ref_picture, pix_op[1], mx, my);
        break;
    case MV_TYPE_FIELD:
        if (s->picture_structure == PICT_FRAME) {
            if(s->quarter_sample){
                for(i=0; i<2; i++){
                    qpel_motion(s, dest_y, dest_cb, dest_cr,
                                1, i, s->field_select[dir][i],
                                ref_picture, pix_op, qpix_op,
                                s->mv[dir][i][0], s->mv[dir][i][1], 8);
                }
            }else{
                /* top field */       
                mpeg_motion(s, dest_y, dest_cb, dest_cr,
                            1, 0, s->field_select[dir][0],
                            ref_picture, pix_op,
                            s->mv[dir][0][0], s->mv[dir][0][1], 8);
                /* bottom field */
                mpeg_motion(s, dest_y, dest_cb, dest_cr,
                            1, 1, s->field_select[dir][1],
                            ref_picture, pix_op,
                            s->mv[dir][1][0], s->mv[dir][1][1], 8);
            }
        } else {
            if(s->picture_structure != s->field_select[dir][0] + 1 && s->pict_type != B_TYPE && !s->first_field){
                ref_picture= s->current_picture_ptr->data;
            } 
            mpeg_motion(s, dest_y, dest_cb, dest_cr,
                        0, 0, s->field_select[dir][0],
                        ref_picture, pix_op,
                        s->mv[dir][0][0], s->mv[dir][0][1], 16);
        }
        break;
    case MV_TYPE_16X8:
        for(i=0; i<2; i++){
            uint8_t ** ref2picture;
            if(s->picture_structure == s->field_select[dir][i] + 1 || s->pict_type == B_TYPE || s->first_field){
                ref2picture= ref_picture;
            }else{
                ref2picture= s->current_picture_ptr->data;
            } 
            mpeg_motion(s, dest_y, dest_cb, dest_cr, 
                        0, 0, s->field_select[dir][i],
                        ref2picture, pix_op,
                        s->mv[dir][i][0], s->mv[dir][i][1] + 16*i, 8);
                
            dest_y += 16*s->linesize;
            dest_cb+= (16>>s->chroma_y_shift)*s->uvlinesize;
            dest_cr+= (16>>s->chroma_y_shift)*s->uvlinesize;
        }        
        break;
    case MV_TYPE_DMV:
        if(s->picture_structure == PICT_FRAME){
            for(i=0; i<2; i++){
                int j;
                for(j=0; j<2; j++){
                    mpeg_motion(s, dest_y, dest_cb, dest_cr,
                                1, j, j^i,
                                ref_picture, pix_op,
                                s->mv[dir][2*i + j][0], s->mv[dir][2*i + j][1], 8);
                }
                pix_op = s->dsp.avg_pixels_tab; 
            }
        }else{
            for(i=0; i<2; i++){
                mpeg_motion(s, dest_y, dest_cb, dest_cr, 
                            0, 0, s->picture_structure != i+1,
                            ref_picture, pix_op,
                            s->mv[dir][2*i][0],s->mv[dir][2*i][1],16);
                // after put we make avg of the same block
                pix_op=s->dsp.avg_pixels_tab; 
                //opposite parity is always in the same frame if this is second field
                if(!s->first_field){
                    ref_picture = s->current_picture_ptr->data;    
                }
            }
        }
    break;
    default: assert(0);
    }
}
/* put block[] to dest[] */
static inline void put_dct(MpegEncContext *s, 
                           DCTELEM *block, int i, uint8_t *dest, int line_size, int qscale)
{
    s->dct_unquantize_intra(s, block, i, qscale);
    s->dsp.idct_put (dest, line_size, block);
}
/* add block[] to dest[] */
static inline void add_dct(MpegEncContext *s, 
                           DCTELEM *block, int i, uint8_t *dest, int line_size)
{
    if (s->block_last_index[i] >= 0) {
        s->dsp.idct_add (dest, line_size, block);
    }
}
static inline void add_dequant_dct(MpegEncContext *s, 
                           DCTELEM *block, int i, uint8_t *dest, int line_size, int qscale)
{
    if (s->block_last_index[i] >= 0) {
        s->dct_unquantize_inter(s, block, i, qscale);
        s->dsp.idct_add (dest, line_size, block);
    }
}
/**
 * cleans dc, ac, coded_block for the current non intra MB
 */
void ff_clean_intra_table_entries(MpegEncContext *s)
{
    int wrap = s->b8_stride;
    int xy = s->block_index[0];
    
    s->dc_val[0][xy           ] = 
    s->dc_val[0][xy + 1       ] = 
    s->dc_val[0][xy     + wrap] =
    s->dc_val[0][xy + 1 + wrap] = 1024;
    /* ac pred */
    memset(s->ac_val[0][xy       ], 0, 32 * sizeof(int16_t));
    memset(s->ac_val[0][xy + wrap], 0, 32 * sizeof(int16_t));
    if (s->msmpeg4_version>=3) {
        s->coded_block[xy           ] =
        s->coded_block[xy + 1       ] =
        s->coded_block[xy     + wrap] =
        s->coded_block[xy + 1 + wrap] = 0;
    }
    /* chroma */
    wrap = s->mb_stride;
    xy = s->mb_x + s->mb_y * wrap;
    s->dc_val[1][xy] =
    s->dc_val[2][xy] = 1024;
    /* ac pred */
    memset(s->ac_val[1][xy], 0, 16 * sizeof(int16_t));
    memset(s->ac_val[2][xy], 0, 16 * sizeof(int16_t));
    
    s->mbintra_table[xy]= 0;
}
/* generic function called after a macroblock has been parsed by the
   decoder or after it has been encoded by the encoder.
   Important variables used:
   s->mb_intra : true if intra macroblock
   s->mv_dir   : motion vector direction
   s->mv_type  : motion vector type
   s->mv       : motion vector
   s->interlaced_dct : true if interlaced dct used (mpeg2)
 */
void MPV_decode_mb(MpegEncContext *s, DCTELEM block[12][64])
{
    int mb_x, mb_y;
    const int mb_xy = s->mb_y * s->mb_stride + s->mb_x;
#ifdef HAVE_XVMC
    if(s->avctx->xvmc_acceleration){
        XVMC_decode_mb(s);//xvmc uses pblocks
        return;
    }
#endif
    mb_x = s->mb_x;
    mb_y = s->mb_y;
    if(s->avctx->debug&FF_DEBUG_DCT_COEFF) {
       /* save DCT coefficients */
       int i,j;
       DCTELEM *dct = &s->current_picture.dct_coeff[mb_xy*64*6];
       for(i=0; i<6; i++)
           for(j=0; j<64; j++)
               *dct++ = block[i][s->dsp.idct_permutation[j]];
    }
    s->current_picture.qscale_table[mb_xy]= s->qscale;
    /* update DC predictors for P macroblocks */
    if (!s->mb_intra) {
        if (s->h263_pred || s->h263_aic) {
            if(s->mbintra_table[mb_xy])
                ff_clean_intra_table_entries(s);
        } else {
            s->last_dc[0] =
            s->last_dc[1] =
            s->last_dc[2] = 128 << s->intra_dc_precision;
        }
    }
    else if (s->h263_pred || s->h263_aic)
        s->mbintra_table[mb_xy]=1;
    if ((s->flags&CODEC_FLAG_PSNR) || !(s->encoding && (s->intra_only || s->pict_type==B_TYPE))) { //FIXME precalc
        uint8_t *dest_y, *dest_cb, *dest_cr;
        int dct_linesize, dct_offset;
        op_pixels_func (*op_pix)[4];
        qpel_mc_func (*op_qpix)[16];
        const int linesize= s->current_picture.linesize[0]; //not s->linesize as this woulnd be wrong for field pics
        const int uvlinesize= s->current_picture.linesize[1];
        const int readable= s->pict_type != B_TYPE || s->encoding || s->avctx->draw_horiz_band;
        /* avoid copy if macroblock skipped in last frame too */
        /* skip only during decoding as we might trash the buffers during encoding a bit */
        if(!s->encoding){
            uint8_t *mbskip_ptr = &s->mbskip_table[mb_xy];
            const int age= s->current_picture.age;
            assert(age);
            if (s->mb_skiped) {
                s->mb_skiped= 0;
                assert(s->pict_type!=I_TYPE);
 
                (*mbskip_ptr) ++; /* indicate that this time we skiped it */
                if(*mbskip_ptr >99) *mbskip_ptr= 99;
                /* if previous was skipped too, then nothing to do !  */
                if (*mbskip_ptr >= age && s->current_picture.reference){
                    return;
                }
            } else if(!s->current_picture.reference){
                (*mbskip_ptr) ++; /* increase counter so the age can be compared cleanly */
                if(*mbskip_ptr >99) *mbskip_ptr= 99;
            } else{
                *mbskip_ptr = 0; /* not skipped */
            }
        }
        dct_linesize = linesize << s->interlaced_dct;
        dct_offset =(s->interlaced_dct)? linesize : linesize*8;
        if(readable){
            dest_y=  s->dest[0];
            dest_cb= s->dest[1];
            dest_cr= s->dest[2];
        }else{
            dest_y = s->b_scratchpad;
            dest_cb= s->b_scratchpad+16*linesize;
            dest_cr= s->b_scratchpad+32*linesize;
        }
        if (!s->mb_intra) {
            /* motion handling */
            /* decoding or more than one mb_type (MC was allready done otherwise) */
            if(!s->encoding){
                if ((!s->no_rounding) || s->pict_type==B_TYPE){                
		    op_pix = s->dsp.put_pixels_tab;
                    op_qpix= s->dsp.put_qpel_pixels_tab;
                }else{
                    op_pix = s->dsp.put_no_rnd_pixels_tab;
                    op_qpix= s->dsp.put_no_rnd_qpel_pixels_tab;
                }
                if (s->mv_dir & MV_DIR_FORWARD) {
                    MPV_motion(s, dest_y, dest_cb, dest_cr, 0, s->last_picture.data, op_pix, op_qpix);
		    op_pix = s->dsp.avg_pixels_tab;
                    op_qpix= s->dsp.avg_qpel_pixels_tab;
                }
                if (s->mv_dir & MV_DIR_BACKWARD) {
                    MPV_motion(s, dest_y, dest_cb, dest_cr, 1, s->next_picture.data, op_pix, op_qpix);
                }
            }
            /* skip dequant / idct if we are really late ;) */
            if(s->hurry_up>1) return;
            /* add dct residue */
            if(s->encoding || !(   s->h263_msmpeg4 || s->codec_id==CODEC_ID_MPEG1VIDEO || s->codec_id==CODEC_ID_MPEG2VIDEO
                                || (s->codec_id==CODEC_ID_MPEG4 && !s->mpeg_quant))){
                add_dequant_dct(s, block[0], 0, dest_y, dct_linesize, s->qscale);
                add_dequant_dct(s, block[1], 1, dest_y + 8, dct_linesize, s->qscale);
                add_dequant_dct(s, block[2], 2, dest_y + dct_offset, dct_linesize, s->qscale);
                add_dequant_dct(s, block[3], 3, dest_y + dct_offset + 8, dct_linesize, s->qscale);
                if(!(s->flags&CODEC_FLAG_GRAY)){
                    add_dequant_dct(s, block[4], 4, dest_cb, uvlinesize, s->chroma_qscale);
                    add_dequant_dct(s, block[5], 5, dest_cr, uvlinesize, s->chroma_qscale);
                }
            } else if(s->codec_id != CODEC_ID_WMV2){
                add_dct(s, block[0], 0, dest_y, dct_linesize);
                add_dct(s, block[1], 1, dest_y + 8, dct_linesize);
                add_dct(s, block[2], 2, dest_y + dct_offset, dct_linesize);
                add_dct(s, block[3], 3, dest_y + dct_offset + 8, dct_linesize);
                if(!(s->flags&CODEC_FLAG_GRAY)){
                    if(s->chroma_y_shift){//Chroma420
                        add_dct(s, block[4], 4, dest_cb, uvlinesize);
                        add_dct(s, block[5], 5, dest_cr, uvlinesize);
                    }else{
                        //chroma422
                        dct_linesize = uvlinesize << s->interlaced_dct;
                        dct_offset =(s->interlaced_dct)? uvlinesize : uvlinesize*8;
                        add_dct(s, block[4], 4, dest_cb, dct_linesize);
                        add_dct(s, block[5], 5, dest_cr, dct_linesize);
                        add_dct(s, block[6], 6, dest_cb+dct_offset, dct_linesize);
                        add_dct(s, block[7], 7, dest_cr+dct_offset, dct_linesize);
                        if(!s->chroma_x_shift){//Chroma444
                            add_dct(s, block[8], 8, dest_cb+8, dct_linesize);
                            add_dct(s, block[9], 9, dest_cr+8, dct_linesize);
                            add_dct(s, block[10], 10, dest_cb+8+dct_offset, dct_linesize);
                            add_dct(s, block[11], 11, dest_cr+8+dct_offset, dct_linesize);
                        }
                    }
                }//fi gray
            }
#ifdef CONFIG_RISKY
            else{
                ff_wmv2_add_mb(s, block, dest_y, dest_cb, dest_cr);
            }
#endif
        } else {
            /* dct only in intra block */
            if(s->encoding || !(s->codec_id==CODEC_ID_MPEG1VIDEO || s->codec_id==CODEC_ID_MPEG2VIDEO)){
                put_dct(s, block[0], 0, dest_y, dct_linesize, s->qscale);
                put_dct(s, block[1], 1, dest_y + 8, dct_linesize, s->qscale);
                put_dct(s, block[2], 2, dest_y + dct_offset, dct_linesize, s->qscale);
                put_dct(s, block[3], 3, dest_y + dct_offset + 8, dct_linesize, s->qscale);
                if(!(s->flags&CODEC_FLAG_GRAY)){
                    put_dct(s, block[4], 4, dest_cb, uvlinesize, s->chroma_qscale);
                    put_dct(s, block[5], 5, dest_cr, uvlinesize, s->chroma_qscale);
                }
            }else{
                s->dsp.idct_put(dest_y                 , dct_linesize, block[0]);
                s->dsp.idct_put(dest_y              + 8, dct_linesize, block[1]);
                s->dsp.idct_put(dest_y + dct_offset    , dct_linesize, block[2]);
                s->dsp.idct_put(dest_y + dct_offset + 8, dct_linesize, block[3]);
                if(!(s->flags&CODEC_FLAG_GRAY)){
                    if(s->chroma_y_shift){
                        s->dsp.idct_put(dest_cb, uvlinesize, block[4]);
                        s->dsp.idct_put(dest_cr, uvlinesize, block[5]);
                    }else{
                        dct_linesize = uvlinesize << s->interlaced_dct;
                        dct_offset =(s->interlaced_dct)? uvlinesize : uvlinesize*8;
                        s->dsp.idct_put(dest_cb,              dct_linesize, block[4]);
                        s->dsp.idct_put(dest_cr,              dct_linesize, block[5]);
                        s->dsp.idct_put(dest_cb + dct_offset, dct_linesize, block[6]);
                        s->dsp.idct_put(dest_cr + dct_offset, dct_linesize, block[7]);
                        if(!s->chroma_x_shift){//Chroma444
                            s->dsp.idct_put(dest_cb + 8,              dct_linesize, block[8]);
                            s->dsp.idct_put(dest_cr + 8,              dct_linesize, block[9]);
                            s->dsp.idct_put(dest_cb + 8 + dct_offset, dct_linesize, block[10]);
                            s->dsp.idct_put(dest_cr + 8 + dct_offset, dct_linesize, block[11]);
                        }
                    }
                }//gray
            }
        }
        if(!readable){
            s->dsp.put_pixels_tab[0][0](s->dest[0], dest_y ,   linesize,16);
            s->dsp.put_pixels_tab[s->chroma_x_shift][0](s->dest[1], dest_cb, uvlinesize,16 >> s->chroma_y_shift);
            s->dsp.put_pixels_tab[s->chroma_x_shift][0](s->dest[2], dest_cr, uvlinesize,16 >> s->chroma_y_shift);
        }
    }
}

/**
 *
 * @param h is the normal height, this will be reduced automatically if needed for the last row
 */
void ff_draw_horiz_band(MpegEncContext *s, int y, int h){
    if (s->avctx->draw_horiz_band) {
        AVFrame *src;
        int offset[4];
        
        if(s->picture_structure != PICT_FRAME){
            h <<= 1;
            y <<= 1;
            if(s->first_field  && !(s->avctx->slice_flags&SLICE_FLAG_ALLOW_FIELD)) return;
        }
        h= FFMIN(h, s->height - y);
        if(s->pict_type==B_TYPE || s->low_delay || (s->avctx->slice_flags&SLICE_FLAG_CODED_ORDER)) 
            src= (AVFrame*)s->current_picture_ptr;
        else if(s->last_picture_ptr)
            src= (AVFrame*)s->last_picture_ptr;
        else
            return;
            
        if(s->pict_type==B_TYPE && s->picture_structure == PICT_FRAME && s->out_format != FMT_H264){
            offset[0]=
            offset[1]=
            offset[2]=
            offset[3]= 0;
        }else{
            offset[0]= y * s->linesize;;
            offset[1]= 
            offset[2]= (y >> s->chroma_y_shift) * s->uvlinesize;
            offset[3]= 0;
        }
        emms_c();
        s->avctx->draw_horiz_band(s->avctx, src, offset,
                                  y, s->picture_structure, h);
    }
}



static void dct_unquantize_mpeg1_intra_c(MpegEncContext *s, 
                                   DCTELEM *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;
    nCoeffs= s->block_last_index[n];
    
    if (n < 4) 
        block[0] = block[0] * s->y_dc_scale;
    else
        block[0] = block[0] * s->c_dc_scale;
    /* XXX: only mpeg1 */
    quant_matrix = s->intra_matrix;
    for(i=1;i<=nCoeffs;i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (int)(level * qscale * quant_matrix[j]) >> 3;
                level = (level - 1) | 1;
                level = -level;
            } else {
                level = (int)(level * qscale * quant_matrix[j]) >> 3;
                level = (level - 1) | 1;
            }
            block[j] = level;
        }
    }
}
static void dct_unquantize_mpeg1_inter_c(MpegEncContext *s, 
                                   DCTELEM *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;
    nCoeffs= s->block_last_index[n];
    
    quant_matrix = s->inter_matrix;
    for(i=0; i<=nCoeffs; i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 4;
                level = (level - 1) | 1;
                level = -level;
            } else {
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 4;
                level = (level - 1) | 1;
            }
            block[j] = level;
        }
    }
}
static void dct_unquantize_mpeg2_intra_c(MpegEncContext *s, 
                                   DCTELEM *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;
    if(s->alternate_scan) nCoeffs= 63;
    else nCoeffs= s->block_last_index[n];
    
    if (n < 4) 
        block[0] = block[0] * s->y_dc_scale;
    else
        block[0] = block[0] * s->c_dc_scale;
    quant_matrix = s->intra_matrix;
    for(i=1;i<=nCoeffs;i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (int)(level * qscale * quant_matrix[j]) >> 3;
                level = -level;
            } else {
                level = (int)(level * qscale * quant_matrix[j]) >> 3;
            }
            block[j] = level;
        }
    }
}
static void dct_unquantize_mpeg2_inter_c(MpegEncContext *s, 
                                   DCTELEM *block, int n, int qscale)
{
    int i, level, nCoeffs;
    const uint16_t *quant_matrix;
    int sum=-1;
    if(s->alternate_scan) nCoeffs= 63;
    else nCoeffs= s->block_last_index[n];
    
    quant_matrix = s->inter_matrix;
    for(i=0; i<=nCoeffs; i++) {
        int j= s->intra_scantable.permutated[i];
        level = block[j];
        if (level) {
            if (level < 0) {
                level = -level;
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 4;
                level = -level;
            } else {
                level = (((level << 1) + 1) * qscale *
                         ((int) (quant_matrix[j]))) >> 4;
            }
            block[j] = level;
            sum+=level;
        }
    }
    block[63]^=sum&1;
}
static void dct_unquantize_h263_intra_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale)
{
    int i, level, qmul, qadd;
    int nCoeffs;
    
    assert(s->block_last_index[n]>=0);
    
    qmul = qscale << 1;
    
    if (!s->h263_aic) {
        if (n < 4) 
            block[0] = block[0] * s->y_dc_scale;
        else
            block[0] = block[0] * s->c_dc_scale;
        qadd = (qscale - 1) | 1;
    }else{
        qadd = 0;
    }
    if(s->ac_pred)
        nCoeffs=63;
    else
        nCoeffs= s->inter_scantable.raster_end[ s->block_last_index[n] ];
    for(i=1; i<=nCoeffs; i++) {
        level = block[i];
        if (level) {
            if (level < 0) {
                level = level * qmul - qadd;
            } else {
                level = level * qmul + qadd;
            }
            block[i] = level;
        }
    }
}
static void dct_unquantize_h263_inter_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale)
{
    int i, level, qmul, qadd;
    int nCoeffs;
    
    assert(s->block_last_index[n]>=0);
    
    qadd = (qscale - 1) | 1;
    qmul = qscale << 1;
    
    nCoeffs= s->inter_scantable.raster_end[ s->block_last_index[n] ];
    for(i=0; i<=nCoeffs; i++) {
        level = block[i];
        if (level) {
            if (level < 0) {
                level = level * qmul - qadd;
            } else {
                level = level * qmul + qadd;
            }
            block[i] = level;
        }
    }
}
static void dct_unquantize_h261_intra_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale)
{
    int i, level, even;
    int nCoeffs;
    
    assert(s->block_last_index[n]>=0);
    
    if (n < 4) 
        block[0] = block[0] * s->y_dc_scale;
    else
        block[0] = block[0] * s->c_dc_scale;
    even = (qscale & 1)^1;
    nCoeffs= s->inter_scantable.raster_end[ s->block_last_index[n] ];
    for(i=1; i<=nCoeffs; i++){
        level = block[i];
        if (level){
            if (level < 0){
                level = qscale * ((level << 1) - 1) + even;
            }else{
                level = qscale * ((level << 1) + 1) - even;
            }
        }
        block[i] = level;
    }
}
static void dct_unquantize_h261_inter_c(MpegEncContext *s, 
                                  DCTELEM *block, int n, int qscale)
{
    int i, level, even;
    int nCoeffs;
    
    assert(s->block_last_index[n]>=0);
    even = (qscale & 1)^1;
    
    nCoeffs= s->inter_scantable.raster_end[ s->block_last_index[n] ];
    for(i=0; i<=nCoeffs; i++){
        level = block[i];
        if (level){
            if (level < 0){
                level = qscale * ((level << 1) - 1) + even;
            }else{
                level = qscale * ((level << 1) + 1) - even;
            }
        }
        block[i] = level;
    }
}

