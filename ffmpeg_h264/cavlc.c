
#include <assert.h>
#include <string.h>
//#include "pred_mb.h"
#include "golomb.h"
#include "fill_func.h"
#include "cavlc.h"
#include "h264data.h"

 VLC coeff_token_vlc[4];
 VLC chroma_dc_coeff_token_vlc;
 VLC total_zeros_vlc[15];
 VLC chroma_dc_total_zeros_vlc[3];
 VLC run_vlc[6];
 VLC run7_vlc;


void decode_init_vlc(H264Context *h){
    static int done = 0;
    if (!done) {
        int i;
        done = 1;
        init_vlc(&chroma_dc_coeff_token_vlc, CHROMA_DC_COEFF_TOKEN_VLC_BITS, 4*5, 
                 &chroma_dc_coeff_token_len [0], 1, 1,
                 &chroma_dc_coeff_token_bits[0], 1, 1);
        for(i=0; i<4; i++){
            init_vlc(&coeff_token_vlc[i], COEFF_TOKEN_VLC_BITS, 4*17, 
                     &coeff_token_len [i][0], 1, 1,
                     &coeff_token_bits[i][0], 1, 1);
        }
        for(i=0; i<3; i++){
            init_vlc(&chroma_dc_total_zeros_vlc[i], CHROMA_DC_TOTAL_ZEROS_VLC_BITS, 4,
                     &chroma_dc_total_zeros_len [i][0], 1, 1,
                     &chroma_dc_total_zeros_bits[i][0], 1, 1);
        }
        for(i=0; i<15; i++){
            init_vlc(&total_zeros_vlc[i], TOTAL_ZEROS_VLC_BITS, 16, 
                     &total_zeros_len [i][0], 1, 1,
                     &total_zeros_bits[i][0], 1, 1);
        }
        for(i=0; i<6; i++){
            init_vlc(&run_vlc[i], RUN_VLC_BITS, 7, 
                     &run_len [i][0], 1, 1,
                     &run_bits[i][0], 1, 1);
        }
        init_vlc(&run7_vlc, RUN7_VLC_BITS, 16, 
                 &run_len [6][0], 1, 1,
                 &run_bits[6][0], 1, 1);
    }
}

/**
 *
 */
int get_level_prefix(GetBitContext *gb){
    unsigned int buf;
    int log;
    
    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf=GET_CACHE(re, gb);
    
    log= 32 - av_log2(buf);
#ifdef TRACE
    print_bin(buf>>(32-log), log);
    printf("%5d %2d %3d lpr @%5d in %s get_level_prefix\n", buf>>(32-log), log, log-1, get_bits_count(gb), __FILE__);
#endif
    LAST_SKIP_BITS(re, gb, log);
    CLOSE_READER(re, gb);
    return log-1;
}

/**
 * decodes a residual block.
 * @param n block index
 * @param scantable scantable
 * @param max_coeff number of coefficients in the block
 * @return <0 if an error occured
 */
static int decode_residual(H264Context *h, GetBitContext *gb, DCTELEM *block, int n, const uint8_t *scantable, int qp, int max_coeff){
    MpegEncContext * const s = &h->s;
    const uint16_t *qmul= dequant_coeff[qp];
    static const int coeff_token_table_index[17]= {0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3};
    int level[16], run[16];
    int suffix_length, zeros_left, coeff_num, coeff_token, total_coeff, i, trailing_ones;
    //FIXME put trailing_onex into the context
    if(n == CHROMA_DC_BLOCK_INDEX){
        coeff_token= get_vlc2(gb, chroma_dc_coeff_token_vlc.table, CHROMA_DC_COEFF_TOKEN_VLC_BITS, 1);
        total_coeff= coeff_token>>2;
    }else{    
        if(n == LUMA_DC_BLOCK_INDEX){
            total_coeff= pred_non_zero_count(h, 0);
            coeff_token= get_vlc2(gb, coeff_token_vlc[ coeff_token_table_index[total_coeff] ].table, COEFF_TOKEN_VLC_BITS, 2);
            total_coeff= coeff_token>>2;
        }else{
            total_coeff= pred_non_zero_count(h, n);
            coeff_token= get_vlc2(gb, coeff_token_vlc[ coeff_token_table_index[total_coeff] ].table, COEFF_TOKEN_VLC_BITS, 2);
            total_coeff= coeff_token>>2;
            h->non_zero_count_cache[ scan8[n] ]= total_coeff;
        }
    }
    //FIXME set last_non_zero?
    if(total_coeff==0)
        return 0;
        
    trailing_ones= coeff_token&3;
    tprintf("trailing:%d, total:%d\n", trailing_ones, total_coeff);
    assert(total_coeff<=16);
    
    for(i=0; i<trailing_ones; i++){
        level[i]= 1 - 2*get_bits1(gb);
    }
    suffix_length= total_coeff > 10 && trailing_ones < 3;
    for(; i<total_coeff; i++){
        const int prefix= get_level_prefix(gb);
        int level_code, mask;
        if(prefix<14){ //FIXME try to build a large unified VLC table for all this
            if(suffix_length)
                level_code= (prefix<<suffix_length) + get_bits(gb, suffix_length); //part
            else
                level_code= (prefix<<suffix_length); //part
        }else if(prefix==14){
            if(suffix_length)
                level_code= (prefix<<suffix_length) + get_bits(gb, suffix_length); //part
            else
                level_code= prefix + get_bits(gb, 4); //part
        }else if(prefix==15){
            level_code= (prefix<<suffix_length) + get_bits(gb, 12); //part
            if(suffix_length==0) level_code+=15; //FIXME doesnt make (much)sense
        }else{
            av_log(h->s.avctx, AV_LOG_ERROR, "prefix too large at %d %d\n", s->mb_x, s->mb_y);
            return -1;
        }
        if(i==trailing_ones && i<3) level_code+= 2; //FIXME split first iteration
        mask= -(level_code&1);
        level[i]= (((2+level_code)>>1) ^ mask) - mask;
        if(suffix_length==0) suffix_length=1; //FIXME split first iteration
#if 1
        if(ABS(level[i]) > (3<<(suffix_length-1)) && suffix_length<6) suffix_length++;
#else        
        if((2+level_code)>>1) > (3<<(suffix_length-1)) && suffix_length<6) suffix_length++;
        /* ? == prefix > 2 or sth */
#endif
        tprintf("level: %d suffix_length:%d\n", level[i], suffix_length);
    }
    if(total_coeff == max_coeff)
        zeros_left=0;
    else{
        if(n == CHROMA_DC_BLOCK_INDEX)
            zeros_left= get_vlc2(gb, chroma_dc_total_zeros_vlc[ total_coeff-1 ].table, CHROMA_DC_TOTAL_ZEROS_VLC_BITS, 1);
        else
            zeros_left= get_vlc2(gb, total_zeros_vlc[ total_coeff-1 ].table, TOTAL_ZEROS_VLC_BITS, 1);
    }
    
    for(i=0; i<total_coeff-1; i++){
        if(zeros_left <=0)
            break;
        else if(zeros_left < 7){
            run[i]= get_vlc2(gb, run_vlc[zeros_left-1].table, RUN_VLC_BITS, 1);
        }else{
            run[i]= get_vlc2(gb, run7_vlc.table, RUN7_VLC_BITS, 2);
        }
        zeros_left -= run[i];
    }
    if(zeros_left<0){
        av_log(h->s.avctx, AV_LOG_ERROR, "negative number of zero coeffs at %d %d\n", s->mb_x, s->mb_y);
        return -1;
    }
    
    for(; i<total_coeff-1; i++){
        run[i]= 0;
    }
    run[i]= zeros_left;
    coeff_num=-1;
    if(n > 24){
        for(i=total_coeff-1; i>=0; i--){ //FIXME merge into rundecode?
            int j;
            coeff_num += run[i] + 1; //FIXME add 1 earlier ?
            j= scantable[ coeff_num ];
            block[j]= level[i];
        }
    }else{
        for(i=total_coeff-1; i>=0; i--){ //FIXME merge into  rundecode?
            int j;
            coeff_num += run[i] + 1; //FIXME add 1 earlier ?
            j= scantable[ coeff_num ];
            block[j]= level[i] * qmul[j];
//            printf("%d %d  ", block[j], qmul[j]);
        }
    }
    return 0;
}


/**
 * decodes a macroblock
 * @returns 0 if ok, AC_ERROR / DC_ERROR / MV_ERROR if an error is noticed
 */
int decode_mb_cavlc(H264Context *h){
    MpegEncContext * const s = &h->s;
    const int mb_xy= s->mb_x + s->mb_y*s->mb_stride;
    int mb_type, partition_count, cbp;
    s->dsp.clear_blocks(h->mb); //FIXME avoid if allready clear (move after skip handlong?    
    tprintf("pic:%d mb:%d/%d\n", h->frame_num, s->mb_x, s->mb_y);
    cbp = 0; /* avoid warning. FIXME: find a solution without slowing
                down the code */
    if(h->slice_type != I_TYPE && h->slice_type != SI_TYPE){
        if(s->mb_skip_run==-1)
            s->mb_skip_run= get_ue_golomb(&s->gb);
        
        if (s->mb_skip_run--) {
            int mx, my;
            /* skip mb */
//FIXME b frame
            mb_type= MB_TYPE_16x16|MB_TYPE_P0L0|MB_TYPE_P1L0;
            memset(h->non_zero_count[mb_xy], 0, 16);
            memset(h->non_zero_count_cache + 8, 0, 8*5); //FIXME ugly, remove pfui
            if(h->sps.mb_aff && s->mb_skip_run==0 && (s->mb_y&1)==0){
                h->mb_field_decoding_flag= get_bits1(&s->gb);
            }
            if(h->mb_field_decoding_flag)
                mb_type|= MB_TYPE_INTERLACED;
            
            fill_caches(h, mb_type); //FIXME check what is needed and what not ...
            pred_pskip_motion(h, &mx, &my);
            fill_rectangle(&h->ref_cache[0][scan8[0]], 4, 4, 8, 0, 1);
            fill_rectangle(  h->mv_cache[0][scan8[0]], 4, 4, 8, pack16to32(mx,my), 4);
            write_back_motion(h, mb_type);
            s->current_picture.mb_type[mb_xy]= mb_type; //FIXME SKIP type
            s->current_picture.qscale_table[mb_xy]= s->qscale;
            h->slice_table[ mb_xy ]= h->slice_num;
            h->prev_mb_skiped= 1;
            return 0;
        }
    }
    if(h->sps.mb_aff /* && !field pic FIXME needed? */){
        if((s->mb_y&1)==0)
            h->mb_field_decoding_flag = get_bits1(&s->gb);
    }else
        h->mb_field_decoding_flag=0; //FIXME som ed note ?!
    
    h->prev_mb_skiped= 0;
    
    mb_type= get_ue_golomb(&s->gb);
    if(h->slice_type == B_TYPE){
        if(mb_type < 23){
            partition_count= b_mb_type_info[mb_type].partition_count;
            mb_type=         b_mb_type_info[mb_type].type;
        }else{
            mb_type -= 23;
            goto decode_intra_mb;
        }
    }else if(h->slice_type == P_TYPE /*|| h->slice_type == SP_TYPE */){
        if(mb_type < 5){
            partition_count= p_mb_type_info[mb_type].partition_count;
            mb_type=         p_mb_type_info[mb_type].type;
        }else{
            mb_type -= 5;
            goto decode_intra_mb;
        }
    }else{
       assert(h->slice_type == I_TYPE);
decode_intra_mb:
        if(mb_type > 25){
            av_log(h->s.avctx, AV_LOG_ERROR, "mb_type %d in %c slice to large at %d %d\n", mb_type, av_get_pict_type_char(h->slice_type), s->mb_x, s->mb_y);
            return -1;
        }
        partition_count=0;
        cbp= i_mb_type_info[mb_type].cbp;
        h->intra16x16_pred_mode= i_mb_type_info[mb_type].pred_mode;
        mb_type= i_mb_type_info[mb_type].type;
    }
    if(h->mb_field_decoding_flag)
        mb_type |= MB_TYPE_INTERLACED;
    s->current_picture.mb_type[mb_xy]= mb_type;
    h->slice_table[ mb_xy ]= h->slice_num;
    
    if(IS_INTRA_PCM(mb_type)){
        const uint8_t *ptr;
        int x, y;
        
        // we assume these blocks are very rare so we dont optimize it
        align_get_bits(&s->gb);
        
        ptr= s->gb.buffer + get_bits_count(&s->gb);
    
        for(y=0; y<16; y++){
            const int index= 4*(y&3) + 64*(y>>2);
            for(x=0; x<16; x++){
                h->mb[index + (x&3) + 16*(x>>2)]= *(ptr++);
            }
        }
        for(y=0; y<8; y++){
            const int index= 256 + 4*(y&3) + 32*(y>>2);
            for(x=0; x<8; x++){
                h->mb[index + (x&3) + 16*(x>>2)]= *(ptr++);
            }
        }
        for(y=0; y<8; y++){
            const int index= 256 + 64 + 4*(y&3) + 32*(y>>2);
            for(x=0; x<8; x++){
                h->mb[index + (x&3) + 16*(x>>2)]= *(ptr++);
            }
        }
    
        skip_bits(&s->gb, 384); //FIXME check /fix the bitstream readers
        
        //FIXME deblock filter, non_zero_count_cache init ...
        memset(h->non_zero_count[mb_xy], 16, 16);
        s->current_picture.qscale_table[mb_xy]= s->qscale;
        
        return 0;
    }
        
    fill_caches(h, mb_type);
    //mb_pred
    if(IS_INTRA(mb_type)){
            if(IS_INTRA4x4(mb_type)){
                int i;
                for(i=0; i<16; i++){
                    const int mode_coded= !get_bits1(&s->gb);
                    const int predicted_mode=  pred_intra_mode(h, i);
                    int mode;
                    if(mode_coded){
                        const int rem_mode= get_bits(&s->gb, 3);
                        if(rem_mode<predicted_mode)
                            mode= rem_mode;
                        else
                            mode= rem_mode + 1;
                    }else{
                        mode= predicted_mode;
                    }
                    
                    h->intra4x4_pred_mode_cache[ scan8[i] ] = mode;
                }
                write_back_intra_pred_mode(h);
                if( check_intra4x4_pred_mode(h) < 0)
                    return -1;
            }else{
                h->intra16x16_pred_mode= check_intra_pred_mode(h, h->intra16x16_pred_mode);
                if(h->intra16x16_pred_mode < 0)
                    return -1;
            }
            h->chroma_pred_mode= get_ue_golomb(&s->gb);
            h->chroma_pred_mode= check_intra_pred_mode(h, h->chroma_pred_mode);
            if(h->chroma_pred_mode < 0)
                return -1;
    }else if(partition_count==4){
        int i, j, sub_partition_count[4], list, ref[2][4];
        
        if(h->slice_type == B_TYPE){
            for(i=0; i<4; i++){
                h->sub_mb_type[i]= get_ue_golomb(&s->gb);
                if(h->sub_mb_type[i] >=13){
                    av_log(h->s.avctx, AV_LOG_ERROR, "B sub_mb_type %d out of range at %d %d\n", h->sub_mb_type[i], s->mb_x, s->mb_y);
                    return -1;
                }
                sub_partition_count[i]= b_sub_mb_type_info[ h->sub_mb_type[i] ].partition_count;
                h->sub_mb_type[i]=      b_sub_mb_type_info[ h->sub_mb_type[i] ].type;
            }
        }else{
            assert(h->slice_type == P_TYPE || h->slice_type == SP_TYPE); //FIXME SP correct ?
            for(i=0; i<4; i++){
                h->sub_mb_type[i]= get_ue_golomb(&s->gb);
                if(h->sub_mb_type[i] >=4){
                    av_log(h->s.avctx, AV_LOG_ERROR, "P sub_mb_type %d out of range at %d %d\n", h->sub_mb_type[i], s->mb_x, s->mb_y);
                    return -1;
                }
                sub_partition_count[i]= p_sub_mb_type_info[ h->sub_mb_type[i] ].partition_count;
                h->sub_mb_type[i]=      p_sub_mb_type_info[ h->sub_mb_type[i] ].type;
            }
        }
        
        for(list=0; list<2; list++){
            const int ref_count= IS_REF0(mb_type) ? 1 : h->ref_count[list];
            if(ref_count == 0) continue;
            for(i=0; i<4; i++){
                if(IS_DIR(h->sub_mb_type[i], 0, list) && !IS_DIRECT(h->sub_mb_type[i])){
                    ref[list][i] = get_te0_golomb(&s->gb, ref_count); //FIXME init to 0 before and skip?
                }else{
                 //FIXME
                    ref[list][i] = -1;
                }
            }
        }
        
        for(list=0; list<2; list++){
            const int ref_count= IS_REF0(mb_type) ? 1 : h->ref_count[list];
            if(ref_count == 0) continue;
            for(i=0; i<4; i++){
                h->ref_cache[list][ scan8[4*i]   ]=h->ref_cache[list][ scan8[4*i]+1 ]=
                h->ref_cache[list][ scan8[4*i]+8 ]=h->ref_cache[list][ scan8[4*i]+9 ]= ref[list][i];
                if(IS_DIR(h->sub_mb_type[i], 0, list) && !IS_DIRECT(h->sub_mb_type[i])){
                    const int sub_mb_type= h->sub_mb_type[i];
                    const int block_width= (sub_mb_type & (MB_TYPE_16x16|MB_TYPE_16x8)) ? 2 : 1;
                    for(j=0; j<sub_partition_count[i]; j++){
                        int mx, my;
                        const int index= 4*i + block_width*j;
                        int16_t (* mv_cache)[2]= &h->mv_cache[list][ scan8[index] ];
                        pred_motion(h, index, block_width, list, h->ref_cache[list][ scan8[index] ], &mx, &my);
                        mx += get_se_golomb(&s->gb);
                        my += get_se_golomb(&s->gb);
                        tprintf("final mv:%d %d\n", mx, my);
                        if(IS_SUB_8X8(sub_mb_type)){
                            mv_cache[ 0 ][0]= mv_cache[ 1 ][0]= 
                            mv_cache[ 8 ][0]= mv_cache[ 9 ][0]= mx;
                            mv_cache[ 0 ][1]= mv_cache[ 1 ][1]= 
                            mv_cache[ 8 ][1]= mv_cache[ 9 ][1]= my;
                        }else if(IS_SUB_8X4(sub_mb_type)){
                            mv_cache[ 0 ][0]= mv_cache[ 1 ][0]= mx;
                            mv_cache[ 0 ][1]= mv_cache[ 1 ][1]= my;
                        }else if(IS_SUB_4X8(sub_mb_type)){
                            mv_cache[ 0 ][0]= mv_cache[ 8 ][0]= mx;
                            mv_cache[ 0 ][1]= mv_cache[ 8 ][1]= my;
                        }else{
                            assert(IS_SUB_4X4(sub_mb_type));
                            mv_cache[ 0 ][0]= mx;
                            mv_cache[ 0 ][1]= my;
                        }
                    }
                }else{
                    uint32_t *p= (uint32_t *)&h->mv_cache[list][ scan8[4*i] ][0];
                    p[0] = p[1]=
                    p[8] = p[9]= 0;
                }
            }
        }
    }else if(!IS_DIRECT(mb_type)){
        int list, mx, my, i;
         //FIXME we should set ref_idx_l? to 0 if we use that later ...
        if(IS_16X16(mb_type)){
            for(list=0; list<2; list++){
                if(h->ref_count[0]>0){
                    if(IS_DIR(mb_type, 0, list)){
                        const int val= get_te0_golomb(&s->gb, h->ref_count[list]);
                        fill_rectangle(&h->ref_cache[list][ scan8[0] ], 4, 4, 8, val, 1);
                    }
                }
            }
            for(list=0; list<2; list++){
                if(IS_DIR(mb_type, 0, list)){
                    pred_motion(h, 0, 4, list, h->ref_cache[list][ scan8[0] ], &mx, &my);
                    mx += get_se_golomb(&s->gb);
                    my += get_se_golomb(&s->gb);
                    tprintf("final mv:%d %d\n", mx, my);
                    fill_rectangle(h->mv_cache[list][ scan8[0] ], 4, 4, 8, pack16to32(mx,my), 4);
                }
            }
        }
        else if(IS_16X8(mb_type)){
            for(list=0; list<2; list++){
                if(h->ref_count[list]>0){
                    for(i=0; i<2; i++){
                        if(IS_DIR(mb_type, i, list)){
                            const int val= get_te0_golomb(&s->gb, h->ref_count[list]);
                            fill_rectangle(&h->ref_cache[list][ scan8[0] + 16*i ], 4, 2, 8, val, 1);
                        }
                    }
                }
            }
            for(list=0; list<2; list++){
                for(i=0; i<2; i++){
                    if(IS_DIR(mb_type, i, list)){
                        pred_16x8_motion(h, 8*i, list, h->ref_cache[list][scan8[0] + 16*i], &mx, &my);
                        mx += get_se_golomb(&s->gb);
                        my += get_se_golomb(&s->gb);
                        tprintf("final mv:%d %d\n", mx, my);
                        fill_rectangle(h->mv_cache[list][ scan8[0] + 16*i ], 4, 2, 8, pack16to32(mx,my), 4);
                    }
                }
            }
        }else{
            assert(IS_8X16(mb_type));
            for(list=0; list<2; list++){
                if(h->ref_count[list]>0){
                    for(i=0; i<2; i++){
                        if(IS_DIR(mb_type, i, list)){ //FIXME optimize
                            const int val= get_te0_golomb(&s->gb, h->ref_count[list]);
                            fill_rectangle(&h->ref_cache[list][ scan8[0] + 2*i ], 2, 4, 8, val, 1);
                        }
                    }
                }
            }
            for(list=0; list<2; list++){
                for(i=0; i<2; i++){
                    if(IS_DIR(mb_type, i, list)){
                        pred_8x16_motion(h, i*4, list, h->ref_cache[list][ scan8[0] + 2*i ], &mx, &my);
                        mx += get_se_golomb(&s->gb);
                        my += get_se_golomb(&s->gb);
                        tprintf("final mv:%d %d\n", mx, my);
                        fill_rectangle(h->mv_cache[list][ scan8[0] + 2*i ], 2, 4, 8, pack16to32(mx,my), 4);
                    }
                }
            }
        }
    }
    
    if(IS_INTER(mb_type))
        write_back_motion(h, mb_type);
    
    if(!IS_INTRA16x16(mb_type)){
        cbp= get_ue_golomb(&s->gb);
        if(cbp > 47){
            av_log(h->s.avctx, AV_LOG_ERROR, "cbp too large (%d) at %d %d\n", cbp, s->mb_x, s->mb_y);
            return -1;
        }
        
        if(IS_INTRA4x4(mb_type))
            cbp= golomb_to_intra4x4_cbp[cbp];
        else
            cbp= golomb_to_inter_cbp[cbp];
    }
    if(cbp || IS_INTRA16x16(mb_type)){
        int i8x8, i4x4, chroma_idx;
        int chroma_qp, dquant;
        GetBitContext *gb= IS_INTRA(mb_type) ? h->intra_gb_ptr : h->inter_gb_ptr;
        const uint8_t *scan, *dc_scan;
        
//        fill_non_zero_count_cache(h);
        if(IS_INTERLACED(mb_type)){
            scan= field_scan;
            dc_scan= luma_dc_field_scan;
        }else{
            scan= zigzag_scan;
            dc_scan= luma_dc_zigzag_scan;
        }
        dquant= get_se_golomb(&s->gb);
        if( dquant > 25 || dquant < -26 ){
            av_log(h->s.avctx, AV_LOG_ERROR, "dquant out of range (%d) at %d %d\n", dquant, s->mb_x, s->mb_y);
            return -1;
        }
        
        s->qscale += dquant;
        if(((unsigned)s->qscale) > 51){
            if(s->qscale<0) s->qscale+= 52;
            else            s->qscale-= 52;
        }
        
        h->chroma_qp= chroma_qp= get_chroma_qp(h, s->qscale);
        if(IS_INTRA16x16(mb_type)){
            if( decode_residual(h, h->intra_gb_ptr, h->mb, LUMA_DC_BLOCK_INDEX, dc_scan, s->qscale, 16) < 0){
                return -1; //FIXME continue if partotioned and other retirn -1 too
            }
            assert((cbp&15) == 0 || (cbp&15) == 15);
            if(cbp&15){
                for(i8x8=0; i8x8<4; i8x8++){
                    for(i4x4=0; i4x4<4; i4x4++){
                        const int index= i4x4 + 4*i8x8;
                        if( decode_residual(h, h->intra_gb_ptr, h->mb + 16*index, index, scan + 1, s->qscale, 15) < 0 ){
                            return -1;
                        }
                    }
                }
            }else{
                fill_rectangle(&h->non_zero_count_cache[scan8[0]], 4, 4, 8, 0, 1);
            }
        }else{
            for(i8x8=0; i8x8<4; i8x8++){
                if(cbp & (1<<i8x8)){
                    for(i4x4=0; i4x4<4; i4x4++){
                        const int index= i4x4 + 4*i8x8;
                        
                        if( decode_residual(h, gb, h->mb + 16*index, index, scan, s->qscale, 16) <0 ){
                            return -1;
                        }
                    }
                }else{
                    uint8_t * const nnz= &h->non_zero_count_cache[ scan8[4*i8x8] ];
                    nnz[0] = nnz[1] = nnz[8] = nnz[9] = 0;
                }
            }
        }
        
        if(cbp&0x30){
            for(chroma_idx=0; chroma_idx<2; chroma_idx++)
                if( decode_residual(h, gb, h->mb + 256 + 16*4*chroma_idx, CHROMA_DC_BLOCK_INDEX, chroma_dc_scan, chroma_qp, 4) < 0){
                    return -1;
                }
        }
        if(cbp&0x20){
            for(chroma_idx=0; chroma_idx<2; chroma_idx++){
                for(i4x4=0; i4x4<4; i4x4++){
                    const int index= 16 + 4*chroma_idx + i4x4;
                    if( decode_residual(h, gb, h->mb + 16*index, index, scan + 1, chroma_qp, 15) < 0){
                        return -1;
                    }
                }
            }
        }else{
            uint8_t * const nnz= &h->non_zero_count_cache[0];
            nnz[ scan8[16]+0 ] = nnz[ scan8[16]+1 ] =nnz[ scan8[16]+8 ] =nnz[ scan8[16]+9 ] =
            nnz[ scan8[20]+0 ] = nnz[ scan8[20]+1 ] =nnz[ scan8[20]+8 ] =nnz[ scan8[20]+9 ] = 0;
        }
    }else{
        uint8_t * const nnz= &h->non_zero_count_cache[0];
        fill_rectangle(&nnz[scan8[0]], 4, 4, 8, 0, 1);
        nnz[ scan8[16]+0 ] = nnz[ scan8[16]+1 ] =nnz[ scan8[16]+8 ] =nnz[ scan8[16]+9 ] =
        nnz[ scan8[20]+0 ] = nnz[ scan8[20]+1 ] =nnz[ scan8[20]+8 ] =nnz[ scan8[20]+9 ] = 0;
    }
    s->current_picture.qscale_table[mb_xy]= s->qscale;
    write_back_non_zero_count(h);
    return 0;
}

