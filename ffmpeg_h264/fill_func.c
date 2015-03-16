

#include <assert.h>
#include <stdio.h>
#include "h264data.h"
#include "fill_func.h"

/**
 * fill a rectangle.
 * @param h height of the recatangle, should be a constant
 * @param w width of the recatangle, should be a constant
 * @param size the size of val (1 or 4), should be a constant
 */
void fill_rectangle(void *vp, int w, int h, int stride, uint32_t val, int size){ //FIXME ensure this IS inlined
    uint8_t *p= (uint8_t*)vp;
    assert(size==1 || size==4);
    
    w      *= size;
    stride *= size;
    
//FIXME check what gcc generates for 64 bit on x86 and possible write a 32 bit ver of it
    if(w==2 && h==2){
        *(uint16_t*)(p + 0)=
        *(uint16_t*)(p + stride)= size==4 ? val : val*0x0101;
    }else if(w==2 && h==4){
        *(uint16_t*)(p + 0*stride)=
        *(uint16_t*)(p + 1*stride)=
        *(uint16_t*)(p + 2*stride)=
        *(uint16_t*)(p + 3*stride)= size==4 ? val : val*0x0101;
    }else if(w==4 && h==1){
        *(uint32_t*)(p + 0*stride)= size==4 ? val : val*0x01010101;
    }else if(w==4 && h==2){
        *(uint32_t*)(p + 0*stride)=
        *(uint32_t*)(p + 1*stride)= size==4 ? val : val*0x01010101;
    }else if(w==4 && h==4){
        *(uint32_t*)(p + 0*stride)=
        *(uint32_t*)(p + 1*stride)=
        *(uint32_t*)(p + 2*stride)=
        *(uint32_t*)(p + 3*stride)= size==4 ? val : val*0x01010101;
    }else if(w==8 && h==1){
        *(uint32_t*)(p + 0)=
        *(uint32_t*)(p + 4)= size==4 ? val : val*0x01010101;
    }else if(w==8 && h==2){
        *(uint32_t*)(p + 0 + 0*stride)=
        *(uint32_t*)(p + 4 + 0*stride)=
        *(uint32_t*)(p + 0 + 1*stride)=
        *(uint32_t*)(p + 4 + 1*stride)=  size==4 ? val : val*0x01010101;
    }else if(w==8 && h==4){
        *(uint64_t*)(p + 0*stride)=
        *(uint64_t*)(p + 1*stride)=
        *(uint64_t*)(p + 2*stride)=
//        *(uint64_t*)(p + 3*stride)= size==4 ? val*0x0100000001ULL : val*0x0101010101010101ULL;
        *(uint64_t*)(p + 3*stride)= size==4 ? val*0x0100000001 : val*0x0101010101010101;
    }else if(w==16 && h==2){
        *(uint64_t*)(p + 0+0*stride)=
        *(uint64_t*)(p + 8+0*stride)=
        *(uint64_t*)(p + 0+1*stride)=
//        *(uint64_t*)(p + 8+1*stride)= size==4 ? val*0x0100000001ULL : val*0x0101010101010101ULL;
        *(uint64_t*)(p + 8+1*stride)= size==4 ? val*0x0100000001 : val*0x0101010101010101;
    }else if(w==16 && h==4){
        *(uint64_t*)(p + 0+0*stride)=
        *(uint64_t*)(p + 8+0*stride)=
        *(uint64_t*)(p + 0+1*stride)=
        *(uint64_t*)(p + 8+1*stride)=
        *(uint64_t*)(p + 0+2*stride)=
        *(uint64_t*)(p + 8+2*stride)=
        *(uint64_t*)(p + 0+3*stride)=
//        *(uint64_t*)(p + 8+3*stride)= size==4 ? val*0x0100000001ULL : val*0x0101010101010101ULL;
        *(uint64_t*)(p + 8+3*stride)= size==4 ? val*0x0100000001 : val*0x0101010101010101;
    }else
        assert(0);
}

void fill_caches(H264Context *h, int mb_type){
    MpegEncContext * const s = &h->s;
    const int mb_xy= s->mb_x + s->mb_y*s->mb_stride;
    int topleft_xy, top_xy, topright_xy, left_xy[2];
    int topleft_type, top_type, topright_type, left_type[2];
    int left_block[4];
    int i;
    //wow what a mess, why didnt they simplify the interlacing&intra stuff, i cant imagine that these complex rules are worth it 
    
    if(h->sps.mb_aff){
    //FIXME
        topleft_xy = 0; /* avoid warning */
        top_xy = 0; /* avoid warning */
        topright_xy = 0; /* avoid warning */
    }else{
        topleft_xy = mb_xy-1 - s->mb_stride;
        top_xy     = mb_xy   - s->mb_stride;
        topright_xy= mb_xy+1 - s->mb_stride;
        left_xy[0]   = mb_xy-1;
        left_xy[1]   = mb_xy-1;
        left_block[0]= 0;
        left_block[1]= 1;
        left_block[2]= 2;
        left_block[3]= 3;
    }
    topleft_type = h->slice_table[topleft_xy ] == h->slice_num ? s->current_picture.mb_type[topleft_xy] : 0;
    top_type     = h->slice_table[top_xy     ] == h->slice_num ? s->current_picture.mb_type[top_xy]     : 0;
    topright_type= h->slice_table[topright_xy] == h->slice_num ? s->current_picture.mb_type[topright_xy]: 0;
    left_type[0] = h->slice_table[left_xy[0] ] == h->slice_num ? s->current_picture.mb_type[left_xy[0]] : 0;
    left_type[1] = h->slice_table[left_xy[1] ] == h->slice_num ? s->current_picture.mb_type[left_xy[1]] : 0;
    if(IS_INTRA(mb_type)){
        h->topleft_samples_available= 
        h->top_samples_available= 
        h->left_samples_available= 0xFFFF;
        h->topright_samples_available= 0xEEEA;
        if(!IS_INTRA(top_type) && (top_type==0 || h->pps.constrained_intra_pred)){
            h->topleft_samples_available= 0xB3FF;
            h->top_samples_available= 0x33FF;
            h->topright_samples_available= 0x26EA;
        }
        for(i=0; i<2; i++){
            if(!IS_INTRA(left_type[i]) && (left_type[i]==0 || h->pps.constrained_intra_pred)){
                h->topleft_samples_available&= 0xDF5F;
                h->left_samples_available&= 0x5F5F;
            }
        }
        
        if(!IS_INTRA(topleft_type) && (topleft_type==0 || h->pps.constrained_intra_pred))
            h->topleft_samples_available&= 0x7FFF;
        
        if(!IS_INTRA(topright_type) && (topright_type==0 || h->pps.constrained_intra_pred))
            h->topright_samples_available&= 0xFBFF;
    
        if(IS_INTRA4x4(mb_type)){
            if(IS_INTRA4x4(top_type)){
                h->intra4x4_pred_mode_cache[4+8*0]= h->intra4x4_pred_mode[top_xy][4];
                h->intra4x4_pred_mode_cache[5+8*0]= h->intra4x4_pred_mode[top_xy][5];
                h->intra4x4_pred_mode_cache[6+8*0]= h->intra4x4_pred_mode[top_xy][6];
                h->intra4x4_pred_mode_cache[7+8*0]= h->intra4x4_pred_mode[top_xy][3];
            }else{
                int pred;
                if(IS_INTRA16x16(top_type) || (IS_INTER(top_type) && !h->pps.constrained_intra_pred))
                    pred= 2;
                else{
                    pred= -1;
                }
                h->intra4x4_pred_mode_cache[4+8*0]=
                h->intra4x4_pred_mode_cache[5+8*0]=
                h->intra4x4_pred_mode_cache[6+8*0]=
                h->intra4x4_pred_mode_cache[7+8*0]= pred;
            }
            for(i=0; i<2; i++){
                if(IS_INTRA4x4(left_type[i])){
                    h->intra4x4_pred_mode_cache[3+8*1 + 2*8*i]= h->intra4x4_pred_mode[left_xy[i]][left_block[0+2*i]];
                    h->intra4x4_pred_mode_cache[3+8*2 + 2*8*i]= h->intra4x4_pred_mode[left_xy[i]][left_block[1+2*i]];
                }else{
                    int pred;
                    if(IS_INTRA16x16(left_type[i]) || (IS_INTER(left_type[i]) && !h->pps.constrained_intra_pred))
                        pred= 2;
                    else{
                        pred= -1;
                    }
                    h->intra4x4_pred_mode_cache[3+8*1 + 2*8*i]=
                    h->intra4x4_pred_mode_cache[3+8*2 + 2*8*i]= pred;
                }
            }
        }
    }
    
    
/*
0 . T T. T T T T 
1 L . .L . . . . 
2 L . .L . . . . 
3 . T TL . . . . 
4 L . .L . . . . 
5 L . .. . . . . 
*/
//FIXME constraint_intra_pred & partitioning & nnz (lets hope this is just a typo in the spec)
    if(top_type){
        h->non_zero_count_cache[4+8*0]= h->non_zero_count[top_xy][0];
        h->non_zero_count_cache[5+8*0]= h->non_zero_count[top_xy][1];
        h->non_zero_count_cache[6+8*0]= h->non_zero_count[top_xy][2];
        h->non_zero_count_cache[7+8*0]= h->non_zero_count[top_xy][3];
    
        h->non_zero_count_cache[1+8*0]= h->non_zero_count[top_xy][7];
        h->non_zero_count_cache[2+8*0]= h->non_zero_count[top_xy][8];
    
        h->non_zero_count_cache[1+8*3]= h->non_zero_count[top_xy][10];
        h->non_zero_count_cache[2+8*3]= h->non_zero_count[top_xy][11];
    }else{
        h->non_zero_count_cache[4+8*0]=      
        h->non_zero_count_cache[5+8*0]=
        h->non_zero_count_cache[6+8*0]=
        h->non_zero_count_cache[7+8*0]=
    
        h->non_zero_count_cache[1+8*0]=
        h->non_zero_count_cache[2+8*0]=
    
        h->non_zero_count_cache[1+8*3]=
        h->non_zero_count_cache[2+8*3]= 64;
    }
    
    if(left_type[0]){
        h->non_zero_count_cache[3+8*1]= h->non_zero_count[left_xy[0]][6];
        h->non_zero_count_cache[3+8*2]= h->non_zero_count[left_xy[0]][5];
        h->non_zero_count_cache[0+8*1]= h->non_zero_count[left_xy[0]][9]; //FIXME left_block
        h->non_zero_count_cache[0+8*4]= h->non_zero_count[left_xy[0]][12];
    }else{
        h->non_zero_count_cache[3+8*1]= 
        h->non_zero_count_cache[3+8*2]= 
        h->non_zero_count_cache[0+8*1]= 
        h->non_zero_count_cache[0+8*4]= 64;
    }
    
    if(left_type[1]){
        h->non_zero_count_cache[3+8*3]= h->non_zero_count[left_xy[1]][4];
        h->non_zero_count_cache[3+8*4]= h->non_zero_count[left_xy[1]][3];
        h->non_zero_count_cache[0+8*2]= h->non_zero_count[left_xy[1]][8];
        h->non_zero_count_cache[0+8*5]= h->non_zero_count[left_xy[1]][11];
    }else{
        h->non_zero_count_cache[3+8*3]= 
        h->non_zero_count_cache[3+8*4]= 
        h->non_zero_count_cache[0+8*2]= 
        h->non_zero_count_cache[0+8*5]= 64;
    }
    
#if 1
    if(IS_INTER(mb_type)){
        int list;
        for(list=0; list<2; list++){
            if((!IS_8X8(mb_type)) && !USES_LIST(mb_type, list)){
                /*if(!h->mv_cache_clean[list]){
                    memset(h->mv_cache [list],  0, 8*5*2*sizeof(int16_t)); //FIXME clean only input? clean at all?
                    memset(h->ref_cache[list], PART_NOT_AVAILABLE, 8*5*sizeof(int8_t));
                    h->mv_cache_clean[list]= 1;
                }*/
                continue; //FIXME direct mode ...
            }
            h->mv_cache_clean[list]= 0;
            
            if(IS_INTER(topleft_type)){
                const int b_xy = h->mb2b_xy[topleft_xy] + 3 + 3*h->b_stride;
                const int b8_xy= h->mb2b8_xy[topleft_xy] + 1 + h->b8_stride;
                *(uint32_t*)h->mv_cache[list][scan8[0] - 1 - 1*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy];
                h->ref_cache[list][scan8[0] - 1 - 1*8]= s->current_picture.ref_index[list][b8_xy];
            }else{
                *(uint32_t*)h->mv_cache[list][scan8[0] - 1 - 1*8]= 0;
                h->ref_cache[list][scan8[0] - 1 - 1*8]= topleft_type ? LIST_NOT_USED : PART_NOT_AVAILABLE;
            }
            
            if(IS_INTER(top_type)){
                const int b_xy= h->mb2b_xy[top_xy] + 3*h->b_stride;
                const int b8_xy= h->mb2b8_xy[top_xy] + h->b8_stride;
                *(uint32_t*)h->mv_cache[list][scan8[0] + 0 - 1*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + 0];
                *(uint32_t*)h->mv_cache[list][scan8[0] + 1 - 1*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + 1];
                *(uint32_t*)h->mv_cache[list][scan8[0] + 2 - 1*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + 2];
                *(uint32_t*)h->mv_cache[list][scan8[0] + 3 - 1*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + 3];
                h->ref_cache[list][scan8[0] + 0 - 1*8]=
                h->ref_cache[list][scan8[0] + 1 - 1*8]= s->current_picture.ref_index[list][b8_xy + 0];
                h->ref_cache[list][scan8[0] + 2 - 1*8]=
                h->ref_cache[list][scan8[0] + 3 - 1*8]= s->current_picture.ref_index[list][b8_xy + 1];
            }else{
                *(uint32_t*)h->mv_cache [list][scan8[0] + 0 - 1*8]= 
                *(uint32_t*)h->mv_cache [list][scan8[0] + 1 - 1*8]= 
                *(uint32_t*)h->mv_cache [list][scan8[0] + 2 - 1*8]= 
                *(uint32_t*)h->mv_cache [list][scan8[0] + 3 - 1*8]= 0;
                *(uint32_t*)&h->ref_cache[list][scan8[0] + 0 - 1*8]= ((top_type ? LIST_NOT_USED : PART_NOT_AVAILABLE)&0xFF)*0x01010101;
            }
            if(IS_INTER(topright_type)){
                const int b_xy= h->mb2b_xy[topright_xy] + 3*h->b_stride;
                const int b8_xy= h->mb2b8_xy[topright_xy] + h->b8_stride;
                *(uint32_t*)h->mv_cache[list][scan8[0] + 4 - 1*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy];
                h->ref_cache[list][scan8[0] + 4 - 1*8]= s->current_picture.ref_index[list][b8_xy];
            }else{
                *(uint32_t*)h->mv_cache [list][scan8[0] + 4 - 1*8]= 0;
                h->ref_cache[list][scan8[0] + 4 - 1*8]= topright_type ? LIST_NOT_USED : PART_NOT_AVAILABLE;
            }
            
            //FIXME unify cleanup or sth
            if(IS_INTER(left_type[0])){
                const int b_xy= h->mb2b_xy[left_xy[0]] + 3;
                const int b8_xy= h->mb2b8_xy[left_xy[0]] + 1;
                *(uint32_t*)h->mv_cache[list][scan8[0] - 1 + 0*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + h->b_stride*left_block[0]];
                *(uint32_t*)h->mv_cache[list][scan8[0] - 1 + 1*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + h->b_stride*left_block[1]];
                h->ref_cache[list][scan8[0] - 1 + 0*8]= 
                h->ref_cache[list][scan8[0] - 1 + 1*8]= s->current_picture.ref_index[list][b8_xy + h->b8_stride*(left_block[0]>>1)];
            }else{
                *(uint32_t*)h->mv_cache [list][scan8[0] - 1 + 0*8]=
                *(uint32_t*)h->mv_cache [list][scan8[0] - 1 + 1*8]= 0;
                h->ref_cache[list][scan8[0] - 1 + 0*8]=
                h->ref_cache[list][scan8[0] - 1 + 1*8]= left_type[0] ? LIST_NOT_USED : PART_NOT_AVAILABLE;
            }
            
            if(IS_INTER(left_type[1])){
                const int b_xy= h->mb2b_xy[left_xy[1]] + 3;
                const int b8_xy= h->mb2b8_xy[left_xy[1]] + 1;
                *(uint32_t*)h->mv_cache[list][scan8[0] - 1 + 2*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + h->b_stride*left_block[2]];
                *(uint32_t*)h->mv_cache[list][scan8[0] - 1 + 3*8]= *(uint32_t*)s->current_picture.motion_val[list][b_xy + h->b_stride*left_block[3]];
                h->ref_cache[list][scan8[0] - 1 + 2*8]= 
                h->ref_cache[list][scan8[0] - 1 + 3*8]= s->current_picture.ref_index[list][b8_xy + h->b8_stride*(left_block[2]>>1)];
            }else{
                *(uint32_t*)h->mv_cache [list][scan8[0] - 1 + 2*8]=
                *(uint32_t*)h->mv_cache [list][scan8[0] - 1 + 3*8]= 0;
                h->ref_cache[list][scan8[0] - 1 + 2*8]=
                h->ref_cache[list][scan8[0] - 1 + 3*8]= left_type[0] ? LIST_NOT_USED : PART_NOT_AVAILABLE;
            }
            h->ref_cache[list][scan8[5 ]+1] = 
            h->ref_cache[list][scan8[7 ]+1] = 
            h->ref_cache[list][scan8[13]+1] =  //FIXME remove past 3 (init somewher else)
            h->ref_cache[list][scan8[4 ]] = 
            h->ref_cache[list][scan8[12]] = PART_NOT_AVAILABLE;
            *(uint32_t*)h->mv_cache [list][scan8[5 ]+1]=
            *(uint32_t*)h->mv_cache [list][scan8[7 ]+1]=
            *(uint32_t*)h->mv_cache [list][scan8[13]+1]= //FIXME remove past 3 (init somewher else)
            *(uint32_t*)h->mv_cache [list][scan8[4 ]]=
            *(uint32_t*)h->mv_cache [list][scan8[12]]= 0;
            if( h->pps.cabac ) {
                /* XXX beurk, Load mvd */
                if(IS_INTER(topleft_type)){
                    const int b_xy = h->mb2b_xy[topleft_xy] + 3 + 3*h->b_stride;
                    *(uint32_t*)h->mvd_cache[list][scan8[0] - 1 - 1*8]= *(uint32_t*)h->mvd_table[list][b_xy];
                }else{
                    *(uint32_t*)h->mvd_cache[list][scan8[0] - 1 - 1*8]= 0;
                }
                if(IS_INTER(top_type)){
                    const int b_xy= h->mb2b_xy[top_xy] + 3*h->b_stride;
                    *(uint32_t*)h->mvd_cache[list][scan8[0] + 0 - 1*8]= *(uint32_t*)h->mvd_table[list][b_xy + 0];
                    *(uint32_t*)h->mvd_cache[list][scan8[0] + 1 - 1*8]= *(uint32_t*)h->mvd_table[list][b_xy + 1];
                    *(uint32_t*)h->mvd_cache[list][scan8[0] + 2 - 1*8]= *(uint32_t*)h->mvd_table[list][b_xy + 2];
                    *(uint32_t*)h->mvd_cache[list][scan8[0] + 3 - 1*8]= *(uint32_t*)h->mvd_table[list][b_xy + 3];
                }else{
                    *(uint32_t*)h->mvd_cache [list][scan8[0] + 0 - 1*8]= 
                    *(uint32_t*)h->mvd_cache [list][scan8[0] + 1 - 1*8]= 
                    *(uint32_t*)h->mvd_cache [list][scan8[0] + 2 - 1*8]= 
                    *(uint32_t*)h->mvd_cache [list][scan8[0] + 3 - 1*8]= 0;
                }
                if(IS_INTER(left_type[0])){
                    const int b_xy= h->mb2b_xy[left_xy[0]] + 3;
                    *(uint32_t*)h->mvd_cache[list][scan8[0] - 1 + 0*8]= *(uint32_t*)h->mvd_table[list][b_xy + h->b_stride*left_block[0]];
                    *(uint32_t*)h->mvd_cache[list][scan8[0] - 1 + 1*8]= *(uint32_t*)h->mvd_table[list][b_xy + h->b_stride*left_block[1]];
                }else{
                    *(uint32_t*)h->mvd_cache [list][scan8[0] - 1 + 0*8]=
                    *(uint32_t*)h->mvd_cache [list][scan8[0] - 1 + 1*8]= 0;
                }
                if(IS_INTER(left_type[1])){
                    const int b_xy= h->mb2b_xy[left_xy[1]] + 3;
                    *(uint32_t*)h->mvd_cache[list][scan8[0] - 1 + 2*8]= *(uint32_t*)h->mvd_table[list][b_xy + h->b_stride*left_block[2]];
                    *(uint32_t*)h->mvd_cache[list][scan8[0] - 1 + 3*8]= *(uint32_t*)h->mvd_table[list][b_xy + h->b_stride*left_block[3]];
                }else{
                    *(uint32_t*)h->mvd_cache [list][scan8[0] - 1 + 2*8]=
                    *(uint32_t*)h->mvd_cache [list][scan8[0] - 1 + 3*8]= 0;
                }
                *(uint32_t*)h->mvd_cache [list][scan8[5 ]+1]=
                *(uint32_t*)h->mvd_cache [list][scan8[7 ]+1]=
                *(uint32_t*)h->mvd_cache [list][scan8[13]+1]= //FIXME remove past 3 (init somewher else)
                *(uint32_t*)h->mvd_cache [list][scan8[4 ]]=
                *(uint32_t*)h->mvd_cache [list][scan8[12]]= 0;
            }
        }
//FIXME
    }
#endif
}