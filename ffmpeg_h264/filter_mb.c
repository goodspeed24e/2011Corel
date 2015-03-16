
#include "h264.h"
#include "fill_func.h"
#include "h264data.h"


int get_chroma_qp(H264Context *h, int qscale){
    
    return chroma_qp[clip(qscale + h->pps.chroma_qp_index_offset, 0, 51)];
}

/**
 *
 */
static void filter_mb_edgev( H264Context *h, uint8_t *pix, int stride, int bS[4], int qp ) {
    int i, d;
    const int index_a = clip( qp + h->slice_alpha_c0_offset, 0, 51 );
    const int alpha = alpha_table[index_a];
    const int beta  = beta_table[clip( qp + h->slice_beta_offset, 0, 51 )];
    for( i = 0; i < 4; i++ ) {
        if( bS[i] == 0 ) {
            pix += 4 * stride;
            continue;
        }
        if( bS[i] < 4 ) {
            const int tc0 = tc0_table[index_a][bS[i] - 1];
            /* 4px edge length */
            for( d = 0; d < 4; d++ ) {
                const int p0 = pix[-1];
                const int p1 = pix[-2];
                const int p2 = pix[-3];
                const int q0 = pix[0];
                const int q1 = pix[1];
                const int q2 = pix[2];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    int tc = tc0;
                    int i_delta;
                    if( ABS( p2 - p0 ) < beta ) {
                        pix[-2] = p1 + clip( ( p2 + ( ( p0 + q0 + 1 ) >> 1 ) - ( p1 << 1 ) ) >> 1, -tc0, tc0 );
                        tc++;
                    }
                    if( ABS( q2 - q0 ) < beta ) {
                        pix[1] = q1 + clip( ( q2 + ( ( p0 + q0 + 1 ) >> 1 ) - ( q1 << 1 ) ) >> 1, -tc0, tc0 );
                        tc++;
                    }
                    i_delta = clip( (((q0 - p0 ) << 2) + (p1 - q1) + 4) >> 3, -tc, tc );
                    pix[-1] = clip_uint8( p0 + i_delta );    /* p0' */
                    pix[0]  = clip_uint8( q0 - i_delta );    /* q0' */
                }
                pix += stride;
            }
        }else{
            /* 4px edge length */
            for( d = 0; d < 4; d++ ) {
                const int p0 = pix[-1];
                const int p1 = pix[-2];
                const int p2 = pix[-3];
                const int q0 = pix[0];
                const int q1 = pix[1];
                const int q2 = pix[2];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    if(ABS( p0 - q0 ) < (( alpha >> 2 ) + 2 )){
                        if( ABS( p2 - p0 ) < beta)
                        {
                            const int p3 = pix[-4];
                            /* p0', p1', p2' */
                            pix[-1] = ( p2 + 2*p1 + 2*p0 + 2*q0 + q1 + 4 ) >> 3;
                            pix[-2] = ( p2 + p1 + p0 + q0 + 2 ) >> 2;
                            pix[-3] = ( 2*p3 + 3*p2 + p1 + p0 + q0 + 4 ) >> 3;
                        } else {
                            /* p0' */
                            pix[-1] = ( 2*p1 + p0 + q1 + 2 ) >> 2;
                        }
                        if( ABS( q2 - q0 ) < beta)
                        {
                            const int q3 = pix[3];
                            /* q0', q1', q2' */
                            pix[0] = ( p1 + 2*p0 + 2*q0 + 2*q1 + q2 + 4 ) >> 3;
                            pix[1] = ( p0 + q0 + q1 + q2 + 2 ) >> 2;
                            pix[2] = ( 2*q3 + 3*q2 + q1 + q0 + p0 + 4 ) >> 3;
                        } else {
                            /* q0' */
                            pix[0] = ( 2*q1 + q0 + p1 + 2 ) >> 2;
                        }
                    }else{
                        /* p0', q0' */
                        pix[-1] = ( 2*p1 + p0 + q1 + 2 ) >> 2;
                        pix[ 0] = ( 2*q1 + q0 + p1 + 2 ) >> 2;
                    }
                }
                pix += stride;
            }
        }
    }
}
static void filter_mb_edgecv( H264Context *h, uint8_t *pix, int stride, int bS[4], int qp ) {
    int i, d;
    const int index_a = clip( qp + h->slice_alpha_c0_offset, 0, 51 );
    const int alpha = alpha_table[index_a];
    const int beta  = beta_table[clip( qp + h->slice_beta_offset, 0, 51 )];
    for( i = 0; i < 4; i++ ) {
        if( bS[i] == 0 ) {
            pix += 2 * stride;
            continue;
        }
        if( bS[i] < 4 ) {
            const int tc = tc0_table[index_a][bS[i] - 1] + 1;
            /* 2px edge length (because we use same bS than the one for luma) */
            for( d = 0; d < 2; d++ ){
                const int p0 = pix[-1];
                const int p1 = pix[-2];
                const int q0 = pix[0];
                const int q1 = pix[1];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    const int i_delta = clip( (((q0 - p0 ) << 2) + (p1 - q1) + 4) >> 3, -tc, tc );
                    pix[-1] = clip_uint8( p0 + i_delta );    /* p0' */
                    pix[0]  = clip_uint8( q0 - i_delta );    /* q0' */
                }
                pix += stride;
            }
        }else{
            /* 2px edge length (because we use same bS than the one for luma) */
            for( d = 0; d < 2; d++ ){
                const int p0 = pix[-1];
                const int p1 = pix[-2];
                const int q0 = pix[0];
                const int q1 = pix[1];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    pix[-1] = ( 2*p1 + p0 + q1 + 2 ) >> 2;   /* p0' */
                    pix[0]  = ( 2*q1 + q0 + p1 + 2 ) >> 2;   /* q0' */
                }
                pix += stride;
            }
        }
    }
}
static void filter_mb_edgeh( H264Context *h, uint8_t *pix, int stride, int bS[4], int qp ) {
    int i, d;
    const int index_a = clip( qp + h->slice_alpha_c0_offset, 0, 51 );
    const int alpha = alpha_table[index_a];
    const int beta  = beta_table[clip( qp + h->slice_beta_offset, 0, 51 )];
    const int pix_next  = stride;
    for( i = 0; i < 4; i++ ) {
        if( bS[i] == 0 ) {
            pix += 4;
            continue;
        }
        if( bS[i] < 4 ) {
            const int tc0 = tc0_table[index_a][bS[i] - 1];
            /* 4px edge length */
            for( d = 0; d < 4; d++ ) {
                const int p0 = pix[-1*pix_next];
                const int p1 = pix[-2*pix_next];
                const int p2 = pix[-3*pix_next];
                const int q0 = pix[0];
                const int q1 = pix[1*pix_next];
                const int q2 = pix[2*pix_next];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    int tc = tc0;
                    int i_delta;
                    if( ABS( p2 - p0 ) < beta ) {
                        pix[-2*pix_next] = p1 + clip( ( p2 + ( ( p0 + q0 + 1 ) >> 1 ) - ( p1 << 1 ) ) >> 1, -tc0, tc0 );
                        tc++;
                    }
                    if( ABS( q2 - q0 ) < beta ) {
                        pix[pix_next] = q1 + clip( ( q2 + ( ( p0 + q0 + 1 ) >> 1 ) - ( q1 << 1 ) ) >> 1, -tc0, tc0 );
                        tc++;
                    }
                    i_delta = clip( (((q0 - p0 ) << 2) + (p1 - q1) + 4) >> 3, -tc, tc );
                    pix[-pix_next] = clip_uint8( p0 + i_delta );    /* p0' */
                    pix[0]         = clip_uint8( q0 - i_delta );    /* q0' */
                }
                pix++;
            }
        }else{
            /* 4px edge length */
            for( d = 0; d < 4; d++ ) {
                const int p0 = pix[-1*pix_next];
                const int p1 = pix[-2*pix_next];
                const int p2 = pix[-3*pix_next];
                const int q0 = pix[0];
                const int q1 = pix[1*pix_next];
                const int q2 = pix[2*pix_next];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    const int p3 = pix[-4*pix_next];
                    const int q3 = pix[ 3*pix_next];
                    if(ABS( p0 - q0 ) < (( alpha >> 2 ) + 2 )){
                        if( ABS( p2 - p0 ) < beta) {
                            /* p0', p1', p2' */
                            pix[-1*pix_next] = ( p2 + 2*p1 + 2*p0 + 2*q0 + q1 + 4 ) >> 3;
                            pix[-2*pix_next] = ( p2 + p1 + p0 + q0 + 2 ) >> 2;
                            pix[-3*pix_next] = ( 2*p3 + 3*p2 + p1 + p0 + q0 + 4 ) >> 3;
                        } else {
                            /* p0' */
                            pix[-1*pix_next] = ( 2*p1 + p0 + q1 + 2 ) >> 2;
                        }
                        if( ABS( q2 - q0 ) < beta) {
                            /* q0', q1', q2' */
                            pix[0*pix_next] = ( p1 + 2*p0 + 2*q0 + 2*q1 + q2 + 4 ) >> 3;
                            pix[1*pix_next] = ( p0 + q0 + q1 + q2 + 2 ) >> 2;
                            pix[2*pix_next] = ( 2*q3 + 3*q2 + q1 + q0 + p0 + 4 ) >> 3;
                        } else {
                            /* q0' */
                            pix[0*pix_next] = ( 2*q1 + q0 + p1 + 2 ) >> 2;
                        }
                    }else{
                        /* p0', q0' */
                        pix[-1*pix_next] = ( 2*p1 + p0 + q1 + 2 ) >> 2;
                        pix[ 0*pix_next] = ( 2*q1 + q0 + p1 + 2 ) >> 2;
                    }
                }
                pix++;
            }
        }
    }
}
static void filter_mb_edgech( H264Context *h, uint8_t *pix, int stride, int bS[4], int qp ) {
    int i, d;
    const int index_a = clip( qp + h->slice_alpha_c0_offset, 0, 51 );
    const int alpha = alpha_table[index_a];
    const int beta  = beta_table[clip( qp + h->slice_beta_offset, 0, 51 )];
    const int pix_next  = stride;
    for( i = 0; i < 4; i++ )
    {
        if( bS[i] == 0 ) {
            pix += 2;
            continue;
        }
        if( bS[i] < 4 ) {
            int tc = tc0_table[index_a][bS[i] - 1] + 1;
            /* 2px edge length (see deblocking_filter_edgecv) */
            for( d = 0; d < 2; d++ ) {
                const int p0 = pix[-1*pix_next];
                const int p1 = pix[-2*pix_next];
                const int q0 = pix[0];
                const int q1 = pix[1*pix_next];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    int i_delta = clip( (((q0 - p0 ) << 2) + (p1 - q1) + 4) >> 3, -tc, tc );
                    pix[-pix_next] = clip_uint8( p0 + i_delta );    /* p0' */
                    pix[0]         = clip_uint8( q0 - i_delta );    /* q0' */
                }
                pix++;
            }
        }else{
            /* 2px edge length (see deblocking_filter_edgecv) */
            for( d = 0; d < 2; d++ ) {
                const int p0 = pix[-1*pix_next];
                const int p1 = pix[-2*pix_next];
                const int q0 = pix[0];
                const int q1 = pix[1*pix_next];
                if( ABS( p0 - q0 ) < alpha &&
                    ABS( p1 - p0 ) < beta &&
                    ABS( q1 - q0 ) < beta ) {
                    pix[-pix_next] = ( 2*p1 + p0 + q1 + 2 ) >> 2;   /* p0' */
                    pix[0]         = ( 2*q1 + q0 + p1 + 2 ) >> 2;   /* q0' */
                }
                pix++;
            }
        }
    }
}
void filter_mb( H264Context *h, int mb_x, int mb_y, uint8_t *img_y, uint8_t *img_cb, uint8_t *img_cr) {
    MpegEncContext * const s = &h->s;
    const int mb_xy= mb_x + mb_y*s->mb_stride;
    int linesize, uvlinesize;
    int dir;
    /* FIXME Implement deblocking filter for field MB */
    if( h->sps.mb_aff ) {
        return;
    }
    linesize = s->linesize;
    uvlinesize = s->uvlinesize;
    /* dir : 0 -> vertical edge, 1 -> horizontal edge */
    for( dir = 0; dir < 2; dir++ )
    {
        int start = 0;
        int edge;
        /* test picture boundary */
        if( ( dir == 0 && mb_x == 0 ) || ( dir == 1 && mb_y == 0 ) ) {
            start = 1;
        }
        /* FIXME test slice boundary */
        if( h->deblocking_filter == 2 ) {
        }
        /* Calculate bS */
        for( edge = start; edge < 4; edge++ ) {
            /* mbn_xy: neighbour macroblock (how that works for field ?) */
            int mbn_xy = edge > 0 ? mb_xy : ( dir == 0 ? mb_xy -1 : mb_xy - s->mb_stride );
            int bS[4];
            int qp;
            if( IS_INTRA( s->current_picture.mb_type[mb_xy] ) ||
                IS_INTRA( s->current_picture.mb_type[mbn_xy] ) ) {
                bS[0] = bS[1] = bS[2] = bS[3] = ( edge == 0 ? 4 : 3 );
            } else {
                int i;
                for( i = 0; i < 4; i++ ) {
                    int x = dir == 0 ? edge : i;
                    int y = dir == 0 ? i    : edge;
                    int b_idx= 8 + 4 + x + 8*y;
                    int bn_idx= b_idx - (dir ? 8:1);
                    if( h->non_zero_count_cache[b_idx] != 0 ||
                        h->non_zero_count_cache[bn_idx] != 0 ) {
                        bS[i] = 2;
                    }
                    else if( h->slice_type == P_TYPE ) {
                        if( h->ref_cache[0][b_idx] != h->ref_cache[0][bn_idx] ||
                            ABS( h->mv_cache[0][b_idx][0] - h->mv_cache[0][bn_idx][0] ) >= 4 ||
                            ABS( h->mv_cache[0][b_idx][1] - h->mv_cache[0][bn_idx][1] ) >= 4 )
                            bS[i] = 1;
                        else
                            bS[i] = 0;
                    }
                    else {
                        /* FIXME Add support for B frame */
                        return;
                    }
                }
                if(bS[0]+bS[1]+bS[2]+bS[3] == 0)
                    continue;
            }
            /* Filter edge */
            qp = ( s->qscale + s->current_picture.qscale_table[mbn_xy] + 1 ) >> 1;
            if( dir == 0 ) {
                filter_mb_edgev( h, &img_y[4*edge], linesize, bS, qp );
                if( (edge&1) == 0 ) {
                    int chroma_qp = ( h->chroma_qp +
                                      get_chroma_qp( h, s->current_picture.qscale_table[mbn_xy] ) + 1 ) >> 1;
                    filter_mb_edgecv( h, &img_cb[2*edge], uvlinesize, bS, chroma_qp );
                    filter_mb_edgecv( h, &img_cr[2*edge], uvlinesize, bS, chroma_qp );
                }
            } else {
                filter_mb_edgeh( h, &img_y[4*edge*linesize], linesize, bS, qp );
                if( (edge&1) == 0 ) {
                    int chroma_qp = ( h->chroma_qp +
                                      get_chroma_qp( h, s->current_picture.qscale_table[mbn_xy] ) + 1 ) >> 1;
                    filter_mb_edgech( h, &img_cb[2*edge*uvlinesize], uvlinesize, bS, chroma_qp );
                    filter_mb_edgech( h, &img_cr[2*edge*uvlinesize], uvlinesize, bS, chroma_qp );
                }
            }
        }
    }
}

