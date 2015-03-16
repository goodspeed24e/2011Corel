
#include <assert.h>
#include "h264.h"
#include "dsputil.h"
#include "h264context.h"
#include "h264data.h"

static void pred4x4_vertical_c(uint8_t *src, uint8_t *topright, int stride){
    const uint32_t a= ((uint32_t*)(src-stride))[0];
    ((uint32_t*)(src+0*stride))[0]= a;
    ((uint32_t*)(src+1*stride))[0]= a;
    ((uint32_t*)(src+2*stride))[0]= a;
    ((uint32_t*)(src+3*stride))[0]= a;
}
static void pred4x4_horizontal_c(uint8_t *src, uint8_t *topright, int stride){
    ((uint32_t*)(src+0*stride))[0]= src[-1+0*stride]*0x01010101;
    ((uint32_t*)(src+1*stride))[0]= src[-1+1*stride]*0x01010101;
    ((uint32_t*)(src+2*stride))[0]= src[-1+2*stride]*0x01010101;
    ((uint32_t*)(src+3*stride))[0]= src[-1+3*stride]*0x01010101;
}
static void pred4x4_dc_c(uint8_t *src, uint8_t *topright, int stride){
    const int dc= (  src[-stride] + src[1-stride] + src[2-stride] + src[3-stride]
                   + src[-1+0*stride] + src[-1+1*stride] + src[-1+2*stride] + src[-1+3*stride] + 4) >>3;
    
    ((uint32_t*)(src+0*stride))[0]= 
    ((uint32_t*)(src+1*stride))[0]= 
    ((uint32_t*)(src+2*stride))[0]= 
    ((uint32_t*)(src+3*stride))[0]= dc* 0x01010101; 
}
static void pred4x4_left_dc_c(uint8_t *src, uint8_t *topright, int stride){
    const int dc= (  src[-1+0*stride] + src[-1+1*stride] + src[-1+2*stride] + src[-1+3*stride] + 2) >>2;
    
    ((uint32_t*)(src+0*stride))[0]= 
    ((uint32_t*)(src+1*stride))[0]= 
    ((uint32_t*)(src+2*stride))[0]= 
    ((uint32_t*)(src+3*stride))[0]= dc* 0x01010101; 
}
static void pred4x4_top_dc_c(uint8_t *src, uint8_t *topright, int stride){
    const int dc= (  src[-stride] + src[1-stride] + src[2-stride] + src[3-stride] + 2) >>2;
    
    ((uint32_t*)(src+0*stride))[0]= 
    ((uint32_t*)(src+1*stride))[0]= 
    ((uint32_t*)(src+2*stride))[0]= 
    ((uint32_t*)(src+3*stride))[0]= dc* 0x01010101; 
}
static void pred4x4_128_dc_c(uint8_t *src, uint8_t *topright, int stride){
    ((uint32_t*)(src+0*stride))[0]= 
    ((uint32_t*)(src+1*stride))[0]= 
    ((uint32_t*)(src+2*stride))[0]= 
    ((uint32_t*)(src+3*stride))[0]= 128U*0x01010101U;
}
#define LOAD_TOP_RIGHT_EDGE\
    const int t4= topright[0];\
    const int t5= topright[1];\
    const int t6= topright[2];\
    const int t7= topright[3];\

#define LOAD_LEFT_EDGE\
    const int l0= src[-1+0*stride];\
    const int l1= src[-1+1*stride];\
    const int l2= src[-1+2*stride];\
    const int l3= src[-1+3*stride];\

#define LOAD_TOP_EDGE\
    const int t0= src[ 0-1*stride];\
    const int t1= src[ 1-1*stride];\
    const int t2= src[ 2-1*stride];\
    const int t3= src[ 3-1*stride];\

static void pred4x4_down_right_c(uint8_t *src, uint8_t *topright, int stride){
    const int lt= src[-1-1*stride];
    LOAD_TOP_EDGE
    LOAD_LEFT_EDGE
    src[0+3*stride]=(l3 + 2*l2 + l1 + 2)>>2; 
    src[0+2*stride]=
    src[1+3*stride]=(l2 + 2*l1 + l0 + 2)>>2; 
    src[0+1*stride]=
    src[1+2*stride]=
    src[2+3*stride]=(l1 + 2*l0 + lt + 2)>>2; 
    src[0+0*stride]=
    src[1+1*stride]=
    src[2+2*stride]=
    src[3+3*stride]=(l0 + 2*lt + t0 + 2)>>2; 
    src[1+0*stride]=
    src[2+1*stride]=
    src[3+2*stride]=(lt + 2*t0 + t1 + 2)>>2;
    src[2+0*stride]=
    src[3+1*stride]=(t0 + 2*t1 + t2 + 2)>>2;
    src[3+0*stride]=(t1 + 2*t2 + t3 + 2)>>2;
}
static void pred4x4_down_left_c(uint8_t *src, uint8_t *topright, int stride){
    LOAD_TOP_EDGE    
    LOAD_TOP_RIGHT_EDGE    
//    LOAD_LEFT_EDGE    
    src[0+0*stride]=(t0 + t2 + 2*t1 + 2)>>2;
    src[1+0*stride]=
    src[0+1*stride]=(t1 + t3 + 2*t2 + 2)>>2;
    src[2+0*stride]=
    src[1+1*stride]=
    src[0+2*stride]=(t2 + t4 + 2*t3 + 2)>>2;
    src[3+0*stride]=
    src[2+1*stride]=
    src[1+2*stride]=
    src[0+3*stride]=(t3 + t5 + 2*t4 + 2)>>2;
    src[3+1*stride]=
    src[2+2*stride]=
    src[1+3*stride]=(t4 + t6 + 2*t5 + 2)>>2;
    src[3+2*stride]=
    src[2+3*stride]=(t5 + t7 + 2*t6 + 2)>>2;
    src[3+3*stride]=(t6 + 3*t7 + 2)>>2;
}
static void pred4x4_vertical_right_c(uint8_t *src, uint8_t *topright, int stride){
    const int lt= src[-1-1*stride];
    LOAD_TOP_EDGE    
    LOAD_LEFT_EDGE    
//    const __attribute__((unused)) int unu= l3;
    const int unu= l3;
    src[0+0*stride]=
    src[1+2*stride]=(lt + t0 + 1)>>1;
    src[1+0*stride]=
    src[2+2*stride]=(t0 + t1 + 1)>>1;
    src[2+0*stride]=
    src[3+2*stride]=(t1 + t2 + 1)>>1;
    src[3+0*stride]=(t2 + t3 + 1)>>1;
    src[0+1*stride]=
    src[1+3*stride]=(l0 + 2*lt + t0 + 2)>>2;
    src[1+1*stride]=
    src[2+3*stride]=(lt + 2*t0 + t1 + 2)>>2;
    src[2+1*stride]=
    src[3+3*stride]=(t0 + 2*t1 + t2 + 2)>>2;
    src[3+1*stride]=(t1 + 2*t2 + t3 + 2)>>2;
    src[0+2*stride]=(lt + 2*l0 + l1 + 2)>>2;
    src[0+3*stride]=(l0 + 2*l1 + l2 + 2)>>2;
}
static void pred4x4_vertical_left_c(uint8_t *src, uint8_t *topright, int stride){
    LOAD_TOP_EDGE    
    LOAD_TOP_RIGHT_EDGE    
//    const __attribute__((unused)) int unu= t7;
    const int unu= t7;
    src[0+0*stride]=(t0 + t1 + 1)>>1;
    src[1+0*stride]=
    src[0+2*stride]=(t1 + t2 + 1)>>1;
    src[2+0*stride]=
    src[1+2*stride]=(t2 + t3 + 1)>>1;
    src[3+0*stride]=
    src[2+2*stride]=(t3 + t4+ 1)>>1;
    src[3+2*stride]=(t4 + t5+ 1)>>1;
    src[0+1*stride]=(t0 + 2*t1 + t2 + 2)>>2;
    src[1+1*stride]=
    src[0+3*stride]=(t1 + 2*t2 + t3 + 2)>>2;
    src[2+1*stride]=
    src[1+3*stride]=(t2 + 2*t3 + t4 + 2)>>2;
    src[3+1*stride]=
    src[2+3*stride]=(t3 + 2*t4 + t5 + 2)>>2;
    src[3+3*stride]=(t4 + 2*t5 + t6 + 2)>>2;
}
static void pred4x4_horizontal_up_c(uint8_t *src, uint8_t *topright, int stride){
    LOAD_LEFT_EDGE    
    src[0+0*stride]=(l0 + l1 + 1)>>1;
    src[1+0*stride]=(l0 + 2*l1 + l2 + 2)>>2;
    src[2+0*stride]=
    src[0+1*stride]=(l1 + l2 + 1)>>1;
    src[3+0*stride]=
    src[1+1*stride]=(l1 + 2*l2 + l3 + 2)>>2;
    src[2+1*stride]=
    src[0+2*stride]=(l2 + l3 + 1)>>1;
    src[3+1*stride]=
    src[1+2*stride]=(l2 + 2*l3 + l3 + 2)>>2;
    src[3+2*stride]=
    src[1+3*stride]=
    src[0+3*stride]=
    src[2+2*stride]=
    src[2+3*stride]=
    src[3+3*stride]=l3;
}
    
static void pred4x4_horizontal_down_c(uint8_t *src, uint8_t *topright, int stride){
    const int lt= src[-1-1*stride];
    LOAD_TOP_EDGE    
    LOAD_LEFT_EDGE    
//    const __attribute__((unused)) int unu= t3;
    const int unu= t3;
    src[0+0*stride]=
    src[2+1*stride]=(lt + l0 + 1)>>1;
    src[1+0*stride]=
    src[3+1*stride]=(l0 + 2*lt + t0 + 2)>>2;
    src[2+0*stride]=(lt + 2*t0 + t1 + 2)>>2;
    src[3+0*stride]=(t0 + 2*t1 + t2 + 2)>>2;
    src[0+1*stride]=
    src[2+2*stride]=(l0 + l1 + 1)>>1;
    src[1+1*stride]=
    src[3+2*stride]=(lt + 2*l0 + l1 + 2)>>2;
    src[0+2*stride]=
    src[2+3*stride]=(l1 + l2+ 1)>>1;
    src[1+2*stride]=
    src[3+3*stride]=(l0 + 2*l1 + l2 + 2)>>2;
    src[0+3*stride]=(l2 + l3 + 1)>>1;
    src[1+3*stride]=(l1 + 2*l2 + l3 + 2)>>2;
}
static void pred16x16_vertical_c(uint8_t *src, int stride){
    int i;
    const uint32_t a= ((uint32_t*)(src-stride))[0];
    const uint32_t b= ((uint32_t*)(src-stride))[1];
    const uint32_t c= ((uint32_t*)(src-stride))[2];
    const uint32_t d= ((uint32_t*)(src-stride))[3];
    
    for(i=0; i<16; i++){
        ((uint32_t*)(src+i*stride))[0]= a;
        ((uint32_t*)(src+i*stride))[1]= b;
        ((uint32_t*)(src+i*stride))[2]= c;
        ((uint32_t*)(src+i*stride))[3]= d;
    }
}
static void pred16x16_horizontal_c(uint8_t *src, int stride){
    int i;
    for(i=0; i<16; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]=
        ((uint32_t*)(src+i*stride))[2]=
        ((uint32_t*)(src+i*stride))[3]= src[-1+i*stride]*0x01010101;
    }
}
static void pred16x16_dc_c(uint8_t *src, int stride){
    int i, dc=0;
    for(i=0;i<16; i++){
        dc+= src[-1+i*stride];
    }
    
    for(i=0;i<16; i++){
        dc+= src[i-stride];
    }
    dc= 0x01010101*((dc + 16)>>5);
    for(i=0; i<16; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]=
        ((uint32_t*)(src+i*stride))[2]=
        ((uint32_t*)(src+i*stride))[3]= dc;
    }
}
static void pred16x16_left_dc_c(uint8_t *src, int stride){
    int i, dc=0;
    for(i=0;i<16; i++){
        dc+= src[-1+i*stride];
    }
    
    dc= 0x01010101*((dc + 8)>>4);
    for(i=0; i<16; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]=
        ((uint32_t*)(src+i*stride))[2]=
        ((uint32_t*)(src+i*stride))[3]= dc;
    }
}
static void pred16x16_top_dc_c(uint8_t *src, int stride){
    int i, dc=0;
    for(i=0;i<16; i++){
        dc+= src[i-stride];
    }
    dc= 0x01010101*((dc + 8)>>4);
    for(i=0; i<16; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]=
        ((uint32_t*)(src+i*stride))[2]=
        ((uint32_t*)(src+i*stride))[3]= dc;
    }
}
static void pred16x16_128_dc_c(uint8_t *src, int stride){
    int i;
    for(i=0; i<16; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]=
        ((uint32_t*)(src+i*stride))[2]=
        ((uint32_t*)(src+i*stride))[3]= 0x01010101U*128U;
    }
}
static inline void pred16x16_plane_compat_c(uint8_t *src, int stride, const int svq3){
  int i, j, k;
  int a;
  uint8_t *cm = cropTbl + MAX_NEG_CROP;
  const uint8_t * const src0 = src+7-stride;
  const uint8_t *src1 = src+8*stride-1;
  const uint8_t *src2 = src1-2*stride;      // == src+6*stride-1;
  int H = src0[1] - src0[-1];
  int V = src1[0] - src2[ 0];
  for(k=2; k<=8; ++k) {
    src1 += stride; src2 -= stride;
    H += k*(src0[k] - src0[-k]);
    V += k*(src1[0] - src2[ 0]);
  }
  if(svq3){
    H = ( 5*(H/4) ) / 16;
    V = ( 5*(V/4) ) / 16;
    /* required for 100% accuracy */
    i = H; H = V; V = i;
  }else{
    H = ( 5*H+32 ) >> 6;
    V = ( 5*V+32 ) >> 6;
  }
  a = 16*(src1[0] + src2[16] + 1) - 7*(V+H);
  for(j=16; j>0; --j) {
    int b = a;
    a += V;
    for(i=-16; i<0; i+=4) {
      src[16+i] = cm[ (b    ) >> 5 ];
      src[17+i] = cm[ (b+  H) >> 5 ];
      src[18+i] = cm[ (b+2*H) >> 5 ];
      src[19+i] = cm[ (b+3*H) >> 5 ];
      b += 4*H;
    }
    src += stride;
  }
}
static void pred16x16_plane_c(uint8_t *src, int stride){
    pred16x16_plane_compat_c(src, stride, 0);
}
static void pred8x8_vertical_c(uint8_t *src, int stride){
    int i;
    const uint32_t a= ((uint32_t*)(src-stride))[0];
    const uint32_t b= ((uint32_t*)(src-stride))[1];
    
    for(i=0; i<8; i++){
        ((uint32_t*)(src+i*stride))[0]= a;
        ((uint32_t*)(src+i*stride))[1]= b;
    }
}
static void pred8x8_horizontal_c(uint8_t *src, int stride){
    int i;
    for(i=0; i<8; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]= src[-1+i*stride]*0x01010101;
    }
}
static void pred8x8_128_dc_c(uint8_t *src, int stride){
    int i;
    for(i=0; i<4; i++){
        ((uint32_t*)(src+i*stride))[0]= 
        ((uint32_t*)(src+i*stride))[1]= 0x01010101U*128U;
    }
    for(i=4; i<8; i++){
        ((uint32_t*)(src+i*stride))[0]= 
        ((uint32_t*)(src+i*stride))[1]= 0x01010101U*128U;
    }
}
static void pred8x8_left_dc_c(uint8_t *src, int stride){
    int i;
    int dc0, dc2;
    dc0=dc2=0;
    for(i=0;i<4; i++){
        dc0+= src[-1+i*stride];
        dc2+= src[-1+(i+4)*stride];
    }
    dc0= 0x01010101*((dc0 + 2)>>2);
    dc2= 0x01010101*((dc2 + 2)>>2);
    for(i=0; i<4; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]= dc0;
    }
    for(i=4; i<8; i++){
        ((uint32_t*)(src+i*stride))[0]=
        ((uint32_t*)(src+i*stride))[1]= dc2;
    }
}
static void pred8x8_top_dc_c(uint8_t *src, int stride){
    int i;
    int dc0, dc1;
    dc0=dc1=0;
    for(i=0;i<4; i++){
        dc0+= src[i-stride];
        dc1+= src[4+i-stride];
    }
    dc0= 0x01010101*((dc0 + 2)>>2);
    dc1= 0x01010101*((dc1 + 2)>>2);
    for(i=0; i<4; i++){
        ((uint32_t*)(src+i*stride))[0]= dc0;
        ((uint32_t*)(src+i*stride))[1]= dc1;
    }
    for(i=4; i<8; i++){
        ((uint32_t*)(src+i*stride))[0]= dc0;
        ((uint32_t*)(src+i*stride))[1]= dc1;
    }
}
static void pred8x8_dc_c(uint8_t *src, int stride){
    int i;
    int dc0, dc1, dc2, dc3;
    dc0=dc1=dc2=0;
    for(i=0;i<4; i++){
        dc0+= src[-1+i*stride] + src[i-stride];
        dc1+= src[4+i-stride];
        dc2+= src[-1+(i+4)*stride];
    }
    dc3= 0x01010101*((dc1 + dc2 + 4)>>3);
    dc0= 0x01010101*((dc0 + 4)>>3);
    dc1= 0x01010101*((dc1 + 2)>>2);
    dc2= 0x01010101*((dc2 + 2)>>2);
    for(i=0; i<4; i++){
        ((uint32_t*)(src+i*stride))[0]= dc0;
        ((uint32_t*)(src+i*stride))[1]= dc1;
    }
    for(i=4; i<8; i++){
        ((uint32_t*)(src+i*stride))[0]= dc2;
        ((uint32_t*)(src+i*stride))[1]= dc3;
    }
}
static void pred8x8_plane_c(uint8_t *src, int stride){
  int j, k;
  int a;
  uint8_t *cm = cropTbl + MAX_NEG_CROP;
  const uint8_t * const src0 = src+3-stride;
  const uint8_t *src1 = src+4*stride-1;
  const uint8_t *src2 = src1-2*stride;      // == src+2*stride-1;
  int H = src0[1] - src0[-1];
  int V = src1[0] - src2[ 0];
  for(k=2; k<=4; ++k) {
    src1 += stride; src2 -= stride;
    H += k*(src0[k] - src0[-k]);
    V += k*(src1[0] - src2[ 0]);
  }
  H = ( 17*H+16 ) >> 5;
  V = ( 17*V+16 ) >> 5;
  a = 16*(src1[0] + src2[8]+1) - 3*(V+H);
  for(j=8; j>0; --j) {
    int b = a;
    a += V;
    src[0] = cm[ (b    ) >> 5 ];
    src[1] = cm[ (b+  H) >> 5 ];
    src[2] = cm[ (b+2*H) >> 5 ];
    src[3] = cm[ (b+3*H) >> 5 ];
    src[4] = cm[ (b+4*H) >> 5 ];
    src[5] = cm[ (b+5*H) >> 5 ];
    src[6] = cm[ (b+6*H) >> 5 ];
    src[7] = cm[ (b+7*H) >> 5 ];
    src += stride;
  }
}



 void write_back_intra_pred_mode(H264Context *h){
    MpegEncContext * const s = &h->s;
    const int mb_xy= s->mb_x + s->mb_y*s->mb_stride;
    h->intra4x4_pred_mode[mb_xy][0]= h->intra4x4_pred_mode_cache[7+8*1];
    h->intra4x4_pred_mode[mb_xy][1]= h->intra4x4_pred_mode_cache[7+8*2];
    h->intra4x4_pred_mode[mb_xy][2]= h->intra4x4_pred_mode_cache[7+8*3];
    h->intra4x4_pred_mode[mb_xy][3]= h->intra4x4_pred_mode_cache[7+8*4];
    h->intra4x4_pred_mode[mb_xy][4]= h->intra4x4_pred_mode_cache[4+8*4];
    h->intra4x4_pred_mode[mb_xy][5]= h->intra4x4_pred_mode_cache[5+8*4];
    h->intra4x4_pred_mode[mb_xy][6]= h->intra4x4_pred_mode_cache[6+8*4];
}
/**
 * checks if the top & left blocks are available if needed & changes the dc mode so it only uses the available blocks.
 */
int check_intra4x4_pred_mode(H264Context *h){
    MpegEncContext * const s = &h->s;
    static const int8_t top [12]= {-1, 0,LEFT_DC_PRED,-1,-1,-1,-1,-1, 0};
    static const int8_t left[12]= { 0,-1, TOP_DC_PRED, 0,-1,-1,-1, 0,-1,DC_128_PRED};
    int i;
    
    if(!(h->top_samples_available&0x8000)){
        for(i=0; i<4; i++){
            int status= top[ h->intra4x4_pred_mode_cache[scan8[0] + i] ];
            if(status<0){
                av_log(h->s.avctx, AV_LOG_ERROR, "top block unavailable for requested intra4x4 mode %d at %d %d\n", status, s->mb_x, s->mb_y);
                return -1;
            } else if(status){
                h->intra4x4_pred_mode_cache[scan8[0] + i]= status;
            }
        }
    }
    
    if(!(h->left_samples_available&0x8000)){
        for(i=0; i<4; i++){
            int status= left[ h->intra4x4_pred_mode_cache[scan8[0] + 8*i] ];
            if(status<0){
                av_log(h->s.avctx, AV_LOG_ERROR, "left block unavailable for requested intra4x4 mode %d at %d %d\n", status, s->mb_x, s->mb_y);
                return -1;
            } else if(status){
                h->intra4x4_pred_mode_cache[scan8[0] + 8*i]= status;
            }
        }
    }
    return 0;
} //FIXME cleanup like next
/**
 * checks if the top & left blocks are available if needed & changes the dc mode so it only uses the available blocks.
 */
int check_intra_pred_mode(H264Context *h, int mode){
    MpegEncContext * const s = &h->s;
    static const int8_t top [7]= {LEFT_DC_PRED8x8, 1,-1,-1};
    static const int8_t left[7]= { TOP_DC_PRED8x8,-1, 2,-1,DC_128_PRED8x8};
    
    if(!(h->top_samples_available&0x8000)){
        mode= top[ mode ];
        if(mode<0){
            av_log(h->s.avctx, AV_LOG_ERROR, "top block unavailable for requested intra mode at %d %d\n", s->mb_x, s->mb_y);
            return -1;
        }
    }
    
    if(!(h->left_samples_available&0x8000)){
        mode= left[ mode ];
        if(mode<0){
            av_log(h->s.avctx, AV_LOG_ERROR, "left block unavailable for requested intra mode at %d %d\n", s->mb_x, s->mb_y);
            return -1;
        } 
    }
    return mode;
}
/**
 * gets the predicted intra4x4 prediction mode.
 */
int pred_intra_mode(H264Context *h, int n){
    const int index8= scan8[n];
    const int left= h->intra4x4_pred_mode_cache[index8 - 1];
    const int top = h->intra4x4_pred_mode_cache[index8 - 8];
    const int min= FFMIN(left, top);
    tprintf("mode:%d %d min:%d\n", left ,top, min);
    if(min<0) return DC_PRED;
    else      return min;
}
void write_back_non_zero_count(H264Context *h){
    MpegEncContext * const s = &h->s;
    const int mb_xy= s->mb_x + s->mb_y*s->mb_stride;
    h->non_zero_count[mb_xy][0]= h->non_zero_count_cache[4+8*4];
    h->non_zero_count[mb_xy][1]= h->non_zero_count_cache[5+8*4];
    h->non_zero_count[mb_xy][2]= h->non_zero_count_cache[6+8*4];
    h->non_zero_count[mb_xy][3]= h->non_zero_count_cache[7+8*4];
    h->non_zero_count[mb_xy][4]= h->non_zero_count_cache[7+8*3];
    h->non_zero_count[mb_xy][5]= h->non_zero_count_cache[7+8*2];
    h->non_zero_count[mb_xy][6]= h->non_zero_count_cache[7+8*1];
    
    h->non_zero_count[mb_xy][7]= h->non_zero_count_cache[1+8*2];
    h->non_zero_count[mb_xy][8]= h->non_zero_count_cache[2+8*2];
    h->non_zero_count[mb_xy][9]= h->non_zero_count_cache[2+8*1];
    h->non_zero_count[mb_xy][10]=h->non_zero_count_cache[1+8*5];
    h->non_zero_count[mb_xy][11]=h->non_zero_count_cache[2+8*5];
    h->non_zero_count[mb_xy][12]=h->non_zero_count_cache[2+8*4];
}
/**
 * gets the predicted number of non zero coefficients.
 * @param n block index
 */
int pred_non_zero_count(H264Context *h, int n){
    const int index8= scan8[n];
    const int left= h->non_zero_count_cache[index8 - 1];
    const int top = h->non_zero_count_cache[index8 - 8];
    int i= left + top;
    
    if(i<64) i= (i+1)>>1;
    tprintf("pred_nnz L%X T%X n%d s%d P%X\n", left, top, n, scan8[n], i&31);
    return i&31;
}

 int fetch_diagonal_mv(H264Context *h, const int16_t **C, int i, int list, int part_width){
    const int topright_ref= h->ref_cache[list][ i - 8 + part_width ];
    if(topright_ref != PART_NOT_AVAILABLE){
        *C= h->mv_cache[list][ i - 8 + part_width ];
        return topright_ref;
    }else{
        tprintf("topright MV not available\n");
        *C= h->mv_cache[list][ i - 8 - 1 ];
        return h->ref_cache[list][ i - 8 - 1 ];
    }
}
/**
 * gets the predicted MV.
 * @param n the block index
 * @param part_width the width of the partition (4, 8,16) -> (1, 2, 4)
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */
void pred_motion(H264Context * const h, int n, int part_width, int list, int ref, int * const mx, int * const my){
    const int index8= scan8[n];
    const int top_ref=      h->ref_cache[list][ index8 - 8 ];
    const int left_ref=     h->ref_cache[list][ index8 - 1 ];
    const int16_t * const A= h->mv_cache[list][ index8 - 1 ];
    const int16_t * const B= h->mv_cache[list][ index8 - 8 ];
    const int16_t * C;
    int diagonal_ref, match_count;
    assert(part_width==1 || part_width==2 || part_width==4);
/* mv_cache
  B . . A T T T T 
  U . . L . . , .
  U . . L . . . .
  U . . L . . , .
  . . . L . . . .
*/
    diagonal_ref= fetch_diagonal_mv(h, &C, index8, list, part_width);
    match_count= (diagonal_ref==ref) + (top_ref==ref) + (left_ref==ref);
    if(match_count > 1){ //most common
        *mx= mid_pred(A[0], B[0], C[0]);
        *my= mid_pred(A[1], B[1], C[1]);
    }else if(match_count==1){
        if(left_ref==ref){
            *mx= A[0];
            *my= A[1];        
        }else if(top_ref==ref){
            *mx= B[0];
            *my= B[1];        
        }else{
            *mx= C[0];
            *my= C[1];        
        }
    }else{
        if(top_ref == PART_NOT_AVAILABLE && diagonal_ref == PART_NOT_AVAILABLE && left_ref != PART_NOT_AVAILABLE){
            *mx= A[0];
            *my= A[1];        
        }else{
            *mx= mid_pred(A[0], B[0], C[0]);
            *my= mid_pred(A[1], B[1], C[1]);
        }
    }
        
    tprintf("pred_motion (%2d %2d %2d) (%2d %2d %2d) (%2d %2d %2d) -> (%2d %2d %2d) at %2d %2d %d list %d\n", top_ref, B[0], B[1],                    diagonal_ref, C[0], C[1], left_ref, A[0], A[1], ref, *mx, *my, h->s.mb_x, h->s.mb_y, n, list);
}

void write_back_motion(H264Context *h, int mb_type){
    MpegEncContext * const s = &h->s;
    const int b_xy = 4*s->mb_x + 4*s->mb_y*h->b_stride;
    const int b8_xy= 2*s->mb_x + 2*s->mb_y*h->b8_stride;
    int list;
    for(list=0; list<2; list++){
        int y;
        if((!IS_8X8(mb_type)) && !USES_LIST(mb_type, list)){
            if(1){ //FIXME skip or never read if mb_type doesnt use it
                for(y=0; y<4; y++){
                    *(uint64_t*)s->current_picture.motion_val[list][b_xy + 0 + y*h->b_stride]=
                    *(uint64_t*)s->current_picture.motion_val[list][b_xy + 2 + y*h->b_stride]= 0;
                }
                if( h->pps.cabac ) {
                    /* FIXME needed ? */
                    for(y=0; y<4; y++){
                        *(uint64_t*)h->mvd_table[list][b_xy + 0 + y*h->b_stride]=
                        *(uint64_t*)h->mvd_table[list][b_xy + 2 + y*h->b_stride]= 0;
                    }
                }
                for(y=0; y<2; y++){
                    *(uint16_t*)s->current_picture.motion_val[list][b8_xy + y*h->b8_stride]= (LIST_NOT_USED&0xFF)*0x0101;
                }
            }
            continue; //FIXME direct mode ...
        }
        
        for(y=0; y<4; y++){
            *(uint64_t*)s->current_picture.motion_val[list][b_xy + 0 + y*h->b_stride]= *(uint64_t*)h->mv_cache[list][scan8[0]+0 + 8*y];
            *(uint64_t*)s->current_picture.motion_val[list][b_xy + 2 + y*h->b_stride]= *(uint64_t*)h->mv_cache[list][scan8[0]+2 + 8*y];
        }
        if( h->pps.cabac ) {
            for(y=0; y<4; y++){
                *(uint64_t*)h->mvd_table[list][b_xy + 0 + y*h->b_stride]= *(uint64_t*)h->mvd_cache[list][scan8[0]+0 + 8*y];
                *(uint64_t*)h->mvd_table[list][b_xy + 2 + y*h->b_stride]= *(uint64_t*)h->mvd_cache[list][scan8[0]+2 + 8*y];
            }
        }
        for(y=0; y<2; y++){
            s->current_picture.ref_index[list][b8_xy + 0 + y*h->b8_stride]= h->ref_cache[list][scan8[0]+0 + 16*y];
            s->current_picture.ref_index[list][b8_xy + 1 + y*h->b8_stride]= h->ref_cache[list][scan8[0]+2 + 16*y];
        }
    }
}

void pred_16x8_motion(H264Context * const h, int n, int list, int ref, int * const mx, int * const my){
    if(n==0){
        const int top_ref=      h->ref_cache[list][ scan8[0] - 8 ];
        const int16_t * const B= h->mv_cache[list][ scan8[0] - 8 ];
        tprintf("pred_16x8: (%2d %2d %2d) at %2d %2d %d list %d", top_ref, B[0], B[1], h->s.mb_x, h->s.mb_y, n, list);
        
        if(top_ref == ref){
            *mx= B[0];
            *my= B[1];
            return;
        }
    }else{
        const int left_ref=     h->ref_cache[list][ scan8[8] - 1 ];
        const int16_t * const A= h->mv_cache[list][ scan8[8] - 1 ];
        
        tprintf("pred_16x8: (%2d %2d %2d) at %2d %2d %d list %d", left_ref, A[0], A[1], h->s.mb_x, h->s.mb_y, n, list);
        if(left_ref == ref){
            *mx= A[0];
            *my= A[1];
            return;
        }
    }
    //RARE
    pred_motion(h, n, 4, list, ref, mx, my);
}
/**
 * gets the directionally predicted 8x16 MV.
 * @param n the block index
 * @param mx the x component of the predicted motion vector
 * @param my the y component of the predicted motion vector
 */
void pred_8x16_motion(H264Context * const h, int n, int list, int ref, int * const mx, int * const my){
    if(n==0){
        const int left_ref=      h->ref_cache[list][ scan8[0] - 1 ];
        const int16_t * const A=  h->mv_cache[list][ scan8[0] - 1 ];
        
        tprintf("pred_8x16: (%2d %2d %2d) at %2d %2d %d list %d", left_ref, A[0], A[1], h->s.mb_x, h->s.mb_y, n, list);
        if(left_ref == ref){
            *mx= A[0];
            *my= A[1];
            return;
        }
    }else{
        const int16_t * C;
        int diagonal_ref;
        diagonal_ref= fetch_diagonal_mv(h, &C, scan8[4], list, 2);
        
        tprintf("pred_8x16: (%2d %2d %2d) at %2d %2d %d list %d", diagonal_ref, C[0], C[1], h->s.mb_x, h->s.mb_y, n, list);
        if(diagonal_ref == ref){ 
            *mx= C[0];
            *my= C[1];
            return;
        }
    }
    //RARE
    pred_motion(h, n, 2, list, ref, mx, my);
}

void pred_pskip_motion(H264Context * const h, int * const mx, int * const my){
    const int top_ref = h->ref_cache[0][ scan8[0] - 8 ];
    const int left_ref= h->ref_cache[0][ scan8[0] - 1 ];
    tprintf("pred_pskip: (%d) (%d) at %2d %2d", top_ref, left_ref, h->s.mb_x, h->s.mb_y);
    if(top_ref == PART_NOT_AVAILABLE || left_ref == PART_NOT_AVAILABLE
       || (top_ref == 0  && *(uint32_t*)h->mv_cache[0][ scan8[0] - 8 ] == 0)
       || (left_ref == 0 && *(uint32_t*)h->mv_cache[0][ scan8[0] - 1 ] == 0)){
       
        *mx = *my = 0;
        return;
    }
        
    pred_motion(h, 0, 4, 0, 0, mx, my);
    return;
}

/**
 * Sets the intra prediction function pointers.
 */
void init_pred_ptrs(H264Context *h){
//    MpegEncContext * const s = &h->s;
    h->pred4x4[VERT_PRED           ]= pred4x4_vertical_c;
    h->pred4x4[HOR_PRED            ]= pred4x4_horizontal_c;
    h->pred4x4[DC_PRED             ]= pred4x4_dc_c;
    h->pred4x4[DIAG_DOWN_LEFT_PRED ]= pred4x4_down_left_c;
    h->pred4x4[DIAG_DOWN_RIGHT_PRED]= pred4x4_down_right_c;
    h->pred4x4[VERT_RIGHT_PRED     ]= pred4x4_vertical_right_c;
    h->pred4x4[HOR_DOWN_PRED       ]= pred4x4_horizontal_down_c;
    h->pred4x4[VERT_LEFT_PRED      ]= pred4x4_vertical_left_c;
    h->pred4x4[HOR_UP_PRED         ]= pred4x4_horizontal_up_c;
    h->pred4x4[LEFT_DC_PRED        ]= pred4x4_left_dc_c;
    h->pred4x4[TOP_DC_PRED         ]= pred4x4_top_dc_c;
    h->pred4x4[DC_128_PRED         ]= pred4x4_128_dc_c;
    h->pred8x8[DC_PRED8x8     ]= pred8x8_dc_c;
    h->pred8x8[VERT_PRED8x8   ]= pred8x8_vertical_c;
    h->pred8x8[HOR_PRED8x8    ]= pred8x8_horizontal_c;
    h->pred8x8[PLANE_PRED8x8  ]= pred8x8_plane_c;
    h->pred8x8[LEFT_DC_PRED8x8]= pred8x8_left_dc_c;
    h->pred8x8[TOP_DC_PRED8x8 ]= pred8x8_top_dc_c;
    h->pred8x8[DC_128_PRED8x8 ]= pred8x8_128_dc_c;
    h->pred16x16[DC_PRED8x8     ]= pred16x16_dc_c;
    h->pred16x16[VERT_PRED8x8   ]= pred16x16_vertical_c;
    h->pred16x16[HOR_PRED8x8    ]= pred16x16_horizontal_c;
    h->pred16x16[PLANE_PRED8x8  ]= pred16x16_plane_c;
    h->pred16x16[LEFT_DC_PRED8x8]= pred16x16_left_dc_c;
    h->pred16x16[TOP_DC_PRED8x8 ]= pred16x16_top_dc_c;
    h->pred16x16[DC_128_PRED8x8 ]= pred16x16_128_dc_c;
}

