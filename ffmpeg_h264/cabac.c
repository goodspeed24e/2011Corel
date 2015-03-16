

/**
 * @file cabac.c
 * Context Adaptive Binary Arithmetic Coder.
 */

#include <string.h>

#include "common.h"

#include "fill_func.h"
#include "h264data.h"
#include "h264context.h"
#include "cabac.h"


const uint8_t ff_h264_lps_range[64][4]= {
{128,176,208,240}, {128,167,197,227}, {128,158,187,216}, {123,150,178,205},
{116,142,169,195}, {111,135,160,185}, {105,128,152,175}, {100,122,144,166},
{ 95,116,137,158}, { 90,110,130,150}, { 85,104,123,142}, { 81, 99,117,135},
{ 77, 94,111,128}, { 73, 89,105,122}, { 69, 85,100,116}, { 66, 80, 95,110},
{ 62, 76, 90,104}, { 59, 72, 86, 99}, { 56, 69, 81, 94}, { 53, 65, 77, 89},
{ 51, 62, 73, 85}, { 48, 59, 69, 80}, { 46, 56, 66, 76}, { 43, 53, 63, 72},
{ 41, 50, 59, 69}, { 39, 48, 56, 65}, { 37, 45, 54, 62}, { 35, 43, 51, 59},
{ 33, 41, 48, 56}, { 32, 39, 46, 53}, { 30, 37, 43, 50}, { 29, 35, 41, 48},
{ 27, 33, 39, 45}, { 26, 31, 37, 43}, { 24, 30, 35, 41}, { 23, 28, 33, 39},
{ 22, 27, 32, 37}, { 21, 26, 30, 35}, { 20, 24, 29, 33}, { 19, 23, 27, 31},
{ 18, 22, 26, 30}, { 17, 21, 25, 28}, { 16, 20, 23, 27}, { 15, 19, 22, 25},
{ 14, 18, 21, 24}, { 14, 17, 20, 23}, { 13, 16, 19, 22}, { 12, 15, 18, 21},
{ 12, 14, 17, 20}, { 11, 14, 16, 19}, { 11, 13, 15, 18}, { 10, 12, 15, 17},
{ 10, 12, 14, 16}, {  9, 11, 13, 15}, {  9, 11, 12, 14}, {  8, 10, 12, 14},
{  8,  9, 11, 13}, {  7,  9, 11, 12}, {  7,  9, 10, 12}, {  7,  8, 10, 11},
{  6,  8,  9, 11}, {  6,  7,  9, 10}, {  6,  7,  8,  9}, {  2,  2,  2,  2},
};

const uint8_t ff_h264_mps_state[64]= {
  1, 2, 3, 4, 5, 6, 7, 8,
  9,10,11,12,13,14,15,16,
 17,18,19,20,21,22,23,24,
 25,26,27,28,29,30,31,32,
 33,34,35,36,37,38,39,40,
 41,42,43,44,45,46,47,48,
 49,50,51,52,53,54,55,56,
 57,58,59,60,61,62,62,63,
};

const uint8_t ff_h264_lps_state[64]= {
  0, 0, 1, 2, 2, 4, 4, 5,
  6, 7, 8, 9, 9,11,11,12,
 13,13,15,15,16,16,18,18,
 19,19,21,21,22,22,23,24,
 24,25,26,26,27,27,28,29,
 29,30,30,30,31,32,32,33,
 33,33,34,34,35,35,35,36,
 36,36,37,37,37,38,38,63,
};


static const uint8_t block_idx_x[16] = {
    0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3
};
static const uint8_t block_idx_y[16] = {
    0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3
};
static const uint8_t block_idx_xy[4][4] = {
    { 0, 2, 8,  10},
    { 1, 3, 9,  11},
    { 4, 6, 12, 14},
    { 5, 7, 13, 15}
};

/**
 *
 * @param buf_size size of buf in bits
 */
void ff_init_cabac_encoder(CABACContext *c, uint8_t *buf, int buf_size){
    init_put_bits(&c->pb, buf, buf_size);

    c->low= 0;
    c->range= 0x1FE;
    c->outstanding_count= 0;
#ifdef STRICT_LIMITS
    c->sym_count =0;
#endif
    
    c->pb.bit_left++; //avoids firstBitFlag
}

/**
 *
 * @param buf_size size of buf in bits
 */
void ff_init_cabac_decoder(CABACContext *c, const uint8_t *buf, int buf_size){
    c->bytestream_start= 
    c->bytestream= buf;
    c->bytestream_end= buf + buf_size;

    c->low= *c->bytestream++;
    c->low= (c->low<<9) + ((*c->bytestream++)<<1);
    c->range= 0x1FE00;
    c->bits_left= 7;
}

void ff_init_cabac_states(CABACContext *c, uint8_t const (*lps_range)[4], 
                          uint8_t const *mps_state, uint8_t const *lps_state, int state_count){
    int i, j;
    
    for(i=0; i<state_count; i++){
        for(j=0; j<4; j++){ //FIXME check if this is worth the 1 shift we save
            c->lps_range[2*i+0][j]=
            c->lps_range[2*i+1][j]= lps_range[i][j];
        }

        c->mps_state[2*i+0]= 2*mps_state[i];
        c->mps_state[2*i+1]= 2*mps_state[i]+1;

        if( i ){
            c->lps_state[2*i+0]= 2*lps_state[i];
            c->lps_state[2*i+1]= 2*lps_state[i]+1;
        }else{
            c->lps_state[2*i+0]= 1;
            c->lps_state[2*i+1]= 0;
        }
    }
}


static int decode_cabac_mb_cbp_luma( H264Context *h) {
    MpegEncContext * const s = &h->s;
    const int mb_xy = s->mb_x + s->mb_y*s->mb_stride;
    int cbp = 0;
    int i8x8;
    h->cbp_table[mb_xy] = 0;  /* FIXME aaahahahah beurk */
    for( i8x8 = 0; i8x8 < 4; i8x8++ ) {
        int mba_xy = -1;
        int mbb_xy = -1;
        int x, y;
        int ctx = 0;
        x = block_idx_x[4*i8x8];
        y = block_idx_y[4*i8x8];
        if( x > 0 )
            mba_xy = mb_xy;
        else if( s->mb_x > 0 )
            mba_xy = mb_xy - 1;
        if( y > 0 )
            mbb_xy = mb_xy;
        else if( s->mb_y > 0 )
            mbb_xy = mb_xy - s->mb_stride;
        /* No need to test for skip as we put 0 for skip block */
        if( mba_xy >= 0 ) {
            int i8x8a = block_idx_xy[(x-1)&0x03][y]/4;
            if( ((h->cbp_table[mba_xy] >> i8x8a)&0x01) == 0 )
                ctx++;
        }
        if( mbb_xy >= 0 ) {
            int i8x8b = block_idx_xy[x][(y-1)&0x03]/4;
            if( ((h->cbp_table[mbb_xy] >> i8x8b)&0x01) == 0 )
                ctx += 2;
        }
        if( get_cabac( &h->cabac, &h->cabac_state[73 + ctx] ) ) {
            cbp |= 1 << i8x8;
            h->cbp_table[mb_xy] = cbp;  /* FIXME aaahahahah beurk */
        }
    }
    return cbp;
}
static int decode_cabac_mb_cbp_chroma( H264Context *h) {
    MpegEncContext * const s = &h->s;
    const int mb_xy = s->mb_x + s->mb_y*s->mb_stride;
    int ctx;
    int cbp_a, cbp_b;
    /* No need to test for skip */
    if( s->mb_x > 0 )
        cbp_a = (h->cbp_table[mb_xy-1]>>4)&0x03;
    else
        cbp_a = -1;
    if( s->mb_y > 0 )
        cbp_b = (h->cbp_table[mb_xy-s->mb_stride]>>4)&0x03;
    else
        cbp_b = -1;
    ctx = 0;
    if( cbp_a > 0 ) ctx++;
    if( cbp_b > 0 ) ctx += 2;
    if( get_cabac( &h->cabac, &h->cabac_state[77 + ctx] ) == 0 )
        return 0;
    ctx = 4;
    if( cbp_a == 2 ) ctx++;
    if( cbp_b == 2 ) ctx += 2;
    if( get_cabac( &h->cabac, &h->cabac_state[77 + ctx] ) )
        return 2;
    else
        return 1;
}
static int decode_cabac_mb_dqp( H264Context *h) {
    MpegEncContext * const s = &h->s;
    int mbn_xy;
    int   ctx = 0;
    int   val = 0;
    if( s->mb_x > 0 )
        mbn_xy = s->mb_x + s->mb_y*s->mb_stride - 1;
    else
        mbn_xy = s->mb_width - 1 + (s->mb_y-1)*s->mb_stride;
    if( mbn_xy >= 0 && h->last_qscale_diff != 0 && ( IS_INTRA16x16(s->current_picture.mb_type[mbn_xy] ) || (h->cbp_table[mbn_xy]&0x3f) ) )
        ctx++;
    while( get_cabac( &h->cabac, &h->cabac_state[60 + ctx] ) ) {
        if( ctx < 2 )
            ctx = 2;
        else
            ctx = 3;
        val++;
    }
    if( val&0x01 )
        return (val + 1)/2;
    else
        return -(val + 1)/2;
}
static int decode_cabac_mb_sub_type( H264Context *h ) {
    if( get_cabac( &h->cabac, &h->cabac_state[21] ) )
        return 0;   /* 8x8 */
    if( !get_cabac( &h->cabac, &h->cabac_state[22] ) )
        return 1;   /* 8x4 */
    if( get_cabac( &h->cabac, &h->cabac_state[23] ) )
        return 2;   /* 4x8 */
    return 3;       /* 4x4 */
}
static int decode_cabac_mb_ref( H264Context *h, int list, int n ) {
    int refa = h->ref_cache[list][scan8[n] - 1];
    int refb = h->ref_cache[list][scan8[n] - 8];
    int ref  = 0;
    int ctx  = 0;
    if( refa > 0 )
        ctx++;
    if( refb > 0 )
        ctx += 2;
    while( get_cabac( &h->cabac, &h->cabac_state[54+ctx] ) ) {
        ref++;
        if( ctx < 4 )
            ctx = 4;
        else
            ctx = 5;
    }
    return ref;
}
static int decode_cabac_mb_mvd( H264Context *h, int list, int n, int l ) {
    int amvd = abs( h->mvd_cache[list][scan8[n] - 1][l] ) +
               abs( h->mvd_cache[list][scan8[n] - 8][l] );
    int ctxbase = (l == 0) ? 40 : 47;
    int ctx;
    int mvd = 0;
    if( amvd < 3 )
        ctx = 0;
    else if( amvd > 32 )
        ctx = 2;
    else
        ctx = 1;
    while( mvd < 9 && get_cabac( &h->cabac, &h->cabac_state[ctxbase+ctx] ) ) {
        mvd++;
        if( ctx < 3 )
            ctx = 3;
        else if( ctx < 6 )
            ctx++;
    }
    if( mvd >= 9 ) {
        int k = 3;
        while( get_cabac_bypass( &h->cabac ) ) {
            mvd += 1 << k;
            k++;
        }
        while( k-- ) {
            if( get_cabac_bypass( &h->cabac ) )
                mvd += 1 << k;
        }
    }
    if( mvd != 0 && get_cabac_bypass( &h->cabac ) )
        return -mvd;
    return mvd;
}
static int get_cabac_cbf_ctx( H264Context *h, int cat, int idx ) {
    MpegEncContext * const s = &h->s;
    const int mb_xy  = s->mb_x + s->mb_y*s->mb_stride;
    int mba_xy = -1;
    int mbb_xy = -1;
    int nza = -1;
    int nzb = -1;
    int ctx = 0;
    if( cat == 0 ) {
        if( s->mb_x > 0 ) {
            mba_xy = mb_xy - 1;
            if( IS_INTRA16x16(s->current_picture.mb_type[mba_xy] ) )
                    nza = h->cbp_table[mba_xy]&0x100;
        }
        if( s->mb_y > 0 ) {
            mbb_xy = mb_xy - s->mb_stride;
            if( IS_INTRA16x16(s->current_picture.mb_type[mbb_xy] ) )
                    nzb = h->cbp_table[mbb_xy]&0x100;
        }
    } else if( cat == 1 || cat == 2 ) {
        int i8x8a, i8x8b;
        int x, y;
        x = block_idx_x[idx];
        y = block_idx_y[idx];
        if( x > 0 )
            mba_xy = mb_xy;
        else if( s->mb_x > 0 )
            mba_xy = mb_xy - 1;
        if( y > 0 )
            mbb_xy = mb_xy;
        else if( s->mb_y > 0 )
            mbb_xy = mb_xy - s->mb_stride;
        /* No need to test for skip */
        if( mba_xy >= 0 ) {
            i8x8a = block_idx_xy[(x-1)&0x03][y]/4;
            if( !IS_INTRA_PCM(s->current_picture.mb_type[mba_xy] ) &&
                ((h->cbp_table[mba_xy]&0x0f)>>i8x8a))
                nza = h->non_zero_count_cache[scan8[idx] - 1];
        }
        if( mbb_xy >= 0 ) {
            i8x8b = block_idx_xy[x][(y-1)&0x03]/4;
            if( !IS_INTRA_PCM(s->current_picture.mb_type[mbb_xy] ) &&
                ((h->cbp_table[mbb_xy]&0x0f)>>i8x8b))
                nzb = h->non_zero_count_cache[scan8[idx] - 8];
        }
    } else if( cat == 3 ) {
        if( s->mb_x > 0 ) {
            mba_xy = mb_xy - 1;
            if( !IS_INTRA_PCM(s->current_picture.mb_type[mba_xy] ) &&
                (h->cbp_table[mba_xy]&0x30) )
                nza = (h->cbp_table[mba_xy]>>(6+idx))&0x01;
        }
        if( s->mb_y > 0 ) {
            mbb_xy = mb_xy - s->mb_stride;
            if( !IS_INTRA_PCM(s->current_picture.mb_type[mbb_xy] ) &&
                (h->cbp_table[mbb_xy]&0x30) )
                nzb = (h->cbp_table[mbb_xy]>>(6+idx))&0x01;
        }
    } else if( cat == 4 ) {
        int idxc = idx % 4 ;
        if( idxc == 1 || idxc == 3 )
            mba_xy = mb_xy;
        else if( s->mb_x > 0 )
            mba_xy = mb_xy -1;
        if( idxc == 2 || idxc == 3 )
            mbb_xy = mb_xy;
        else if( s->mb_y > 0 )
            mbb_xy = mb_xy - s->mb_stride;
        if( mba_xy >= 0 &&
            !IS_INTRA_PCM(s->current_picture.mb_type[mba_xy] ) &&
            (h->cbp_table[mba_xy]&0x30) == 0x20 )
            nza = h->non_zero_count_cache[scan8[16+idx] - 1];
        if( mbb_xy >= 0 &&
            !IS_INTRA_PCM(s->current_picture.mb_type[mbb_xy] ) &&
            (h->cbp_table[mbb_xy]&0x30) == 0x20 )
            nzb = h->non_zero_count_cache[scan8[16+idx] - 8];
    }
    if( ( mba_xy < 0 && IS_INTRA( s->current_picture.mb_type[mb_xy] ) ) ||
        ( mba_xy >= 0 && IS_INTRA_PCM(s->current_picture.mb_type[mba_xy] ) ) ||
          nza > 0 )
        ctx++;
    if( ( mbb_xy < 0 && IS_INTRA( s->current_picture.mb_type[mb_xy] ) ) ||
        ( mbb_xy >= 0 && IS_INTRA_PCM(s->current_picture.mb_type[mbb_xy] ) ) ||
          nzb > 0 )
        ctx += 2;
    return ctx + 4 * cat;
}
static int decode_cabac_residual( H264Context *h, DCTELEM *block, int cat, int n, const uint8_t *scantable, int qp, int max_coeff) {
    const int mb_xy  = h->s.mb_x + h->s.mb_y*h->s.mb_stride;
    const uint16_t *qmul= dequant_coeff[qp];
    static const int significant_coeff_flag_offset[5] = { 0, 15, 29, 44, 47 };
    static const int last_significant_coeff_flag_offset[5] = { 0, 15, 29, 44, 47 };
    static const int coeff_abs_level_m1_offset[5] = { 0, 10, 20, 30, 39 };
    int coeff[16];
    int last = 0;
    int coeff_count = 0;
    int nz[16] = {0};
    int i;
    int abslevel1 = 0;
    int abslevelgt1 = 0;
    /* cat: 0-> DC 16x16  n = 0
     *      1-> AC 16x16  n = luma4x4idx
     *      2-> Luma4x4   n = luma4x4idx
     *      3-> DC Chroma n = iCbCr
     *      4-> AC Chroma n = 4 * iCbCr + chroma4x4idx
     */
    /* read coded block flag */
    if( get_cabac( &h->cabac, &h->cabac_state[85 + get_cabac_cbf_ctx( h, cat, n ) ] ) == 0 ) {
        if( cat == 1 || cat == 2 )
            h->non_zero_count_cache[scan8[n]] = 0;
        else if( cat == 4 )
            h->non_zero_count_cache[scan8[16+n]] = 0;
        return 0;
    }
    while( last < max_coeff - 1 ) {
        int ctx = FFMIN( last, max_coeff - 2 );
        if( get_cabac( &h->cabac, &h->cabac_state[105+significant_coeff_flag_offset[cat]+ctx] ) == 0 ) {
            nz[last++] = 0;
        }
        else {
            nz[last++] = 1;
            coeff_count++;
            if( get_cabac( &h->cabac, &h->cabac_state[166+last_significant_coeff_flag_offset[cat]+ctx] ) ) {
                while( last < max_coeff ) {
                    nz[last++] = 0;
                }
                break;
            }
        }
    }
    if( last == max_coeff -1 ) {
        nz[last++] = 1;
        coeff_count++;
    }
    if( cat == 0 && coeff_count > 0 )
        h->cbp_table[mb_xy] |= 0x100;
    else if( cat == 1 || cat == 2 )
        h->non_zero_count_cache[scan8[n]] = coeff_count;
    else if( cat == 3 && coeff_count > 0 )
        h->cbp_table[mb_xy] |= 0x40 << n;
    else if( cat == 4 )
        h->non_zero_count_cache[scan8[16+n]] = coeff_count;
    for( i = coeff_count - 1; i >= 0; i-- ) {
        int coeff_abs_m1;
        int ctx = (abslevelgt1 != 0 ? 0 : FFMIN( 4, abslevel1 + 1 )) + coeff_abs_level_m1_offset[cat];
        if( get_cabac( &h->cabac, &h->cabac_state[227+ctx] ) == 0 ) {
            coeff_abs_m1 = 0;
        } else {
            coeff_abs_m1 = 1;
            ctx = 5 + FFMIN( 4, abslevelgt1 ) + coeff_abs_level_m1_offset[cat];
            while( coeff_abs_m1 < 14 && get_cabac( &h->cabac, &h->cabac_state[227+ctx] ) ) {
                coeff_abs_m1++;
            }
        }
        if( coeff_abs_m1 >= 14 ) {
            int j = 0;
            while( get_cabac_bypass( &h->cabac ) ) {
                coeff_abs_m1 += 1 << j;
                j++;
            }
            while( j-- ) {
                if( get_cabac_bypass( &h->cabac ) )
                    coeff_abs_m1 += 1 << j ;
            }
        }
        if( get_cabac_bypass( &h->cabac ) )
            coeff[i] = -1 *( coeff_abs_m1 + 1 );
        else
            coeff[i] = coeff_abs_m1 + 1;
        if( coeff_abs_m1 == 0 )
            abslevel1++;
        else
            abslevelgt1++;
    }
    if( cat == 0 || cat == 3 ) { /* DC */
        int j;
        for( i = 0, j = 0; j < coeff_count; i++ ) {
            if( nz[i] ) {
                block[scantable[i]] = coeff[j];
                j++;
            }
        }
    } else { /* AC */
        int j;
        for( i = 0, j = 0; j < coeff_count; i++ ) {
            if( nz[i] ) {
                block[scantable[i]] = coeff[j] * qmul[scantable[i]];
                j++;
            }
        }
    }
    return 0;
}

static int decode_cabac_mb_chroma_pre_mode( H264Context *h) {
    MpegEncContext * const s = &h->s;
    const int mb_xy = s->mb_x + s->mb_y*s->mb_stride;
    const int mba_xy = mb_xy - 1;
    const int mbb_xy = mb_xy - s->mb_stride;
    int ctx = 0;
    /* No need to test for IS_INTRA4x4 and IS_INTRA16x16, as we set chroma_pred_mode_table to 0 */
    if( s->mb_x > 0 && h->chroma_pred_mode_table[mba_xy] != 0 )
        ctx++;
    if( s->mb_y > 0 && h->chroma_pred_mode_table[mbb_xy] != 0 )
        ctx++;
    if( get_cabac( &h->cabac, &h->cabac_state[64+ctx] ) == 0 )
        return 0;
    if( get_cabac( &h->cabac, &h->cabac_state[64+3] ) == 0 )
        return 1;
    if( get_cabac( &h->cabac, &h->cabac_state[64+3] ) == 0 )
        return 2;
    else
        return 3;
}
static int decode_cabac_mb_intra4x4_pred_mode( H264Context *h, int pred_mode ) {
    int mode = 0;
    if( get_cabac( &h->cabac, &h->cabac_state[68] ) )
        return pred_mode;
    if( get_cabac( &h->cabac, &h->cabac_state[69] ) )
        mode += 1;
    if( get_cabac( &h->cabac, &h->cabac_state[69] ) )
        mode += 2;
    if( get_cabac( &h->cabac, &h->cabac_state[69] ) )
        mode += 4;
    if( mode >= pred_mode )
        return mode + 1;
    else
        return mode;
}

static int decode_cabac_mb_type( H264Context *h ) {
    MpegEncContext * const s = &h->s;
    if( h->slice_type == I_TYPE ) {
        const int mb_xy= s->mb_x + s->mb_y*s->mb_stride;
        int ctx = 0;
        int mb_type;
        if( s->mb_x > 0 && !IS_INTRA4x4( s->current_picture.mb_type[mb_xy-1] ) )
            ctx++;
        if( s->mb_y > 0 && !IS_INTRA4x4( s->current_picture.mb_type[mb_xy-s->mb_stride] ) )
            ctx++;
        if( get_cabac( &h->cabac, &h->cabac_state[3+ctx] ) == 0 )
            return 0;   /* I4x4 */
        if( get_cabac_terminate( &h->cabac ) )
            return 25;  /* PCM */
        mb_type = 1;    /* I16x16 */
        if( get_cabac( &h->cabac, &h->cabac_state[3+3] ) )
            mb_type += 12;  /* cbp_luma != 0 */
        if( get_cabac( &h->cabac, &h->cabac_state[3+4] ) ) {
            if( get_cabac( &h->cabac, &h->cabac_state[3+5] ) )
                mb_type += 4 * 2;   /* cbp_chroma == 2 */
            else
                mb_type += 4 * 1;   /* cbp_chroma == 1 */
        }
        if( get_cabac( &h->cabac, &h->cabac_state[3+6] ) )
            mb_type += 2;
        if( get_cabac( &h->cabac, &h->cabac_state[3+7] ) )
            mb_type += 1;
        return mb_type;
    } else if( h->slice_type == P_TYPE ) {
        if( get_cabac( &h->cabac, &h->cabac_state[14] ) == 0 ) {
            /* P-type */
            if( get_cabac( &h->cabac, &h->cabac_state[15] ) == 0 ) {
                if( get_cabac( &h->cabac, &h->cabac_state[16] ) == 0 )
                    return 0; /* P_L0_D16x16; */
                else
                    return 3; /* P_8x8; */
            } else {
                if( get_cabac( &h->cabac, &h->cabac_state[17] ) == 0 )
                    return 2; /* P_L0_D8x16; */
                else
                    return 1; /* P_L0_D16x8; */
            }
        } else {
            int mb_type;
            /* I-type */
            if( get_cabac( &h->cabac, &h->cabac_state[17] ) == 0 )
                return 5+0; /* I_4x4 */
            if( get_cabac_terminate( &h->cabac ) )
                return 5+25; /*I_PCM */
            mb_type = 5+1;    /* I16x16 */
            if( get_cabac( &h->cabac, &h->cabac_state[17+1] ) )
                mb_type += 12;  /* cbp_luma != 0 */
            if( get_cabac( &h->cabac, &h->cabac_state[17+2] ) ) {
                if( get_cabac( &h->cabac, &h->cabac_state[17+2] ) )
                    mb_type += 4 * 2;   /* cbp_chroma == 2 */
                else
                    mb_type += 4 * 1;   /* cbp_chroma == 1 */
            }
            if( get_cabac( &h->cabac, &h->cabac_state[17+3] ) )
                mb_type += 2;
            if( get_cabac( &h->cabac, &h->cabac_state[17+3] ) )
                mb_type += 1;
            return mb_type;
        }
    } else {
        /* TODO do others frames types */
        return -1;
    }
}
static int decode_cabac_mb_skip( H264Context *h) {
    MpegEncContext * const s = &h->s;
    const int mb_xy = s->mb_x + s->mb_y*s->mb_stride;
    const int mba_xy = mb_xy - 1;
    const int mbb_xy = mb_xy - s->mb_stride;
    int ctx = 0;
    if( s->mb_x > 0 && !IS_SKIP( s->current_picture.mb_type[mba_xy] ) )
        ctx++;
    if( s->mb_y > 0 && !IS_SKIP( s->current_picture.mb_type[mbb_xy] ) )
        ctx++;
    if( h->slice_type == P_TYPE || h->slice_type == SP_TYPE)
        return get_cabac( &h->cabac, &h->cabac_state[11+ctx] );
    else /* B-frame */
        return get_cabac( &h->cabac, &h->cabac_state[24+ctx] );
}
/**
 * decodes a macroblock
 * @returns 0 if ok, AC_ERROR / DC_ERROR / MV_ERROR if an error is noticed
 */
int decode_mb_cabac(H264Context *h) {
    MpegEncContext * const s = &h->s;
    const int mb_xy= s->mb_x + s->mb_y*s->mb_stride;
    int mb_type, partition_count, cbp = 0;
    	
    s->dsp.clear_blocks(h->mb); /*宏块清零，0--15: Y MB,16--19: U MB, 20--23: V MB*/
    /*not supported*/
    if( h->slice_type == B_TYPE ) {
        av_log( h->s.avctx, AV_LOG_ERROR, "B-frame not supported with CABAC\n" );
        return -1;
    }
    if( h->sps.mb_aff ) {
        av_log( h->s.avctx, AV_LOG_ERROR, "Fields not supported with CABAC\n" );
        return -1;
    }
    /*not supported*/
    /*P帧跳过宏块熵解码，即没有编码的宏块*/
    if( h->slice_type != I_TYPE && h->slice_type != SI_TYPE ) {
        if( decode_cabac_mb_skip( h ) ) {/*从码流中读取跳过标记*/
            int mx, my;
            /* 跳过的宏块模式 */
            mb_type= MB_TYPE_16x16|MB_TYPE_P0L0|MB_TYPE_P1L0|MB_TYPE_SKIP;
            memset(h->non_zero_count[mb_xy], 0, 16);       /*清零*/
            memset(h->non_zero_count_cache + 8, 0, 8*5); /*清零*/

            fill_caches(h, mb_type);			/*根据宏块模式mb_type，填充缓冲区*/
            pred_pskip_motion(h, &mx, &my);/*运动向量预测*/
            fill_rectangle(&h->ref_cache[0][scan8[0]], 4, 4, 8, 0, 1); /*填充参考帧的缓冲区*/
            fill_rectangle(  h->mvd_cache[0][scan8[0]], 4, 4, 8, pack16to32(0,0), 4);/*填充运动向量残差*/
            fill_rectangle(  h->mv_cache[0][scan8[0]], 4, 4, 8, pack16to32(mx,my), 4);/*填充运动向量*/
            write_back_motion(h, mb_type);	/*写回运动向量和参考帧信息*/
            s->current_picture.mb_type[mb_xy]= mb_type; 		/*保存宏块模式*/
            s->current_picture.qscale_table[mb_xy]= s->qscale;	/*保存量化步长*/
            h->slice_table[ mb_xy ]= h->slice_num;			/*保存片数目*/
            h->cbp_table[mb_xy] = 0;						/*编码块模式CBP清零*/
            h->chroma_pred_mode_table[mb_xy] = 0;			/*色度预测模式清零*/
            h->last_qscale_diff = 0;							/*量化残差*/
            h->prev_mb_skiped= 1;							/*记录宏块跳过模式*/
            return 0;
        }
    }
    h->prev_mb_skiped = 0;		/*无宏块跳过编码*/

    if( ( mb_type = decode_cabac_mb_type( h ) ) < 0 ) {/*从码流中读取宏块类型*/
        av_log( h->s.avctx, AV_LOG_ERROR, "decode_cabac_mb_type failed\n" );
        return -1;
    }
    if( h->slice_type == P_TYPE ) {         /*P帧*/
        if( mb_type < 5) {
            partition_count= p_mb_type_info[mb_type].partition_count;/*P帧的宏块数目*/
            mb_type=         p_mb_type_info[mb_type].type;		    /*P帧的宏块模式*/
        } else {
            mb_type -= 5;
            goto decode_intra_mb;
        }
    } else {						   /*I帧*/
       assert(h->slice_type == I_TYPE);
decode_intra_mb:
        partition_count = 0;
        cbp= i_mb_type_info[mb_type].cbp;		/*由mb_type读取块编码模式CBP*/
        h->intra16x16_pred_mode= i_mb_type_info[mb_type].pred_mode;/*由mb_type读取块预测模式*/
        mb_type= i_mb_type_info[mb_type].type;/*I帧的宏块模式*/
    }

    s->current_picture.mb_type[mb_xy]= mb_type;	/*保存宏块模式*/
    h->slice_table[ mb_xy ]= h->slice_num;		/*保存片数目*/

    if(IS_INTRA_PCM(mb_type)) {		/*I_PCM是H.264特有的一种编码方式,解码器不支持*/
								/* 对像素直接编码，不做变换、量化熵编码等 */
        h->cbp_table[mb_xy] = 0xf +4*2;
        h->chroma_pred_mode_table[mb_xy] = 0;
        s->current_picture.qscale_table[mb_xy]= s->qscale;
        return -1;
    }
    fill_caches(h, mb_type);			/*根据宏块模式mb_type，填充缓冲区*/
    if( IS_INTRA( mb_type ) ) {		/*Intra宏块*/
        if( IS_INTRA4x4( mb_type ) ) {	/*4*4的Intra块*/
            int i;
            for( i = 0; i < 16; i++ ) {
                int pred = pred_intra_mode( h, i );
                h->intra4x4_pred_mode_cache[ scan8[i] ] = decode_cabac_mb_intra4x4_pred_mode( h, pred );
                //av_log( s->avctx, AV_LOG_ERROR, "i4x4 pred=%d mode=%d\n", pred, h->intra4x4_pred_mode_cache[ scan8[i] ] );
            }
            write_back_intra_pred_mode(h);
            if( check_intra4x4_pred_mode(h) < 0 ) return -1;
        } else {						/*16*16的Intra块*/
            h->intra16x16_pred_mode= check_intra_pred_mode( h, h->intra16x16_pred_mode );
            if( h->intra16x16_pred_mode < 0 ) return -1;
        }
        h->chroma_pred_mode_table[mb_xy] =
            h->chroma_pred_mode          = decode_cabac_mb_chroma_pre_mode( h );
        h->chroma_pred_mode= check_intra_pred_mode( h, h->chroma_pred_mode );
        if( h->chroma_pred_mode < 0 ) return -1;
    } else if( partition_count == 4 ) {	/*Inter宏块，且4个子块*/
        int i, j, sub_partition_count[4], list, ref[2][4];
        /* Only P-frame */
        for( i = 0; i < 4; i++ ) {
            h->sub_mb_type[i] = decode_cabac_mb_sub_type( h );
            sub_partition_count[i]= p_sub_mb_type_info[ h->sub_mb_type[i] ].partition_count;
            h->sub_mb_type[i]=      p_sub_mb_type_info[ h->sub_mb_type[i] ].type;
        }
        for( list = 0; list < 2; list++ ) {
            if( h->ref_count[list] > 0 ) {
                for( i = 0; i < 4; i++ ) {
                    if(IS_DIR(h->sub_mb_type[i], 0, list) && !IS_DIRECT(h->sub_mb_type[i])){
                        if( h->ref_count[list] > 1 )
                            ref[list][i] = decode_cabac_mb_ref( h, list, 4*i );
                        else
                            ref[list][i] = 0;
                    } else {
                        ref[list][i] = -1;
                    }
                                                       h->ref_cache[list][ scan8[4*i]+1 ]=
                    h->ref_cache[list][ scan8[4*i]+8 ]=h->ref_cache[list][ scan8[4*i]+9 ]= ref[list][i];
                }
            }
        }
        for(list=0; list<2; list++){
            for(i=0; i<4; i++){
                h->ref_cache[list][ scan8[4*i]   ]=h->ref_cache[list][ scan8[4*i]+1 ];
                if(IS_DIR(h->sub_mb_type[i], 0, list) && !IS_DIRECT(h->sub_mb_type[i])){
                    const int sub_mb_type= h->sub_mb_type[i];
                    const int block_width= (sub_mb_type & (MB_TYPE_16x16|MB_TYPE_16x8)) ? 2 : 1;
                    for(j=0; j<sub_partition_count[i]; j++){
                        int mpx, mpy;
                        int mx, my;
                        const int index= 4*i + block_width*j;
                        int16_t (* mv_cache)[2]= &h->mv_cache[list][ scan8[index] ];
                        int16_t (* mvd_cache)[2]= &h->mvd_cache[list][ scan8[index] ];
                        pred_motion(h, index, block_width, list, h->ref_cache[list][ scan8[index] ], &mpx, &mpy);
                        mx = mpx + decode_cabac_mb_mvd( h, list, index, 0 );
                        my = mpy + decode_cabac_mb_mvd( h, list, index, 1 );
                        tprintf("final mv:%d %d\n", mx, my);
                        if(IS_SUB_8X8(sub_mb_type)){
                            mv_cache[ 0 ][0]= mv_cache[ 1 ][0]=
                            mv_cache[ 8 ][0]= mv_cache[ 9 ][0]= mx;
                            mv_cache[ 0 ][1]= mv_cache[ 1 ][1]=
                            mv_cache[ 8 ][1]= mv_cache[ 9 ][1]= my;
                            mvd_cache[ 0 ][0]= mvd_cache[ 1 ][0]=
                            mvd_cache[ 8 ][0]= mvd_cache[ 9 ][0]= mx - mpx;
                            mvd_cache[ 0 ][1]= mvd_cache[ 1 ][1]=
                            mvd_cache[ 8 ][1]= mvd_cache[ 9 ][1]= my - mpy;
                        }else if(IS_SUB_8X4(sub_mb_type)){
                            mv_cache[ 0 ][0]= mv_cache[ 1 ][0]= mx;
                            mv_cache[ 0 ][1]= mv_cache[ 1 ][1]= my;
                            mvd_cache[ 0 ][0]= mvd_cache[ 1 ][0]= mx- mpx;
                            mvd_cache[ 0 ][1]= mvd_cache[ 1 ][1]= my - mpy;
                        }else if(IS_SUB_4X8(sub_mb_type)){
                            mv_cache[ 0 ][0]= mv_cache[ 8 ][0]= mx;
                            mv_cache[ 0 ][1]= mv_cache[ 8 ][1]= my;
                            mvd_cache[ 0 ][0]= mvd_cache[ 8 ][0]= mx - mpx;
                            mvd_cache[ 0 ][1]= mvd_cache[ 8 ][1]= my - mpy;
                        }else{
                            assert(IS_SUB_4X4(sub_mb_type));
                            mv_cache[ 0 ][0]= mx;
                            mv_cache[ 0 ][1]= my;
                            mvd_cache[ 0 ][0]= mx - mpx;
                            mvd_cache[ 0 ][1]= my - mpy;
                        }
                    }
                }else{
                    uint32_t *p= (uint32_t *)&h->mv_cache[list][ scan8[4*i] ][0];
                    uint32_t *pd= (uint32_t *)&h->mvd_cache[list][ scan8[4*i] ][0];
                    p[0] = p[1] = p[8] = p[9] = 0;
                    pd[0]= pd[1]= pd[8]= pd[9]= 0;
                }
            }
        }
    } else if( !IS_DIRECT(mb_type) ) {
    /*非直接预测模式*/
        int list, mx, my, i, mpx, mpy;
        if(IS_16X16(mb_type)){		/*16x16*/
            for(list=0; list<2; list++){
                if(IS_DIR(mb_type, 0, list)){
                    if(h->ref_count[list] > 0 ){
                        const int ref = h->ref_count[list] > 1 ? decode_cabac_mb_ref( h, list, 0 ) : 0;
                        fill_rectangle(&h->ref_cache[list][ scan8[0] ], 4, 4, 8, ref, 1);
                    }
                }
            }
            for(list=0; list<2; list++){
                if(IS_DIR(mb_type, 0, list)){
                    pred_motion(h, 0, 4, list, h->ref_cache[list][ scan8[0] ], &mpx, &mpy);
                    mx = mpx + decode_cabac_mb_mvd( h, list, 0, 0 );
                    my = mpy + decode_cabac_mb_mvd( h, list, 0, 1 );
                    tprintf("final mv:%d %d\n", mx, my);
                    fill_rectangle(h->mvd_cache[list][ scan8[0] ], 4, 4, 8, pack16to32(mx-mpx,my-mpy), 4);
                    fill_rectangle(h->mv_cache[list][ scan8[0] ], 4, 4, 8, pack16to32(mx,my), 4);
                }
            }
        }
        else if(IS_16X8(mb_type)){		/*16x8*/
            for(list=0; list<2; list++){
                if(h->ref_count[list]>0){
                    for(i=0; i<2; i++){
                        if(IS_DIR(mb_type, i, list)){
                            const int ref= h->ref_count[list] > 1 ? decode_cabac_mb_ref( h, list, 8*i ) : 0;
                            fill_rectangle(&h->ref_cache[list][ scan8[0] + 16*i ], 4, 2, 8, ref, 1);
                        }
                    }
                }
            }
            for(list=0; list<2; list++){
                for(i=0; i<2; i++){
                    if(IS_DIR(mb_type, i, list)){
                        pred_16x8_motion(h, 8*i, list, h->ref_cache[list][scan8[0] + 16*i], &mpx, &mpy);
                        mx = mpx + decode_cabac_mb_mvd( h, list, 8*i, 0 );
                        my = mpy + decode_cabac_mb_mvd( h, list, 8*i, 1 );
                        tprintf("final mv:%d %d\n", mx, my);
                        fill_rectangle(h->mvd_cache[list][ scan8[0] + 16*i ], 4, 2, 8, pack16to32(mx-mpx,my-mpy), 4);
                        fill_rectangle(h->mv_cache[list][ scan8[0] + 16*i ], 4, 2, 8, pack16to32(mx,my), 4);
                    }
                }
            }
        }else{/*8x16*/
            assert(IS_8X16(mb_type));
            for(list=0; list<2; list++){
                if(h->ref_count[list]>0){
                    for(i=0; i<2; i++){
                        if(IS_DIR(mb_type, i, list)){ //FIXME optimize
                            const int ref= h->ref_count[list] > 1 ? decode_cabac_mb_ref( h, list, 4*i ) : 0;
                            fill_rectangle(&h->ref_cache[list][ scan8[0] + 2*i ], 2, 4, 8, ref, 1);
                        }
                    }
                }
            }
            for(list=0; list<2; list++){
                for(i=0; i<2; i++){
                    if(IS_DIR(mb_type, i, list)){
                        pred_8x16_motion(h, i*4, list, h->ref_cache[list][ scan8[0] + 2*i ], &mpx, &mpy);
                        mx = mpx + decode_cabac_mb_mvd( h, list, 4*i, 0 );
                        my = mpy + decode_cabac_mb_mvd( h, list, 4*i, 1 );
                        tprintf("final mv:%d %d\n", mx, my);
                        fill_rectangle(h->mvd_cache[list][ scan8[0] + 2*i ], 2, 4, 8, pack16to32(mx-mpx,my-mpy), 4);
                        fill_rectangle(h->mv_cache[list][ scan8[0] + 2*i ], 2, 4, 8, pack16to32(mx,my), 4);
                    }
                }
            }
        }
    }
	
   if( IS_INTER( mb_type ) ) {
        h->chroma_pred_mode_table[mb_xy] = 0;
        write_back_motion( h, mb_type );
   }
    if( !IS_INTRA16x16( mb_type ) ) {
        cbp  = decode_cabac_mb_cbp_luma( h );
        cbp |= decode_cabac_mb_cbp_chroma( h ) << 4;
    }
    h->cbp_table[mb_xy] = cbp;
    if( cbp || IS_INTRA16x16( mb_type ) ) {
        const uint8_t *scan, *dc_scan;
        int dqp;
        if(IS_INTERLACED(mb_type)){
            scan= field_scan;
            dc_scan= luma_dc_field_scan;
        }else{
            scan= zigzag_scan;
            dc_scan= luma_dc_zigzag_scan;
        }
        h->last_qscale_diff = dqp = decode_cabac_mb_dqp( h );
        s->qscale += dqp;
        if(((unsigned)s->qscale) > 51){
            if(s->qscale<0) s->qscale+= 52;
            else            s->qscale-= 52;
        }
        h->chroma_qp = get_chroma_qp(h, s->qscale);
        if( IS_INTRA16x16( mb_type ) ) {
            int i;
            //av_log( s->avctx, AV_LOG_ERROR, "INTRA16x16 DC\n" );
            if( decode_cabac_residual( h, h->mb, 0, 0, dc_scan, s->qscale, 16) < 0)
                return -1;
            if( cbp&15 ) {
                for( i = 0; i < 16; i++ ) {
                    //av_log( s->avctx, AV_LOG_ERROR, "INTRA16x16 AC:%d\n", i );
                    if( decode_cabac_residual(h, h->mb + 16*i, 1, i, scan + 1, s->qscale, 15) < 0 )
                        return -1;
                }
            } else {
                fill_rectangle(&h->non_zero_count_cache[scan8[0]], 4, 4, 8, 0, 1);
            }
        } else {
            int i8x8, i4x4;
            for( i8x8 = 0; i8x8 < 4; i8x8++ ) {
                if( cbp & (1<<i8x8) ) {
                    for( i4x4 = 0; i4x4 < 4; i4x4++ ) {
                        const int index = 4*i8x8 + i4x4;
                        //av_log( s->avctx, AV_LOG_ERROR, "Luma4x4: %d\n", index );
                        if( decode_cabac_residual(h, h->mb + 16*index, 2, index, scan, s->qscale, 16) < 0 )
                            return -1;
                    }
                } else {
                    uint8_t * const nnz= &h->non_zero_count_cache[ scan8[4*i8x8] ];
                    nnz[0] = nnz[1] = nnz[8] = nnz[9] = 0;
                }
            }
        }
        if( cbp&0x30 ){
            int c;
            for( c = 0; c < 2; c++ ) {
                //av_log( s->avctx, AV_LOG_ERROR, "INTRA C%d-DC\n",c );
                if( decode_cabac_residual(h, h->mb + 256 + 16*4*c, 3, c, chroma_dc_scan, h->chroma_qp, 4) < 0)
                    return -1;
            }
        }
        if( cbp&0x20 ) {
            int c, i;
            for( c = 0; c < 2; c++ ) {
                for( i = 0; i < 4; i++ ) {
                    const int index = 16 + 4 * c + i;
                    //av_log( s->avctx, AV_LOG_ERROR, "INTRA C%d-AC %d\n",c, index - 16 );
                    if( decode_cabac_residual(h, h->mb + 16*index, 4, index - 16, scan + 1, h->chroma_qp, 15) < 0)
                        return -1;
                }
            }
        } else {
            uint8_t * const nnz= &h->non_zero_count_cache[0];
            nnz[ scan8[16]+0 ] = nnz[ scan8[16]+1 ] =nnz[ scan8[16]+8 ] =nnz[ scan8[16]+9 ] =
            nnz[ scan8[20]+0 ] = nnz[ scan8[20]+1 ] =nnz[ scan8[20]+8 ] =nnz[ scan8[20]+9 ] = 0;
        }
    } else {
        memset( &h->non_zero_count_cache[8], 0, 8*5 );
    }
    s->current_picture.qscale_table[mb_xy]= s->qscale;
    write_back_non_zero_count(h);
    return 0;
}


