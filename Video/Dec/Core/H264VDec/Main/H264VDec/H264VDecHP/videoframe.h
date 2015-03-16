//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2004 - 2005  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
//

#ifndef _VIDEOFRAME_H_
#define _VIDEOFRAME_H_

//#include "ts.h"
class CH264VideoFrameMgr;
#include "mbuffer.h"

class CH264VideoFrame : public StorablePicture
{
public:
	friend class CH264VideoFrameMgr;
	friend int write_out_picture_h264_interface PARGS1(StorablePicture *p);


	CH264VideoFrame();
	~CH264VideoFrame();
	int AddRef();
	int Release();

	// implementation variables

protected:
	LONG refcount;				// counts references by the decoder
	CH264VideoFrame *next;		// used by CH264VideoFrameMgr
	CH264VideoFrame *dep[3];	// additional dependencies to be freed on release.
	CH264VideoFrameMgr *mgr;
};

class CH264VideoFrameMgr
{
public:
	CH264VideoFrameMgr(int nframes);
	~CH264VideoFrameMgr();
	int GetFreeCount() const {	return m_freelistcount;	}
	int GetDisplayCount() const {	return m_displayqueuecount; }
	CH264VideoFrame *GetFreeFrame(int *pstruct, int *pmem);
	CH264VideoFrame *GetDisplayFrame(int view_index);
	CH264VideoFrame *PeekDisplayFrame();
	int PutDisplayFrame(CH264VideoFrame *f);
	int PutFreeFrame(CH264VideoFrame *f);
	int FlushDisplayQueue(LPVOID pH264DxvaBase);
	int WaitForDecodeFrame();
	int m_nFlushDPBsize;

protected:
	int m_nframes;				// number of frames allocated
	CH264VideoFrame *m_frames;       // base address of all video frames
	CH264VideoFrame *m_displayqueue; // next frame to be displayed
	CH264VideoFrame *m_freelist;		// free frames
	LONG m_displayqueuecount;
	LONG m_freelistcount;
	CRITICAL_SECTION m_cs;
	HANDLE m_hFreeEvent;
};

#endif /* _VIDEOFRAME_H_ */
