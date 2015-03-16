#include"stdafx.h"
#include"Global.h"
/**********************************************************************
 *
 *	Name:        CodeOneInter
 *	Description:	code one image normally or two images
 *                      as a PB-frame (CodeTwoPB and CodeOnePred merged)
 *	
 *	Input:        pointer to image, prev_image, prev_recon, Q
 *        
 *	Returns:	pointer to reconstructed image
 *	Side effects:	memory is allocated to recon image
 *
 ************************************************************************/
void CodeOneInter(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
				  int QP, int frameskip, Bits *bits, Pict *pic)
{
	 ZeroBits(bits);
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int i,j,k;
	 int newgob,Mode;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
       
	 /* 缓冲器控制变量 */
     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */

	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr;
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}

	/* 整数和半象素运动估值 */
    MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
	                    pic->seek_dist, MV, pic->use_gobsync);


    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      /* 初始化码率控制 */
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
    #endif

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) {

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      /* 更新QP */      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     /* 更新dquant */
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	
         /* 当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) */
         Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         MV[0][j+1][i+1]->Mode = Mode;

		 pic->MB = i + j * (pels/MB_SIZE);

         if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		 }
         else {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块树组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }

         /* P或INTRA宏块 */
         qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
         CBP = FindCBP(qcoeff_P, Mode, 64); 
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
           ZeroMBlock(diff); //宏块数据设为0
         else
           MB_Decode(qcoeff_P, diff, QP_xmitted, Mode);//反变换
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
        
         if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0

		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          /* 当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码*/
         if (!syntax_arith_coding)
             CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
         else {      /* 正常编码宏块 */
          if (!syntax_arith_coding) { /* VLC */
             CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
            }
            else {
             /* MODE_INTRA 或 MODE_INTRA_Q */
             bits->no_intra++;
             if (pic->PB)
               CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
            }
            if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		 }//end Normal MB

         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
         free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;
  /* 释放内存 */
  free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
  return;

}

void ZeroVec(MotionVector *MV)
{
  MV->x = 0;
  MV->y = 0;
  MV->x_half = 0;
  MV->y_half = 0;
  return;
}
void MarkVec(MotionVector *MV)
{
  MV->x = NO_VEC;
  MV->y = NO_VEC;
  MV->x_half = 0;
  MV->y_half = 0;
  return;
}

void CopyVec(MotionVector *MV2, MotionVector *MV1)
{
  MV2->x = MV1->x;
  MV2->x_half = MV1->x_half;
  MV2->y = MV1->y;
  MV2->y_half = MV1->y_half;
  return;
}

int EqualVec(MotionVector *MV2, MotionVector *MV1)
{
  if (MV1->x != MV2->x)
    return 0;
  if (MV1->y != MV2->y)
    return 0;
  if (MV1->x_half != MV2->x_half)
    return 0;
  if (MV1->y_half != MV2->y_half)
    return 0;
  return 1;
}

unsigned char *InterpolateImage(unsigned char *image, int width, int height)
{
  unsigned char *ipol_image, *ii, *oo;
  int i,j;

  ipol_image = (unsigned char *)malloc(sizeof(char)*width*height*4);
  ii = ipol_image;
  oo = image;

  /* main image */
  for (j = 0; j < height-1; j++) {
    for (i = 0; i  < width-1; i++) {
      *(ii + (i<<1)) = *(oo + i);
      *(ii + (i<<1)+1) = (unsigned char)((*(oo + i) + *(oo + i + 1) + 1)>>1); 
      *(ii + (i<<1)+(width<<1)) = (unsigned char)((*(oo + i) + *(oo + i + width) + 1)>>1); 
      *(ii + (i<<1)+1+(width<<1)) = (unsigned char)((*(oo+i) + *(oo+i+1) +
         *(oo+i+width) + *(oo+i+1+width) + 2)>>2);									 
    }
    /* last pels on each line */
    *(ii+ (width<<1) - 2) = *(oo + width - 1);
    *(ii+ (width<<1) - 1) = *(oo + width - 1);
    *(ii+ (width<<1)+ (width<<1)-2) = (unsigned char)((*(oo+width-1)+*(oo+width+width-1)+1)>>1); 
    *(ii+ (width<<1)+ (width<<1)-1) = (unsigned char)((*(oo+width-1)+*(oo+width+width-1)+1)>>1); 
    ii += (width<<2);
    oo += width;
  }

  /* last lines */
  for (i = 0; i < width-1; i++) {
    *(ii+ (i<<1)) = *(oo + i);    
    *(ii+ (i<<1)+1) = (unsigned char)((*(oo + i) + *(oo + i + 1) + 1)>>1);
    *(ii+ (width<<1)+ (i<<1)) = *(oo + i);    
    *(ii+ (width<<1)+ (i<<1)+1) = (unsigned char)((*(oo + i) + *(oo + i + 1) + 1)>>1);
          
  }

  /* bottom right corner pels */
  *(ii + (width<<1) - 2) = *(oo + width -1);
  *(ii + (width<<1) - 1) = *(oo + width -1);
  *(ii + (width<<2) - 2) = *(oo + width -1);
  *(ii + (width<<2) - 1) = *(oo + width -1);

  return ipol_image;
}

void MotionEstimatePicture(unsigned char *curr, unsigned char *prev, 
           unsigned char *prev_ipol, int seek_dist, 
           MotionVector *MV[6][MBR+1][MBC+2], int gobsync)
{
  int i,j,k;
  int pmv0,pmv1,xoff,yoff;
  int curr_mb[16][16];
  int sad8 = INT_MAX, sad16, sad0;
  int newgob;
  MotionVector *f0,*f1,*f2,*f3,*f4;

  /* 运动估计并存储结果MV */
  for ( j = 0; j < lines/MB_SIZE; j++) {

    newgob = 0;
    if (gobsync && j%gobsync == 0) {
      newgob = 1;
    }

    for ( i = 0; i < pels/MB_SIZE; i++) {
      for (k = 0; k < 6; k++)
        MV[k][j+1][i+1] = (MotionVector *)malloc(sizeof(MotionVector));

      /* 整象素搜索 */
      f0 = MV[0][j+1][i+1];
      f1 = MV[1][j+1][i+1];
      f2 = MV[2][j+1][i+1];
      f3 = MV[3][j+1][i+1];
      f4 = MV[4][j+1][i+1];

      FindPMV(MV,i+1,j+1,&pmv0,&pmv1,0,newgob,0);

      if (long_vectors) {
        xoff = pmv0/2; /* 总是能被2整除 */
        yoff = pmv1/2;
      }
      else {
        xoff = yoff = 0;
      }
      
      MotionEstimation(curr, prev, i*MB_SIZE, j*MB_SIZE, 
               xoff, yoff, seek_dist, MV, &sad0);

      sad16 = f0->min_error;
      if (advanced)
        sad8 = f1->min_error + f2->min_error + f3->min_error + f4->min_error;

      f0->Mode = ChooseMode(curr,i*MB_SIZE,j*MB_SIZE, mmin(sad8,sad16));

      /* 半象素精度搜索 */
      if (f0->Mode != MODE_INTRA) {
        FindMB(i*MB_SIZE,j*MB_SIZE ,curr, curr_mb);//当前宏块放入curr_mb
        FindHalfPel(i*MB_SIZE,j*MB_SIZE,f0, prev_ipol, &curr_mb[0][0],16,0);
        sad16 = f0->min_error;

        if (advanced) {
          FindHalfPel(i*MB_SIZE,j*MB_SIZE,f1, prev_ipol, &curr_mb[0][0],8,0);
          FindHalfPel(i*MB_SIZE,j*MB_SIZE,f2, prev_ipol, &curr_mb[0][8],8,1);
          FindHalfPel(i*MB_SIZE,j*MB_SIZE,f3, prev_ipol, &curr_mb[8][0],8,2);
          FindHalfPel(i*MB_SIZE,j*MB_SIZE,f4, prev_ipol, &curr_mb[8][8],8,3);

          sad8 = f1->min_error +f2->min_error +f3->min_error +f4->min_error;
          sad8 += PREF_16_VEC;
          
          /* 选择0运动矢量, 基于8x8或16x16的运动矢量  */
          if (sad0 < sad8 && sad0 < sad16) {
            f0->x = f0->y = 0;
            f0->x_half = f0->y_half = 0;
          }
          else {
            if (sad8 < sad16) 
              f0->Mode = MODE_INTER4V;
          }
        }
        else {
          /* 选择0运动矢量或基于16x16的运动矢量 */
          if (sad0 < sad16) {
            f0->x = f0->y = 0;
            f0->x_half = f0->y_half = 0;
          }
        }

      }
      else 
        for (k = 0; k < 5; k++)
          ZeroVec(MV[k][j+1][i+1]);
    }
  }

  return;
}
           
void ZeroMBlock(MB_Structure *data)
{
  int n;
  register int m;

  for (n = 0; n < MB_SIZE; n++)
    for (m = 0; m < MB_SIZE; m++)
      data->lum[n][m] = 0;
  for (n = 0; n < (MB_SIZE>>1); n++)
    for (m = 0; m < (MB_SIZE>>1); m++) {
      data->Cr[n][m] = 0;
      data->Cb[n][m] = 0;
    }
  return;
}
