#pragma once

//-----------------------------------------------------------------------------
// CEvrSamplePool class
//
// Manages a list of allocated samples.
//-----------------------------------------------------------------------------

class CEvrSamplePool 
{
public:
    CEvrSamplePool(UINT nSampleSize);
    virtual ~CEvrSamplePool();

	HRESULT Initialize(IDirect3DDevice9 *pDevice, UINT uWidth, UINT uHeight, UINT uToken, DWORD dwFormat); // combine default initialize and allocateSamples function
    HRESULT Initialize(CEvrVideoSampleList& samples);
	HRESULT AllocateSamples(IDirect3DDevice9 *pDevice, UINT uWidth, UINT uHeight, CEvrVideoSampleList& videoSampleQueue);
	HRESULT Initialize(IDirect3DDevice9 *pDevice, UINT uWidth, UINT uHeight);
    HRESULT Clear();
   
    HRESULT GetSample(IMFSample **ppSample);    // Does not block.
    HRESULT ReturnSample(IMFSample *pSample);   
    BOOL    AreSamplesPending();
	LONG	GetSamplePendingCount();

private:
    CCritSec                     m_lock;

    CEvrVideoSampleList             m_VideoSampleQueue;			// Available queue

    BOOL                        m_bInitialized;
    volatile LONG				m_cPending;

	UINT							m_nSampleSize;
};