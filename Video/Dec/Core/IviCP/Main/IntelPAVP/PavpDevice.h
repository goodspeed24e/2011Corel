#ifndef PAVP_DEVICE_INCLUDE
#define PAVP_DEVICE_INCLUDE

#include "IntelAuxiliaryDevice.h"
#include "intel_pavp_api.h"
#include "../Imports/utilcrit.h"
#include "EPID/EPIDKeyExchange.h"
#include "../CPDefines.h"

typedef enum
{	
	PAVP_PROXY_THREAD_INIT = 0,
	PAVP_PROXY_THREAD_OPEN_KEY_EXCHANGE = 1,
	PAVP_PROXY_THREAD_CLEANUP_EPID = 2,
	PAVP_PROXY_THREAD_UPDATE_STREAM_KEY = 3,
	PAVP_PROXY_THREAD_INVALIDATE_STREAM_KEY = 4,
	PAVP_PROXY_THREAD_CLEAN_AUXILIARY_DEVICE = 5
} PAVP_PROXY_THREAD_STATUS;

class CryptographicLibrary;

class CPavpDevice : public CIntelAuxiliaryDevice
{

public:

	static CPavpDevice* GetInstance(IDirect3DDevice9 *pD3DDevice9);
	static CPavpDevice* GetInstance(IMFGetService *pMFGetService);
	static CUTILcrit m_PavpDeviceCS;
		
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

	HRESULT Open(BOOL bAudio=FALSE, BOOL bShowMsg=FALSE);   
	HRESULT Close(BOOL bAudio=FALSE);
	
	HRESULT UpdateStreamKey(BOOL bAudio=FALSE);
	
	HRESULT ScrambleData(PVOID pAccel, const byte *input, byte *output, int bufLen);
	HRESULT ScrambleDataAudio(const byte *input, byte *output, int bufLen);

	BOOL IsPresent() {return m_bIsPresent;}
	BOOL IsPAVP15();
	BOOL IsPAVP10();
	BOOL IsSessionEstablished(){return m_bSessionEstablished;}
	
	void SetAudioStreamId(PAVPStreamId StreamId);

	static HRESULT EPIDExec(PVOID pvContext, LPVOID pInBuf, DWORD dwInBufLen, LPVOID pOutBuf, DWORD dwOutBufLen);
	PAVP_CREATE_DEVICE GetPavpCreateDevice() {return m_sPavpCreateDevice;}

	//not use
	HRESULT GetFreshnessValue();
	HRESULT UseFreshnessValue();
	HRESULT GetConnectionState();

private:
	
	CPavpDevice(IMFGetService *pMFGetService);
	CPavpDevice(IDirect3DDevice9 *pD3DDevice9);
	~CPavpDevice();

	HRESULT _QueryCaps();
	HRESULT _CreateService();
	HRESULT _DoKeyExchange();	
	HRESULT _DoFixedKeyExchange();
	HRESULT _DoEPIDKeyExchange();
	HRESULT _UpdateStreamKey(BOOL bAudio);
	HRESULT _InvalidateStreamKey(BOOL bAudio);

	HRESULT _SetAESCounter(PVOID pAccel, DWORD dwBufferSize, DWORD dwAesCounter);
	void _SetPavpDevice(PAVP_CREATE_DEVICE* psPavpCreateDevice) { m_sPavpCreateDevice = *psPavpCreateDevice; }	

	HRESULT _OpenKeyExchange();
	void _CleanAuxiliaryDevice();


	static CPavpDevice* m_PavpDevice;

	LONG    m_lRefCount;	
	BOOL	m_bIsPresent;
	BOOL	m_bKeysExchanged;
	DWORD	m_dwSessionKey[4];
	PAVP_CREATE_DEVICE m_sPavpCreateDevice;
	CryptographicLibrary*	m_pCryptImg;
	DWORD	m_dwAesCounter;

	PAVPStreamId m_AudioStreamId;
	BOOL m_bSessionEstablished;	

	BOOL m_bIsCapQueried;
	CEPIDKeyExchange *m_pEPID;
	BOOL m_bAudio;

	StreamKey  m_VideoStreamKey;
	StreamKey  m_AudioStreamKey;	

	
	HANDLE m_hProxyThread;
	HANDLE m_hProcessEvent;
	HANDLE m_hWaitEvent;
	PAVP_PROXY_THREAD_STATUS m_threadstatus;
	static unsigned int __stdcall ProxyThread(void *arg);
	unsigned int m_uiThreadId;	
	HRESULT m_hhr;
};


#endif // PAVP_DEVICE_INCLUDE
