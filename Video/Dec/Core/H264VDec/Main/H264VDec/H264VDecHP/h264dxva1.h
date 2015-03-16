#ifndef _H264_DXVA_1_H_
#define _H264_DXVA_1_H_

#include "h264dxvabase.h"
#include "nvh264_mcp.h"

#include <process.h>
#include <queue>
using namespace std;


//////////////////////////////////////////////////////////////////////////
// For ATI
typedef struct _ExecuteBuffersStruct_DXVA1
{
	int m_Intra_lCompBufIndex;
	int UnCompress_Idx;
	int m_MVBufStride;
	int m_PicWidth;
	int	m_PicHeight;
	int m_ReBufStride;
	int m_SBBufOffset;
	BYTE *m_lpMVBuf;
	BYTE *m_lpMV;
	BYTE *m_lpRESDBuf;
	BYTE *m_lpRESD_Intra_Luma;
	DXVA_BufferDescription		m_pDxvaBufferDescription[6];
	AMVABUFFERINFO				m_pBufferInfo[6];

	DXVA_PictureParameters_H264		m_PictureDecode;
	ICPService *m_pIviCP;

	BYTE bPicStructure;
	int smart_dec;
	short type;
	int slice_struct;
	int slice_number;
	int combine_status;
	int picture_type;
	int Hybrid_Decoding;
	int m_is_MTMS;

	int framepoc;  //For DXVA Data Dump Usage
}  ExecuteBuffersStruct_DXVA1, *pExecuteBuffersStruct_DXVA1;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// H264DXVA1 class
class CH264DXVA1 : public CH264DXVABase
{
public:
	CH264DXVA1();
	virtual ~CH264DXVA1();

protected:
	virtual int Open PARGS1(int nSurfaceFrame);
	virtual int Close PARGS0();

	// General
	virtual void BeginDecodeFrame PARGS0()=0;
	virtual void EndDecodeFrame PARGS0()=0;
	virtual void ReleaseDecodeFrame PARGS1(int frame_index)=0;
	virtual void DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb)=0;

	virtual void DMA_Transfer PARGS0()=0;
	virtual void StoreImgRowToImgPic PARGS2(int start_x, int end_x)=0;
	virtual void TransferData_at_SliceEnd PARGS0()=0;

	void SetIHVDService(IHVDService *pIHVDService, H264VDecHP_OpenOptionsEx *pOptions);

	// MC and IT mode
	virtual int decode_one_macroblock_Intra PARGS0()=0;
	virtual int decode_one_macroblock_Inter PARGS0()=0;
	virtual int build_picture_decode_buffer PARGS0()=0;
	virtual int build_slice_parameter_buffer PARGS0()=0;
	virtual void build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset)=0;
	virtual void build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset)=0;
	virtual void build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset)=0;
	virtual void build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset)=0;
	virtual int ExecuteBuffers PARGS1(DWORD dwFlags)=0;

	// VLD mode
	virtual CREL_RETURN decode_one_picture_short PARGS2(int* header, BOOL bSkip);
	virtual CREL_RETURN decode_one_picture_long PARGS2(int* header, BOOL bSkip);
	virtual CREL_RETURN BA_build_picture_decode_buffer PARGS0();
	virtual CREL_RETURN BA_ExecuteBuffers PARGS0();

	virtual void ResetDXVABuffers();

	virtual HRESULT GetDXVABuffers();
	virtual void ReleaseDXVABuffers();

	int BA_GetCompBufferIndex();
	void BA_ResetLastCompBufStaus();

	HRESULT GetBuffer(DWORD dwBufferType, DWORD dwBufferIndex, BOOL bReadOnly, LPVOID *ppBuffer, LONG *pBufferStride);
	HRESULT ReleaseBuffer(DWORD dwBufferType, DWORD dwBufferIndex);
	HRESULT BeginFrame(DWORD dwDestSurfaceIndex, DWORD dwFlags);
	HRESULT Execute(DWORD dwFunction, LPVOID lpPrivateInputData, DWORD cbPrivateInputData, LPVOID lpPrivateOutputData, DWORD cbPrivateOutputData, DWORD dwNumBuffers, AMVABUFFERINFO *pamvaBufferInfo);
	HRESULT EndFrame(DWORD dwDestSurfaceIndex);

public:
	StreamParameters*	stream_global;

protected:
	CRITICAL_SECTION crit_PICDEC;
	CRITICAL_SECTION crit_COMP;
	CRITICAL_SECTION crit_EXECUTE;
	CRITICAL_SECTION crit_GetCompBuf;

	//CompressBuffer
	AMVACompBufferInfo m_pAMVACompBufInfo[23];
	BYTE *m_pbPictureParamBuf[MAX_COMP_BUF];
	BYTE *m_pbMacroblockCtrlBuf[MAX_COMP_BUF];
	BYTE *m_pbResidualDiffBuf[MAX_COMP_BUF];
	BYTE *m_pbDeblockingCtrlBuf[MAX_COMP_BUF];
	BYTE *m_pbInverseQuantMatrixBuf[MAX_COMP_BUF];
	BYTE *m_pbSliceCtrlBuf[MAX_COMP_BUF];
	BYTE *m_pbBitstreamBuf[MAX_COMP_BUF];
	BYTE *m_pbMotionVectorBuf[MAX_COMP_BUF];

	DWORD m_dwNumCompBuffers;
	DWORD m_dwNumTypesCompBuffers;

	BOOL m_bNotGetCompBuf;
	BOOL m_bDeblockingFlag;

	BYTE*		m_lpBADATA;
	BYTE*		m_lpBADATACTRL;

	IHVDServiceDxva1 *m_pIHVDServiceDxva1;

};

//////////////////////////////////////////////////////////////////////////
// H264DXVA1_NV class
class CH264DXVA1_NV : public CH264DXVA1
{
public:
	CH264DXVA1_NV();
	virtual ~CH264DXVA1_NV();

private:
	//virtual function implementation
	int Open PARGS1(int nSurfaceFrame);
	int Close PARGS0();

	void BeginDecodeFrame PARGS0();
	void EndDecodeFrame PARGS0();
	void ReleaseDecodeFrame PARGS1(int frame_index);
	void DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb);

	void DMA_Transfer PARGS0();
	void StoreImgRowToImgPic PARGS2(int start_x, int end_x){};
	void TransferData_at_SliceEnd PARGS0(){};

	int decode_one_macroblock_Intra PARGS0();
	int decode_one_macroblock_Inter PARGS0();
	int build_picture_decode_buffer PARGS0();
	int build_slice_parameter_buffer PARGS0();
	void build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset);
	int ExecuteBuffers PARGS1(DWORD dwFlags);

	//CREL_RETURN decode_one_picture_short PARGS2(int* header, BOOL bSkip);
	//CREL_RETURN decode_one_picture_long PARGS2(int* header, BOOL bSkip);
	//CREL_RETURN BA_build_picture_decode_buffer PARGS0();
	//CREL_RETURN BA_ExecuteBuffers PARGS0();

	void ResetDXVABuffers();

	HRESULT GetDXVABuffers();
	void ReleaseDXVABuffers();

	int GetCompBufferIndex();

	void build_residual_buffer_Inter_Luma PARGS0();
	void build_residual_buffer_Inter_Chroma PARGS0();

	int I_Pred_luma_16x16 PARGS1(int predmode);
	int I_Pred_luma_4x4 PARGS4(int ioff, int joff, int img_block_x, int img_block_y);
	int I_Pred_luma_8x8 PARGS1(int b8);
	int I_Pred_chroma PARGS1(int uv);

	void (CH264DXVA1_NV::*build_macroblock_buffer_Inter[2]) PARGS5(BYTE pred_dir,
		BYTE index,
		BYTE list_offset,
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition,
	struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C);

	void build_macroblock_buffer_Inter_Int PARGS5(BYTE pred_dir,
		BYTE index,
		BYTE list_offset,
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition,
	struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C);

	void build_macroblock_buffer_Inter_Ori PARGS5(BYTE pred_dir,
		BYTE index,
		BYTE list_offset,
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition,
	struct nvh264vp1MacroblockControlInterChroma* MB_Partition_C);
	void build_macroblock_buffer_Inter_16x16_Luma PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_16x8_Luma PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x16_Luma PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x8_Luma PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_Luma PARGS4(BYTE pred_dir,
		BYTE index,
		BYTE list_offset,
	struct nvh264vp1MacroblockControlInterLuma* MB_Partition);

	//Deblocking
	void DeblockMacroblock PARGS2(StorablePicture *p, int mb_nr);
	void build_deblocking_control_buffer PARGS7(StorablePicture *p,
		int MbQAddr,
		int mbx,
		int mby,
		Macroblock_s *Mb_left,
		Macroblock_s *Mb_top,
		Macroblock_s *MbQ);
	void build_deblocking_control_buffer_mbaff_sse2 PARGS7(StorablePicture *p,
		int MbQAddr,
		int mbx,
		int mby,
		Macroblock_s *Mb_left,
		Macroblock_s *Mb_top,
		Macroblock_s *MbQ);
	void build_deblocking_control_buffer_mbaff_c PARGS7(StorablePicture *p,
		int MbQAddr,
		int mbx,
		int mby,
		Macroblock_s *Mb_left,
		Macroblock_s *Mb_top,
		Macroblock_s *MbQ);

private:
	int m_nFrameIndex;
	int	m_nFirstDecBuf;

	DWORD m_dwNumPicDecBuffers;
	BOOL  m_bPicCompBufStaus[MAX_PICDEC_Buffer];
	int   m_nLastPicBufIdx;

	int   m_nQueryCount;

	BOOL m_bFirst_Slice;
};

//////////////////////////////////////////////////////////////////////////
// H264DXVA1_ATI class
class CH264DXVA1_ATI : public CH264DXVA1
{
public:
	CH264DXVA1_ATI();
	virtual ~CH264DXVA1_ATI();

private:
	//virtual function implementation
	int Open PARGS1(int nSurfaceFrame);
	int Close PARGS0();

	void BeginDecodeFrame PARGS0();
	void EndDecodeFrame PARGS0();
	void ReleaseDecodeFrame PARGS1(int frame_index);
	void DeblockSlice PARGS3(StorablePicture *p, int start_mb, int num_mb){};

	void DMA_Transfer PARGS0();
	void StoreImgRowToImgPic PARGS2(int start_x, int end_x);
	void TransferData_at_SliceEnd PARGS0();

	int decode_one_macroblock_Intra PARGS0();
	int decode_one_macroblock_Inter PARGS0();
	int build_picture_decode_buffer PARGS0();
	int build_slice_parameter_buffer PARGS0();
	void build_macroblock_buffer_Inter_16x16 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_16x8 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x16 PARGS1(int list_offset);
	void build_macroblock_buffer_Inter_8x8 PARGS1(int list_offset);
	int ExecuteBuffers PARGS1(DWORD dwFlags);

	//CREL_RETURN decode_one_picture_short PARGS2(int* header, BOOL bSkip);
	//CREL_RETURN decode_one_picture_long PARGS2(int* header, BOOL bSkip);
	//CREL_RETURN BA_build_picture_decode_buffer PARGS0();
	//CREL_RETURN BA_ExecuteBuffers PARGS0();

	void ResetDXVABuffers();

	HRESULT GetDXVABuffers();
	void ReleaseDXVABuffers();

	int GetCompBufferIndex();

	void build_residual_buffer_Inter PARGS0();
	void StoreImgRowToDXVACompressedBuffer PARGS2(int start_x, int end_x);

	int I_Pred_luma_16x16 PARGS1(int predmode);
	int I_Pred_luma_4x4 PARGS4(int ioff, int joff, int img_block_x, int img_block_y);
	int I_Pred_luma_8x8 PARGS1(int b8);
	int I_Pred_chroma PARGS1(int uv);

private:
	int m_PicWidth;
	int m_MbBufStride;
	int m_MVBufStride;
	int m_ReBufStride;

	BYTE* m_pMVtmp[MAX_COMP_BUF];
	BYTE* m_pREtmp[MAX_COMP_BUF];

	//Use Queue for DXVA1 MC mode
	queue <ExecuteBuffersStruct_DXVA1> *m_Queue_DXVA1;
	CRITICAL_SECTION crit_ACCESS_QUEUE;
	unsigned long	m_ThreadExecuteHandle_DXVA1;
	bool bRunThread;
	HANDLE hEndCpyThread;
	static unsigned int __stdcall ThreadExecute_DXVA1(void * arg);


};



#endif //_H264_DXVA_1_H_