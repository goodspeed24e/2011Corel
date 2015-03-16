// Gethdr.cpp: implementation of the CGethdr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "codedecoder.h"
#include "Gethdr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern int mv_outside_frame,long_vectors;
int adv_pred_mode,pb_frame;
extern int syntax_arith_coding;
int trb,trd,temp_ref;
extern int  quiet,source_format,pict_type,quant,bquant,verbose;
extern ldecode *ld;

CGethdr::CGethdr()
{

}

CGethdr::~CGethdr()
{

}

int CGethdr::getheader()
{
  unsigned int code, gob;
  startcode();  // 寻找startcode 
  code = m_getbits.getbits(PSC_LENGTH);//17
  gob = m_getbits.getbits(5);//GN
  if (gob == SEC) 
      return 0;
  if (gob == 0) //第一个块组和图像头再一块
  {
    getpicturehdr();
    if (syntax_arith_coding)		
      m_sacode.decoder_reset();	        
  }
  return gob + 1;

}

void CGethdr::startcode()
{
  while (m_getbits.showbits(PSC_LENGTH)!=1l) //移位搜索
  	 m_getbits.flushbits(1);

}

void CGethdr::getpicturehdr()
{
  int pos, pei, tmp;
  int prev_temp_ref;

  pos = ld->bitcnt;
  prev_temp_ref = temp_ref;
  //时间参考
  temp_ref = m_getbits.getbits(8);
  trd = temp_ref - prev_temp_ref;
  if (trd < 0)
    trd += 256;
  tmp = m_getbits.getbits(1); // 总是 "1"
  if (!tmp)
    if (!quiet)
      AfxMessageBox("warning: spare in picture header should be \"1\"!");
  tmp = m_getbits.getbits(1); // 总是 "0"
  if (tmp)
    if (!quiet)
      AfxMessageBox("warning: H.261 distinction bit should be \"0\"!");
  tmp = m_getbits.getbits(1); // split_screen_indicator
  if (tmp) 
  {
    if (!quiet)
      AfxMessageBox("error: split-screen not supported in this version!");
      exit (-1);
  }
  tmp = m_getbits.getbits(1); //document_camera_indicator 
  if (tmp)
    if (!quiet)
      AfxMessageBox("warning: document camera indicator not supported in this version!");

  tmp = m_getbits.getbits(1); // freeze_picture_release
  if (tmp)
    if (!quiet)
      AfxMessageBox("warning: frozen picture not supported in this version!");

  source_format = m_getbits.getbits(3);
  pict_type     = m_getbits.getbits(1);
  //可选模式
  mv_outside_frame = m_getbits.getbits(1);  //0
  long_vectors     = (mv_outside_frame ? 1 : 0);
  syntax_arith_coding = m_getbits.getbits(1); //SAC
  adv_pred_mode     = m_getbits.getbits(1);       //advance
  mv_outside_frame  = (adv_pred_mode ? 1 : mv_outside_frame);//Umv
  pb_frame = m_getbits.getbits(1); //pb-mode
  //量化因子
  quant    = m_getbits.getbits(5);    //quant
  tmp      = m_getbits.getbits(1);   //CPM
  if (tmp) 
  {
    if (!quiet)
      AfxMessageBox("error: CPM not supported in this version!");
    exit(-1);
  }
  if (pb_frame) 
  {
    trb    = m_getbits.getbits(3);
    bquant = m_getbits.getbits(2);
  }
  //PEI
  pei = m_getbits.getbits(1);
  while(pei)
  {
	  m_getbits.getbits(8);
	  pei=m_getbits.getbits(1);
  }
    
  if (verbose>0) 
  {
	CString picinfor,hdrinfor;
	picinfor.Format("picture header (byte %d)\n",(pos>>3)-4);
    if (verbose>1) 
	{
      hdrinfor.Format("  temp_ref=%d\n",temp_ref);
      picinfor+=hdrinfor;
	  hdrinfor.Format("  pict_type=%d\n",pict_type);
      picinfor+=hdrinfor;
	  hdrinfor.Format("  source_format=%d\n", source_format);
      picinfor+=hdrinfor;
	  hdrinfor.Format("  quant=%d\n",quant);
      picinfor+=hdrinfor;
	  if (syntax_arith_coding) 
      {
		  hdrinfor.Format("  SAC coding mode used \n");
		  picinfor+=hdrinfor;
	  }
      if (mv_outside_frame)
	  {
		  hdrinfor.Format("  unrestricted motion vector mode used\n");
		  picinfor+=hdrinfor;
	  }
      if (adv_pred_mode)
	  {
		  hdrinfor.Format("  advanced prediction mode used\n");
		  picinfor+=hdrinfor;
	  }
      if (pb_frame) 
	  {
	     hdrinfor.Format("  pb-frames mode used\n");
		 picinfor+=hdrinfor;
	     hdrinfor.Format("  trb=%d\n",trb);
	     picinfor+=hdrinfor;
		 hdrinfor.Format("  bquant=%d\n", bquant);
		 picinfor+=hdrinfor;
      }
    }
  }

}
