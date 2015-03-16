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

#include <windows.h>
#include "videoframe.h"
#include "h264dxvabase.h"

#ifdef _WIN32_WCE
#define CACHELINE_SIZE 64
#else
#define CACHELINE_SIZE 128
#endif //_WIN32_WCE


CH264VideoFrame::CH264VideoFrame()
{
	ZeroMemory(static_cast<StorablePicture *>(this),sizeof(StorablePicture));
	refcount = 1;	/* implementation */
	next = 0;
	ZeroMemory(dep,sizeof(dep));
	mgr = 0;
}

CH264VideoFrame::~CH264VideoFrame()
{
	free_storable_picture(static_cast<StorablePicture *>(this),false);
}

int CH264VideoFrame::AddRef()
{
	return InterlockedIncrement(&refcount);
}

int CH264VideoFrame::Release()
{
	int i, relval;

	relval = InterlockedDecrement(&refcount);
	if(relval<=0)
	{
		is_output = 0;
		for(i=0;i<3;i++)
		{
			if(dep[i])
			{
				dep[i]->Release();
				dep[i] = 0;
			}
		}
		if(mgr)
			mgr->PutFreeFrame(this);
		else
			delete this;
		return 0;
	}
	return relval;
}

CH264VideoFrameMgr::CH264VideoFrameMgr(int nframes)
{
	int i;

	InitializeCriticalSection(&m_cs);
	m_hFreeEvent = CreateEvent(0,TRUE,TRUE,0);
	m_nframes = 0;
	m_frames = 0;
	m_displayqueue = 0;
	m_displayqueuecount = 0;
	m_nFlushDPBsize = 0;
	m_freelist = 0;
	m_freelistcount = 0;
	m_frames = new CH264VideoFrame[nframes];
	if(m_frames==0)
		return;
	m_nframes = nframes;
	for(i=0; i<nframes; ++i)
	{
		m_frames[i].mgr = this;
		m_frames[i].Release();	/* put on free frame queue */
	}
}

CH264VideoFrameMgr::~CH264VideoFrameMgr()
{
	if(m_frames)
	{
		if(m_freelistcount!=m_nframes)
		{
			//			ALERT(("CH264VideoFrameMgr: delete called with %d outstanding frames\n",m_nframes-m_freelistcount));
		}
		delete[] m_frames;
		m_frames = m_displayqueue = m_freelist = 0;
		m_nframes = 0;
		m_displayqueuecount = 0;
		m_nFlushDPBsize = 0;
		m_freelistcount = 0;
	}
	CloseHandle(m_hFreeEvent);
	DeleteCriticalSection(&m_cs);
}

int CH264VideoFrameMgr::FlushDisplayQueue(LPVOID pH264DxvaBase)
{
	int i;
	CH264VideoFrame *vf;
#if defined(_HW_ACCEL_)
	CH264DXVABase *pH264DXVA = (CH264DXVABase*)pH264DxvaBase;
#endif

	//Terry: we should release the dep buffer of each frame in the display queue.
	if(m_displayqueue)
	{
		for(vf=m_displayqueue; vf; vf=vf->next)
		{
#if defined(_HW_ACCEL_)
			if (vf->m_DXVAVer && vf->pic_store_idx != -1)
				pH264DXVA->m_pUncompBufQueue->PutItem(vf->pic_store_idx);
#endif
			for(i=0;i<3;i++)
			{
				if(vf->dep[i])
				{
					(vf->dep[i])->Release();
					vf->dep[i] = 0;
				}
			}
		}
	}

	EnterCriticalSection(&m_cs);
	if(m_displayqueue)
	{
		for(vf=m_displayqueue; vf->next; vf=vf->next);	// get to the end
		vf->next = m_freelist;
		m_freelist = m_displayqueue;
		m_freelistcount += m_displayqueuecount;
		m_displayqueue = 0;
		m_displayqueuecount = 0;
		m_nFlushDPBsize = 0;
		if(m_freelistcount==m_displayqueuecount)
			SetEvent(m_hFreeEvent);
	}
	LeaveCriticalSection(&m_cs);
	return 0;
}

CH264VideoFrame *CH264VideoFrameMgr::GetFreeFrame(int *pstruct, int *pmem)
{
	CH264VideoFrame *prev, *freeframe = 0;

	EnterCriticalSection(&m_cs);
	if(pstruct || pmem)
	{
//* For Error detection debug
/*
		FILE* pFileDebug;
		CH264VideoFrame *outputframe;
		CH264VideoFrame *firstframe;
		BOOL  bErrFound = FALSE;


		
		firstframe	= m_freelist;
		

		for (int k = 0; k<(m_freelistcount -1); k++) {

			outputframe = firstframe->next;

			for (int j = 0; j < (m_freelistcount - 1 - k); j++ ) {
				if (outputframe == firstframe)  {
					bErrFound = TRUE;
					break;
				}
				outputframe = outputframe->next;

			}

			firstframe = firstframe->next;	
			

		}


		//if (debug_number == 187) {
		if (bErrFound) {
			pFileDebug = fopen("C:\\test-sequences\\H264\\ForErrorHandling\\freesp.txt", "w");
			outputframe = m_freelist;

			fprintf(pFileDebug, "stream_global->number = %d \n", debug_number);

			for (int k = 0; k<m_freelistcount; k++) {
				fprintf(pFileDebug, "freelist[%d] = %x,  freelist[%d] = %x;  \n", k, (long)outputframe, k+1, (long)(outputframe->next));
				outputframe = outputframe->next;
			}

			fclose(pFileDebug);
		}
*/
		for(prev=0,freeframe=m_freelist; freeframe; prev=freeframe,freeframe=freeframe->next)
		{
			if((pstruct==0 || freeframe->structure==*pstruct) &&
				(pmem==0 || freeframe->mem_layout==*pmem)
#ifdef _COLLECT_PIC_
				&& (freeframe->used_for_first_field_reference == 0)
#endif
				)	// prefer a particular structure
				break;
		}
		if(freeframe==0)
		{
			for(prev=0,freeframe=m_freelist; freeframe; prev=freeframe,freeframe=freeframe->next)
			{
				if(freeframe->imgY==0)	// prefer an unallocated frame
					break;
			}
		}
	}
	if(freeframe==0)
	{
		prev = 0;
		freeframe = m_freelist;
	}
	if(freeframe)
	{
		m_freelistcount--;
		if(prev)
			prev->next = freeframe->next;
		else
			m_freelist = freeframe->next;
	}
	if(m_freelistcount==0)
		ResetEvent(m_hFreeEvent);
	LeaveCriticalSection(&m_cs);
	return freeframe;
}

CH264VideoFrame *CH264VideoFrameMgr::GetDisplayFrame(int view_index)
{
	CH264VideoFrame *dispframe, *pre_frame = NULL;

	EnterCriticalSection(&m_cs);
	dispframe = m_displayqueue;

	while (dispframe)
	{
		StorablePicture *p = static_cast<StorablePicture *>(dispframe);
		if(p->view_index == view_index)
			break;
		pre_frame = dispframe;
		dispframe = dispframe->next;
	}
	if(dispframe)
	{
		m_displayqueuecount--;
		if(pre_frame == NULL)
			m_displayqueue = dispframe->next;
		else
			pre_frame->next = dispframe->next;

		dispframe->next = NULL;
	}
	LeaveCriticalSection(&m_cs);
	return dispframe;
}

CH264VideoFrame *CH264VideoFrameMgr::PeekDisplayFrame()
{
	CH264VideoFrame *dispframe;

	EnterCriticalSection(&m_cs);
	dispframe = m_displayqueue;
	if(dispframe)
		dispframe->AddRef();
	LeaveCriticalSection(&m_cs);
	return dispframe;
}

int CH264VideoFrameMgr::PutDisplayFrame(CH264VideoFrame *f)
{
	CH264VideoFrame *vf;
	CH264VideoFrame *outputframe;
	BOOL bErrFound;

	DP_QUEUE("[FrameMgr] PutDisplayFrame: %d", f->pic_store_idx);

	if(f==0)
		return -1;
	EnterCriticalSection(&m_cs);

	if (m_displayqueue) {
		outputframe	= m_displayqueue;

		for (int k = 0; k<m_displayqueuecount; k++) {

			if (outputframe == f)  {
				bErrFound = TRUE;
				LeaveCriticalSection(&m_cs);
				return 0;

			}
			outputframe = outputframe->next;

		}
	}

	if (m_freelist) {
		outputframe	= m_freelist;

		for (int k = 0; k<m_freelistcount; k++) {

			if (outputframe == f)  {
				bErrFound = TRUE;
				LeaveCriticalSection(&m_cs);
				return 0;

			}
			outputframe = outputframe->next;

		}
	}


	m_displayqueuecount++;
	f->AddRef();
	/* put at end of list */
	f->next = 0;
	if(m_displayqueue==0)
		m_displayqueue = f;
	else
	{
		for(vf=m_displayqueue; vf->next; vf=vf->next);
		vf->next = f;
	}
	LeaveCriticalSection(&m_cs);

	DP_QUEUE("[FrameMgr] PutDisplayFrame: %d END", f->pic_store_idx);
	return 0;
}

int CH264VideoFrameMgr::PutFreeFrame(CH264VideoFrame *f)
{
	CH264VideoFrame *outputframe;
	BOOL  bErrFound = FALSE;

	DP_QUEUE("[FrameMgr] PutFreeFrame: %d", f->pic_store_idx);

	if(f==0)
		return -1;
	EnterCriticalSection(&m_cs);

	if (m_displayqueue) {
		outputframe	= m_displayqueue;

		for (int k = 0; k<m_displayqueuecount; k++) {

			if (outputframe == f)  {
				bErrFound = TRUE;
				LeaveCriticalSection(&m_cs);
				return 0;

			}
			outputframe = outputframe->next;

		}
	}
	
	if (m_freelist) {
		outputframe	= m_freelist;

		for (int k = 0; k<m_freelistcount; k++) {

			if (outputframe == f)  {
				bErrFound = TRUE;
				LeaveCriticalSection(&m_cs);
				return 0;
				
			}
			outputframe = outputframe->next;

		}
	}

	m_freelistcount++;
	if(m_freelistcount==1)
		SetEvent(m_hFreeEvent);
	f->AddRef();
	/* put at start of list */


	

	f->next = m_freelist;
	m_freelist = f;
	LeaveCriticalSection(&m_cs);

	DP_QUEUE("[FrameMgr] PutFreeFrame: %d END", f->pic_store_idx);
	return 0;
}

int CH264VideoFrameMgr::WaitForDecodeFrame()
{
	EnterCriticalSection(&m_cs);
	while(m_freelistcount==0)
	{
		LeaveCriticalSection(&m_cs);
		WaitForSingleObject(m_hFreeEvent,INFINITE);
		EnterCriticalSection(&m_cs);
	}
	LeaveCriticalSection(&m_cs);
	return 0;
}

