#ifndef _MB_MOTCOMP_H_
#define _MB_MOTCOMP_H_


CREL_RETURN MB_I4MB_Luma PARGS2(imgpel * imgY, int stride);
CREL_RETURN MB_I8MB_Luma PARGS2(imgpel * imgY, int stride);
CREL_RETURN MB_I16MB_Luma PARGS2(imgpel * imgY, int stride);
CREL_RETURN MB_I4MB_Chroma PARGS2(imgpel * imgUV, int stride_UV);

void MB_InterPred16x16 PARGS3(int vec_x_base, int vec_y_base, int list_offset);
void MB_InterPred16x8 PARGS3(int vec_x_base, int vec_y_base, int list_offset);
void MB_InterPred8x16 PARGS3(int vec_x_base, int vec_y_base, int list_offset);

void MB_InterPred8x8_BiDir PARGS4(int vec_x_base, int vec_y_base, int list_offset, int b8);
void MB_InterPred4x4_BiDir PARGS4(int vec_x_base, int vec_y_base, int list_offset, int b8);
void MB_InterPred8x8_1Dir PARGS5(int vec_x_base,  int vec_y_base, int list_offset, int b8, int dir);
void MB_InterPred4x4_1Dir PARGS5(int vec_x_base,  int vec_y_base, int list_offset, int b8, int dir);

//For weighting
void MB_InterPred16x16_1 PARGS3(int vec_x_base, int vec_y_base, int list_offset);
void MB_InterPred16x8_1 PARGS3(int vec_x_base, int vec_y_base, int list_offset);
void MB_InterPred8x16_1 PARGS3(int vec_x_base, int vec_y_base, int list_offset);

void MB_InterPred8x8_BiDir_1 PARGS4(int vec_x_base, int vec_y_base, int list_offset, int b8);
void MB_InterPred4x4_BiDir_1 PARGS4(int vec_x_base, int vec_y_base, int list_offset, int b8);
void MB_InterPred8x8_1Dir_1 PARGS5(int vec_x_base,  int vec_y_base, int list_offset, int b8, int dir);
void MB_InterPred4x4_1Dir_1 PARGS5(int vec_x_base,  int vec_y_base, int list_offset, int b8, int dir);

void MB_InterPred_b8mode_P0 PARGS3(int vec_x_base, int vec_y_base, int list_offset);
void MB_InterPred_b8mode_P1 PARGS3(int vec_x_base, int vec_y_base, int list_offset);
void MB_InterPred_b8mode_B0 PARGS3(int vec_x_base, int vec_y_base, int list_offset);
void MB_InterPred_b8mode_B1 PARGS3(int vec_x_base, int vec_y_base, int list_offset);


typedef void MB_itrans_Luma_t PARGS2(imgpel * imgY, int stride);
typedef void MB_itrans_Chroma_t PARGS2(imgpel * imgUV, int stride_UV);

MB_itrans_Luma_t MB_itrans4x4_Luma_c;
MB_itrans_Luma_t MB_itrans4x4_Luma_sse;
MB_itrans_Luma_t MB_itrans4x4_Luma_sse2;
MB_itrans_Luma_t MB_itrans8x8_Luma_c;
MB_itrans_Luma_t MB_itrans8x8_Luma_sse2;
MB_itrans_Luma_t MB_itrans8x8_Luma_sse;
MB_itrans_Chroma_t MB_itrans4x4_Chroma_c;
MB_itrans_Chroma_t MB_itrans4x4_Chroma_mmx;
MB_itrans_Chroma_t MB_itrans4x4_Chroma_sse2;

extern MB_itrans_Luma_t *MB_itrans4x4_Luma;
extern MB_itrans_Luma_t *MB_itrans8x8_Luma;
extern MB_itrans_Chroma_t *MB_itrans4x4_Chroma;

#endif //_MB_MOTCOMP_H_
