#pragma once

#include <d3d9.h>
#include "../../Exports/Inc/HVDService.h"
#include "IH264VDec.h"
#include "IMP2VDec.h"
#include "IVC1VDec.h"
#include "imp4vdec.h"
#include "GMediaBuffer.h"
#include "GMOHelper.h"
#include "IVdoParameter.h"

#define STREAM_PACKET 40960
#define PACKET_SIZE 40960
#define BITSTREAM_BUFFER_SIZE 2000000 // 2000 k bytes
#if defined (_DEBUG)
#define DP printf
#else
#define DP
#endif

enum E_H264_VGACARD
{
	E_H264_VGACARD_ATI     = 1,
	E_H264_VGACARD_NVIDIA  = 1 << 1,
	E_H264_VGACARD_INTEL   = 1 << 2,
	E_H264_VGACARD_S3      = 1 << 3,
	E_H264_VGACARD_GENERAL = 1 << 4
};

enum E_H264_DXVA_MODE
{
	E_H264_DXVA_ATI_PROPRIETARY_A    = 1,		// ATI DXVA PROPRIETARY HWMC level
	E_H264_DXVA_ATI_PROPRIETARY_E    = 1 << 1,	// ATI DXVA PROPRIETARY BA level
	E_H264_DXVA_NVIDIA_PROPRIETARY_A = 1 << 2,	// NVIDIA DXVA PROPRIETARY HWMC level
	E_H264_DXVA_MODE_A			= 1 << 3,	// MS DXVA HWMC level
	E_H264_DXVA_MODE_B			= 1 << 4,	// MS DXVA HWMC-FGT level
	E_H264_DXVA_MODE_C			= 1 << 5,	// MS DXVA IDCT level
	E_H264_DXVA_MODE_D			= 1 << 6,	// MS DXVA IDCT-FGT level
	E_H264_DXVA_MODE_E			= 1 << 7,	// MS DXVA BA level
	E_H264_DXVA_MODE_F			= 1 << 8,	// MS DXVA BA-FGT level
	E_H264_DXVA_INTEL_MODE_E	= 1 << 9,	// Intel DXVA BA level
};

enum E_DEC_BUFFER_STATE
{
	E_DEC_BUFFER_STATE_EOF = 0,
	E_DEC_BUFFER_STATE_STOCK = 1,
};

enum E_VC1_VGACARD
{
	E_VC1_VGACARD_ATI		= 1,
	E_VC1_VGACARD_NVIDIA	= 1 << 1,
	E_VC1_VGACARD_INTEL		= 1 << 2,
	E_VC1_VGACARD_NotSupport
};

enum E_VC1_DXVA_PROFILE_ID
{
	E_VC1_DXVA_PROFILE_N	= 0x00,
	E_VC1_DXVA_PROFILE_A	= 0xA0,
	E_VC1_DXVA_PROFILE_B	= 0xA1,
	E_VC1_DXVA_PROFILE_C	= 0xA2,
	E_VC1_DXVA_PROFILE_D	= 0xA3,
	E_INTEL_VC1_DXVA_PROFILE_D	= 0x6D,
};

enum E_MP4_VGACARD
{
    E_MP4_VGACARD_ATI     = 1,
    E_MP4_VGACARD_NVIDIA  = 1 << 1,
    E_MP4_VGACARD_GENERAL = 1 << 4
};

enum MPEG4_ASP_VARIANT
{
    MPEG4_ASP_GENERIC = 0,
    MPEG4_ASP_DIVX_311 = 1,
    MPEG4_ASP_DIVX_4 = 2,
    MPEG4_ASP_DIVX_5 = 3,
    MPEG4_ASP_DIVX_6 = 4,
    MPEG4_ASP_XVID = 5,
    MPEG4_ASP_NERO_DIGITAL = 6
};

class CDXVATestProgram
{
public:
	CDXVATestProgram(void);
public:
	virtual ~CDXVATestProgram(void);

	void SetDecodeLogFP(FILE* fpDecodeLog){ m_fpDecodeLog=fpDecodeLog;};	
	static HRESULT GetParamFromHVDTest(DWORD dwPropID, PVOID pvContext, LPVOID *ppOutBuffer, DWORD *pdwOutBufferLen, LPVOID pInBuffer, DWORD dwInBufferLen);

protected:
	CGMOPtr<IGMediaObject> m_pDecGMO;
	CGMOPtr<IVdoParameter> m_pIParam;
	IHVDService *m_pService;

	TCHAR m_tcConfigFile[256];
	TCHAR m_tcKeyName[256];
	FILE* m_fpDecodeLog;
public:
	virtual void SetHVDService(IHVDService* pService) {m_pService = pService;}
	virtual void SetConfigFile(TCHAR *ptcConfigFile) { _tcscpy_s( m_tcConfigFile, 256, ptcConfigFile); }
	virtual UINT GetClipNum();
	virtual UINT GetClipWidth(TCHAR *ptcClipFile);
	virtual UINT GetClipHeight(TCHAR *ptcClipFile);
//	virtual void GetClipInputDir(TCHAR *ptcClipFile, TCHAR *InputDir);
//	virtual void GetClipOutputDir(TCHAR *ptcClipFile, TCHAR *OutputDir);
	static HRESULT DXVA_Get_Resolution(void *pvContext, int width, int height, int aspect_ratio_information, LPVOID *pEncrypt){return S_OK;}
	virtual void NV24WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp);	
	virtual void NV12WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp);
	virtual void IMC3WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp);
	virtual void YV12WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp);
};

class CH264DXVATestProgram : public CDXVATestProgram 
{
public:
	CH264DXVATestProgram();
	virtual ~CH264DXVATestProgram();
private:
	H264VDecParam::H264VDecHP_OpenOptions OpenOptions;
	H264VDecParam::H264VDecHP_DecodeOptions DecodeOptions;
	H264VDecParam::H264VDecHP_GetFrameOptions GetFrameOptions;
	H264VDecParam::H264VDecHP_Frame H264Frame;

	FILE* fp;
	DWORD dwFrameCounter, dwNumberOfFrames;
	DWORD dwStartTime, dwStopTime, dwElapsedTime;

public:

	HRESULT H264HP_GetDataGMO(
		IN PVOID pvContext,
		OUT const BYTE **ppbOutBuffer,
		OUT DWORD *pcbNumberOfBytesRead,
		OUT BOOL *pbHasPTS,
		OUT H264VDecParam::H264_TS *pTimeStamp
		);
	void H264Decoder(int uiClipCounter);	
	int H264DecodeGMO(int DXVASelect);
};

class CMP2DXVATestProgram : public CDXVATestProgram
{
public:
	CMP2DXVATestProgram();
	virtual ~CMP2DXVATestProgram();

private:
	CGMOPtr<IMP2VDecDllAPI> m_pIDecoderAPI;

	FILE *read_fp, *write_fp;
	DWORD dwFrameCounter, dwNumberOfFrames;

public:

	HRESULT GetDataFromStream(
		IN PVOID pvContext,
		OUT const BYTE **ppbOutBuffer,
		OUT DWORD *pcbNumberOfBytesRead,
		OUT BOOL *pbHasPTS,
		OUT MP2V_TS *pTimeStamp
		);

	void MP2Decoder(int uiClipCounter);	
	int MP2DecodeGMO(int DXVASelect);
};

class CVC1DXVATestProgram : public CDXVATestProgram
{
public:
	CVC1DXVATestProgram();
	virtual ~CVC1DXVATestProgram();

private:
	FILE *read_fp, *write_fp;
	DWORD dwFrameCounter, dwNumberOfFrames;

public:
	void ReadStream(BYTE** pbBuffer, DWORD* pdwBufferSize);

	void VC1Decoder(int uiClipCounter);	
	int VC1DecodeGMO(int DXVASelect);
};

class CMP4DXVATestProgram : public CDXVATestProgram 
{
public:
    CMP4DXVATestProgram();
    virtual ~CMP4DXVATestProgram();
private:
    //CGMOPtr<IMP2VDecDllAPI> m_pIDecoderAPI;

    FILE *read_fp, *write_fp;
    DWORD dwFrameCounter, dwNumberOfFrames;
    DWORD frame_size;

public:
    void GetDataGMO(FILE *pfInputFile, const BYTE **ppbOutBuffer, DWORD *pdwNumberOfBytesRead);

    void MP4Decoder(int uiClipCounter);	
    int MP4DecodeGMO(int DXVASelect);
    void NV12WriteFrame(BYTE* pbYBufferStart, BYTE* pbUVBufferStart, DWORD dwWidth, DWORD dwHeight, DWORD dwStride, FILE *fp);
};