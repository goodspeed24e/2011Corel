#ifndef _H264_DXVA_BASE_H_
#define _H264_DXVA_BASE_H_

#include "global.h"
#include "dxva_error.h"
#include "nalu.h"
#include "parset.h"
#include "header.h"
#include "sei.h"
#include "annexb.h"
#include "atih264.h"
#include "HVDService.h"


#define MAX_COMP_BUF 100
#define MAX_PICDEC_Buffer 12
#define MAP_CP_PIC_TYPE(type) (type == I_SLICE ?  CP_I_TYPE : (type == P_SLICE ? CP_P_TYPE : CP_B_TYPE))

void iDCT_4x4_ATI(short* dest, short *src, short dest_stride);
void iDCT_8x8_ATI(short* dest, short *src, short dest_stride);
void iDCT_4x4_UV_ATI(short* dest, short *src_u, int residual_u, int residual_v, short stride);

void iDCT_4x4_nonATI(short* dest, short *src, short dest_stride);
void iDCT_8x8_nonATI(short* dest, short *src, short dest_stride);
void iDCT_4x4_UV_nonATI(short* dest, short *src_u, int residual_u, int residual_v, short stride);

enum E_DXVA_EXECUTION_FLAG
{
	E_USING_NO_FLAGS = 0x00,
	E_OUTPUT_INTRA_MBS = 0x01,
	E_USING_DEBLOCK = 0x02
};

enum E_BA_RAW_FLAG
{
	E_BA_RAW_0 = 0x00,
	E_BA_RAW_LONGFORMAT = 0x01,
	E_BA_RAW_SHORTFORMAT = 0x02
};

typedef struct __SURFACE_INFO__
{
	BYTE SInfo[2];
	void Set()
	{
		SInfo[0] = 0xFF;
		SInfo[1] = 0;
	}
	__SURFACE_INFO__()
	{
		Set();
	}
}
SURFACE_INFO;

typedef struct _REF_INFO {
	int frame_num;
	int top_poc;
	int bot_poc;
} MSBA_REF_INFO;

const byte SNGL_SCAN_2D[16] =
{
	0,  1,  4,  8,
	5,  2,  3,  6,
	9, 12, 13, 10,
	7, 11, 14, 15
};

const byte SNGL_SCAN8x8_2D[64] = {  
	0*8+0, 0*8+1, 1*8+0, 2*8+0, 1*8+1, 0*8+2, 0*8+3, 1*8+2, 2*8+1, 3*8+0, 4*8+0, 3*8+1, 2*8+2, 1*8+3, 0*8+4, 0*8+5,
	1*8+4, 2*8+3, 3*8+2, 4*8+1, 5*8+0, 6*8+0, 5*8+1, 4*8+2, 3*8+3, 2*8+4, 1*8+5, 0*8+6, 0*8+7, 1*8+6, 2*8+5, 3*8+4,
	4*8+3, 5*8+2, 6*8+1, 7*8+0, 7*8+1, 6*8+2, 5*8+3, 4*8+4, 3*8+5, 2*8+6, 1*8+7, 2*8+7, 3*8+6, 4*8+5, 5*8+4, 6*8+3,
	7*8+2, 7*8+3, 6*8+4, 5*8+5, 4*8+6, 3*8+7, 4*8+7, 5*8+6, 6*8+5, 7*8+4, 7*8+5, 6*8+6, 5*8+7, 6*8+7, 7*8+6, 7*8+7
};

typedef
void (*PFN_DXVA_IDCT)(
											short* dest,
											short* src,
											short dest_stride
											);

typedef
void (*PFN_DXVA_IDCT_UV)(
												 short* dest,
												 short* src_u,
												 int residual_u,
												 int residual_v,
												 short stride
												 );

class CUncompressBufferQueue
{
public:
	CUncompressBufferQueue(unsigned long max)
	{
		mutex = CreateMutex(NULL,FALSE,NULL);
		semaphore_get = CreateSemaphore(NULL,0,max,NULL);
		semaphore_put = CreateSemaphore(NULL,max,max,NULL);
		queue = new int [max];
		this->max = max;
		count = 0;
		idx_get = idx_put = 0;
		init_status = 1;
		QueryPerformanceFrequency(&m_freq);
	}

	~CUncompressBufferQueue()
	{
		delete [] queue;
		CloseHandle(mutex);
		CloseHandle(semaphore_get);
		CloseHandle(semaphore_put);
	}

	int GetItem();
	int PeekItem(unsigned long offset);
	void PutItem(int item);
	inline unsigned long GetCount() { return count;};
	inline unsigned long GetMax() {return max;};

protected:
	HANDLE mutex;
	HANDLE semaphore_get, semaphore_put;
	int *queue;
	unsigned long max;
	unsigned long idx_get, idx_put;
	unsigned long init_status;

public:
	unsigned long count;
	LARGE_INTEGER m_freq;
};

//////////////////////////////////////////////////////////////////////////
// H264DXVABase class
class CH264DXVABase
{
public:
	CH264DXVABase();
	virtual ~CH264DXVABase();

	virtual int Open PARGS1(int nSurfaceFrame) = 0;
	virtual int Close PARGS0() = 0;

	// General
	virtual void BeginDecodeFrame PARGS0() = 0;
	virtual void EndDecodeFrame PARGS0() = 0;
	virtual void ReleaseDecodeFrame PARGS1(int nFrameIndex);
	virtual void DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb) {};

	virtual void DMA_Transfer PARGS0() = 0;
	virtual void StoreImgRowToImgPic PARGS2(int start_x, int end_x) = 0;
	virtual void TransferData_at_SliceEnd PARGS0() = 0;

	virtual void SetIHVDService(IHVDService *pIHVDService, H264VDecHP_OpenOptionsEx *pOptions);

	// MC and IT mode
	virtual int decode_one_macroblock_Intra PARGS0() = 0;
	virtual int decode_one_macroblock_Inter PARGS0() = 0;
	virtual int build_picture_decode_buffer PARGS0() = 0;
	virtual int build_slice_parameter_buffer PARGS0() = 0;
	virtual void build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset) = 0;
	virtual void build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset) = 0;
	virtual void build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset) = 0;
	virtual void build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset) = 0;
	virtual int ExecuteBuffers PARGS1(DWORD dwFlags) = 0;

	// VLD mode
	virtual CREL_RETURN decode_one_picture_short PARGS2(int* header, BOOL bSkip) = 0;
	virtual CREL_RETURN decode_one_picture_long PARGS2(int* header, BOOL bSkip) = 0;
	virtual CREL_RETURN BA_build_picture_decode_buffer PARGS0() = 0;
	virtual CREL_RETURN BA_ExecuteBuffers PARGS0() {return 0;};

	virtual void ResetDXVABuffers();

	virtual void BA_ResetLastCompBufStaus()=0;

	int GetBitstreamRawConfig() {return m_bConfigBitstreamRaw;}
	
	void InitialUnCompressBufferQueue();

	int  GetVGAType() {return m_nVGAType;};
	int  GetDXVAMode() {return m_nDXVAMode;};
	int  GetHWBufCount() {return m_pUncompBufQueue->GetCount();};

	void SetSurfaceInfo PARGS3(BYTE id, BYTE flag, BYTE overwrite);

public:
	BOOL m_bResolutionChange;

	CUncompressBufferQueue *m_pUncompBufQueue;

	BYTE *m_lpnalu;
	int  m_nNALUSkipByte;

protected:
	CRITICAL_SECTION m_cs;

	IHVDService *m_pIHVDService;
	IHVDServiceDxva *m_pIHVDServiceDxva;
	int m_nVGAType;
	int m_nDXVAMode;

	int m_nSurfaceFrame;
	int m_nFrameCounter;

	BYTE m_bConfigBitstreamRaw;
	GUID m_guidConfigBitstreamEncryption;

	DXVA_PicEntry_H264 m_RefList[20];
	MSBA_REF_INFO      m_RefInfo[20];

	//UnCompressBuffer
	SURFACE_INFO *m_pSurfaceInfo;

	//CompressBuffer
	BYTE m_bCompBufStaus[MAX_COMP_BUF]; //index: Buffer Index
	int m_nLastCompBufIdx;	//to record the last index of compressed buffer (dxva1/dxva2)

	BYTE *m_lpPictureParamBuf;
	BYTE *m_lpMacroblockCtrlBuf;
	BYTE *m_lpResidualDiffBuf;
	BYTE *m_lpDeblockingCtrlBuf;
	BYTE *m_lpInverseQuantMatrixBuf;
	BYTE *m_lpSliceCtrlBuf;
	BYTE *m_lpBitstreamBuf;
	BYTE *m_lpMotionVectorBuf;


	ICPService *m_pIviCP;


	PFN_DXVA_IDCT       iDCT_4x4_fcn;
	PFN_DXVA_IDCT       iDCT_8x8_fcn;
	PFN_DXVA_IDCT_UV    iDCT_4x4_UV_fcn;


	
};

CREL_RETURN HW_DecodeOnePicture PARGS2(int *header, BOOL bSkip);

void* HW_Open PARGS4(int nframe, int iVGAType, int iDXVAMode, ICPService *m_pIviCP);
CREL_RETURN HW_Close PARGS0();
CREL_RETURN HW_BeginDecodeFrame PARGS0();
CREL_RETURN HW_EndDecodeFrame PARGS0();
CREL_RETURN HW_ReleaseDecodeFrame PARGS1(int frame_index);
void HW_DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb);

CREL_RETURN HW_TransferData_at_SliceEnd PARGS0();
CREL_RETURN HW_StoreImgRowToImgPic PARGS2(int start_x, int end_x);

CREL_RETURN HW_decode_one_macroblock_Intra PARGS0();
CREL_RETURN HW_decode_one_macroblock_Inter PARGS0();

//CREL_RETURN HW_decode_one_picture PARGS1(int* header);

void DMA_Transfer PARGS0();
void EXECUTE PARGS0();

#endif //_H264_DXVA_BASE_H_