
#ifndef _EPID_KEY_EXCHANGE_
#define _EPID_KEY_EXCHANGE_

#include <windows.h>
#include <dxva.h>
#include "intel_pavp_api.h"
#include "../Imports/utilcrit.h"

#define EPID_SAFE_RELEASE(p,v) if((p)!=NULL) {(p)->Release(v); (p)=NULL;}

typedef HRESULT (*PFN_EPID_EXEC)(
	PVOID pvContext,
	LPVOID pInBuf,
	DWORD dwInBufLen,
	LPVOID pOutBuf,
	DWORD dwOutBufLen
	);

typedef enum
{
	PROXY_THREAD_ERROR = -1,
	PROXY_THREAD_OPEN = 0,
	PROXY_THREAD_CREATE_SESSION = 1,
	PROXY_THREAD_INVALIDATE_KEY = 2,
	PROXY_THREAD_UPDATE_KEY = 3,
	PROXY_THREAD_CLOSE = 4
} PROXY_THREAD_STATUS;

class CEPIDKeyExchange 
{

public:
	
	CEPIDKeyExchange(BOOL bUseProxyThread=TRUE);
	~CEPIDKeyExchange();

	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)(PVOID pvDataContext=NULL);

	static CEPIDKeyExchange* GetInstance(BOOL bUseProxyThread=TRUE);
	HRESULT Open(PVOID pvDataContext, PFN_EPID_EXEC pfnEPIDExecCallback, IUnknown *pExtObj=NULL, LPVOID pSharedData=NULL, BOOL bShowMsg=FALSE);
	HRESULT CreateSession();	
	HRESULT UpdateStreamKey(StreamKey *pStreamKey, PVOID pvDataContext=NULL, BOOL bAudio=FALSE);
	HRESULT GetCurrStreamKey(StreamKey *pStreamKey, BOOL bAudio=FALSE);
	HRESULT InvalidateStreamKey(PVOID pvDataContext=NULL, BOOL bAudio=FALSE);
			
	LPVOID  GetSharedData(){return m_pSharedData;}
	IUnknown* GetExtObj();
	VOID    SetAudioStreamID(DWORD dwAudioStreamID){m_AudioStreamId = dwAudioStreamID;}	
	BOOL	IsSessionCreated(){return m_bSessionIsCreated;}	
	BOOL	IsVideoStreamKeyChanged(){return m_bChangeVideoStreamKey;}

	static CUTILcrit m_csEPID;

private:

	VOID DP(CHAR* szMsg, ...);	
	HRESULT _ResetParams();
	HRESULT _CreateSession();
	HRESULT _Close();
	HRESULT _InvalidateStreamKey();
	HRESULT _UpdateStreamKey();	
	HRESULT _CloseSession();
	
	LONG m_lRefCount;	

	//for proxy thread
	BOOL m_bUseProxyThread;
	HANDLE m_hProxyThread;
	HANDLE m_hProcessEvent;
	HANDLE m_hWaitEvent;
	PROXY_THREAD_STATUS m_threadstatus;
	static unsigned int __stdcall ProxyThread(void *arg);		

	//for creating session
	SigmaSessionKey	m_SessionKey;		// "Session" key used to wrap stream keys
	StreamKey       m_StreamKey;		// "Stream" key that would encrypt actual video
	SigmaMacKey		m_SigningKey;		// HMAC signing key
	PAVPSessId		m_SigmaSessionId;	

	PAVPStreamId m_AudioStreamId;
	BOOL m_bAudio;		
	BOOL m_bShowMsg;

	PFN_EPID_EXEC m_pfnEPIDExecCallback;	
	PVOID m_pvDataContext;
	static CEPIDKeyExchange* m_pEPID;	
	
	HANDLE m_hSessionIsCreated;		
	HANDLE m_hGotVideoStreamKey;
	HANDLE m_hGotAudioStreamKey;
	
	BOOL m_bSessionIsCreated;
	BOOL m_bGetVideoStreamKey;
	BOOL m_bGetAudioStreamKey;	
	BOOL m_bChangeVideoStreamKey;

	StreamKey m_VideoStreamKey;	
	StreamKey m_AudioStreamKey;	

	IUnknown *m_pExtObj;	
	LPVOID m_pSharedData;

};

#endif