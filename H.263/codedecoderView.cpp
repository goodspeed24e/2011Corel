// 视频编解码器View.cpp : implementation of the CMyView class
//

#include "stdafx.h"
#include <math.h>                   
#include "codedecoder.h"

#include "codedecoderDoc.h"
#include "codedecoderView.h"
#include "OpenFiles.h"
#include "Global.h"
#include "ProgressBar.h"
#include "Display.h"
#include "PropertyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int pels,lines,mv_outside_frame,long_vectors,cpels;
CFile streamfile,psnrfile;
static void SeekPsnr(PictImage *curr,PictImage *recon,int width,int height,double psnr[3]);

#define OFFLINE_RATE_CONTROL//jwp
/////////////////////////////////////////////////////////////////////////////
// CMyView

IMPLEMENT_DYNCREATE(CMyView, CView)

BEGIN_MESSAGE_MAP(CMyView, CView)
	//{{AFX_MSG_MAP(CMyView)
	ON_COMMAND(ID_Encode, OnEncode)
	ON_COMMAND(ID_Decode, OnDecode)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_Encode, OnEncode)
	ON_COMMAND(ID_Decode, OnDecode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
// CMyView construction/destruction

CMyView::CMyView()

{
	m_font.CreateFont(20,0,0,0,FW_THIN,FALSE,FALSE,FALSE,GB2312_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH|FF_MODERN,"宋体");
	bFlag=0;
}

CMyView::~CMyView()
{
	m_font.DeleteObject();

}

BOOL CMyView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMyView drawing

void CMyView::OnDraw(CDC* pDC)
{
	CBitmap bitmap;
	CDC dcMemory;
	bitmap.LoadBitmap(IDB_car);
	dcMemory.CreateCompatibleDC(pDC);
	dcMemory.SelectObject(&bitmap);
	pDC->SetTextColor(0x64e896);

	if(bFlag==1)//编码
	{	
	   CFont *pOldFont;
	   pOldFont=pDC->SelectObject(&m_font);
	   pDC->SetBkMode(TRANSPARENT);

		pDC->StretchBlt(0,0,1024*3/4,768*3/4,&dcMemory,0,0,1024,768,SRCCOPY);
        CString kk;
		kk.Format("正在编码!共%d帧",MaxFrame);
		pDC->TextOut(60,60,kk);
		
		kk.Format("每%d帧进行一次帧内编码，进度情况请看状态栏!",Pbetween);
	    pDC->TextOut(60,100,kk);
	}
    else if(bFlag==2)//解码
	{
	CBrush Brush(RGB(100,200,150));//190,190,190
	CBrush* oldBrush=pDC->SelectObject(&Brush);
    CPen pen(0,2,RGB(100,200,150));
	CPen *oldPen=pDC->SelectObject(&pen);
	pDC->Rectangle(40,40,440,370);//画矩形
	pDC->MoveTo(40,370); // (wide,height)
	pDC->LineTo(620,370);
	pDC->MoveTo(620,40);        
	pDC->LineTo(620,370);
	pDC->MoveTo(40,40);        
	pDC->LineTo(620,40);	
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);

	//显示解码图象
	CDisplay m_display;
	m_display.play_movie(pDC,DecfileName);
	conclusion=m_display.conclusion;

	//输出解码结果
	pDC->SetTextColor(RGB(200,100,150));
	int dd=conclusion.Find(",",0);
	CString kk=conclusion.Right(conclusion.GetLength()-dd-1);
	int zz=kk.Find(",",0);
	pDC->TextOut(470,180,conclusion.Left(dd));
	pDC->TextOut(470,200,kk.Left(zz));
	pDC->TextOut(500,220,kk.Right(kk.GetLength()-zz-1));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMyView printing

BOOL CMyView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMyView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMyView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMyView diagnostics

#ifdef _DEBUG
void CMyView::AssertValid() const
{
	CView::AssertValid();
}

void CMyView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMyDoc* CMyView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMyDoc)));
	return (CMyDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMyView message handlers

void CMyView::OnEncode() 
{

	CPropertyDlg properties("编码属性设置",this,0);
	int ff;
	CString dd;
	if(properties.DoModal()==IDOK)
	{
		m_szFilePathName=properties.encodeDlg.m_InitDir;
		m_szFileName=m_szFilePathName;
		m_Type=properties.encodeDlg.typeindex;
		MaxFrame=properties.encodeDlg.m_MaxFrame;
        QP=properties.encodeDlg.m_QP;
		QPI=properties.encodeDlg.m_QPI;
		ifPsnr=properties.encodeDlg.m_ifPsnr;
		Pbetween=properties.encodeDlg.m_Pbetween+1;
        
		ratecontrol=properties.m_page1.m_ifratecontrol;
		targetrate=properties.m_page1.m_bitrate;
		switch(properties.m_page1.m_framerate)
		{
		case 1:
            ref_frame_rate=30.0;break;
		case 0:
            ref_frame_rate=25.0;break;
		case 2:
			ref_frame_rate=10.0;break;
		default:
			ref_frame_rate=25.0;
		}
		
		int kk=targetrate;
		ff=properties.m_page1.m_ifratecontrol;
		dd.Format("采用率控制，码率是%d，帧率是%f。",kk,ref_frame_rate);
        if(ff)
		MessageBox(dd);	

	if(m_Type==2)
		CodeYUV();
    else
		CodeBmps();
	}
	else
		return;

  
	
}

void CMyView::CodeYUV()
{ 
	//率控制
  #ifndef OFFLINE_RATE_CONTROL
    float DelayBetweenFramesInSeconds;
    int CommBacklog;
  #else
    PictImage *stored_image = NULL;
    int start_rate_control;
  #endif

	CMyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	//初始化
	CProgressBar bar(_T("Encode Progress"), 50, MaxFrame);
    m_pImageData=new unsigned char[352*288*3/2];
	CFile file;
	if((file.Open(m_szFilePathName,CFile::modeRead|CFile::shareDenyNone))==NULL)
	{
		AfxMessageBox("Can not open the yuv file!");
		return;
	}

	PictImage *prev_image = NULL;
    PictImage *curr_image = NULL;
    PictImage *curr_recon = NULL;
    PictImage *prev_recon = NULL;

	int frame_no,first_frameskip=0;  
    int start=1;//从第1帧到第MaxFrame帧
    int orig_frameskip=1;//输入图像原始偏移
    int frameskip=1;//非发送帧

	long_vectors =0;//帧间编码的参数
	mv_outside_frame=0;

	Pict *pic = (Pict *)malloc(sizeof(Pict));
	Bits *bits = (Bits *)malloc(sizeof(Bits));
	//率控制
	Bits *total_bits = (Bits *)malloc(sizeof(Bits));
    Bits *intra_bits = (Bits *)malloc(sizeof(Bits));

	pic->BQUANT = DEF_BQUANT;
    pic->seek_dist = DEF_SEEK_DIST;
    pic->use_gobsync = DEF_INSERT_SYNC;//=0
    pic->PB = 0;
    pic->TR = 0;
    pic->QP_mean = (float)0.0;
    pic->unrestricted_mv_mode = 0;
	pic->picture_coding_type =0; // PCT_INTRA;
	m_orgHeight=288;
	m_orgWidth=352;
    pic->source_format = SF_CIF;
	switch (pic->source_format) {
    case (SF_SQCIF):
      fprintf(stdout, "Encoding format: SQCIF (128x96)\n");
      pels = 128;
      lines = 96;
      break;
    case (SF_QCIF):
      fprintf(stdout, "Encoding format: QCIF (176x144)\n");
      pels = 176;
      lines = 144;
      break;
    case (SF_CIF):
      fprintf(stdout, "Encoding format: CIF (352x288)\n");
      pels = 352;
      lines = 288;
      break;
    case (SF_4CIF):
      fprintf(stdout, "Encoding format: 4CIF (704x576)\n");
      pels = 704;
      lines = 576;
      break;
    case (SF_16CIF):
      fprintf(stdout, "Encoding format: 16CIF (1408x1152)\n");
      pels = 1408;
      lines = 1152;
      break;
    default:
      fprintf(stderr,"Illegal coding format\n");
      exit(-1);
	}
    cpels = pels/2;
	curr_recon = InitImage(pels*lines);

//率控制
  #ifndef OFFLINE_RATE_CONTROL
  // rate control variables 
  pic->bit_rate = targetrate;
  pic->src_frame_rate = (int)(ref_frame_rate / orig_frameskip);
  DelayBetweenFramesInSeconds = (float) 1.0/(float)pic->src_frame_rate;
  InitializeRateControl();
  #endif
  #ifdef OFFLINE_RATE_CONTROL
  start_rate_control = 0;//DEF_START_RATE_CONTROL;
  #else
  pic->target_frame_rate = (float)DEF_TARGET_FRAME_RATE;
  #endif

	//建立输出文件
	CString outfilename=m_szFileName.Left(m_szFileName.GetLength()-4);
	CFileDialog dlg(FALSE,".263",outfilename,OFN_OVERWRITEPROMPT,"263 Files(*.263)|*.263|",NULL);
	if (dlg.DoModal()==IDOK)
	{
		bFlag=1;
		pDoc->UpdateAllViews(NULL);
 
		CString tempname;
		tempname=dlg.GetPathName();
		outfilename=tempname.Left(tempname.GetLength()-4);
	    if((streamfile.Open(tempname,CFile::modeWrite|CFile::modeCreate))==FALSE)
			AfxMessageBox("Can't create file!"); 
		streamfile.SeekToBegin();
     	initbits ();
		

		CTime StartTime=CTime::GetCurrentTime();
        CTimeSpan ElapsedTime;

    file.Read(m_pImageData,sizeof(BYTE)*352*288*3/2);
	pic->QUANT=QPI;
	curr_image = FillImage(m_pImageData);
	curr_recon = CodeOneIntra(curr_image, QPI, bits, pic);
	bits->header += alignbits (); // pictures shall be byte aligned 

	//率控制
  AddBitsPicture(bits);
  memcpy(intra_bits,bits,sizeof(Bits));
  ZeroBits(total_bits);
    //* number of seconds to encode *
  int chosen_frameskip=1;//jwp
    //* compute first frameskip *
  #ifdef OFFLINE_RATE_CONTROL
   float seconds = (MaxFrame - start + chosen_frameskip) * orig_frameskip/ ref_frame_rate;
   first_frameskip = chosen_frameskip;
   frameskip = chosen_frameskip;
  #else
  CommBacklog = intra_bits->total -(int)(DelayBetweenFramesInSeconds * pic->bit_rate);

  if (pic->bit_rate == 0) {
    frameskip = chosen_frameskip;
  }
  else {  //* rate control is used *
    frameskip = 1;
    while ( (int)(DelayBetweenFramesInSeconds*pic->bit_rate) <= CommBacklog) {
      CommBacklog -= (int) ( DelayBetweenFramesInSeconds * pic->bit_rate );
      frameskip += 1;
    }
  }
  first_frameskip = frameskip;
  #endif

	CString kk,m_Spsnr;
    if(ifPsnr)
	{
	if(psnrfile.Open(outfilename+".doc",CFile::modeWrite|CFile::modeCreate)==FALSE)
    MessageBox("Cannot create the output psnr file!", "Error",MB_ICONERROR | MB_OK);
	SeekPsnr(curr_image,curr_recon,352,288,psnrs);
	frame_no=1;
	kk.Format("第%d帧的lum峰值信噪比为%6.4fdB\n",frame_no,psnrs[0]);
	m_Spsnr=kk;
	kk.Format("第%d帧的Cb峰值信噪比为%6.4fdB\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4fdB\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;
	MessageBox(m_Spsnr);
	psnrfile.SeekToBegin();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
	}
 //第二帧 
pic->QUANT=QP;
   for(frame_no=start+first_frameskip;frame_no<=MaxFrame;frame_no+=frameskip)
	{
	file.Seek((frame_no-1)*352*288*3/2,SEEK_SET);
    file.Read(m_pImageData,sizeof(BYTE)*352*288*3/2);
	if(m_pImageData==NULL)
		  return;
	   
  	  pic->picture_coding_type =1; // PCT_INTER;

	  if(!ratecontrol)
	  pic->QUANT=QP;
	  
      prev_image=curr_image;
	  prev_recon=curr_recon;
      curr_image = FillImage(m_pImageData);
	  pic->TR+=(((frameskip+(pic->PB?98:0))*orig_frameskip)%256);
	  if(frameskip+(pic->PB?98:0)>256)
		MessageBox("Warning:frameskip>256");

     streamfile.SeekToEnd();
    if(((frame_no-1)%Pbetween)==0)
	 {
	   pic->picture_coding_type =0; // PCT_INTRA;
	   pic->QUANT=QPI;
       curr_recon = CodeOneIntra(curr_image, QPI, bits, pic);
	   AddBitsPicture(bits);
	   memcpy(intra_bits,bits,sizeof(Bits));

	 }
    else
	{ 
	  CodeOneInter(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic);
	  AddBitsPicture(bits); 
	}
   bits->header += alignbits (); //* pictures shall be byte aligned *

 
   if(ifPsnr)
   {	SeekPsnr(curr_image,curr_recon,352,288,psnrs);
	kk.Format("第%d帧的lum峰值信噪比为%6.4f\n",frame_no,psnrs[0]);
	m_Spsnr=kk;
	kk.Format("第%d帧的Cb峰值信噪比为%6.4f\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4f\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;
	psnrfile.SeekToEnd();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
   }
//率控制

   AddBits(total_bits, bits);
#ifndef OFFLINE_RATE_CONTROL
    if (pic->bit_rate != 0 && pic->PB)
      CommBacklog -= (int)
      ( DelayBetweenFramesInSeconds*pic->bit_rate ) * pdist;

    if (pic->bit_rate != 0) {
      UpdateRateControl(bits->total);

      CommBacklog += bits->total;
      frameskip = 1;
      CommBacklog -= (int)
        (frameskip * DelayBetweenFramesInSeconds *pic->bit_rate);

      while ( (int)(DelayBetweenFramesInSeconds*pic->bit_rate) <= CommBacklog)
      {
        CommBacklog -= (int) ( DelayBetweenFramesInSeconds * pic->bit_rate );
        frameskip += 1;
      }
    }
#else
    //* Aim for the targetrate with a once per frame rate control scheme *
    if (targetrate != 0)
      if (frame_no - start > (MaxFrame - start) * start_rate_control/100.0)

        pic->QUANT = FrameUpdateQP(total_bits->total + intra_bits->total,
           bits->total / (pic->PB?2:1),
           (MaxFrame-frame_no) / chosen_frameskip ,
           pic->QUANT, targetrate, seconds);
   frameskip = chosen_frameskip;
#endif
	  CString kkk;
	  kkk.Format("编码率为%d,%d帧的total_bits->total 为%d,intra_bits->total 为 %d, bits->total 为%d,new quant is %d.",
		  targetrate,frame_no,total_bits->total,intra_bits->total,bits->total,pic->QUANT);
	  if(ifPsnr)
	  {
		  psnrfile.SeekToEnd();
	      psnrfile.Write(kkk,kkk.GetLength());
	  }

	//显示进度信息
	CString str;
	str.Format("%d%% complete", frame_no*100/MaxFrame);
	bar.SetText(str);
	bar.StepIt();
    PeekAndPump(); //调用函数实现消息的转发
   }//end for frame_no
	
//  pDoc->SetModifiedFlag(TRUE);
  file.Close();
  if(ifPsnr)
  psnrfile.Close();
  streamfile.Close();
  delete prev_image;
  delete prev_recon;
  delete curr_image;
  curr_recon=NULL;
//  delete curr_recon;
  free(bits);
  free(pic);
  

    long hours,minutes,second;//计算所用的时间
    ElapsedTime = CTime::GetCurrentTime() - StartTime;
	bFlag=0;    
	hours = ElapsedTime.GetTotalHours();
    minutes = ElapsedTime.GetTotalMinutes();
    second = ElapsedTime.GetTotalSeconds();
    second = second + 60*minutes + 3600*hours;
    csTimeElapse.Format("编码%d帧视频序列,耗时:%d秒！",frame_no-1,second);
    MessageBox(csTimeElapse);
	pDoc->UpdateAllViews(NULL);
	}
	else
    MessageBox("No file is saved.","系统提示",MB_OK);
//	delete m_pImageData;

}

void CMyView::CodeBmps()
{
   //初始化
	CMyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CProgressBar bar(_T("Encode Progress"), 50, MaxFrame);

    m_pImageData=new unsigned char[352*288+352*288/2];
	m_pImageData = OpenImageFile(m_szFilePathName, &m_orgWidth, &m_orgHeight, m_Type);

    PictImage *prev_image = NULL;
    PictImage *curr_image = NULL;
    PictImage *curr_recon = NULL;
    PictImage *prev_recon = NULL;

	int frame_no,first_frameskip=0;  
    int start=2;//从第1帧到第MaxFrame帧
    int orig_frameskip=1;//输入图像原始偏移
    int frameskip=1;//非发送帧

	long_vectors =0;//帧间编码的参数
	mv_outside_frame=0;

	Pict *pic = (Pict *)malloc(sizeof(Pict));
	Bits *bits = (Bits *)malloc(sizeof(Bits));
	pic->BQUANT = DEF_BQUANT;
    pic->seek_dist = DEF_SEEK_DIST;
    pic->use_gobsync = DEF_INSERT_SYNC;//=0
    pic->PB = 0;
    pic->TR = 0;
    pic->QP_mean = (float)0.0;
    pic->unrestricted_mv_mode = 0;
	pic->picture_coding_type =0; // PCT_INTRA;
	if((m_orgHeight==288)&&(m_orgWidth==352))
       pic->source_format = SF_CIF;
	switch (pic->source_format) {
    case (SF_SQCIF):
      fprintf(stdout, "Encoding format: SQCIF (128x96)\n");
      pels = 128;
      lines = 96;
      break;
    case (SF_QCIF):
      fprintf(stdout, "Encoding format: QCIF (176x144)\n");
      pels = 176;
      lines = 144;
      break;
    case (SF_CIF):
      fprintf(stdout, "Encoding format: CIF (352x288)\n");
      pels = 352;
      lines = 288;
      break;
    case (SF_4CIF):
      fprintf(stdout, "Encoding format: 4CIF (704x576)\n");
      pels = 704;
      lines = 576;
      break;
    case (SF_16CIF):
      fprintf(stdout, "Encoding format: 16CIF (1408x1152)\n");
      pels = 1408;
      lines = 1152;
      break;
    default:
      fprintf(stderr,"Illegal coding format\n");
      exit(-1);
	}
    cpels = pels/2;
	curr_recon = InitImage(pels*lines);

	//建立输出文件
	CString outfilename=m_szFileName.Left(m_szFileName.GetLength()-4);
	CFileDialog dlg(FALSE,".263",outfilename,OFN_OVERWRITEPROMPT,"263 Files(*.263)|*.263|",NULL);
	if (dlg.DoModal()==IDOK)
	{  
		bFlag=1;
		pDoc->UpdateAllViews(NULL);

        CString tempname;
		tempname=dlg.GetPathName();
		outfilename=tempname.Left(tempname.GetLength()-4);

	    if((streamfile.Open(tempname,CFile::modeWrite|CFile::modeCreate))==FALSE)
			AfxMessageBox("Can't create file!"); 
		streamfile.SeekToBegin();
     	initbits ();
		
		CTime StartTime=CTime::GetCurrentTime();
        CTimeSpan ElapsedTime;

	pic->QUANT=QPI;
	curr_image = FillImage(m_pImageData);
	curr_recon = CodeOneIntra(curr_image, QPI, bits, pic);
	bits->header += alignbits (); /* pictures shall be byte aligned */

	CString kk,m_Spsnr;
   if(ifPsnr)
   {
	if(psnrfile.Open(outfilename+".doc",CFile::modeWrite|CFile::modeCreate)==FALSE)
    MessageBox("Cannot create the output psnr file!", "Error",MB_ICONERROR | MB_OK);
	SeekPsnr(curr_image,curr_recon,352,288,psnrs);
	frame_no=1;
	kk.Format("第%d帧的lum峰值信噪比为%6.4f\n",frame_no,psnrs[0]);
	m_Spsnr=kk;
	kk.Format("第%d帧的Cb峰值信噪比为%6.4f\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4f\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;
	MessageBox(m_Spsnr);
	psnrfile.SeekToBegin();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
   }

	 //第二帧 


    for(frame_no=start+first_frameskip;frame_no<=MaxFrame;frame_no+=frameskip)
	{
  	  m_szFilePathName = GetNextFileName(m_szFilePathName,1);
   	  m_pImageData = OpenImageFile(m_szFilePathName, &m_orgWidth, &m_orgHeight, m_Type);
	  if(m_pImageData==NULL)
		  return;
	   
  	  pic->picture_coding_type =1; // PCT_INTER;
	  pic->QUANT=QP;
      prev_image=curr_image;
	  prev_recon=curr_recon;
      curr_image = FillImage(m_pImageData);
	  pic->TR+=(((frameskip+(pic->PB?98:0))*orig_frameskip)%256);
	  if(frameskip+(pic->PB?98:0)>256)
		MessageBox("Warning:frameskip>256");

     streamfile.SeekToEnd();
     if(((frame_no-1)%Pbetween)==0)
	 {
	   pic->picture_coding_type =0; // PCT_INTRA;
	   pic->QUANT=QPI;
       curr_recon = CodeOneIntra(curr_image, QPI, bits, pic);
	 }
    else
    CodeOneInter(prev_image,curr_image,prev_recon,curr_recon,QP,frameskip,bits,pic);
    bits->header += alignbits (); /* pictures shall be byte aligned */
	
   if(ifPsnr)
   {	
	SeekPsnr(curr_image,curr_recon,352,288,psnrs);
	kk.Format("第%d帧的lum峰值信噪比为%6.4f\n",frame_no,psnrs[0]);
	m_Spsnr=kk;
	kk.Format("第%d帧的Cb峰值信噪比为%6.4f\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4f\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;
	psnrfile.SeekToEnd();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
   }

	//显示进度信息
	CString str;
	str.Format("%d%% complete", frame_no*100/MaxFrame);
	bar.SetText(str);
	bar.StepIt();
    PeekAndPump(); //调用函数实现消息的转发

   }//end for frame_no

//  pDoc->SetModifiedFlag(TRUE);
  if(ifPsnr)
  psnrfile.Close();

  streamfile.Close();
  delete prev_image;
  delete prev_recon;
  delete curr_image;
  curr_recon=NULL;
//  delete curr_recon;
  free(bits);
  free(pic);

    long hours,minutes,second;//计算所用的时间
    ElapsedTime = CTime::GetCurrentTime() - StartTime;
	bFlag=0;
    hours = ElapsedTime.GetTotalHours();
    minutes = ElapsedTime.GetTotalMinutes();
    second = ElapsedTime.GetTotalSeconds();
    second = second + 60*minutes + 3600*hours;

    csTimeElapse.Format("编码%d帧图像序列,耗时:%d秒！",frame_no-1,second);
    MessageBox(csTimeElapse);
	pDoc->UpdateAllViews(NULL);

	}
	else
    MessageBox("No file is saved.","系统提示",MB_OK);
	delete m_pImageData;
}

BOOL CMyView::PeekAndPump()
{

	static MSG msg;
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) 
	{
		if (!AfxGetApp()->PumpMessage()) 
		{
			::PostQuitMessage(0);
			return FALSE;
		}	
	}
	return TRUE;

}

void CMyView::OnDecode() 
{	
	CMyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CDC *pDC=GetDC();

	CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"263 Files(*.263)|*.263|");
	decfile.m_ofn.lpstrTitle="打开263文件";
	if(decfile.DoModal()==IDOK)
	   DecfileName=decfile.GetPathName();//获取263文件路径
	bFlag=2;	
	pDoc->UpdateAllViews(NULL);	
}

void SeekPsnr(PictImage *curr, PictImage *recon, int width,int height, double psnr[3])
{
	int i;
	double MSE1=0;
	double MSE2=0;

    for(i=0;i<width*height;i++)
	    MSE1+=((double)recon->lum[i]-(double)curr->lum[i])
			    *((double)recon->lum[i]-(double)curr->lum[i]);
	MSE1/=(width*height);
	if(MSE1==0)
		psnr[0]=0;
	else
		psnr[0]=10*log10(255*255/MSE1);

	MSE1=0;
	for(i=0;i<((width*height)>>2);i++)
	{
		MSE1+=((double)recon->Cb[i]-(double)curr->Cb[i])
			    *((double)recon->Cb[i]-(double)curr->Cb[i]);
		MSE2+=((double)recon->Cr[i]-(double)curr->Cr[i])
			    *((double)recon->Cr[i]-(double)curr->Cr[i]);
	}  
	MSE1/=(width*height)>>2;
	MSE2/=(width*height)>>2;
	if(MSE1==0)
		psnr[1]=0;
	else
		psnr[1]=10*log10(255*255/MSE1);
	if(MSE2==0)
		psnr[2]=0;
	else
		psnr[2]=10*log10(255*255/MSE2);

}



void CMyView::OnFileOpen() 
{
	CDC* pDC;
    pDC=GetDC();

	CFile look_result;
	CString resultshow="";
	CString kk;
   	CFileDialog lookresult(TRUE,NULL,NULL,OFN_HIDEREADONLY,"文本 Files(*.txt;*.doc)|*.txt;*.doc|");
	lookresult.m_ofn.lpstrTitle="打开编码结果文件";
	lookresult.m_ofn.lpstrInitialDir="F:\\standard_pictures\\MISSUSA_raw";
	if(lookresult.DoModal()==IDOK)
	{
	    resultshow=lookresult.GetPathName();
	    if (look_result.Open(resultshow,CFile::modeRead)==NULL)
		{
	    AfxMessageBox("Can not open the result file!");
	    return;
		}
	    look_result.SeekToBegin(); 
	}
	
}
