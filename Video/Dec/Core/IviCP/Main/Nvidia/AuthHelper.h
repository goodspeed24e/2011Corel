/*
 *  file        :   AuthenticationHelper
 *  description :   declaration of hte authentication helper
 *  history     :   
 *
 *
 *
 */

#pragma once

#define XRML_TAG_MODULUS                ("Modulus")
#define XRML_TAG_EXPONENT               ("Exponent")
#define XRML_TAG_RSAKEYVALUE            ("RSAKeyValue")

#define RSA_MODULUS_BYTE_LENGTH  512
#define AES_BLOCKLEN             16
//#include "CryptoHelper.h"
class CryptographicLibrary;

/*
 *  class       :   COPPCertHelper
 *  description :   certificate parsing helper function
 *  inherits    :   
 *  notes       :   
 */
class COPPAuthHelper
{
public:

    //  construction
    COPPAuthHelper();
    ~COPPAuthHelper();

    HRESULT     Initialize(const BYTE*  pbGHCert, 
                            const DWORD cbGHCert);

    //  functionality
    HRESULT     PrepareSignature(BYTE*      pbSignature, 
                                const DWORD cbSignature, 
                                const GUID* pgGHRandom, 
                                const DWORD dwStatusSeqIdx, 
                                const DWORD dwCmdSeqIdx,
                                BYTE*		pbSessionKey);

    HRESULT     Sign(const BYTE*    pbData, 
                    const DWORD     cbData, 
                    BYTE*           pbSignature, 
                    const DWORD     cbSignature);

    HRESULT     VerifySignature(const BYTE* pbData, 
                                const DWORD cbData, 
                                const BYTE* pbSignature, 
                                const DWORD cbSignature);

    //  tools
    HRESULT GenerateRandomNumber(BYTE *pbNumber, const DWORD cbSize);
//    {   
//        //ASSERT(NULL != m_pCryptoHelper);
//        return m_pCryptoHelper->GenerateRandom(pbNumber, cbSize);
//    }
    HRESULT SwitchEndianess(BYTE *pbOutput, BYTE *pbIntput, const DWORD cbSize)
	{
		if(pbOutput==pbIntput) return E_FAIL;
		for(unsigned int i=0; i<cbSize; i++)
			pbOutput[i] = pbIntput[cbSize-1-i];
		return S_OK;
	}

private:
//    CryptoHelper*       m_pCryptoHelper;
    COPPCertHelper      m_CertHelper;
    CryptographicLibrary *m_pCrypto;

	BYTE*  m_pbPublicKey;
	DWORD  m_cbKeyLen;
	DWORD  m_dwExponent;
	BYTE   m_abSessionKey[16];
};  //  class COPPAuthHelper

void    PrintByteArray(const BYTE*  pbArray, 
                       const DWORD  cbArray);
