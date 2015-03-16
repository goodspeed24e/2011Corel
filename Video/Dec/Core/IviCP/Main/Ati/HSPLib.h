//////////////////////////////////////////////////////////////////////////////
// HSPLib.h
// ATI Host Secure Playback
//
// Copyright AMD 2007 - AMD Confidential
// Copyright ATI Technologies 2006 - ATI Confidential
//
// The host secure playback library (atihsp.lib) defines classes for clients 
// to authenticate the graphics device & content encryption.
// 
#ifndef _ATI_HSP_LIB_
#define _ATI_HSP_LIB_

// Win32 & DX9 data types
#include "Windows.h"
#include "strmif.h"
#include "Videoacc.h"

#include "DRM-common.h"

// Data type returned by CATIHSP_Authentication::GetCaps()
// Client is responsible to verify result of AES-OMAC over 
// first 24 bytes following dwSize parameter:
typedef struct 
{
    DWORD       dwSize;							// [IN]  Size of struct ATIDRM_DEVICE_CAPS
    DWORD       dwDeviceCaps;					// [OUT] Device specific caps
    DWORD       dwCipherCaps;					// [OUT] Decryption ciphers supported by device
	DWORD		dwMaxKeySlots;					// [OUT] Content key slots available for pre-loading
	DWORD		dwMaxSessions;					// [OUT] Authentication sessions for this stream

	BYTE		RequestID[16];                  // [IN]  Random number
    BYTE		MAC[16];                        // [OUT] AES-OMAC using Session Key
	BYTE		Reserved[64];
}
ATIDRM_DEVICE_CAPS;


// Operation flags used by ATIDRM_PROTBLT_PARAMS::bltOP
enum ATIDRM_PROTBLT_OP
{
	ATIDRM_PROTBLT_NONE			=0x0000,		// None
	ATIDRM_PROTBLT_RENDERTGT	=0x0001			// Blt to render target
};


// Parameters for CATIHSP_Authentication::ProtectedBlt()
typedef struct
{
	DWORD				dwSize;					// [IN] Size of struct ATIDRM_PROTBLT_PARAMS
	ATIDRM_PROTBLT_OP	bltOP;					// [IN] Blt operation flags
	RECT				srcRect;				// [IN] Source rectangle for blt
	BYTE				srcAddr[32];			// [IN] Source address Obfuscated using Ks
	DWORD				srcPitch;				// [IN] Source pitch
	GUID				srcFormat;				// [IN] Source DX subtype (NV12/YV12)
	DWORD				srcWidth;				// [IN] Source width
	DWORD				srcHeight;				// [IN] Source height
	DWORD				dstOffset;				// [IN] Byte offset in destination to blt
	BYTE				Reserved[64];
}
ATIDRM_PROTBLT_PARAMS;


// Foward Declarations
class CHSP_Auth;
class CHSP_Encrypt;

//
// Define class to perform authentication with graphics device
//
class CATIHSP_Authentication 
{
public:
    CATIHSP_Authentication(void);
    ~CATIHSP_Authentication(void);

    // Open()
    //
    // Purpose: Initialize interface
    //
    // Params:  pVA         [IN]  Video Acceleration I/F
    //          dwAppVenID  [IN]  ATI supplied Vendor ID for client application 
    //
    // Result:  S_OK on Success
    //
    // Notes:   Application may open a session after establishing a DXVA connection 
    //          between the decoder & video render, and invoke Close() to release pVA.
    //          Both the Authentication protocol & content key obfuscation will be based
    //          upon dwAppVenID.
    //
    //          More than once session may be opened by instantiatting multiple instance
    //          of this object and completeing the authentication for each instance.
    //          
    HRESULT Open (IUnknown* pVA, DWORD dwAppPVenID);


    // Close()
    //
    // Purpose: Close interface
    //
    // Params:  None
    //
    // Result:  S_OK on Success
    //
    // Notes:   None
    //
    HRESULT Close (void);


    // GetCaps()
    //
    // Purpose: Get Capabilities
    //
    // Params:  pDeviceCaps     [IN/OUT]  Supported capabilities 
    //
    // Result:  S_OK on Success
    //
    // Notes:   This methods may be invoked after completeing the authentication with 
    //          the graphics device and establishing a session key.  Clients should 
    //          verify the integrity of pDeviceCaps using OMAC-AES. 
    //          
    HRESULT GetCaps (ATIDRM_DEVICE_CAPS* pDeviceCaps);


    // SendCommand()
    //
    // Purpose: Send authentication commands to graphics device
    //
    // Params:  dwCommand       [IN]		Authentication Command 
    //          pCmdData        [IN/OUT]	Input & Output data buffer defined by command
    //          dwCmdSize       [IN]		Size of data buffer
    //
    // Result:  S_OK on Success
    //
    // Notes:   Command sequence is dependant upon authentication scheme specified by 
    //          dwAppVendID in Open(); client is responsible for allocation of pCmdData
    //
    HRESULT SendAuthCommand (DWORD dwCommand, LPVOID pCmdData, DWORD dwCmdSize);


    // AddDecryptionKey()
    //
    // Purpose: Send obfuscated content key to device 
    //
    // Params:  dwCipher		[IN]  Decryption ciper: See ATIDRM_CIPHERCAPS
	//			pKey            [IN]  Pointer to content key
    //          dwKeyBits       [IN]  Size of pKey in bits
	//			pKeyData		[IN]  Pointer to associated key data such as AES CTR, etc…
	//			dwDataSize		[IN]  Size of pKeyData in bytes
	//          dwKeyIndex		[OUT] Index assigned to this key
    //
    // Result:  S_OK				Success
	//			E_ACCESSDENIED		Current slot is in use from ::SetDecryptionKey()
	//			E_FAIL				Uknown failure
    //
    // Notes:   The function provides the graphics device with a content key used to 
    //          decrypt video data being sent  through DXVA; thus clients must invoke
    //          thus function at least once prior to sending encrypted content.  The obfuscation
    //          scheme for the key is dependant upon dwAppVendID specified in Open(), pData is 
	//          sent in the clear.  
    //
    HRESULT AddDecryptionKey (DWORD dwCipher, LPVOID pKey, DWORD dwKeyBits, LPVOID pKeyData, DWORD dwDataSize, DWORD* pKeyIndex);


    // SetNextDecryptionKey()
    //
    // Purpose: Inform graphics device to use next content key for decryption
    //
    // Params:  pdwKeyIndex		[OUT] Index of selected key 
    //
    // Result:  S_OK				Success
	//			E_PENDING			Specified key is not yet ready for use
	//			E_FAIL				Key does not exist / Uknown failure
    //
    // Notes:   The function informs the graphics device to cycle the content key which was 
	//			pre-loaded via AddDecryptionKey();  clients must invoke this method prior
	//			to sending encrypted content through DXVA.
	//           
    HRESULT SetNextDecryptionKey (DWORD* pdwKeyIndex);

    // SetNullDecryptionKey()
    //
    // Purpose: Inform graphics device to set a NULL key for decryption
    //
    // Params:  None 
    //
    // Result:  S_OK				Success
	//			E_FAIL				Failure
    //
    // Notes:   The function informs the graphics device to set a NULL decryption key.
	//			Client's should invoke this method before sending clear content which
	//          does not require decryption.
	//           
    HRESULT SetNullDecryptionKey (void);

	// ProtectedBlt()
	//
	// Purpose: Perform protected blt
	//
	// Params:  pParams		[IN] Parameters describing blt operation
	//
	// Result:  S_OK 			Success
	//          E_FAIL			Failure
	//
	// Notes:   The function compliments the software rendering path by performing a protected blt 
	//			from system to video memory.  Client application must authenticate with graphics
	//			device to establish Ks such that the data pointer (srcAddr) is obfuscated via the 
	//	 		same scheme used to wrap content keys.
	//           
	HRESULT ProtectedBlt (ATIDRM_PROTBLT_PARAMS* pParams);    

private:
	CHSP_Auth	*m_pObj;
};
typedef CATIHSP_Authentication*	LPCATIHSP_Authentication;


//
// Define helper class to perform content encryption
//
class CATIHSP_Encryption
{
public:
    CATIHSP_Encryption(void);
    ~CATIHSP_Encryption(void);


    // SetEncryptionFormat()
    //
    // Purpose: Set encryption format
    //
    // Params:  dwType          [IN]  Format of encrypted content based on GetCaps
    //          dwBits          [OUT] Required key size for specified ciper in bits
    //          bUseHSPEncrypt  [OUT] TRUE indicates client must invoke Encrypt() 
    //                                function for content encryption
    //          
    // Result:  S_OK on Success
    //
    // Notes:   Use this method to specify the cipher for content encryption taking place
    //          via Encrypt().  Client must ensure that device can support the same cipher
	//          for content decryption based on result of CATIHSP_Authentication::GetCaps()
    //
    HRESULT SetEncryptionFormat (DWORD dwCipher, DWORD* dwBits, BOOL* bUseHSPEncrypt);


	// SetEncryptionKey()
	//
	// Purpose: 	Set content encryption key 
	//
	// Params:  	pKey		[IN]  Pointer to encryption key
	//				dwKeyBits	[IN]  Size of encryption key in bits
	//				pData		[IN]  Pointer to associated key data such as AES CTR, etc…
	//				dwDataSize	[IN]  Size of data in bytes
	//
	// Result:  	S_OK on Success
	//
	// Notes:   	Prior to invoking Encrypt() clients must set the encryption key; the
    //      		object only maintain a pointer to the key (data is not copied) thus
    //      		client must ensure key & data buffer is valid before invoking Encrypt().
	//
	HRESULT SetEnryptionKey	(LPVOID pKey, DWORD dwKeyBits, LPVOID pData, DWORD dwDataSize);



    // Encrypt()
    //
    // Purpose: Encrypt Content
    //
    // Params:  pDataIn         [IN]  Input data buffer (clear)
    //          pDataOut        [IN]  Output data buffer (encrypted)
    //          dwDataSize      [IN]  Size of input & output data in bytes
    //          dwDataPitch     [IN]  Pitch of input & output (set to 0 for linear data)
    //
    // Result:  S_OK on Success
    //
    // Notes:   Provides HSPLib encryption capabilities to decoder; client must invoke to 
    //          encrypt video data when bUseHSPEncrypt from SetCipherFormat() returns TRUE.
    //          SetEncryptionKey() must also be invoked prior to calling this method.
    //
    //          pDataIn & pDataOut can point to the same buffer
    //
    HRESULT Encrypt (LPVOID pDataIn, LPVOID pDataOut, DWORD dwDataSize, DWORD dwDataPitch = 0);

private:
	CHSP_Encrypt	*m_pObj;
};
typedef CATIHSP_Encryption*	LPCATIHSP_Encryption;

#endif
