#ifndef _H264_DXVA_2_H_
#define _H264_DXVA_2_H_

#include "h264dxvabase.h"

#include <process.h>
#include <queue>
using namespace std;

#define MAX_COMP_BUF_FOR_DXVA2 12

typedef enum
{
	E_RES_DATA_FORMAT_MS,
	E_RES_DATA_FORMAT_NV,
	E_RES_DATA_FORMAT_INTEL
} RESIDUAL_DATA_FORMAT;

typedef struct _RESD_INFO
{
	BYTE *m_lpRESD_Intra_Luma;
	BYTE *m_lpRESD_Intra_Chroma;
	BYTE *m_lpRESD_Inter_Luma;
	BYTE *m_lpRESD_Inter_Chroma;
	int m_lmbCount_Inter;
	int m_lmbCount_Intra;
	int m_iInterRESBufUsage_L;
	int m_iInterRESBufUsage_C;
} RESD_INFO;

typedef struct _ExecuteBuffersStruct
{
	int currentSlice_structure;
	int currentSlice_picture_type;
	int UnCompress_Idx;
	int     m_iIntraMCBufUsage;
	int     m_iIntraRESBufUsage;
	int     m_iInterMCBufUsage;
	int smart_dec;
	int slice_number;
	int nal_reference_idc;
	int m_is_MTMS;
	int FrameSizeInMbs;
	int PicSizeInMbs;

	int m_iVGAType;
	int m_pic_combine_status;
	int m_Intra_lCompBufIndex;
	int m_lFrame_Counter;
	int m_iMbaffFrameFlag;

	BYTE *m_pnv1PictureDecode;
	BYTE *m_lpQMatrix;
	BYTE *m_lpMBLK_Intra_Luma;
	BYTE *m_lpSLICE;
	BYTE *m_lpMV;

	BYTE *m_lpRESD_Intra_Luma;
	BYTE *m_lpRESD_Intra_Chroma;
	BYTE *m_lpRESD_Inter_Luma;
	BYTE *m_lpRESD_Inter_Chroma;

	int m_lmbCount_Inter;
	int m_lmbCount_Intra;
	int m_iInterRESBufUsage_L;
	int m_iInterRESBufUsage_C;

	BYTE	*m_lpPicDec;
	BYTE    *m_lpQMatrixCtl;
	BYTE	*m_lpSliCtl;
	BYTE	*m_lpMbCtl;
	BYTE	*m_lpMv;
	BYTE *m_lpResdual;

	//only for ATI
	int width;
	int	m_PicHeight;
	int m_SBBufOffset;
	int m_ReBufStride;
	int m_MVBufStride;

	int m_unique_id;
	ICPService *m_pIviCP;
	bool m_bSkipedBFrame;
	byte m_bResidualDataFormat;
	bool m_bDMATransfer;
	int	 m_iDXVAMode_temp;

	RESD_INFO	m_sResinfo[20];

	int poc;

}  ExecuteBuffersStruct, *pExecuteBuffersStruct;

typedef struct _DXVA_PicParams_H264_MVC
{
	USHORT num_views_minus1;
	USHORT view_id;
	UCHAR inter_view_flag;
	UCHAR num_inter_view_refs_l0;
	UCHAR num_inter_view_refs_l1;
	UCHAR MVCReserved8Bits;
	DXVA_PicEntry_H264 InterViewRefsL0[16];
	DXVA_PicEntry_H264 InterViewRefsL1[16];
} DXVA_PicParams_H264_MVC;

//////////////////////////////////////////////////////////////////////////
// H264DXVA2 class
class CH264DXVA2 : public CH264DXVABase
{
public:
	CH264DXVA2();
	virtual ~CH264DXVA2();

	DWORD GetCurrentTotalByteCount_SECOP() { return m_dwTotalByteCount;}; //for realtime bitrate expression

	//for MVC field coding
	int m_iMVCFieldNeedExecuteIndex;

protected:
	virtual int Open PARGS1(int nSurfaceFrame);
	virtual int Close PARGS0();

	// General
	virtual void BeginDecodeFrame PARGS0();
	virtual void EndDecodeFrame PARGS0();
	//virtual void ReleaseDecodeFrame PARGS1(int frame_index) {};
	virtual void DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb) {};

	virtual void DMA_Transfer PARGS0();
	virtual void StoreImgRowToImgPic PARGS2(int start_x, int end_x);
	virtual void TransferData_at_SliceEnd PARGS0();

	void SetIHVDService(IHVDService *pIHVDService, H264VDecHP_OpenOptionsEx *pOptions);

	// MC and IT mode
	virtual int decode_one_macroblock_Intra PARGS0();
	virtual int decode_one_macroblock_Inter PARGS0();
	virtual int build_picture_decode_buffer PARGS0();
	virtual int build_slice_parameter_buffer PARGS0();
	virtual void build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset);
	virtual void build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset);
	virtual void build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset);
	virtual void build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset);
	virtual int ExecuteBuffers PARGS1(DWORD dwFlags);

	// VLD mode
	virtual CREL_RETURN decode_one_picture_short PARGS2(int* header, BOOL bSkip);
	virtual CREL_RETURN decode_one_picture_long PARGS2(int* header, BOOL bSkip);
	virtual CREL_RETURN BA_build_picture_decode_buffer PARGS0();
	virtual CREL_RETURN BA_ExecuteBuffers PARGS0();

	void ResetDXVABuffers();

	void GetCompressBufferSizeInfo(int nBufferType);
	void GetAllCompressBufferSizeInfo();

	void BA_ResetLastCompBufStaus();

	HRESULT GetBuffer(DWORD dwBufferType, LPVOID *ppBuffer, DWORD *pBufferSize);
	HRESULT ReleaseBuffer(DWORD dwBufferType);
	HRESULT BeginFrame(DWORD dwDestSurfaceIndex, DWORD dwFlags);
	HRESULT Execute(DWORD dwFunction, LPVOID lpPrivateInputData, DWORD cbPrivateInputData, LPVOID lpPrivateOutputData, DWORD cbPrivateOutputData, DWORD dwNumBuffers, AMVABUFFERINFO *pamvaBufferInfo);
	HRESULT EndFrame();

private:
	int GetCompBufferIndex();

	USHORT  getPatternCode PARGS1(int luma_transform_size_8x8_flag);
	UCHAR   getMBresidDataQuantity PARGS3(unsigned long cbp, int blk_8x8, int luma_transform_size_8x8_flag);
	int     getIntraPredAvailFlags PARGS1(int nIntraMode);

	void build_residual_buffer_Inter_Luma PARGS0();
	void build_residual_buffer_Chroma PARGS0();
	void build_weight_data_block_INTEL PARGS5(int pred_dir, char *pData, int ref_idx_0, int ref_idx_1, int list_offset);

	//for NV_SECOP
	CREL_RETURN BA_build_picture_decode_buffer_SECOP PARGS0();

protected:
	StreamParameters*	stream_global;

	BYTE m_bResidualDataFormat;

	//CompressBuffer
	int m_nCompBufSize[23]; //index: Buffer Type
	BYTE m_bCompBufUsed[23]; //index: Buffer Type
	BYTE *m_pbPictureParamBuf[MAX_COMP_BUF_FOR_DXVA2];
	BYTE *m_pbIQMatrixBuf[MAX_COMP_BUF_FOR_DXVA2];
	BYTE *m_pbMacroblockCtrlBuf[MAX_COMP_BUF_FOR_DXVA2];
	BYTE *m_pbResidualDiffBuf[MAX_COMP_BUF_FOR_DXVA2];
	BYTE *m_pbSliceCtrlBuf[MAX_COMP_BUF_FOR_DXVA2];
	BYTE *m_pbMotionVectorBuf[MAX_COMP_BUF_FOR_DXVA2];

	int m_nMVBufStride;
	int m_nMBBufStride;
	int m_nReBufStride;

	RESD_INFO m_ResInfo[20];

	//Use Queue for DXVA2 MC and IT mode
	queue <ExecuteBuffersStruct> *m_Queue;
	CRITICAL_SECTION crit_ACCESS_QUEUE;
	DWORD	m_dwThreadExecuteHandle;
	BOOL m_bRunThread;
	HANDLE m_hEndCpyThread;
	static unsigned int __stdcall ThreadExecute(void * arg);

	//for Intel
	BYTE *m_lpBitstreamBuf_Intel;
	int m_nSliceNum; //for Intel long format VLD

	IHVDServiceDxva2 *m_pIHVDServiceDxva2;

	//for NV SECOP
	unsigned char m_cNALUType;
	unsigned char m_cMetaDataSize;
	DWORD m_dwNALUByteCount;
	DWORD m_dwSliceHandle;
	DWORD m_dwOffset;
	DWORD m_dwTotalByteCount; //for realtime bitrate expression

};

//////////////////////////////////////////////////////////////////////////
// CH264DXVA2_AMD
class CH264DXVA2_ATI : public CH264DXVA2
{
public:
	CH264DXVA2_ATI();
	virtual ~CH264DXVA2_ATI();

private:
	//virtual function implementation
	int decode_one_macroblock_Intra PARGS0();
	int decode_one_macroblock_Inter PARGS0();
	int build_picture_decode_buffer PARGS0();
	int build_slice_parameter_buffer PARGS0();
	void build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset);
	int ExecuteBuffers PARGS1(DWORD dwFlags);

	CREL_RETURN decode_one_picture_short PARGS2(int* header, BOOL bSkip);
	CREL_RETURN BA_build_picture_decode_buffer PARGS0();
	CREL_RETURN BA_ExecuteBuffers PARGS0();


	void build_residual_buffer_Inter PARGS0();

	int I_Pred_luma_16x16 PARGS1(int predmode);
	int I_Pred_luma_4x4 PARGS4(int ioff, int joff, int img_block_x, int img_block_y);
	int I_Pred_luma_8x8 PARGS1(int b8);
	int I_Pred_chroma PARGS1(int uv);

private:
};

#endif //_H264_DXVA_2_H_