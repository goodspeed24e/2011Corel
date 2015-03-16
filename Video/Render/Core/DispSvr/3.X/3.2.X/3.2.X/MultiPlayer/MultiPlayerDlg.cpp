//------------------------------------------------------------------------------
// File: MultiPlayerDlg.cpp
//
// Desc: Display Server MultiPlayer sample
//
// Copyright (c) InterVideo Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <windows.h>
#include "MultiPlayer.h"
#include "MultiPlayerDlg.h"
#include "VMR9Subgraph.h"

#include <list>
using namespace std;

DEFINE_GUID(CLSID_IviDsDemultiplexer,0x5723ab6c, 0x81b8, 0x41fa, 0x99, 0x88, 0x9b, 0xbf, 0x25, 0x5f, 0xcb, 0xd2);

DEFINE_GUID(CLSID_VIDEO,0x246ca20, 0x776d, 0x11d2, 0x80, 0x10, 0x0, 0x10, 0x4b, 0x9b, 0x85, 0x92);

DEFINE_GUID(CLSID_CLdemux, 0x555c90cd, 0xd094, 0x4672, 0x9b, 0x26, 0xb7, 0x3b, 0xdd, 0x38, 0xfe, 0xee);

DEFINE_GUID(CLSID_CLAudio, 0x284dc28a, 0x4a7d, 0x442c, 0xbc, 0x2e, 0xd7, 0x48, 0x05, 0x56, 0xe4, 0xd8);

DEFINE_GUID(CLSID_CLVideo,0x8acd52ed, 0x9c2d, 0x4008, 0x91, 0x29, 0xdc, 0xe9, 0x55, 0xd8, 0x60, 0x65);

DEFINE_GUID(CLSID_MSDEMUX, 0xafb6c280,0x2c41, 0x11d3, 0x8a, 0x60, 0x00, 0x00, 0xf8, 0x1e, 0x0e,0x4a);

DEFINE_GUID(CLSID_CIVIAudioFilter,0x7e2e0dc1, 0x31fd, 0x11d2, 0x9c, 0x21, 0x0, 0x10, 0x4b, 0x38, 0x1, 0xf6);

DEFINE_GUID(AM_KSPROPSETID_DemuxConfiguration,	0xcbf67519, 0x1e9d, 0x47f6, 0xb7, 0xa2, 0x5d, 0xd7, 0x9f, 0x3a, 0xe9, 0x67);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const DWORD g_TimerID = 0x12345;
extern HINSTANCE g_hInstance;


// auxiliary array to simplify management of the dialog's controls
UINT g_nControlIDs[] = 
{
    IDC_COMBO_SOURCES,
    IDC_SLIDER_TIME,
    IDC_SLIDER_ALPHA,
    IDC_SLIDER_YPOS,
    IDC_SLIDER_XSIZE,
    IDC_SLIDER_ZORDER,
    IDC_SLIDER_XPOS,
    IDC_SLIDER_YSIZE,
    IDC_BUTTON_START,
    IDC_STATIC_TOTAL,
    IDC_STATIC_PATH,
    IDC_STATIC_STATE,
    IDC_STATIC_ALPHA,
    IDC_STATIC_XPOS,
    IDC_STATIC_XSIZE,
    IDC_STATIC_ZORDER,
    IDC_STATIC_YPOS,
    IDC_STATIC_YSIZE,
    IDC_SLIDER_SETFPS,
    IDC_STATIC_SETFPS,
};

int g_nControls = sizeof(g_nControlIDs) / sizeof(UINT);

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

/******************************Public*Routine******************************\
* CMultiPlayerSession::Initialize
*
* default initialization of the wizard
\**************************************************************************/
HRESULT CMultiPlayerSession::Initialize()
{
	HRESULT hr = S_OK;

	try
	{
		if (m_pWizard || m_pVideoRoot)
			throw VFW_E_WRONG_STATE;

		// create wizard
		hr = CoCreateInstance( CLSID_DisplayServer,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IDisplayServer,
			(void**)&m_pWizard);
		CHECK_HR( hr, DbgMsg("CMultiPlayerSession::Initialize: Failed to create instance of DisplayServer, hr = 0x%08x", hr));

		// create video window
		hr = CreateVideoWindow_(720, 480, WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_SYSMENU);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to create video window"));

		DWORD ddFlags = 0;

//		if (GetRegInt("DispSvrWFV", 0))
//			ddFlags |= DISPSVR_WAITING_FOR_VSYNC;

//		if (GetRegInt("DispSvrDeviceLostNotify", 0))
//			ddFlags |= DISPSVR_DEVICE_LOST_NOTIFY;

//		if (GetRegInt("DispSvrHijack", 0))
//			ddFlags |= DISPSVR_DETECT_D3D_HIJACK;

//		if (GetRegInt("DispSvrD3D9Ex", 1))
			ddFlags |= DISPSVR_USE_D3D9EX;

//		if (GetRegInt("DispSvrRTV", 0))
//			ddFlags |= DISPSVR_USE_RT_VIRTUALIZATION;

//		if (GetRegInt("DispSvrMode", 0))
//			ddFlags |= DISPSVR_USE_CUSTOMIZED_OUTPUT;

		// initialize wizard with default
		hr = m_pWizard->Initialize(ddFlags, m_hwndVideo, NULL);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to initialize the wizard, hr = 0x%08x", hr));

		// get interfaces
		hr = m_pWizard->GetRenderEngine( &m_pRenderEngine);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to get Render Engine, hr = 0x%08x", hr));

		// get top-most root
		CComPtr<IParentDisplayObject> pRoot;
		hr = m_pRenderEngine->GetRootObject(IID_IParentDisplayObject, (void**)&pRoot);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to get root object, hr = 0x%08x", hr));

		// create UI root
		CComPtr<IParentDisplayObject> pUIRoot;
		hr = CoCreateInstance(CLSID_CompositeDisplayObject, NULL, CLSCTX_INPROC_SERVER, IID_IParentDisplayObject, (void**)&pUIRoot);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to get root object, hr = 0x%08x", hr));
		hr = pRoot->AddChild(0, pUIRoot);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to get root object, hr = 0x%08x", hr));

		// create video root
		hr = CoCreateInstance(CLSID_CompositeDisplayObject, NULL, CLSCTX_INPROC_SERVER, IID_IParentDisplayObject, (void**)&m_pVideoRoot);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to get root object, hr = 0x%08x", hr));
		hr = pRoot->AddChild(1, m_pVideoRoot);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to get root object, hr = 0x%08x", hr));

		// add do into UI root
		CComPtr<IDisplayObject> pShineDO;
		hr = CoCreateInstance(CLSID_ShineDisplayObject, 
			NULL, CLSCTX_INPROC_SERVER,
			IID_IDisplayObject, 
			(void**)&pShineDO);
		CHECK_HR(hr, DbgMsg("CRenderEngine::Initialize: failed to create state DO, hr = 0x%08x", hr));
		hr = pUIRoot->AddChild(0, pShineDO);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to add obj, hr = 0x%08x", hr));

		CComPtr<IDisplayObject> pStateDO;
		hr = CoCreateInstance(CLSID_ServerStateDisplayObject, 
			NULL, CLSCTX_INPROC_SERVER,
			IID_IDisplayObject, 
			(void**)&pStateDO);
		CHECK_HR(hr, DbgMsg("CRenderEngine::Initialize: failed to create state DO, hr = 0x%08x", hr));
		hr = pUIRoot->AddChild(0, pStateDO);
		CHECK_HR(hr, DbgMsg("CMultiPlayerSession::Initialize: failed to add obj, hr = 0x%08x", hr));

		m_pUILayer = pRoot;
	}
	catch( HRESULT hr1 )
	{
		hr = hr1;
	}

	return hr;
}

/******************************Public*Routine******************************\
* CMultiPlayerSession::Terminate
*
* default termination of the wizard
\**************************************************************************/
HRESULT CMultiPlayerSession::Terminate()
{
	// since CMultiPlayerSession adds no data to the base CMultigraphSession,
	// default cleaning in the ~CMultigraphSession() is all we need
	CMultigraphSession::Terminate();
	m_pUILayer.Release();
	m_pWizard.Release();
	m_pRenderEngine.Release();
	m_pVideoRoot.Release();

	return S_OK;
}

// CMultiPlayerDlg dialog

/******************************Public*Routine******************************\
* CMultiPlayerDlg
*
* constructor
\**************************************************************************/
CMultiPlayerDlg::CMultiPlayerDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CMultiPlayerDlg::IDD, pParent)
    , m_pSession( NULL )
    , m_strTotal(_T(""))
    , m_strPath(_T(""))
    , m_strSourceState(_T(""))
    , m_strAlpha(_T(""))
    , m_strXPos(_T(""))
    , m_strXSize(_T(""))
    , m_strYPos(_T(""))
    , m_strYSize(_T(""))
    , m_strZOrder(_T(""))
    , m_strFPS(_T(""))
    , m_dwTotalSources(0L)
    , m_dwSourceCounter(0)
    , m_fSetFPS( 30.f)
    , m_fGetFPS( 30.f)
    , m_nTimer( 0)
    , m_bmpAttach( NULL )
    , m_bmpAttachGray( NULL )
    , m_bmpDetach( NULL )
    , m_bmpDetachGray( NULL )
    , m_bmpPlay( NULL )
    , m_bmpPlayGray( NULL )
    , m_bmpPause( NULL )
    , m_bmpPauseGray( NULL )
    , m_bmpScale( NULL )
    , m_bmpScaleGray( NULL )
    , m_bmpColor( NULL )
    , m_bmpColorGray( NULL )
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	UserDefDemux=SUBGRAPH_DEMUX_DEFAULT;
	UserDefVideo=SUBGRAPH_VIDEODECODER_DEFAULT;
	UserDefAudio=SUBGRAPH_AUDIODECODER_DEFAULT;
	UserDefVideoRender=SUBGRAPH_VIDEORENDERER_DEFAULT;
	UserDefVideoMixer=SUBGRAPH_VIDEOMIXER_DISPVIDEOMIXER;
}

/******************************Public*Routine******************************\
* DoDataExchange
*
* MFC-generated
\**************************************************************************/
void CMultiPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_TOTAL, m_strTotal);
	DDX_Text(pDX, IDC_STATIC_PATH, m_strPath);
	DDX_Text(pDX, IDC_STATIC_ALPHA, m_strAlpha);
	DDX_Text(pDX, IDC_STATIC_XPOS, m_strXPos);
	DDX_Text(pDX, IDC_STATIC_XSIZE, m_strXSize);
	DDX_Text(pDX, IDC_STATIC_YPOS, m_strYPos);
	DDX_Text(pDX, IDC_STATIC_YSIZE, m_strYSize);
	DDX_Text(pDX, IDC_STATIC_ZORDER, m_strZOrder);
	DDX_Text(pDX, IDC_STATIC_SETFPS, m_strFPS);
	DDX_Text(pDX, IDC_STATIC_START, m_strStartTime);
	DDX_Text(pDX, IDC_STATIC_CURTIME, m_strCurTime);
	DDX_Text(pDX, IDC_STATIC_STOP, m_strStopTime);
	DDX_Control(pDX, IDC_COMBO_SOURCES, m_comboSources);
	DDX_Control(pDX, IDC_SLIDER_TIME, m_sliderTime);
	DDX_Control(pDX, IDC_SLIDER_ALPHA, m_sliderAlpha);
	DDX_Control(pDX, IDC_SLIDER_XPOS, m_sliderXPos);
	DDX_Control(pDX, IDC_SLIDER_XSIZE, m_sliderXSize);
	DDX_Control(pDX, IDC_SLIDER_YPOS, m_sliderYPos);
	DDX_Control(pDX, IDC_SLIDER_YSIZE, m_sliderYSize);
	DDX_Control(pDX, IDC_SLIDER_ZORDER, m_sliderZOrder);
	DDX_Control(pDX, IDC_SLIDER_SETFPS, m_sliderFPS);
	DDX_Control(pDX, IDC_CHECK_EVR, m_checkboxEvr);
	DDX_Control(pDX, IDC_CHECK_IVIDSDEMUX, m_checkboxIVIDsDemux);
	DDX_Control(pDX, IDC_CHECK_MSDEMUX, m_checkboxMsDemux);
	DDX_Control(pDX, IDC_CHECK_CLDEMUX, m_checkboxCLDemux);
	DDX_Control(pDX, IDC_CHECK_IVIVIDEO, m_checkboxIVIVideo);
	DDX_Control(pDX, IDC_CHECK_CLVIDEO, m_checkboxCLVideo);
	DDX_Control(pDX, IDC_CHECK_IVIAUDIO, m_checkboxIVIAudio);
	DDX_Control(pDX, IDC_CHECK_CLAUDIO, m_checkboxCLAudio);
	DDX_Control(pDX, IDC_CHECK_VMR9, m_checkboxVMR9);
	DDX_Control(pDX, IDC_CHECK_IVIVIDEODXVA, m_checkboxIVIVideoDXVA);
	DDX_Control(pDX, IDC_CHECK_VIDEOSOURCEDO, m_checkboxVideoSourceDO);

	DDX_Control(pDX, IDC_RADIO_IVI_DEMUX, m_radioIVIDsDemux);
	DDX_Control(pDX, IDC_RADIO_CL_DEMUX, m_radioCLDemux);
	DDX_Control(pDX, IDC_RADIO_MS_DEMUX, m_radioMsDemux);
	DDX_Control(pDX, IDC_RADIO_IVI_VIDEO, m_radioIVIVideo);
	DDX_Control(pDX, IDC_RADIO_CL_VIDEO, m_radioCLVideo);
	DDX_Control(pDX, IDC_RADIO_IVI_AUDIO, m_radioIVIAudio);
	DDX_Control(pDX, IDC_RADIO_CL_AUDIO, m_radioCLAudio);
	DDX_Control(pDX, IDC_RADIO_DXVA, m_radioDXVA);

	DDX_Control(pDX, IDC_RADIO_EVR, m_radioEvr);
	DDX_Control(pDX, IDC_RADIO_VIDEOSOURCEDO, m_radioVideoSourceDO);

	DDX_Control(pDX, IDC_RADIO_DSHOWSYNC, m_radioDShowSync);
}

CButton m_radioIVIDsDemux;
CButton m_radioMsDemux;
CButton m_radioCLDemux;
CButton m_radioIVIVideo;
CButton m_radioCLVideo;
CButton m_radioIVIAudio;
CButton m_radioCLAudio;
CButton m_radioDXVA;


/******************************Public*Routine******************************\
* MESSAGE_MAP
*
* MFC-generated
\**************************************************************************/
BEGIN_MESSAGE_MAP(CMultiPlayerDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_WM_QUERYDRAGICON()
    ON_CBN_SELCHANGE(IDC_COMBO_SOURCES, OnCbnSelchangeComboSources)
    ON_BN_CLICKED(IDC_BUTTON_ATTACH, OnBnClickedButtonAttach)
    ON_BN_CLICKED(IDC_BUTTON_DETACH, OnBnClickedButtonDetach)
    ON_BN_CLICKED(IDC_BUTTON_PLAY, OnBnClickedButtonPlay)
    ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnBnClickedButtonPause)
    ON_BN_CLICKED(IDOK, OnOK)
    ON_BN_CLICKED(IDC_BUTTON_FIT, OnBnClickedButtonFit)
    ON_BN_CLICKED(IDC_BUTTON_COLOR, OnBnClickedButtonColor)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_ALPHA, OnNMReleasedcaptureSliderAlpha)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SETFPS, OnNMReleasedcaptureSliderSetfps)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_TIME, OnNMReleasedcaptureSliderTime)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_ZORDER, OnNMReleasedcaptureSliderZorder)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_XPOS, OnNMReleasedcaptureSliderXpos)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_YPOS, OnNMReleasedcaptureSliderYpos)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_XSIZE, OnNMReleasedcaptureSliderXsize)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_YSIZE, OnNMReleasedcaptureSliderYsize)
	ON_BN_CLICKED(IDC_RADIO_IVI_DEMUX, &CMultiPlayerDlg::OnBnClickedRadioIviDemux)
	ON_BN_CLICKED(IDC_RADIO_CL_DEMUX, &CMultiPlayerDlg::OnBnClickedRadioClDemux)
	ON_BN_CLICKED(IDC_RADIO_MS_DEMUX, &CMultiPlayerDlg::OnBnClickedRadioMsDemux)
	ON_BN_CLICKED(IDC_RADIO_IVI_VIDEO, &CMultiPlayerDlg::OnBnClickedRadioIviVideo)
	ON_BN_CLICKED(IDC_RADIO_CL_VIDEO, &CMultiPlayerDlg::OnBnClickedRadioClVideo)
	ON_BN_CLICKED(IDC_RADIO_EVR, &CMultiPlayerDlg::OnBnClickedRadioEvr)
	ON_BN_CLICKED(IDC_RADIO_DXVA, &CMultiPlayerDlg::OnBnClickedRadioDxva)
	ON_BN_CLICKED(IDC_RADIO_IVI_AUDIO, &CMultiPlayerDlg::OnBnClickedRadioIviAudio)
	ON_BN_CLICKED(IDC_RADIO_CL_AUDIO, &CMultiPlayerDlg::OnBnClickedRadioClAudio)
	ON_BN_CLICKED(IDC_RADIO_VIDEOSOURCEDO, &CMultiPlayerDlg::OnBnClickedRadioVideoSourceDO)
	ON_BN_CLICKED(IDC_RADIO_DSHOWSYNC, &CMultiPlayerDlg::OnBnClickedRadioDshowsync)
END_MESSAGE_MAP()


// CMultiPlayerDlg message handlers
/******************************Public*Routine******************************\
* OnInitDialog
*
* In addition to dialog initialization, set initial ranges and positions
* of sliders and default string values; load buttons' bitmaps, initialize
* timer.
* Once this is done, cocreate instance of the wizard
*
\**************************************************************************/
BOOL CMultiPlayerDlg::OnInitDialog()
{
	HRESULT hr = S_OK;
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // set sliders
    m_sliderAlpha.SetRange(0,100,TRUE);
    m_sliderTime.SetRange(0,100,TRUE);
    m_sliderXPos.SetRange(0,100,TRUE);
    m_sliderXSize.SetRange(0,100,TRUE);
    m_sliderYPos.SetRange(0,100,TRUE);
    m_sliderYSize.SetRange(0,100,TRUE);
    m_sliderZOrder.SetRange(0,1,TRUE);
    m_sliderFPS.SetRange(0,100,TRUE);

    m_sliderAlpha.SetTicFreq(5);
    m_sliderTime.SetTicFreq(5);
    m_sliderXPos.SetTicFreq(10);
    m_sliderYPos.SetTicFreq(10);
    m_sliderXSize.SetTicFreq(10);
    m_sliderYSize.SetTicFreq(10);
    m_sliderFPS.SetTicFreq(5);


    // EVR filter checking 
	IBaseFilter *pFilter = 0;
	hr = CoCreateInstance(CLSID_EnhancedVideoRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxEvr.EnableWindow(FALSE);
		m_radioEvr.EnableWindow(FALSE);
	}
	else
	{
		// If VISTA, default is EVR
		// If XP, default is VMR9

		OSVERSIONINFO  osviEx;

		ZeroMemory(&osviEx, sizeof(OSVERSIONINFO));
		osviEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(GetVersionEx((OSVERSIONINFO *) &osviEx))
		{
			if ( osviEx.dwPlatformId  == VER_PLATFORM_WIN32_NT && osviEx.dwMajorVersion >= 6 )  // VISTA or later
			{
				m_radioEvr.SetCheck(BST_CHECKED);
			}

		}
		pFilter->Release();
	}

	// IVI Demux filter checking
	hr = CoCreateInstance(CLSID_IviDsDemultiplexer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxIVIDsDemux.EnableWindow(FALSE);
		m_radioIVIDsDemux.EnableWindow(FALSE);
	}
	else
	{
		pFilter->Release();
	}

	// CL Demux filter checking
	hr = CoCreateInstance(CLSID_CLdemux, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxCLDemux.EnableWindow(FALSE);
		m_radioCLDemux.EnableWindow(FALSE);
	}
	else
	{
		pFilter->Release();
	}

	// MS Demux filter checking
	hr = CoCreateInstance(CLSID_MSDEMUX, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxMsDemux.EnableWindow(FALSE);
		m_radioMsDemux.EnableWindow(FALSE);
	}
	else
	{
		pFilter->Release();
	}

	// IVI Video filter checking
	hr = CoCreateInstance(CLSID_VIDEO, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxIVIVideo.EnableWindow(FALSE);
		m_radioIVIVideo.EnableWindow(FALSE);
	}
	else
	{
		pFilter->Release();
	}

	// CL Video filter checking
	hr = CoCreateInstance(CLSID_CLVideo, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxCLVideo.EnableWindow(FALSE);
		m_radioCLVideo.EnableWindow(FALSE);
	}
	else
	{
		pFilter->Release();
	}

	// IVI Audio filter checking
	hr = CoCreateInstance(CLSID_CIVIAudioFilter, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxIVIAudio.EnableWindow(FALSE);
		m_radioIVIAudio.EnableWindow(FALSE);
	}
	else
	{
		pFilter->Release();
	}

	// CL Audio filter checking
	hr = CoCreateInstance(CLSID_CLAudio, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pFilter));
	if (FAILED(hr))
	{
		m_checkboxCLAudio.EnableWindow(FALSE);
		m_radioCLAudio.EnableWindow(FALSE);
	}
	else
	{
		pFilter->Release();
	}

    HINSTANCE hInst = AfxGetInstanceHandle();
    m_bmpAttach = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_ATTACH));
    m_bmpAttachGray = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_ATTACH_DISABLE));
    m_bmpDetach = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_DETACH));
    m_bmpDetachGray = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_DETACH_DISABLE));
    m_bmpPlay = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_PLAY));
    m_bmpPlayGray = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_PLAY_DISABLE));
    m_bmpPause = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_PAUSE));
    m_bmpPauseGray = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_PAUSE_DISABLE));
    m_bmpScale = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_SCALE));
    m_bmpScaleGray = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_SCALE_DISABLE));
    m_bmpColor = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_COLOR));
    m_bmpColorGray = LoadBitmap( hInst, MAKEINTRESOURCE(IDB_BITMAP_COLOR_DISABLE));


    m_nTimer = SetTimer( g_TimerID, 500, 0);
    SetWindowPos(NULL,1,1,0,0,SWP_NOOWNERZORDER|SWP_NOSIZE);
    UpdateWindow();

    m_pSession = new CMultiPlayerSession;
    if( !m_pSession )
    {
        AfxMessageBox(_T("Memory allocation error"),MB_OK,0);
        return FALSE;
    }

    hr = m_pSession->Initialize();
    if( FAILED(hr))
    {
        AfxMessageBox(_T("Failed to create DispSvr object.\r\nMake sure you have registered DispSvr.dll"),MB_OK,0);
        return FALSE;
    }

    UpdateMediaButtons_();
    UpdateState_();
    UpdateSubgraphInfo_();

    UINT uintIDs[] = 
    {
        IDC_BUTTON_ATTACH,
        IDC_BUTTON_DETACH,
        IDC_BUTTON_PLAY,
        IDC_BUTTON_PAUSE,
        IDC_BUTTON_FIT,
        IDC_BUTTON_COLOR
    };

    UINT uintTips[] = 
    {
        IDS_TT_ADD,
        IDS_TT_DETACH,
        IDS_TT_PLAY,
        IDS_TT_PAUSE,
        IDS_TT_FIT,
        IDS_TT_BKCOLOR
    };

    // initialize tool tips
    for( int i=0; i< g_nButtons; i++ )
    {
        m_hwndToolTips[i] = ::CreateWindow(TOOLTIPS_CLASS, NULL,
                                         WS_POPUP | TTS_NOPREFIX,
                                         0, 0, 0, 0, NULL, NULL, g_hInstance, NULL );
        if( m_hwndToolTips[i] )
        {
            ::SetWindowPos( m_hwndToolTips[i], HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
            HWND hwndBtn = ::GetDlgItem( GetSafeHwnd(), uintIDs[i]);
            if( !IsWindow( hwndBtn ) )
            {
                continue;
            }

            m_ti[i].cbSize = sizeof( TOOLINFO );
            m_ti[i].uFlags = TTF_TRANSPARENT | TTF_CENTERTIP | TTF_SUBCLASS | TTF_IDISHWND;
            m_ti[i].hwnd = hwndBtn;
            m_ti[i].uId =  (UINT_PTR)hwndBtn;
            m_ti[i].hinst = g_hInstance;
            m_ti[i].lpszText =MAKEINTRESOURCE(uintTips[i]);
            ::GetClientRect( m_hwndToolTips[i], &(m_ti[i].rect));
            LRESULT lRes = ::SendMessage( m_hwndToolTips[i], TTM_ADDTOOL, 0, (LPARAM)&(m_ti[i]) );
        } // if
    }// for

    //m_ToolTips.Activate( TRUE );
    
    //SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOSIZE);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/******************************Public*Routine******************************\
* OnOK
*
* Response to "close" button
*
* First off, clean DispSvr-specific stuff,
* terminate and destroy Wizard
\**************************************************************************/
void CMultiPlayerDlg::OnOK()
{
    Clean_();
    HRESULT hr = S_OK;
    CString strMsg;

    if( m_pSession )
    {
        hr = m_pSession->Terminate();
        delete m_pSession;
        m_pSession = NULL;
    }

    CDialog::OnOK();
}


/******************************Public*Routine******************************\
* OnSysCommand
*
* MFC-generated
\**************************************************************************/
void CMultiPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/******************************Public*Routine******************************\
* OnPaint
*
* MFC-generated
\**************************************************************************/
void CMultiPlayerDlg::OnPaint() 
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

/******************************Public*Routine******************************\
* OnQueryDragIcon
*
* MFC-generated
\**************************************************************************/
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMultiPlayerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

/******************************Public*Routine******************************\
* OnDestroy
*
* clean DispSvr-specific stuff, delete bitmaps, kill the timer
\**************************************************************************/
void CMultiPlayerDlg::OnDestroy()
{
    CDialog::OnDestroy();

    KillTimer( m_nTimer );
    Clean_();

    DeleteObject( m_bmpAttach);
    DeleteObject( m_bmpAttachGray);
    DeleteObject( m_bmpDetach);
    DeleteObject( m_bmpDetachGray);
    DeleteObject( m_bmpPlay);
    DeleteObject( m_bmpPlayGray);
    DeleteObject( m_bmpPause);
    DeleteObject( m_bmpPauseGray);
    DeleteObject( m_bmpScale);
    DeleteObject( m_bmpScaleGray);
    DeleteObject( m_bmpColor);
    DeleteObject( m_bmpColorGray);

    for( int i=0; i<g_nButtons; i++)
    {
        ::SendMessage( m_hwndToolTips[i], TTM_DELTOOL, 0, (LPARAM)&(m_ti[i]) );
        ::DestroyWindow( m_hwndToolTips[i] );
    }

}


/******************************Public*Routine******************************\
* OnTimer
*
* call-back function to update dialog controls reflecting states and 
* properties of video sources
\**************************************************************************/
void CMultiPlayerDlg::OnTimer(UINT nIDEvent)
{
    ISubgraph *pSubgraph = NULL;
    int FramesPerSecBy100;
    int nItem = -1;
    DWORD dwZ = 0L;

    HRESULT hr = S_OK;

    if( !m_pSession )
    {
        return;
    }

    FramesPerSecBy100 = m_pSession->GetFrameRate();
    m_fGetFPS = (float)FramesPerSecBy100/100.f;

    nItem = m_comboSources.GetCurSel();
    if( nItem == -1 )
    {
        return;
    }

    IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
    pSubgraph = m_pSession->GetSubgraph(pMixer);
    // for actively selected source
    if( !pSubgraph )
    {
        return;
    }

    UpdateSubgraphInfo_();

    // also, for all sources, check if a movie is close to the end and 
    // restart to provide looping play mode
    m_pSession->LoopSources();

    CDialog::OnTimer(nIDEvent);
}

/******************************Public*Routine******************************\
* GetHHMMSS
*
* converts LONGLONG value of the reftime (timestamp from IMediaSeeking) to
* the CString of format "HH:MM:SS" (hours, minutes, seconds)
\**************************************************************************/
CString CMultiPlayerDlg::GetHHMMSS( LONGLONG llT )
{
    int hh;
    int mm;
    int ss;
    LONG SecTotal;
    CString strRet;

    SecTotal = (LONG)(llT / 10000000L);
    hh = (int)(SecTotal / 3600);
    mm = (int)((SecTotal - hh * 3600) / 60);
    ss = (int)(SecTotal - hh * 3600 - mm * 60);

    strRet.Format(_T("%02d:%02d:%02d"), hh, mm, ss);
    return strRet;
}

/******************************Public*Routine******************************\
* OnBnClickedButtonAttach
*
* response to "Attach" button
\**************************************************************************/
void CMultiPlayerDlg::OnBnClickedButtonAttach()
{
    TCHAR achPath[MAX_PATH];
    WCHAR wcPath[MAX_PATH];
    HRESULT hr = S_OK;
    int nItem;
    HWND hwnd = NULL;

    if( !m_pSession )
    {
        return;
    }

    if( m_pSession->GetSize() > 16 ) 
    {
        AfxMessageBox(_T("This sample allows to render no more than 16 different streams at once"), MB_OK, NULL);
        return;
    }

    CFileDialog file(TRUE, NULL, NULL,OFN_FILEMUSTEXIST,
        _T("Media Files (*.asf;*.avi;*.mpg;*.mpeg;*.wmv;*.trp)|*.asf;*.avi;*.mpg;*.mpeg;*.wmv;*.trp|All Files (*.*)|*.*\0\0"),
        NULL);

    if( IDOK == file.DoModal())
    {
        lstrcpy( achPath, LPCTSTR(file.GetPathName()));
#ifdef UNICODE
        wcscpy( wcPath, achPath );
#else
        MultiByteToWideChar(CP_ACP, 0, achPath,-1, wcPath, MAX_PATH);
#endif

		SUBGRAPH_DEMUX eDemux;
		SUBGRAPH_VIDEODECOER eVideo;
		SUBGRAPH_AUDIODECODER eAudio;
		SUBGRAPH_VIDEORENDERER eVideoRender;
		SUBGRAPH_VIDEOMIXER eVideoMixer;

		if ( m_radioIVIDsDemux.GetCheck() )
		{
			eDemux=SUBGRAPH_DEMUX_IVI;
		}
		else if ( m_radioMsDemux.GetCheck() )
		{
			eDemux=SUBGRAPH_DEMUX_MS;
		}
		else if ( m_radioCLDemux.GetCheck() )
		{
			eDemux=SUBGRAPH_DEMUX_CL;
		}
		else 
		{
			eDemux=SUBGRAPH_DEMUX_DEFAULT;
		}

		if ( m_radioIVIVideo.GetCheck() )
		{
			eVideo=SUBGRAPH_VIDEODECODER_IVI;
		}
		else if ( m_radioCLVideo.GetCheck() )
		{
			eVideo=SUBGRAPH_VIDEODECODER_CL;
		}
		else
		{
			eVideo=SUBGRAPH_VIDEODECODER_DEFAULT;
		}

		if ( m_radioIVIAudio.GetCheck() )
		{
			eAudio=SUBGRAPH_AUDIODECODER_IVI;
		}
		else if ( m_radioCLAudio.GetCheck() )
		{
			eAudio=SUBGRAPH_AUDIODECODER_CL;
		}
		else
		{
			eAudio=SUBGRAPH_AUDIODECODER_DEFAULT;
		}

		if ( m_radioEvr.GetCheck() )
		{
			eVideoRender=SUBGRAPH_VIDEORENDERER_EVR;
		}
		else
		{
			eVideoRender=SUBGRAPH_VIDEORENDERER_VMR9;
		}

		if ( m_radioVideoSourceDO.GetCheck() )
		{
			eVideoMixer=SUBGRAPH_VIDEOMIXER_VIDEOSOURCEDO;
		}
		else
		{
			eVideoMixer=SUBGRAPH_VIDEOMIXER_DISPVIDEOMIXER;
		}

		CComPtr<IDisplayVideoMixer> pMixer;
		hr = m_pSession->AddSource(wcPath, &pMixer, eDemux, eVideo, eAudio,eVideoRender, eVideoMixer);
        if( FAILED(hr))
        {
            CString strMessage;
            strMessage.Format(_T("Failed to attach media source to DispSvr. Error code 0x%08x"),hr);
            AfxMessageBox( strMessage, MB_OK, 0);
            return;
        }
        nItem = m_comboSources.AddString( achPath );
        m_comboSources.SetItemData( nItem, (DWORD) pMixer.p);
        m_comboSources.SetCurSel( nItem );

		if (pMixer && m_radioDShowSync.GetCheck())
		{
			CComPtr<IDisplayVideoSource> pVidSrc;
			pMixer->GetVideoSourceByIndex((eVideoMixer == SUBGRAPH_VIDEOMIXER_VIDEOSOURCEDO) ? 1:0, &pVidSrc);
			if (pVidSrc)
				pVidSrc->EnableInitiativeDisplay(TRUE);
			//else
			//{
			//	pMixer->GetVideoSourceByIndex(1, &pVidSrc);
			//	if (pVidSrc)
			//		pVidSrc->EnableInitiativeDisplay(TRUE);
			//}
		}

        hwnd = m_pSession->GetWindow();
        ::ShowWindow( hwnd, SW_SHOW);

        UpdateState_();
        UpdateSubgraphInfo_();
        
        UpdateMediaButtons_();
        OnBnClickedButtonPlay();
    }
}

/******************************Public*Routine******************************\
* OnBnClickedButtonDetach
*
* response to "Detach" button
\**************************************************************************/
void CMultiPlayerDlg::OnBnClickedButtonDetach()
{
    HRESULT hr = S_OK;
    int nItem = -1;
    HWND hwnd = NULL;

    if( !m_pSession )
        return;

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
        return;

    IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
    hr = m_pSession->DeleteSource(pMixer);
    if( FAILED( hr))
    {
        CString strMsg;
        strMsg.Format(TEXT("Failed to detach the graph from the wizard: hr = 0x%08x"), hr);
        MessageBox( LPCTSTR(strMsg));
        return;
    }

    m_comboSources.DeleteString(nItem);
    if( m_comboSources.GetCount() == 0)
    {
        m_comboSources.SetWindowText(TEXT(""));
        hwnd = m_pSession->GetWindow();
        ::ShowWindow( hwnd, SW_HIDE);
    }
    else
    {
        m_comboSources.SetCurSel(0);
    }

    UpdateData(FALSE);
    UpdateSubgraphInfo_();
    UpdateMediaButtons_();
}

/******************************Public*Routine******************************\
* OnBnClickedButtonPlay
*
* response to "Play" button
\**************************************************************************/
void CMultiPlayerDlg::OnBnClickedButtonPlay()
{
    HRESULT hr = S_OK;
    ISubgraph *pSubgraph = NULL;
    int nItem = -1;
    void* pData = NULL;

    if( !m_pSession )
    {
        return;
    }

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
    {
        return;
    }

    IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
    pSubgraph = m_pSession->GetSubgraph(pMixer);

    if( !pSubgraph ) 
        return;

    pSubgraph->Run();
    UpdateMediaButtons_();
}

/******************************Public*Routine******************************\
* OnBnClickedButtonPause
*
* response to "Pause" button
\**************************************************************************/
void CMultiPlayerDlg::OnBnClickedButtonPause()
{
    HRESULT hr = S_OK;
    ISubgraph *pSubgraph = NULL;
    int nItem = -1;
    void* pData = NULL;

    if( !m_pSession )
    {
        return;
    }

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
    {
        return;
    }

    IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
    pSubgraph = m_pSession->GetSubgraph(pMixer);

    if( !pSubgraph ) 
        return;

    pSubgraph->Pause();
    UpdateMediaButtons_();
}

/******************************Public*Routine******************************\
* OnBnClickedButtonColor
*
* response to "Set background color" button
\**************************************************************************/
void CMultiPlayerDlg::OnBnClickedButtonColor()
{
    HRESULT hr = S_OK;
    COLORREF color;
    CColorDialog ClrDlg(0,CC_ANYCOLOR,this);

    if( !m_pSession )
        return;

    if( IDOK == ClrDlg.DoModal())
    {
        color = ClrDlg.GetColor();
        m_pSession->SetColor( color );
    }
}

/******************************Public*Routine******************************\
* OnBnClickedButtonFit
*
* response to "Fit source to the window" button
\**************************************************************************/
void CMultiPlayerDlg::OnBnClickedButtonFit()
{
    HRESULT hr = S_OK;
    NORMALIZEDRECT nr;
    int nItem = -1;
    RECT rc;
    HWND hwnd = NULL;    

    if( !m_pSession )
        return;

    ::GetClientRect( hwnd, &rc);

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
    {
        return;
    }
    
	IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );

	CComPtr<IDisplayVideoSource> pVidSrc;
	pMixer->GetVideoSourceByIndex(0, &pVidSrc);

    hwnd = m_pSession->GetWindow();
    if( !IsWindow( hwnd ))
        return;

    ::GetClientRect( hwnd, &rc);

    hr = pMixer->GetIdealOutputRect( pVidSrc, rc.right, rc.bottom, &nr);
    hr = pMixer->SetOutputRect( pVidSrc, &nr);
}

/******************************Public*Routine******************************\
* OnCbnSelchangeComboSources
*
* response to selection change in the combobox
\**************************************************************************/
void CMultiPlayerDlg::OnCbnSelchangeComboSources()
{
    UpdateMediaButtons_();
    UpdateSubgraphInfo_();
}

/***************************** sliders ************************************/

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderAlpha
*
* slider for alpha
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderAlpha(NMHDR *pNMHDR, LRESULT *pResult)
{
    int nItem = -1;
    HRESULT hr = S_OK;
    float alpha;

    if( !m_pSession )
    {
        return;
    }

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
    {
        return;
    }
    
	IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData(nItem);

    int nPos = m_sliderAlpha.GetPos();
    alpha = (float)nPos/100.f;

	CComPtr<IDisplayVideoSource> pVidSrc;
	pMixer->GetVideoSourceByIndex(0, &pVidSrc);
	pMixer->SetAlpha(pVidSrc, alpha);
    *pResult = 0;
}

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderSetfps
*
* slider for desired FPS
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderSetfps(NMHDR *pNMHDR, LRESULT *pResult)
{
    HRESULT hr = S_OK;
    int nPos = m_sliderFPS.GetPos();

    CComPtr<IDisplayRenderEngine> pRenderEngine;

    if( !m_pSession )
        return;

    pRenderEngine = m_pSession->GetRenderEngine();

    if( !pRenderEngine )
        return;

    m_fSetFPS = (float)nPos;
    hr = pRenderEngine->SetFrameRate( nPos * 100);
    *pResult = 0;
}

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderTime
*
* slider for timeline
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderTime(NMHDR *pNMHDR, LRESULT *pResult)
{
    HRESULT hr = S_OK;
    ISubgraph *pSubgraph = NULL;
    LONGLONG  llCur, llDur;
    int nItem = -1;
    int nPos;

    if( !m_pSession )
        return;

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
        return;

    IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
    pSubgraph = m_pSession->GetSubgraph(pMixer);
    if( !pSubgraph )
        return;

    hr = pSubgraph->GetTimes( llCur, llDur );
    if( FAILED(hr))
        return;

    nPos = m_sliderTime.GetPos();
    llCur = nPos * llDur / 100L;
    pSubgraph->SetTime( llCur );

    *pResult = 0;
}

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderZorder
*
* slider for Z-order
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderZorder(NMHDR *pNMHDR, LRESULT *pResult)
{
    HRESULT hr = S_OK;
    int nItem = -1;
    int nPos;

    if (!m_pSession)
        return;

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
        return;

    IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem);
	IParentDisplayObject *pVideoRoot = m_pSession->GetVideoRoot();
    nPos = m_sliderZOrder.GetPos();

	if (pVideoRoot)
		pVideoRoot->SetZOrder(pMixer, nPos);
    *pResult = 0;
}

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderXpos
*
* slider for X-position
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderXpos(NMHDR *pNMHDR, LRESULT *pResult)
{
    UpdateOutputRect_();
    *pResult = 0;
}

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderYpos
*
* slider for Y-position
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderYpos(NMHDR *pNMHDR, LRESULT *pResult)
{
    UpdateOutputRect_();
    *pResult = 0;
}

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderXsize
*
* slider for X-size
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderXsize(NMHDR *pNMHDR, LRESULT *pResult)
{
    UpdateOutputRect_();
    *pResult = 0;
}

/******************************Public*Routine******************************\
* OnNMReleasedcaptureSliderYsize
*
* slider for Y-size
\**************************************************************************/
void CMultiPlayerDlg::OnNMReleasedcaptureSliderYsize(NMHDR *pNMHDR, LRESULT *pResult)
{
    UpdateOutputRect_();
    *pResult = 0;
}

////////////////////////////// PRIVATE ROUTINE /////////////////////////////

/******************************Private*Routine*****************************\
* Clean_
*
* Cleans DispSvr-specific data; stops all the subgraphs and detaches 
* them from the wizard; destroys m_listSources container; cleans combo box
*
\**************************************************************************/
void CMultiPlayerDlg::Clean_()
{
    HRESULT hr = S_OK;
    CString strMsg;

    // detach all players
    if( m_pSession )
    {
        m_pSession->Terminate();
        delete m_pSession;
        m_pSession = NULL;
    }
    m_comboSources.Clear();

    KillTimer( g_TimerID );
    Sleep(100);

}

/******************************Private*Routine*****************************\
* UpdateState_
*
* Updates controls depending on whether we have any source selected or not
* (when list is empty)
\**************************************************************************/
void CMultiPlayerDlg::UpdateState_()
{
    int i = 0;

    if( !m_pSession )
        return;

    int nItems = m_pSession->GetSize();

    if( 0 == nItems ) // we are empty
    {
        // disable anything but 'Add new video', 'total sources', and 'close',
        for( i=0; i<g_nControls; i++)
        {
            if( GetDlgItem( g_nControlIDs[i] ))
            {
                if( IDC_STATIC_TOTAL == g_nControlIDs[i] )
                {
                    GetDlgItem( g_nControlIDs[i])->EnableWindow( TRUE );
                }
                else
                {
                    GetDlgItem( g_nControlIDs[i])->EnableWindow( FALSE );
                }
            }
        }
    }
    else // we are not empty
    {
        // enable everything
        for( i=0; i<g_nControls; i++)
        {
            if( GetDlgItem( g_nControlIDs[i] ))
            {
                GetDlgItem( g_nControlIDs[i])->EnableWindow( TRUE );
            }
        }
    }

    UpdateData( FALSE );
}

/******************************Private*Routine*****************************\
* UpdateSubgraphInfo_
*
* Updates controls to reflect currently selected source
\**************************************************************************/
void CMultiPlayerDlg::UpdateSubgraphInfo_()
{
    int nItem = -1;
    ISubgraph *pSubgraph = NULL;
    void *pData = NULL;

    if( m_pSession )
    {
        m_strTotal.Format(_T("Total sources: %ld"), m_pSession->GetSize());
        m_strFPS.Format( _T("Desired FPS = %5.2f, Actual FPS = %5.2f"), m_fSetFPS, m_fGetFPS);
    }

    SetSliderPosition_( m_sliderFPS, (int)(m_fGetFPS + 10.f));

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem || !m_pSession)
    {
        // clean and return to defaults
        // update strings
        m_strPath = _T("");
        m_strAlpha.Format(_T("Alpha-level: 1.0"));
        m_strXPos.Format(_T("X-position = 0.0"));
        m_strYPos.Format(_T("Y-position = 0.0"));
        m_strXSize.Format(_T("X-size = 1.0"));
        m_strYSize.Format(_T("Y-size = 1.0"));
        m_strZOrder.Format(_T("Z-order = 0"));

        // update sliders
        SetSliderPosition_( m_sliderAlpha, 100);
        SetSliderPosition_( m_sliderXPos, 0);
        SetSliderPosition_( m_sliderYPos, 0);
        SetSliderPosition_( m_sliderXSize, 100);
        SetSliderPosition_( m_sliderYSize, 100);
        SetSliderPosition_( m_sliderZOrder, 0);
    }
    else
    {
        TCHAR achPath[MAX_PATH];
        float alpha = 1.f;
        LONG dwZ = 0;
        LONGLONG llCur;
        LONGLONG llDur;
        NORMALIZEDRECT nr;
        NORMALIZEDRECT nrI;
        HWND hwnd;
        RECT rc;

        IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
		IParentDisplayObject *pVideoRoot = m_pSession->GetVideoRoot();
		if (!pMixer)
			return;
        pSubgraph = m_pSession->GetSubgraph(pMixer);

		CComPtr<IDisplayVideoSource> pVidSrc;
		pMixer->GetVideoSourceByIndex(0, &pVidSrc);

        if (pSubgraph)
        {
            pSubgraph->GetPathT( achPath );
            m_strPath.Format( _T("%s"), achPath);

            // Update strings
            pMixer->GetAlpha( pVidSrc, &alpha );
            m_strAlpha.Format( _T("Alpha = %5.2f"), alpha );

            pSubgraph->GetTimes( llCur, llDur );
            m_strStartTime = GetHHMMSS( 0L);
            m_strCurTime = GetHHMMSS( llCur );
            m_strStopTime = GetHHMMSS( llDur );

			if (pVideoRoot)
			{
				pVideoRoot->GetZOrder(pMixer, &dwZ);
				m_strZOrder.Format( _T("Z-order = %ld"), dwZ);
			}

			SUBGRAPH_DEMUX eDemux;
			SUBGRAPH_VIDEODECOER eVideo;
			SUBGRAPH_AUDIODECODER eAudio;
			SUBGRAPH_VIDEORENDERER eVideoRender;
			SUBGRAPH_VIDEOMIXER eVideoMixer = UserDefVideoMixer;

			pSubgraph->GetFilterConnections(eDemux,eVideo,eAudio,eVideoRender);

#if 0
			char temp[1024];
			sprintf(temp,"edemux=%d evideo=%d eaudio=%d\n", eDemux,eVideo,eAudio);
			OutputDebugString(temp);
#endif

			m_checkboxIVIDsDemux.SetCheck(eDemux == SUBGRAPH_DEMUX_IVI);
			m_checkboxMsDemux.SetCheck(eDemux == SUBGRAPH_DEMUX_MS);
			m_checkboxCLDemux.SetCheck(eDemux == SUBGRAPH_DEMUX_CL);

			m_checkboxIVIVideo.SetCheck(eVideo == SUBGRAPH_VIDEODECODER_IVI);
			m_checkboxCLVideo.SetCheck(eVideo == SUBGRAPH_VIDEODECODER_CL);

			m_checkboxIVIAudio.SetCheck(eAudio == SUBGRAPH_AUDIODECODER_IVI);
			m_checkboxCLAudio.SetCheck(eAudio == SUBGRAPH_AUDIODECODER_CL);

			m_checkboxEvr.SetCheck(eVideoRender == SUBGRAPH_VIDEORENDERER_EVR);
			m_checkboxVMR9.SetCheck(eVideoRender == SUBGRAPH_VIDEORENDERER_VMR9);
			m_checkboxVideoSourceDO.SetCheck( eVideoMixer == SUBGRAPH_VIDEOMIXER_VIDEOSOURCEDO );

            // Update sliders
            SetSliderPosition_( m_sliderAlpha, (int)(alpha * 100.f));
            if( llDur )
            {
                SetSliderPosition_( m_sliderTime, 
                    (int)(llCur * 100 / llDur));
            }
            m_sliderZOrder.SetRange( 0, m_pSession->GetSize()-1, TRUE);
            SetSliderPosition_( m_sliderZOrder, dwZ );

            hwnd = m_pSession->GetWindow();
            ::GetClientRect( hwnd, &rc);

            pMixer->GetOutputRect( pVidSrc, &nr);
            pMixer->GetIdealOutputRect( pVidSrc, rc.right, rc.bottom, &nrI);

			float wi = nrI.right - nrI.left;
            float hi = nrI.top - nrI.bottom;
			float fYRatio = (nrI.top - nr.top) / hi;
			float fXRatio = (nr.left - nrI.left) / wi;
			float fWidthRatio = (nr.right - nr.left) / wi;
			float fHeightRatio = (nr.top - nr.bottom) / hi;

            // set strings
            m_strXPos.Format(_T("X-position = %5.2f"), fXRatio);
            m_strYPos.Format(_T("Y-position = %5.2f"), fYRatio);
            m_strXSize.Format(_T("X-size = %5.2f"), fWidthRatio);
            m_strYSize.Format(_T("Y-size = %5.2f"), fHeightRatio);

            // set sliders
            SetSliderPosition_( m_sliderXPos, (int)(fXRatio * 100.f));
            SetSliderPosition_( m_sliderYPos, (int)(fYRatio * 100.f));
            SetSliderPosition_( m_sliderXSize, (int)(fWidthRatio * 100.f));
            SetSliderPosition_( m_sliderYSize, (int)(fHeightRatio * 100.f));
        }
    }

    UpdateData(FALSE);
    return;
}

/******************************Private*Routine*****************************\
* UpdateMediaButtons_
*
* Updates media buttons depending on the number of attached sources and 
* the state of currently selected source
\**************************************************************************/
void CMultiPlayerDlg::UpdateMediaButtons_()
{
    OAFilterState state;
    ISubgraph *pSubgraph = NULL;
    int nItems;

    if( !m_pSession )
        return;

    nItems = m_pSession->GetSize();

    if( GetDlgItem(IDC_BUTTON_ATTACH) )
    {
        ::SendMessage(  GetDlgItem(IDC_BUTTON_ATTACH)->GetSafeHwnd(), 
                        BM_SETIMAGE, 
                        (WPARAM)IMAGE_BITMAP, 
                        (LPARAM)( nItems<16 ? m_bmpAttach : m_bmpAttachGray) );
    }
    if( GetDlgItem(IDC_BUTTON_DETACH) )
    {
        ::SendMessage(  GetDlgItem(IDC_BUTTON_DETACH)->GetSafeHwnd(), 
                        BM_SETIMAGE, 
                        (WPARAM)IMAGE_BITMAP, 
                        (LPARAM)(nItems>0 ? m_bmpDetach : m_bmpDetachGray));
    }

    if( GetDlgItem(IDC_BUTTON_COLOR) )
    {
        ::SendMessage(  GetDlgItem(IDC_BUTTON_COLOR)->GetSafeHwnd(), 
                        BM_SETIMAGE, 
                        (WPARAM)IMAGE_BITMAP, 
                        (LPARAM)m_bmpColor);
    }


    if( 0 < m_comboSources.GetCount() && m_pSession)
    {
        int nItem = 0;
        
        nItem = m_comboSources.GetCurSel();
        if( -1 != nItem )
        {
            IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
            pSubgraph = m_pSession->GetSubgraph(pMixer);
        }
    }

    if( NULL == pSubgraph )
    {
        ::SendMessage(  GetDlgItem(IDC_BUTTON_PLAY)->GetSafeHwnd(), 
                        BM_SETIMAGE, 
                        (WPARAM)IMAGE_BITMAP, 
                        (LPARAM)(m_bmpPlayGray));
        ::SendMessage(  GetDlgItem(IDC_BUTTON_PAUSE)->GetSafeHwnd(), 
                        BM_SETIMAGE, 
                        (WPARAM)IMAGE_BITMAP, 
                        (LPARAM)(m_bmpPauseGray));

        ::SendMessage(  GetDlgItem(IDC_BUTTON_FIT)->GetSafeHwnd(), 
                        BM_SETIMAGE, 
                        (WPARAM)IMAGE_BITMAP, 
                        (LPARAM)(m_bmpScaleGray));
    }
    else
    {
        ::SendMessage(  GetDlgItem(IDC_BUTTON_FIT)->GetSafeHwnd(), 
                        BM_SETIMAGE, 
                        (WPARAM)IMAGE_BITMAP, 
                        (LPARAM)(m_bmpScale));

        state = pSubgraph->GetState();
        if( State_Running == state )
        {
            ::SendMessage(  GetDlgItem(IDC_BUTTON_PLAY)->GetSafeHwnd(), 
                            BM_SETIMAGE, 
                            (WPARAM)IMAGE_BITMAP, 
                            (LPARAM)(m_bmpPlayGray));
            ::SendMessage(  GetDlgItem(IDC_BUTTON_PAUSE)->GetSafeHwnd(), 
                            BM_SETIMAGE, 
                            (WPARAM)IMAGE_BITMAP, 
                            (LPARAM)(m_bmpPause));

        }
        else
        {
            ::SendMessage(  GetDlgItem(IDC_BUTTON_PLAY)->GetSafeHwnd(), 
                            BM_SETIMAGE, 
                            (WPARAM)IMAGE_BITMAP, 
                            (LPARAM)(m_bmpPlay));
            ::SendMessage(  GetDlgItem(IDC_BUTTON_PAUSE)->GetSafeHwnd(), 
                            BM_SETIMAGE, 
                            (WPARAM)IMAGE_BITMAP, 
                            (LPARAM)(m_bmpPauseGray));
        }
    }
}

/******************************Private*Routine*****************************\
* UpdateOutputRect_
*
* Transforms positions of "pos" and "size" sliders to NORMALIZEDRECT
\**************************************************************************/
void CMultiPlayerDlg::UpdateOutputRect_()
{
    int nItem = -1;
    ISubgraph *pSubgraph = NULL;
    int nPos = 0;
    NORMALIZEDRECT nr;
    NORMALIZEDRECT nrI;
    HWND hwnd;
    RECT rc;
    HRESULT hr = S_OK;
    
    if( !m_pSession )
        return;

    nItem = m_comboSources.GetCurSel();
    if( -1 == nItem )
        return;

    IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
    pSubgraph = m_pSession->GetSubgraph(pMixer);
    if( !pSubgraph )
        return;

	CComPtr<IDisplayVideoSource> pVidSrc;
	pMixer->GetVideoSourceByIndex(0, &pVidSrc);

    hwnd = m_pSession->GetWindow();
    if( !IsWindow( hwnd ))
        return;

    ::GetClientRect( hwnd, &rc);

    pMixer->GetOutputRect( pVidSrc, &nr);
    pMixer->GetIdealOutputRect( pVidSrc, rc.right, rc.bottom, &nrI);

	float wi = nrI.right - nrI.left;
    float hi = nrI.top - nrI.bottom;

    if (0.f == wi || 0.f == hi)
        return;

	float fXRatio = m_sliderXPos.GetPos() / 100.f;
	float fYRatio = m_sliderYPos.GetPos() / 100.f;
	float fWidthRatio = m_sliderXSize.GetPos() / 100.f;
	float fHeightRatio = m_sliderYSize.GetPos() / 100.f;

	nr.top    = nrI.top - hi * fYRatio;
	nr.left   = nrI.left + wi * fXRatio;
    nr.right  = nr.left + wi * fWidthRatio;
    nr.bottom = nr.top - hi * fHeightRatio;

    pMixer->SetOutputRect( pVidSrc, &nr);
}

/******************************Private*Routine*****************************\
* SetSliderPosition_
*
* Sets slider position only if slider is not captured by mouse
\**************************************************************************/
void CMultiPlayerDlg::SetSliderPosition_( CSliderCtrl& slider, int Pos)
{
    CWnd *pwnd = GetCapture();

    if( pwnd ==(CWnd*)(&slider) )
    {
        return;
    }
    slider.SetPos( Pos );
}



void CMultiPlayerDlg::OnBnClickedRadioIviDemux()
{
	if ( !m_radioIVIDsDemux.GetCheck())
	{
		UserDefDemux=SUBGRAPH_DEMUX_DEFAULT;
		m_radioIVIDsDemux.SetCheck(FALSE);
	}
	else
	{
		UserDefDemux=SUBGRAPH_DEMUX_IVI;
		m_radioIVIDsDemux.SetCheck(TRUE);
		m_radioCLDemux.SetCheck(FALSE);
		m_radioMsDemux.SetCheck(FALSE);
	}
}

void CMultiPlayerDlg::OnBnClickedRadioClDemux()
{
	if ( !m_radioCLDemux.GetCheck())
	{
		UserDefDemux=SUBGRAPH_DEMUX_DEFAULT;
		m_radioCLDemux.SetCheck(FALSE);
	}
	else
	{
		UserDefDemux=SUBGRAPH_DEMUX_CL;
		m_radioIVIDsDemux.SetCheck(FALSE);
		m_radioCLDemux.SetCheck(TRUE);
		m_radioMsDemux.SetCheck(FALSE);
	}
}

void CMultiPlayerDlg::OnBnClickedRadioMsDemux()
{
	if ( !m_radioMsDemux.GetCheck())
	{
		UserDefDemux=SUBGRAPH_DEMUX_DEFAULT;
		m_radioMsDemux.SetCheck(FALSE);
	}
	else
	{
		UserDefDemux=SUBGRAPH_DEMUX_MS;
		m_radioIVIDsDemux.SetCheck(FALSE);
		m_radioCLDemux.SetCheck(FALSE);
		m_radioMsDemux.SetCheck(TRUE);
	}
}

void CMultiPlayerDlg::OnBnClickedRadioIviVideo()
{
	if ( !m_radioIVIVideo.GetCheck())
	{
		UserDefVideo=SUBGRAPH_VIDEODECODER_DEFAULT;
		m_radioIVIVideo.SetCheck(FALSE);
	}
	else
	{
		UserDefVideo=SUBGRAPH_VIDEODECODER_IVI;
		m_radioIVIVideo.SetCheck(TRUE);
		m_radioCLVideo.SetCheck(FALSE);
	}
}

void CMultiPlayerDlg::OnBnClickedRadioClVideo()
{
	if ( !m_radioCLVideo.GetCheck())
	{
		UserDefVideo=SUBGRAPH_VIDEODECODER_DEFAULT;
		m_radioCLVideo.SetCheck(FALSE);
	}
	else
	{
		UserDefVideo=SUBGRAPH_VIDEODECODER_CL;
		m_radioIVIVideo.SetCheck(FALSE);
		m_radioCLVideo.SetCheck(TRUE);
	}
}

void CMultiPlayerDlg::OnBnClickedRadioEvr()
{
	if (!m_radioEvr.GetCheck())
		UserDefVideoRender=SUBGRAPH_VIDEORENDERER_EVR;
	else
		UserDefVideoRender=SUBGRAPH_VIDEORENDERER_VMR9;
}

void CMultiPlayerDlg::OnBnClickedRadioDxva()
{
	// TODO: Add your control notification handler code here
}

void CMultiPlayerDlg::OnBnClickedRadioIviAudio()
{
	if (!m_radioIVIAudio.GetCheck())
	{
		UserDefAudio=SUBGRAPH_AUDIODECODER_DEFAULT;
		m_radioIVIAudio.SetCheck(FALSE);
	}
	else
	{
		UserDefAudio=SUBGRAPH_AUDIODECODER_IVI;
		m_radioIVIAudio.SetCheck(TRUE);
		m_radioCLAudio.SetCheck(FALSE);
	}
}

void CMultiPlayerDlg::OnBnClickedRadioClAudio()
{
	if (!m_radioCLAudio.GetCheck())
	{
		UserDefAudio=SUBGRAPH_AUDIODECODER_DEFAULT;
		m_radioCLAudio.SetCheck(FALSE);
	}
	else
	{
		UserDefAudio=SUBGRAPH_AUDIODECODER_CL;
		m_radioIVIAudio.SetCheck(FALSE);
		m_radioCLAudio.SetCheck(TRUE);
	}
}

void CMultiPlayerDlg::OnBnClickedRadioVideoSourceDO()
{
	if(!m_radioVideoSourceDO.GetCheck())
		UserDefVideoMixer=SUBGRAPH_VIDEOMIXER_DISPVIDEOMIXER;
	else
		UserDefVideoMixer=SUBGRAPH_VIDEOMIXER_VIDEOSOURCEDO;
}

void CMultiPlayerDlg::OnBnClickedRadioDshowsync()
{
	// TODO: Add your control notification handler code here
    ISubgraph *pSubgraph = NULL;
	int nItem = -1;
	nItem = m_comboSources.GetCurSel();
	if( nItem == -1 )
	{
		return;
	}

	IDisplayVideoMixer* pMixer = (IDisplayVideoMixer*) m_comboSources.GetItemData( nItem );
	pSubgraph = m_pSession->GetSubgraph(pMixer);
	// for actively selected source
	if( !pSubgraph )
	{
		return;
	}

	if (pMixer)
	{
		BOOL value = m_radioDShowSync.GetCheck();
		CComPtr<IDisplayVideoSource> pVidSrc;
		pMixer->GetVideoSourceByIndex(0, &pVidSrc);
		if (pVidSrc)
		{
			pVidSrc->EnableInitiativeDisplay(value);
		}
		else
		{
			pMixer->GetVideoSourceByIndex(1, &pVidSrc); // For VideoSourceDo
			if (pVidSrc)
			{
				pVidSrc->EnableInitiativeDisplay(value);
			}
		}
	}
}
