// EncodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "codedecoder.h"
#include "EncodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEncodeDlg property page

IMPLEMENT_DYNCREATE(CEncodeDlg, CPropertyPage)

CEncodeDlg::CEncodeDlg() : CPropertyPage(CEncodeDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncodeDlg)
	m_ifPsnr = FALSE;
	m_InitDir = _T("F:\\standard_pictures\\MISSUSA_raw\\mobile.yuv");
	m_InType = _T("yuv文件");
	m_MaxFrame = 120;
	m_Pbetween = 19;
	m_QP = 4;
	m_QPI = 8;
	//}}AFX_DATA_INIT
	typeindex=2;
}

CEncodeDlg::~CEncodeDlg()
{
}

void CEncodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncodeDlg)
	DDX_Check(pDX, IDC_CHECK1, m_ifPsnr);
	DDX_Text(pDX, IDC_InitDir, m_InitDir);
	DDX_CBString(pDX, IDC_InType, m_InType);
	DDX_Text(pDX, IDC_MaxFrame, m_MaxFrame);
	DDV_MinMaxInt(pDX, m_MaxFrame, 3, 500);
	DDX_Text(pDX, IDC_Pnum, m_Pbetween);
	DDV_MinMaxInt(pDX, m_Pbetween, 0, 29);
	DDX_Text(pDX, IDC_QP, m_QP);
	DDV_MinMaxInt(pDX, m_QP, 2, 31);
	DDX_Text(pDX, IDC_QPI, m_QPI);
	DDV_MinMaxInt(pDX, m_QPI, 2, 31);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEncodeDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CEncodeDlg)
	ON_BN_CLICKED(IDC_Browse, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncodeDlg message handlers

void CEncodeDlg::OnBrowse() 
{
	UpdateData(true);

    static char szFilter[] = "BMP Files(*.bmp)|*.bmp|YUV File(*.yuv)|*.yuv|RAW Files(*.raw)|*.raw|SIF Files(*.sif)|*.sif|";

	CFileDialog FileDlg( TRUE, NULL,NULL,OFN_HIDEREADONLY, szFilter);
	FileDlg.m_ofn.lpstrInitialDir="F:\\standard_pictures\\MISSUSA_raw";

	if(m_InType=="bmp序列")
        typeindex=1;
	else if(m_InType=="YUV文件")
	    typeindex=2;
	else if(m_InType=="raw序列")
		typeindex=3;
	else if(m_InType=="sif 序列")
		typeindex=4;


	FileDlg.m_ofn.nFilterIndex=typeindex;

    if( FileDlg.DoModal() == IDOK ) 
	{
		m_InitDir=FileDlg.GetPathName();
  	    CString dd=FileDlg.GetFileExt();
		CString szFileExt = "bmpyuvrawsif";
		dd.MakeLower();
		int type= szFileExt.Find(dd);
		if(typeindex!=type/3+1)
		{  
			AfxMessageBox("文件类型错误!");
            m_InitDir = "F:\\standard_pictures\\MISSUSA_raw\\mobile.yuv";
		    m_InType=="yuv文件";
		    typeindex=2;
		}
	}
	else
	{  
		 m_InitDir = "F:\\standard_pictures\\MISSUSA_raw\\mobile.yuv";
		 m_InType=="yuv文件";
		 typeindex=2;
	}
    UpdateData(false);
}



BOOL CEncodeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    browse.AutoLoad(IDC_Browse,this);
	browse.SizeToContent();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
