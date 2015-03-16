
#include"stdafx.h"
#include"Global.h"

#include"indices.h"
#include "OutputVlc.h"
 
int arith_used = 0;
int syntax_arith_coding;
int advanced;

/************************************************************************/

int CountBitsPicture(Pict *pic)
{
  int bits = 0;

  /* Picture start code */
  putbits(PSC_LENGTH,PSC);
  bits += PSC_LENGTH;

  /* Group number */
  putbits(5,0);
  bits += 5;

  /* Time reference */
  putbits(8,pic->TR);
  bits += 8;

 /* bit 1 */
  putbits(1,1);
  bits += 1;

  /* bit 2 */
  putbits(1,0);
  bits += 1;

  /* bit 3 */
  putbits(1,0);     /* no support for split-screen in this software */
  bits += 1;

  /* bit 4 */
  putbits(1,0);
  bits += 1;

  /* bit 5 */
  putbits(1,0);
  bits += 1;

  /* bit 6-8 */
  putbits(3,pic->source_format);
  bits += 3;

  /* bit 9 */
  putbits(1,pic->picture_coding_type);
  bits += 1;

  /* bit 10 */
  putbits(1,pic->unrestricted_mv_mode);  /* Unrestricted Motion Vector mode */
  bits += 1;

  /* bit 11 */
  putbits(1,syntax_arith_coding); /* Syntax-based Arithmetic Coding mode */
  bits += 1;

  /* bit 12 */
  putbits(1,advanced); /* Advanced Prediction mode */
  bits += 1;

  /* bit 13 */
  putbits(1,pic->PB);
  bits += 1;

  /* QUANT */
  putbits(5,pic->QUANT);
  bits += 5;

  /* Continuous Presence Multipoint (CPM) */
  putbits(1,0); /* CPM is not supported in this software */
  bits += 1;

  if (pic->PB) {
    putbits(3,pic->TRB);
    bits += 3;

    putbits(2,pic->BQUANT);
    bits += 2;

  }

  putbits(1,0);
  bits += 1;

  return bits;
}

/************************************************************************/

void CountBitsMB(int Mode, int COD, int CBP, int CBPB, Pict *pic, Bits *bits)
{
  int cbpy, cbpcm, length;

  if (pic->picture_coding_type == PCT_INTER) {
    putbits(1,COD);
    bits->COD++;
  }

  if (COD)
    return;    /* not coded */

  /* CBPCM */
  cbpcm = Mode | ((CBP&3)<<4);
  if (pic->picture_coding_type == PCT_INTRA)
    length = put_cbpcm_intra (CBP, Mode);
  else
    length = put_cbpcm_inter (CBP, Mode);
  bits->CBPCM += length;

    /* MODB & CBPB */
  if (pic->PB) {
    switch (pic->MODB) {
    case PBMODE_NORMAL:
      putbits(1,0);
      bits->MODB += 1;
      break;
    case PBMODE_MVDB:
      putbits(2,2);
      bits->MODB += 2;
      break;
    case PBMODE_CBPB_MVDB:
      putbits(2,3);
      bits->MODB += 2;
      /* CBPB */
      putbits(6,CBPB);
      bits->CBPB += 6;
      break;
    }
  }

  /* CBPY */
  cbpy = CBP>>2;
  if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) /* Intra */
    cbpy = cbpy^15;
  length = put_cbpy (CBP, Mode);

  bits->CBPY += length;

  /* DQUANT */
  if ((Mode == MODE_INTER_Q) || (Mode == MODE_INTRA_Q)) {
    switch (pic->DQUANT) {
    case -1:
      putbits(2,0);
      break;
    case -2:
      putbits(2,1);
      break;
    case 1:
      putbits(2,2);
      break;
    case 2:
      putbits(2,3);
      break;
    default:
      fprintf(stderr,"Invalid DQUANT\n");
      exit(-1);
    }
    bits->DQUANT += 2;
  }
  return;
}



/************************************************************************/

int CountBitsSlice(int slice, int quant)
{
  int bits = 0;

  /* Picture Start Code */
  putbits(PSC_LENGTH,PSC); /* PSC */
  bits += PSC_LENGTH;

  /* Group Number */

  putbits(5,slice);
  bits += 5;

  /* GOB Frame ID */
  putbits(2, 0);
  bits += 2;

  /* Gquant */
  putbits(5,quant);
  bits += 5;

  return bits;
}


/*************************************************************************/

void CountBitsCoeff(int *qcoeff, int Mode, int CBP, Bits *bits, int ncoeffs)
{
  
  int i;

  if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) {
    for (i = 0; i < 4; i++) {
      bits->Y += CodeCoeff(Mode, qcoeff,i,ncoeffs);
    }
    for (i = 4; i < 6; i++) {
      bits->C += CodeCoeff(Mode, qcoeff,i,ncoeffs);
    }
  }
  else {
    for (i = 0; i < 4; i++) {
      if ((i==0 && CBP&32) || 
          (i==1 && CBP&16) ||
          (i==2 && CBP&8) || 
          (i==3 && CBP&4) || 
          (i==4 && CBP&2) ||
          (i==5 && CBP&1)) {
        bits->Y += CodeCoeff(Mode, qcoeff, i, ncoeffs);
      }
    }
    for (i = 4; i < 6; i++) {
      if ((i==0 && CBP&32) || 
          (i==1 && CBP&16) ||
          (i==2 && CBP&8) || 
          (i==3 && CBP&4) || 
          (i==4 && CBP&2) ||
          (i==5 && CBP&1)) {
        bits->C += CodeCoeff(Mode, qcoeff, i, ncoeffs);
      }
    }
  }
  return;
}
 
/*************************************************************************/
 
int CodeCoeff(int Mode, int *qcoeff, int block, int ncoeffs)
{
  int j, bits;
  int prev_run, run, prev_level, level, first;
  int prev_s, s, length;

  run = bits = 0;
  first = 1;
  prev_run = prev_level = level = s = prev_s = 0;

  for (j = block*ncoeffs; j< (block + 1)*ncoeffs; j++) {
    /* Do this block's DC-coefficient first */
    if (!(j%ncoeffs) && (Mode == MODE_INTRA || Mode == MODE_INTRA_Q)) {
      /* DC coeff */
      if (qcoeff[block*ncoeffs] != 128)
        putbits(8,qcoeff[block*ncoeffs]);
      else
        putbits(8,255);
      bits += 8;
    }
    else {
      /* AC coeff */
      s = 0;
      /* Increment run if coeff is zero */
      if ((level = qcoeff[j]) == 0) {
        run++;
      }
      else {
        /* code run & level and count bits */
        if (level < 0) {
          s = 1;
          level = -level;
        }

        if (!first) {
          /* Encode the previous coefficient */
          if (prev_level  < 13 && prev_run < 64)
            length = put_coeff (0,prev_run, prev_level);  
          else
            length = 0;
          if (length == 0) {  /* Escape coding */
            if (prev_s == 1) {prev_level = (prev_level^0xff)+1;}
            putbits(7,3);	/* Escape code */
            putbits(1,0);
            putbits(6,prev_run);
            putbits(8,prev_level);
            bits += 22;
          }
          else {
            putbits(1,prev_s);
            bits += length + 1;
          }
        }
        prev_run = run; prev_s = s;
        prev_level = level;

        run = first = 0;
      }
    }
  }
  /* Encode the last coeff */
  if (!first) {
    if (prev_level  < 13 && prev_run < 64)
      length = put_coeff (1,prev_run, prev_level);
    else
      length = 0;
    if (length == 0) {  /* Escape coding */
      if (prev_s == 1) {prev_level = (prev_level^0xff)+1;}
      putbits (7,3);	/* Escape code */
      putbits(1,1);
      putbits(6,prev_run);
      putbits(8,prev_level);
      bits += 22;
    }
    else {
      putbits(1,prev_s);
      bits += length + 1;
    }
  }
  return bits;
}

/*************************************************************************/

int FindCBP(int *qcoeff, int Mode, int ncoeffs)
{
  
  int i,j;
  int CBP = 0;
  int intra = (Mode == MODE_INTRA || Mode == MODE_INTRA_Q);

  /* Set CBP for this Macroblock */
  for (i = 0; i < 6; i++) {
    for (j = i*ncoeffs + intra; j < (i+1)*ncoeffs; j++) {
      if (qcoeff[j]) {
        if (i == 0) {CBP |= 32;}
        else if (i == 1) {CBP |= 16;}
        else if (i == 2) {CBP |= 8;}
        else if (i == 3) {CBP |= 4;}
        else if (i == 4) {CBP |= 2;}
        else if (i == 5) {CBP |= 1;}
        else {
          fprintf(stderr,"Error in CBP assignment\n");
          exit(-1);
        }
        break;
      }
    }
  }

  return CBP;
}

/*************************************************************************/

void ZeroBits(Bits *bits)
{
  bits->Y = 0;
  bits->C = 0;
  bits->vec = 0;
  bits->CBPY = 0;
  bits->CBPCM = 0;
  bits->MODB = 0;
  bits->CBPB = 0;
  bits->COD = 0;
  bits->DQUANT = 0;
  bits->header = 0;
  bits->total = 0;
  bits->no_inter = 0;
  bits->no_inter4v = 0;
  bits->no_intra = 0;
  return;
}

/*************************************************************************/

void AddBits(Bits *total, Bits *bits)
{
  total->Y += bits->Y;
  total->C += bits->C;
  total->vec += bits->vec;
  total->CBPY += bits->CBPY;
  total->CBPCM += bits->CBPCM;
  total->MODB += bits->MODB;
  total->CBPB += bits->CBPB;
  total->COD += bits->COD;
  total->DQUANT += bits->DQUANT;
  total->header += bits->header;
  total->total += bits->total;
  total->no_inter += bits->no_inter;
  total->no_inter4v += bits->no_inter4v;
  total->no_intra += bits->no_intra;
  return;
}

/*************************************************************************/

void AddBitsPicture(Bits *bits)
{
  bits->total = 
    bits->Y + 
    bits->C + 
    bits->vec +  
    bits->CBPY + 
    bits->CBPCM + 
    bits->MODB +
    bits->CBPB +
    bits->COD + 
    bits->DQUANT +
    bits->header ;
} 

/*************************************************************************/

void FindPMV(MotionVector *MV[6][MBR+1][MBC+2], int x, int y,
             int *pmv0, int *pmv1, int block, int newgob, int half_pel)
{
  int p1,p2,p3;
  int xin1,xin2,xin3;
  int yin1,yin2,yin3;
  int vec1,vec2,vec3;
  int l8,o8,or8;


  l8 = o8 = or8 = 0;
  if (MV[0][y][x-1]->Mode == MODE_INTER4V)
    l8 = 1;
  if (MV[0][y-1][x]->Mode == MODE_INTER4V)
    o8 = 1;
  if (MV[0][y-1][x+1]->Mode == MODE_INTER4V)
    or8 = 1;

  switch (block) {
  case 0:
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 1:
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 2:
    vec1 = 1            ; yin1 = y  ; xin1 = x;
    vec2 = (o8 ? 4 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 3:
    vec1 = (l8 ? 4 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  case 4:
    vec1 = 3            ; yin1 = y  ; xin1 = x;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  default:
    fprintf(stderr,"Illegal block number in FindPMV (countbit.c)\n");
    exit(-1);
    break;
  }
  if (half_pel) {
    p1 = 2*MV[vec1][yin1][xin1]->x + MV[vec1][yin1][xin1]->x_half;
    p2 = 2*MV[vec2][yin2][xin2]->x + MV[vec2][yin2][xin2]->x_half;
    p3 = 2*MV[vec3][yin3][xin3]->x + MV[vec3][yin3][xin3]->x_half;
  }
  else {
    p1 = 2*MV[vec1][yin1][xin1]->x;
    p2 = 2*MV[vec2][yin2][xin2]->x;
    p3 = 2*MV[vec3][yin3][xin3]->x;
  }
  if (newgob && (block == 0 || block == 1 || block == 2))
    p2 = 2*NO_VEC;

  if (p2 == 2*NO_VEC) { p2 = p3 = p1; }

  *pmv0 = p1+p2+p3 - mmax(p1,mmax(p2,p3)) - mmin(p1,mmin(p2,p3));

  if (half_pel) {
    p1 = 2*MV[vec1][yin1][xin1]->y + MV[vec1][yin1][xin1]->y_half;
    p2 = 2*MV[vec2][yin2][xin2]->y + MV[vec2][yin2][xin2]->y_half;
    p3 = 2*MV[vec3][yin3][xin3]->y + MV[vec3][yin3][xin3]->y_half;
  }
  else {
    p1 = 2*MV[vec1][yin1][xin1]->y;
    p2 = 2*MV[vec2][yin2][xin2]->y;
    p3 = 2*MV[vec3][yin3][xin3]->y;
  }
  if (newgob && (block == 0 || block == 1 || block == 2))
    p2 = 2*NO_VEC;

  if (p2 == 2*NO_VEC) { p2 = p3 = p1; }

  *pmv1 = p1+p2+p3 - mmax(p1,mmax(p2,p3)) - mmin(p1,mmin(p2,p3));
  
  return;
}
/*************************************************************************/

void CountBitsVectors(MotionVector *MV[6][MBR+1][MBC+2], Bits *bits, 
              int x, int y, int Mode, int newgob, Pict *pic)
{
  int y_vec, x_vec;
  int pmv0, pmv1;
  int start,stop,block;

  x++;y++;

  if (Mode == MODE_INTER4V) {
    start = 1; stop = 4;
  }
  else {
    start = 0; stop = 0;
  }

  for (block = start; block <= stop;  block++) {

    FindPMV(MV,x,y,&pmv0,&pmv1, block, newgob, 1);

    x_vec = (2*MV[block][y][x]->x + MV[block][y][x]->x_half) - pmv0;
    y_vec = (2*MV[block][y][x]->y + MV[block][y][x]->y_half) - pmv1;

    if (!long_vectors) {
      if (x_vec < -32) x_vec += 64;
      else if (x_vec > 31) x_vec -= 64;

      if (y_vec < -32) y_vec += 64;
      else if (y_vec > 31) y_vec -= 64;
    }
    else {
      if (pmv0 < -31 && x_vec < -63) x_vec += 64;
      else if (pmv0 > 32 && x_vec > 63) x_vec -= 64;

      if (pmv1 < -31 && y_vec < -63) y_vec += 64;
      else if (pmv1 > 32 && y_vec > 63) y_vec -= 64;
    }


    if (x_vec < 0) x_vec += 64;
    if (y_vec < 0) y_vec += 64;

    bits->vec += put_mv (x_vec);
    bits->vec += put_mv (y_vec);
     
  }

  /* PB帧的偏差矢量 */
  if (pic->PB)
    if (pic->MODB == PBMODE_MVDB || pic->MODB == PBMODE_CBPB_MVDB) {

      x_vec = MV[5][y][x]->x;
      y_vec = MV[5][y][x]->y;

      /* x_vec和y_vec是PB帧的偏差矢量 */

      if (x_vec < 0) x_vec += 64;
      if (y_vec < 0) y_vec += 64;

      bits->vec += put_mv (x_vec);
      bits->vec += put_mv (y_vec);
    }
  return;
}