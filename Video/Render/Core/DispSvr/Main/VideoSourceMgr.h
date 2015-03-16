#ifndef _VIDEOSOURCE_MGR_H
#define _VIDEOSOURCE_MGR_H

#include "DispSvr.h"
#include <vector>

class CVideoSourceManager : 
	public CUnknown,
	public IDisplayVideoSourceManager
{
public:

	CVideoSourceManager(IDisplayLock* pLock, IDisplayVideoSink* pVideoSink);
	virtual ~CVideoSourceManager();

	// IUnknown implementation
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

	void Cleanup();

	STDMETHOD(AddVideoSource)(IBaseFilter* pVMRFilter, IDisplayVideoSource** ppVidSrc);
	STDMETHOD(RemoveVideoSource)(IDisplayVideoSource* pVidSrc);
	STDMETHOD(GetVideoSourceCount)(LONG* plCount);
	STDMETHOD(GetVideoSourceByIndex)(LONG lIndex, IDisplayVideoSource** ppVideoSource);

	STDMETHOD(BeginDeviceLoss)();
	STDMETHOD(EndDeviceLoss)(IUnknown* pDevice);

	void SetLock(IDisplayLock* pLock)
	{
		m_pLock = pLock;
	}

private:

	typedef std::vector<IDisplayVideoSource*> VideoSources;
	VideoSources m_videoSources;

	IDisplayVideoSink *m_pVideoSink;
	IDisplayLock *m_pLock;
};


#endif	// _VIDEOSOURCE_MGR_H
