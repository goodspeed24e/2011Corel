
#include"stdafx.h"
#include"Global.h"

//帧内编码
PictImage *CodeOneIntra(PictImage *curr, int QP, Bits *bits, Pict *pic)
{
  PictImage *recon;
  MB_Structure *data = (MB_Structure *)malloc(sizeof(MB_Structure));
  int * qcoeff;
  int Mode =3;//  MODE_INTRA;
  int CBP,COD;
  int i,j;

  recon = InitImage(pels*lines);
  ZeroBits(bits);
  
  pic->QUANT = QP;
  bits->header += CountBitsPicture(pic);

  COD = 0; 
  for ( j = 0; j < lines/MB_SIZE; j++) {

    /* 如果允许GOB同步，则输出GOB同步头  */
    if (pic->use_gobsync && j != 0)
      bits->header += CountBitsSlice(j,QP);

    for ( i = 0; i < pels/MB_SIZE; i++) {
      pic->MB = i + j * (pels/MB_SIZE);
      bits->no_intra++;
      FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, data);//写亮度图像16*16 curr:图像数据 data:宏块树组
      FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, data);//写色度图像2*8*8 
      qcoeff = MB_Encode(data, QP, Mode); //变换并量化后数据到qcoeff=16*16+2*8*8个单元
      CBP = FindCBP(qcoeff,Mode,64);

      if (!syntax_arith_coding) {
        CountBitsMB(Mode,COD,CBP,0,pic,bits);//输出宏块层信息
        CountBitsCoeff(qcoeff, Mode, CBP,bits,64);
      } 

      MB_Decode(qcoeff, data, QP, Mode);  //反变换
      Clip(data);                       //使 0<=data<=255 
      ReconImage(i,j,data,recon);     //将重建宏块data输出到整个图像recon中
      free(qcoeff);
    }
  }
  pic->QP_mean = (float)QP;

  free(data);
  return recon;
}

void Clip(MB_Structure *data)
{
  int m,n;

  for (n = 0; n < 16; n++) 
    for (m = 0; m < 16; m++) 
      data->lum[n][m] = mmin(255,mmax(0,data->lum[n][m]));
  for (n = 0; n < 8; n++) 
    for (m = 0; m < 8; m++) {
      data->Cr[n][m] = mmin(255,mmax(0,data->Cr[n][m]));
      data->Cb[n][m] = mmin(255,mmax(0,data->Cb[n][m]));
    }
  
}

/**********************************************************************/

void ReconImage (int i, int j, MB_Structure *data, PictImage *recon)
{
  int n;
  register int m;

  int x_curr, y_curr;

  x_curr = i * MB_SIZE;
  y_curr = j * MB_SIZE;

  /* Fill in luminance data */
  for (n = 0; n < MB_SIZE; n++)
    for (m= 0; m < MB_SIZE; m++) {
      *(recon->lum + x_curr+m + (y_curr+n)*pels) = (unsigned char)data->lum[n][m]; 
    }

  /* Fill in chrominance data */
  for (n = 0; n < (MB_SIZE>>1); n++)	
    for (m = 0; m < (MB_SIZE>>1); m++) {
      *(recon->Cr + (x_curr>>1)+m + ((y_curr>>1)+n)*cpels) = (unsigned char)data->Cr[n][m];
      *(recon->Cb + (x_curr>>1)+m + ((y_curr>>1)+n)*cpels) = (unsigned char)data->Cb[n][m];
    }
  return;
}


/***********************************************************************/

void Quant(int *coeff, int *qcoeff, int QP, int Mode)
{
  int i;

  for (i = 0; i < 64; i++) 
    qcoeff[i] = coeff[i]/(2*QP);
    
	qcoeff[0]=qcoeff[0]*2*QP/8;
 
  return;
}

/***********************************************************************/

void Dequant(int *qcoeff, int *rcoeff, int QP, int Mode)
{
  int i;
    for (i = 0; i < 64; i++)
      rcoeff[i] = qcoeff[i]*2*QP;
  rcoeff[0]=rcoeff[0]*8/(2*QP);
  return;
}

/***********************************************************************/

void FillLumBlock( int x, int y, PictImage *image, MB_Structure *data)
{
  int n;
  register int m;

  for (n = 0; n < MB_SIZE; n++)
    for (m = 0; m < MB_SIZE; m++)
      data->lum[n][m] = 
        (int)(*(image->lum + x+m + (y+n)*pels));
  return;
}

/***********************************************************************/

void FillChromBlock(int x_curr, int y_curr, PictImage *image,
            MB_Structure *data)
{
  int n;
  register int m;
  int x, y;

  x = x_curr>>1;
  y = y_curr>>1;

  for (n = 0; n < (MB_SIZE>>1); n++)
    for (m = 0; m < (MB_SIZE>>1); m++) {
      data->Cr[n][m] = 
        (int)(*(image->Cr +x+m + (y+n)*cpels));
      data->Cb[n][m] = 
        (int)(*(image->Cb +x+m + (y+n)*cpels));
    }
  return;
}

/***********************************************************************/

int *MB_Encode(MB_Structure *mb_orig, int QP, int I)
{
  int        i, j, k, l, row, col;
  int        fblock[64];
  int        coeff[384];
  int        *coeff_ind;
  int        *qcoeff;
  int        *qcoeff_ind;

  if ((qcoeff=(int *)malloc(sizeof(int)*384)) == 0) {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }

  coeff_ind = coeff;
  qcoeff_ind = qcoeff;
  for (k=0;k<16;k+=8) {
    for (l=0;l<16;l+=8) {
      for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          fblock[row+col] = mb_orig->lum[i][j];
        }
      }
      Dct(fblock,coeff_ind);
      Quant(coeff_ind,qcoeff_ind,QP,I);
      coeff_ind += 64;
      qcoeff_ind += 64;
    }
  }
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cb[i][j];
    }
  }
  Dct(fblock,coeff_ind);
  Quant(coeff_ind,qcoeff_ind,QP,I); 
  coeff_ind += 64;
  qcoeff_ind += 64;

  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cr[i][j];
    }
  }
  Dct(fblock,coeff_ind);
  Quant(coeff_ind,qcoeff_ind,QP,I); 
  return qcoeff;
}

/************************************************************************/
     
int MB_Decode(int *qcoeff, MB_Structure *mb_recon, int QP, int I)
{
  int	i, j, k, l, row, col;
  int	*iblock;
  int	*qcoeff_ind;
  int	*rcoeff, *rcoeff_ind;

  if ((iblock = (int *)malloc(sizeof(int)*64)) == NULL) {
    fprintf(stderr,"MB_Coder: Could not allocate space for iblock\n");
    exit(-1);
  }
  if ((rcoeff = (int *)malloc(sizeof(int)*384)) == NULL) {
    fprintf(stderr,"MB_Coder: Could not allocate space for rcoeff\n");
    exit(-1);
  }  

  for (i = 0; i < 16; i++)
    for (j = 0; j < 16; j++)
      mb_recon->lum[j][i] = 0;
  for (i = 0; i < 8; i++) 
    for (j = 0; j < 8; j++) {
      mb_recon->Cb[j][i] = 0;
      mb_recon->Cr[j][i] = 0;
    }

  qcoeff_ind = qcoeff;
  rcoeff_ind = rcoeff;


  for (k=0;k<16;k+=8) {
    for (l=0;l<16;l+=8) {
      Dequant(qcoeff_ind,rcoeff_ind,QP,I);
      idct(rcoeff_ind,iblock); 
      qcoeff_ind += 64;
      rcoeff_ind += 64;
      for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          mb_recon->lum[i][j] = *(iblock+row+col);
        }
      }
    }
  }
  Dequant(qcoeff_ind,rcoeff_ind,QP,I);

  idct(rcoeff_ind,iblock); 
  qcoeff_ind += 64;
  rcoeff_ind += 64;
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      mb_recon->Cb[i][j] = *(iblock+i*8+j);
    }
  }
  Dequant(qcoeff_ind,rcoeff_ind,QP,I);
  idct(rcoeff_ind,iblock); 
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      mb_recon->Cr[i][j] = *(iblock+i*8+j);
    }
  }
  free(iblock);
  free(rcoeff);
  return 0;
}


