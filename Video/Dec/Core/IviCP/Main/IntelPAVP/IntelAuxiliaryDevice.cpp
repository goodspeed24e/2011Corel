
#include "IntelAuxiliaryDevice.h"
//#include "DVDPLAYER\LIBDS\dxva.h"

#define HD_WIDTH	1920
#define HD_HEIGHT	1080

#define MAX_VAR 1024
#define AUXDEVICE_DPMESSAGE DP
//#define DP

VOID CIntelAuxiliaryDevice::DP(char* szMsg, ...)
{
	if(m_bShowMsg)
	{		
		char szFullMessage[MAX_PATH];
		char szFormatMessage[MAX_PATH];

		// format message
		va_list ap;
		va_start(ap, szMsg);
		_vsnprintf_s(szFormatMessage, MAX_PATH, _TRUNCATE, szMsg, ap);
		va_end(ap);
		strncat_s(szFormatMessage, MAX_PATH,"\n", MAX_PATH);				
		strcpy_s(szFullMessage, MAX_PATH, "[PAVP] ");
		strcat_s(szFullMessage, MAX_PATH, szFormatMessage);		
		OutputDebugStringA(szFullMessage);
	}
}

CIntelAuxiliaryDevice::CIntelAuxiliaryDevice(IMFGetService *pMFGetService):
CiviDxva2Decoder(pMFGetService), m_pAuxiliaryDevice(NULL), m_pPictureDecode(NULL), m_bShowMsg(FALSE)
{
	AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::Create\n");

	UINT uiConfig=0, uiDecoders=0, i=0;
	GUID      *pGuidDecoders = NULL;
	D3DFORMAT	*pFormats = NULL;

	GetVideoAccelService();

	SetRenderTargetInfo(64, 64, NULL, (GUID*)(&DXVA2_Intel_Auxiliary_Device), 1);				

	GetDecoderDeviceGUIDs(&uiDecoders, &pGuidDecoders);

	for (i = 0; i < uiDecoders; i++)
	{
		if (IsEqualGUID(pGuidDecoders[i], DXVA2_Intel_Auxiliary_Device)) break;
	}

	CoTaskMemFree(pGuidDecoders);

	if (i >= uiDecoders)
	{
		DP("Failed to find Intel Auxiliary Device Interface error\n");
		return;
	}

	AllocRenderTargets(1, (D3DFORMAT)MAKEFOURCC('I', 'M', 'C', '3'));

	m_DeviceGuid = DXVA2_Intel_Auxiliary_Device;
	m_sDxva2VidDesc.SampleWidth                         = 64;
	m_sDxva2VidDesc.SampleHeight                        = 64;
	m_sDxva2VidDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	m_sDxva2VidDesc.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
	m_sDxva2VidDesc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
	m_sDxva2VidDesc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	m_sDxva2VidDesc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	m_sDxva2VidDesc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
	m_sDxva2VidDesc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
	m_sDxva2VidDesc.Format                              = *(D3DFORMAT*)GetRenderTargetFormat();
	m_sDxva2VidDesc.InputSampleFreq.Numerator           = m_sDxva2VidDesc.OutputFrameFreq.Numerator   = 60000;
	m_sDxva2VidDesc.InputSampleFreq.Denominator         = m_sDxva2VidDesc.OutputFrameFreq.Denominator = 1001;
	m_sDxva2VidDesc.UABProtectionLevel                  = 0;
	m_sDxva2VidDesc.Reserved                            = 0;

	GetDecoderConfigurations(&m_sDxva2VidDesc, &uiConfig, &m_pPictureDecode);

	CreateVideoDecoder(&m_sDxva2VidDesc, &m_pPictureDecode[0], &m_pAuxiliaryDevice);
}

CIntelAuxiliaryDevice::CIntelAuxiliaryDevice(IDirect3DDevice9 *pD3DDevice9):
CiviDxva2Decoder((IDirect3DDevice9 *)NULL), m_pAuxiliaryDevice(NULL), m_pPictureDecode(NULL), m_bShowMsg(FALSE)
{
	AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::Create\n");

	UINT uiConfig=0, uiDecoders=0, i=0;
	GUID      *pGuidDecoders = NULL;
	D3DFORMAT	*pFormats = NULL;


	//Create D3D Device Begin.
	HRESULT hr = E_FAIL;
	IDirect3D9	*gpD3D;
	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	gpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	
	if(gpD3D==NULL)
	{
		AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::Create D3D::Direct3DCreate9() fail.\n");
		return;
	}
	else
		AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::Create D3D::Direct3DCreate9() Succeed.\n");

	d3dpp.BackBufferWidth	= HD_WIDTH;
	d3dpp.BackBufferHeight	= HD_HEIGHT;
	d3dpp.BackBufferFormat	= D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount	= 1;
	d3dpp.SwapEffect		= D3DSWAPEFFECT_DISCARD;
	d3dpp.Windowed			= TRUE;

	d3dpp.Flags							= D3DPRESENTFLAG_VIDEO;
	d3dpp.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval			= D3DPRESENT_INTERVAL_ONE;

	hr = gpD3D->CreateDevice(	D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		NULL, 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, 
		&m_pD3DDev9 );

	if(gpD3D != NULL)
	{
		gpD3D->Release();
		gpD3D = NULL;
	}

	if(FAILED(hr))
	{
		AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::Create D3D::CreateDevice() fail.\n hr = %d", hr);
		return;
	}
	else
		AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::Create D3D::CreateDevice() Succeed.\n");

	if(FAILED(LoadDxva2Lib()))
		return;

	GetVideoAccelService();

	SetRenderTargetInfo(64, 64, NULL, (GUID*)(&DXVA2_Intel_Auxiliary_Device), 1);				

	GetDecoderDeviceGUIDs(&uiDecoders, &pGuidDecoders);

	for (i = 0; i < uiDecoders; i++)
	{
		if (IsEqualGUID(pGuidDecoders[i], DXVA2_Intel_Auxiliary_Device)) break;
	}

	CoTaskMemFree(pGuidDecoders);

	if (i >= uiDecoders)
	{
		DP("Failed to find Intel Auxiliary Device Interface error\n");
		return;
	}

	AllocRenderTargets(1, (D3DFORMAT)MAKEFOURCC('I', 'M', 'C', '3'));

	m_DeviceGuid = DXVA2_Intel_Auxiliary_Device;
	m_sDxva2VidDesc.SampleWidth                         = 64;
	m_sDxva2VidDesc.SampleHeight                        = 64;
	m_sDxva2VidDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	m_sDxva2VidDesc.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
	m_sDxva2VidDesc.SampleFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_BT709;
	m_sDxva2VidDesc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
	m_sDxva2VidDesc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
	m_sDxva2VidDesc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
	m_sDxva2VidDesc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
	m_sDxva2VidDesc.Format                              = *(D3DFORMAT*)GetRenderTargetFormat();
	m_sDxva2VidDesc.InputSampleFreq.Numerator           = m_sDxva2VidDesc.OutputFrameFreq.Numerator   = 60000;
	m_sDxva2VidDesc.InputSampleFreq.Denominator         = m_sDxva2VidDesc.OutputFrameFreq.Denominator = 1001;
	m_sDxva2VidDesc.UABProtectionLevel                  = 0;
	m_sDxva2VidDesc.Reserved                            = 0;

	GetDecoderConfigurations(&m_sDxva2VidDesc, &uiConfig, &m_pPictureDecode);

	CreateVideoDecoder(&m_sDxva2VidDesc, &m_pPictureDecode[0], &m_pAuxiliaryDevice);
}

CIntelAuxiliaryDevice::~CIntelAuxiliaryDevice()
{
	AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::Destroy\n");

	CoTaskMemFree(m_pPictureDecode);
}

HRESULT CIntelAuxiliaryDevice::QueryAccelGuids(GUID **ppAccelGuids, UINT *puAccelGuidCount)
{
	AUXDEVICE_DPMESSAGE("CIntelAuxiliaryDevice::QueryAccelGuids()\n");

    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT hRes       = E_INVALIDARG;
    UINT    uGuidCount = 0;
    GUID   *pGuids     = NULL;

    // Check parameters
    if (!puAccelGuidCount || !ppAccelGuids)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelGuids: Invalid parameters\n");
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelGuids: Auxiliary Device is invalid\n");
        goto cleanup;
    }

    // Get GUID count
    sExtension.Function              = AUXDEV_GET_ACCEL_GUID_COUNT;
    sExtension.pPrivateInputData     = NULL;
    sExtension.PrivateInputDataSize  = 0;
    sExtension.pPrivateOutputData    = &uGuidCount;
    sExtension.PrivateOutputDataSize = sizeof(int);

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	AUXDEVICE_DPMESSAGE("AuxiliaryDevice::Execute() :AUXDEV_GET_ACCEL_GUID_COUNT, ret-0x%x, GuidCount: %d\n", hRes, uGuidCount);

    if (FAILED(hRes))
    {
        AUXDEVICE_DPMESSAGE("QueryAccelGuids: Failed to obtain GUID Count error 0x%x\n", hRes);
        goto cleanup;
    }

    // Allocate array of GUIDs
    pGuids = (GUID *) malloc(uGuidCount * sizeof(GUID));
    if (!pGuids)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelGuids: Failed to allocate array of GUIDs\n");
        hRes = E_FAIL;
        goto cleanup;
    }

    // Get Acceleration Service Guids
    sExtension.Function              = AUXDEV_GET_ACCEL_GUIDS;
    sExtension.pPrivateInputData     = NULL;
    sExtension.PrivateInputDataSize  = 0;
    sExtension.pPrivateOutputData    = pGuids;
    sExtension.PrivateOutputDataSize = uGuidCount * sizeof(GUID);

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	AUXDEVICE_DPMESSAGE("AuxiliaryDevice::Execute() :AUXDEV_GET_ACCEL_GUIDS, ret-0x%x\n", hRes);

    if (FAILED(hRes))
    {
        AUXDEVICE_DPMESSAGE("QueryAccelGuids: Failed to obtain GUIDs error 0x%x\n", hRes);
        goto cleanup;
    }

cleanup:
    if (FAILED(hRes))
    {
        uGuidCount = 0;
        if (pGuids)
        {
            free(pGuids);
            pGuids = NULL;
        }
    }

    *puAccelGuidCount = uGuidCount;
    *ppAccelGuids     = pGuids;

    return hRes;
}

HRESULT CIntelAuxiliaryDevice::QueryAccelRTFormats(CONST GUID *pAccelGuid, D3DFORMAT **ppFormats, UINT *puCount)
{
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT    hRes     = E_INVALIDARG;
    UINT       uCount   = 0;
    D3DFORMAT *pFormats = NULL;


    // Check parameters
    if (!pAccelGuid || !ppFormats || !puCount)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelRTFormats: Invalid parameters\n");
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelRTFormats: Auxiliary Device is invalid\n");
        goto cleanup;
    }

    // Get GUID count
    sExtension.Function              = AUXDEV_GET_ACCEL_RT_FORMAT_COUNT;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = &uCount;
    sExtension.PrivateOutputDataSize = sizeof(int);

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        AUXDEVICE_DPMESSAGE("QueryAccelRTFormats: Failed to obtain RT format count error 0x%x\n", hRes);
        goto cleanup;
    }

    // Allocate array of formats
    pFormats = (D3DFORMAT *) malloc(uCount * sizeof(D3DFORMAT));
    if (!pFormats)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelRTFormats: Failed to allocate array of RT formats\n");
        hRes = E_FAIL;
        goto cleanup;
    }

    // Get Guids
    sExtension.Function              = AUXDEV_GET_ACCEL_RT_FORMATS;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pFormats;
    sExtension.PrivateOutputDataSize = uCount * sizeof(D3DFORMAT);

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);
    if (FAILED(hRes))
    {
        AUXDEVICE_DPMESSAGE("QueryAccelRTFormats: Failed to obtain array of RT formats error 0x%x\n", hRes);
        goto cleanup;
    }

cleanup:
    if (FAILED(hRes))
    {
        uCount = 0;
        if (pFormats)
        {
            free(pFormats);
            pFormats = NULL;
        }
    }

    *puCount   = uCount;
    *ppFormats = pFormats;

    return hRes;
}

HRESULT CIntelAuxiliaryDevice::QueryAccelCaps(CONST GUID *pAccelGuid, void *pCaps, UINT *puCapSize)
{
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    HRESULT hRes = E_INVALIDARG;

    // Check parameters (pCaps may be NULL => puCapSize will receive the size of the device caps)
    if (!pAccelGuid || !puCapSize)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelCaps: Invalid parameters\n");
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        AUXDEVICE_DPMESSAGE("QueryAccelCaps: Auxiliary Device is invalid\n");
        goto cleanup;
    }

    // Query Caps
    sExtension.Function              = AUXDEV_QUERY_ACCEL_CAPS;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pCaps;
    sExtension.PrivateOutputDataSize = *puCapSize;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	AUXDEVICE_DPMESSAGE("AuxiliaryDevice::Execute() :AUXDEV_QUERY_ACCEL_CAPS, ret-0x%x\n", hRes);

    if (FAILED(hRes))
    {
        AUXDEVICE_DPMESSAGE("QueryAccelCaps: Failed to obtain caps error 0x%x\n", hRes);
        goto cleanup;
    }

cleanup:
    *puCapSize = sExtension.PrivateOutputDataSize;
    return hRes;
}


HRESULT CIntelAuxiliaryDevice::CreateAccelService(CONST GUID *pAccelGuid, void *pCreateParams, UINT *puCreateParamSize)
{
    HRESULT hRes = E_INVALIDARG;
    DXVA2_DecodeExecuteParams sExecute;
    DXVA2_DecodeExtensionData sExtension;

    // Check parameters
    if (!pAccelGuid || !pCreateParams || !puCreateParamSize)
    {
        AUXDEVICE_DPMESSAGE("CreateAccelService: Invalid parameters\n");
        goto cleanup;
    }

    // Check the decoder object
    if (!m_pAuxiliaryDevice)
    {
        AUXDEVICE_DPMESSAGE("CreateAccelService: Auxiliary Device is invalid\n");
        goto cleanup;
    }

    // Query Caps
    sExtension.Function              = AUXDEV_CREATE_ACCEL_SERVICE;
    sExtension.pPrivateInputData     = (PVOID)pAccelGuid;
    sExtension.PrivateInputDataSize  = sizeof(GUID);
    sExtension.pPrivateOutputData    = pCreateParams;
    sExtension.PrivateOutputDataSize = *puCreateParamSize;

    sExecute.NumCompBuffers     = 0;
    sExecute.pCompressedBuffers = 0;
    sExecute.pExtensionData     = &sExtension;

    hRes = m_pAuxiliaryDevice->Execute(&sExecute);

	AUXDEVICE_DPMESSAGE("AuxiliaryDevice::Execute() :AUXDEV_CREATE_ACCEL_SERVICE, ret-0x%x\n", hRes);

    if (FAILED(hRes))
    {
		AUXDEVICE_DPMESSAGE("CreateAccelService: Failed to create the acceleration service error 0x%x\n", hRes);
        goto cleanup;
    }

cleanup:
    *puCreateParamSize = sExtension.PrivateOutputDataSize;
    return hRes;
}

HRESULT CIntelAuxiliaryDevice::ExecuteFunction(UINT uiFuncID, void *pDataIn, UINT uiSizeIn, void *pOutputData, UINT uiSizeOut)
{
	HRESULT hRes = E_FAIL;

	DXVA2_DecodeExecuteParams	execParams = {0};
	DXVA2_DecodeExtensionData	execExtData = {0};

	do
	{
		PrepareAuxFuncStruct( &execParams, &execExtData, uiFuncID, pDataIn, 
			uiSizeIn, pOutputData, uiSizeOut );
		hRes = m_pAuxiliaryDevice->Execute(&execParams);
		if( FAILED(hRes) )
		{
			AUXDEVICE_DPMESSAGE("CAuxiliaryDevice::ExecuteFunction: Auxiliary Execute failed", hRes);
			break;
		}

	} while( FALSE );

	return hRes;
}

void CIntelAuxiliaryDevice::PrepareAuxFuncStruct(DXVA2_DecodeExecuteParams * const pExecParams, DXVA2_DecodeExtensionData *pExtData, 
												 UINT funcId, void const * const pGuid, INT sizeIn, void const * const pDataOut, INT sizeOut)
{	
	if( NULL == pExecParams || NULL == pExtData )
		return;

	pExecParams->pExtensionData = pExtData;
	pExecParams->pExtensionData->Function				= funcId;
	pExecParams->pExtensionData->pPrivateInputData		= (void *)pGuid;
	pExecParams->pExtensionData->PrivateInputDataSize	= sizeIn;
	pExecParams->pExtensionData->pPrivateOutputData		= (void *)pDataOut;
	pExecParams->pExtensionData->PrivateOutputDataSize	= sizeOut;
}
