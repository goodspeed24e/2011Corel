/*
 *  file        :   AuthenticationHelper
*/

#include <streams.h>
#include "strsafe.h"
//#include "if_certifiedoutputprotection.h"

//#include "CryptoHelper.h"
#include "CertHelper.h"
#include "AuthHelper.h"
#include "Crypto.h"
/*
 *        class:    COPPAuthHelper
 *  description:    helper class for authentification mechanism
 */

COPPAuthHelper::COPPAuthHelper
(): //m_pCryptoHelper(NULL)
	m_pbPublicKey(0)
{
	m_pCrypto = new CryptographicLibrary();

}

COPPAuthHelper::~COPPAuthHelper
()
{
    if(m_pCrypto != NULL)
    {
        delete m_pCrypto;
        m_pCrypto=NULL;
    }
    
    if (0 != m_pbPublicKey)
    {
        delete [] m_pbPublicKey;
        m_pbPublicKey = 0;
    }
    //  the respective destructors reset any keys
//    if (0 != m_pCryptoHelper)
//    {
//        delete m_pCryptoHelper;
//        m_pCryptoHelper = 0;
//    }
}

/*
 *  function    :
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
COPPAuthHelper::Initialize
(
    const BYTE* pbCert,
    const DWORD dwCertLen
)
{
    HRESULT     hr = S_OK;
    BYTE*       pbPublicKey = 0;
    DWORD       cbKeyLen = 0;
    DWORD       dwExponent = 0;

    //  validation
    if ((0 == pbCert) || (0 == dwCertLen))
        return E_INVALIDARG;

    do
    {
        //  set this certificate
        if (FAILED(hr = m_CertHelper.SetCertificate(pbCert, dwCertLen)))
            break;

        //  extract the public key from the certificate
        if (FAILED(hr = m_CertHelper.ExtractPublicKey(&pbPublicKey, &cbKeyLen, &dwExponent)))
            break;

        //  initialize the crypto helper
        //  assert we're creating a new object
		if (0 != m_pbPublicKey)
			delete [] m_pbPublicKey;
		m_pbPublicKey = new BYTE[cbKeyLen+64];
		memcpy(m_pbPublicKey, pbPublicKey, cbKeyLen); //Scott: new! should replace the following!
		m_cbKeyLen   = cbKeyLen;
		m_dwExponent = dwExponent;

//		ASSERT(0 == m_pCryptoHelper);
//        m_pCryptoHelper = new CryptoHelper(pbPublicKey, cbKeyLen, dwExponent, &hr);
//        if (0 == m_pCryptoHelper)
//        {
//            hr = E_OUTOFMEMORY;
//        }

		// Release the memory allocated in the base functions
		if(pbPublicKey != NULL)
			delete [] pbPublicKey;
    }   while (FALSE);

//    SafeSecureMemDeleteArray(pbPublicKey, cbKeyLen);

    return hr;
}   //  COPPAuthHelper::Initialize


/*
 *  function    :   PrepareSignature
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
COPPAuthHelper::PrepareSignature
(
    BYTE*       pbSignature,
    const DWORD cbSigSize,
    const GUID* pgRandomNumber,
    const DWORD dwStatusSeqStart,
    const DWORD dwCommandSeqStart,
    BYTE*       pbSessionKey
)
{
    HRESULT     hr = S_OK;
    DWORD       cbOffset = 0;
    BYTE*       pbEncData = 0;
    DWORD       cbEncData = 0;
    DWORD       cbKeyLen = 0;
    DWORD       cbToEncrypt = 0;
    BYTE*       pbAESKey = 0;
    BYTE*       pbBSafeEncData = 0;
    DWORD       cbBSafeEncData = 0;

    //  validation
    //ASSERT((0 != pbSignature) && (0 != cbSigSize));
    if ((0 == pbSignature) || (0 == cbSigSize))
        return E_INVALIDARG;

    do
    {
        //  assembly data
        //  first comes the random 128 bit nr
        CopyMemory(pbSignature + cbOffset, pgRandomNumber, sizeof GUID);
        cbOffset += sizeof GUID;

        //  then the AES key
		//Scott: new! Should replace the following lines!
		GenerateRandomNumber(m_abSessionKey, 16);
		CopyMemory(pbSignature + cbOffset, m_abSessionKey, 16);
		cbOffset += 16;

        CopyMemory(pbSessionKey, m_abSessionKey, 16);

//        if (FAILED(hr = m_pCryptoHelper->ExportAESKey(&pbAESKey, &cbKeyLen)))
//            break;
//        CopyMemory(pbSignature + cbOffset, pbAESKey, cbKeyLen);
//        SafeSecureMemDeleteArray(pbAESKey, cbKeyLen);
//        cbOffset += cbKeyLen;

        //  then the status seq number
        CopyMemory(pbSignature + cbOffset, &dwStatusSeqStart, sizeof DWORD);
        cbOffset += sizeof DWORD;

        //  and the cmd seq number
        CopyMemory(pbSignature + cbOffset, &dwCommandSeqStart, sizeof DWORD);
        cbOffset += sizeof DWORD;

        //  zero the rest
        //ASSERT(cbSigSize >= cbOffset);
        ZeroMemory(pbSignature + cbOffset, cbSigSize - cbOffset);

        //OutputDebugStringA("** COPPSrcFilter: printing un-encrypted AMCOPPSignature..\n");
        PrintByteArray(pbSignature, sizeof AMCOPPSignature);

 		//Scott: New! replaces the following m_pCryptoHelper->Encrypt().
		{
			R_RSA_PUBLIC_KEY publickey;
			publickey.RSA_ModulusBit = 2048;
			ZeroMemory(publickey.RSA_publicExponent, MAX_RSA_MODULUS_LEN);
			ZeroMemory(publickey.RSA_Modulus, MAX_RSA_MODULUS_LEN);
			publickey.RSA_publicExponent[MAX_RSA_MODULUS_LEN-1] = (BYTE)(m_dwExponent    );
			publickey.RSA_publicExponent[MAX_RSA_MODULUS_LEN-2] = (BYTE)(m_dwExponent>> 8);
			publickey.RSA_publicExponent[MAX_RSA_MODULUS_LEN-3] = (BYTE)(m_dwExponent>>16);
			publickey.RSA_publicExponent[MAX_RSA_MODULUS_LEN-4] = (BYTE)(m_dwExponent>>24);
			SwitchEndianess( &publickey.RSA_Modulus[MAX_RSA_MODULUS_LEN-256], m_pbPublicKey, 256); //switch to big Endian


			BYTE temp_sig[40], temp_sig_encrypted[256];
			SwitchEndianess( temp_sig, pbSignature, 40); //switch to big Endian

			unsigned int outputlen;
			int ret = m_pCrypto->RSA_Encryption(temp_sig_encrypted,&outputlen,temp_sig,40,&publickey);

			SwitchEndianess( pbSignature, temp_sig_encrypted, 256); //switch to little Endian
		}
        //  encrypt
//        cbToEncrypt = sizeof AMCOPPSignature;
//        if (FAILED(hr = m_pCryptoHelper->Encrypt(pbSignature, cbToEncrypt, cbSigSize,
//                                                &pbEncData, &cbEncData)))
//            break;
        //OutputDebugStringA("** COPPSrcFilter: printing encrypted AMCOPPSignature..\n");
        PrintByteArray(pbEncData, cbEncData);

#ifdef  CRYPTO_DEBUG
        //  crypto_debug assumes we also know the private RSA key
        //  and we can verify the signature
        DWORD       cbDecDataSize = 256;
        BYTE*       pbDecData = 0;

        if (FAILED(m_RSAHelper.RSADecPrivate(pbEncData, cbEncData, (LPBYTE*)&pbDecData, &cbDecDataSize)) ||
            (0 == cbDecDataSize) || (cbDecDataSize != cbSigSize))
        {
            OutputDebugStringA("** COPPSrc: decryption failed!");
            break;
        }

        if (memcmp(pbSignature, pbDecData, cbDecDataSize))
            OutputDebugStringA("** COPPSrc: asymmetrical RSA decryption!!");
        else
        {
            OutputDebugStringA("** COPPSrcFilter: printing decrypted AMCOPPSignature..\n");
            PrintByteArray(pbDecData, cbDecDataSize);
        }

        delete [] pbDecData;
#endif  //  CRYPTO_DEBUG

        //  transfer result
//        ASSERT((0 != pbEncData) && (0 != cbEncData));
//        ZeroMemory(pbSignature, cbSigSize);
//        CopyMemory(pbSignature, pbEncData, cbEncData);
    }   while (FALSE);

//    SafeSecureMemDeleteArray(pbEncData, cbKeyLen);

    return hr;
}   //  COPPAuthHelper::PrepareSignature

/*
 *  function    :   COPPAuthHelper::Sign
 *  description :   signs the specified data buffer (OMAC-1)
 *  arguments   :   data buf, data size, out sig buf, sig size
 *  returns     :   hresult
 *  notes       :
 */
HRESULT
COPPAuthHelper::Sign
(
    const BYTE* pbData,
    const DWORD cbDataSize,
    BYTE*       pbSignature,
    const DWORD cbSigSize
)
{
    HRESULT     hr = S_OK;
    BYTE        rgbSignature[AES_BLOCKLEN] = {0};

    //  validation
    if ((0 == pbData) || (0 == cbDataSize) ||
        (0 == pbSignature) || (0 == cbSigSize))
        return E_INVALIDARG;

	//Scott: new! replaces the following line.
	hr = m_pCrypto->OMAC1_128(m_abSessionKey,pbData, cbDataSize,rgbSignature);
	
//    hr = m_pCryptoHelper->Sign((BYTE*)pbData, cbDataSize, rgbSignature);
    if (SUCCEEDED(hr))
        CopyMemory(pbSignature, rgbSignature, min(cbSigSize, AES_BLOCKLEN));

    return hr;
}   //  COPPAuthHelper::Sign

/*
 *  function    :   COPPAuthHelper::VerifySignature
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
COPPAuthHelper::VerifySignature
(
    const BYTE* pbData,
    const DWORD cbDataSize,
    const BYTE* pbSignature,
    const DWORD cbSigSize
)
{
    HRESULT     hr = S_OK;
    BYTE        rgbSignature[AES_BLOCKLEN] = {0};

    //  validation
    if ((0 == pbData) || (0 == cbDataSize) ||
        (0 == pbSignature) || (0 == cbSigSize))
        return E_INVALIDARG;

    if (cbSigSize != AES_BLOCKLEN)
        return E_INVALIDARG;

    do
    {
		//Scott: new! replaces the following line.
		if(!m_pCrypto->OMAC1_128(m_abSessionKey,pbData, cbDataSize,rgbSignature))
			break;
        //  calculate our own signature
//        if (FAILED(hr = m_pCryptoHelper->Sign((BYTE*)pbData, cbDataSize, rgbSignature)))
//            break;

        if (memcmp(pbSignature, rgbSignature, min(cbSigSize, AES_BLOCKLEN)))
            hr = E_FAIL;

    }   while (FALSE);

    return hr;
}   //  COPPAuthHelper::VerifySignature


HRESULT COPPAuthHelper::GenerateRandomNumber(BYTE *pbNumber, const DWORD cbSize)
{
	BYTE *p;
	for(p=pbNumber; p <= pbNumber+cbSize-4; p++)
	{
		DWORD randtime;
		__asm
		{
			push  eax
			push  edx
			rdtsc
			mov   randtime, eax
			pop   edx
			pop   eax
		}

		if(cbSize<=4)
		{
			memcpy(pbNumber, &randtime, cbSize);
			break;
		}

		*(DWORD *)p = *(DWORD *)p ^ randtime;
	}
	return S_OK;
}


int DecodeBase64( BYTE *inarray, int inputsize, BYTE *outarray, int outputsize)
{
	BYTE *pin  = inarray;
	BYTE *pout = outarray;
	int out[4];

	while( pin+4 <= inarray+inputsize && pout < outarray+outputsize)
	{
		//decode one 4-byte segment
		int inputbytesdecoded = 0;
		while( inputbytesdecoded<4 && pin < inarray+inputsize)
		{
			if(*pin >= 'A' && *pin <= 'Z')
				out[inputbytesdecoded++] = *pin - 'A';
			else if(*pin >= 'a' && *pin <= 'z')
				out[inputbytesdecoded++] = *pin - 'a' + 26;
			else if(*pin >= '0' && *pin <= '9')
				out[inputbytesdecoded++] = *pin - '0' + 52;
			else if(*pin == '+')
				out[inputbytesdecoded++] = 62;
			else if(*pin == '/')
				out[inputbytesdecoded++] = 63;
			else if(*pin == '=')
				out[inputbytesdecoded++] = -1; //indicate padding
			else
				out[inputbytesdecoded] = 0; //useless line.
			pin++;
		}
		if(inputbytesdecoded < 4)
			break;

		//assemble the 3-byte output
		if(pout >= outarray+outputsize || out[0]<0 || out[1]<0 || out[2]<0 && out[3]>=0) 
			break; //illegal input. Stop decoding.
		*pout++ = (out[0]<<2) | (out[1]>>4);
		if(pout >= outarray+outputsize || out[2]<0) 
			break;
		*pout++ = (out[1]<<4) | (out[2]>>2);
		if(pout >= outarray+outputsize || out[3]<0) 
			break;
		*pout++ = (out[2]<<6) | out[3];
	}
	return pout-outarray;
}


/*
 *  function    :   DecodeBase64String
 *  description :   allocates and decodes a base64 string to a binary array
 *  arguments   :
 *  returns     :   hresult
 *  notes       :   don't forget to deallocate the buffer when done
HRESULT
DecodeBase64String
(
    IN const CHAR* pszBase64Key,
    IN const DWORD  cchBase64KeyLen,
    OUT BYTE**      ppbBinaryKey,
    OUT DWORD*      pcbBinaryKeyLen
)
{
    HRESULT         hr = S_OK;
    DWORD           cbLen = 0, cbWritten = 0;
    BYTE*           pbBuf = 0;
    DWORD           dwSkip = 0, dwFlags = 0;

    do
    {
        if ((0 == pszBase64Key) || (0 == cchBase64KeyLen))
        {
            hr = E_INVALIDARG;
            break;
        }

        if ((0 == ppbBinaryKey) || (0 == pcbBinaryKeyLen))
        {
            hr = E_POINTER;
            break;
        }

        if (!CryptStringToBinaryA(pszBase64Key, cchBase64KeyLen, CRYPT_STRING_BASE64, 0, &cbLen, &dwSkip, &dwFlags))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        pbBuf = new BYTE [cbLen];
        if (0 == pbBuf)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if (!CryptStringToBinaryA(pszBase64Key, cchBase64KeyLen, CRYPT_STRING_BASE64, pbBuf, &cbLen, &dwSkip, &dwFlags))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        *pcbBinaryKeyLen = cbLen;
        *ppbBinaryKey = pbBuf;

    }   while (FALSE);

    if (FAILED(hr))
    {
        delete [] pbBuf;
    }

    return hr;
}   //  DecodeBase64String
 */


/*
 *  function    :   DecodeBase64RSAKey
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
DecodeBase64RSAKey
(
    IN const CHAR* pszBase64Modulus,
    IN const DWORD  cchBase64ModulusLen,
    IN const DWORD  cbModulusLen,
    IN const CHAR* pszBase64Exponent,
    IN const DWORD  cchBase64Exponent,
    OUT BYTE**      ppbModulus,
    OUT DWORD*      pcbModulus,
    OUT DWORD*      pdwExponent
)
{
    HRESULT         hr = S_OK;
    DWORD           dwExp = 0;
    BYTE*           pbMod = NULL;
    DWORD           cbMod = 0;
    BYTE*           pbExpNet = NULL;
    DWORD           cbExpNet = 0;
    BYTE*           pbModNet = NULL;
    DWORD           cbModNet = 0;
    DWORD           i = 0;

    do
    {
        //  validate params
        if ((0 == pszBase64Modulus) ||
            (0 == cchBase64ModulusLen) ||
            (0 == cbModulusLen) ||
            (0 == pszBase64Exponent) ||
            (0 == cchBase64Exponent))
        {
            hr = E_INVALIDARG;
            break;
        }

        if ((0 == ppbModulus) ||
            (0 == pcbModulus) ||
            (0 == pdwExponent))
        {
            hr = E_POINTER;
            break;
        }


		//Scott: new! replaces the following line.
		pbExpNet = new BYTE[cchBase64Exponent];
		cbExpNet = DecodeBase64((BYTE*) pszBase64Exponent, cchBase64Exponent, pbExpNet, cchBase64Exponent);
		hr = cbExpNet ? S_OK : E_FAIL;
        //  decode exponent
//        hr = DecodeBase64String(pszBase64Exponent,
//                                cchBase64Exponent,
//                                &pbExpNet, &cbExpNet);

        if (FAILED(hr))
        {
            break;
        }
        if (cbExpNet > sizeof DWORD)
        {
            hr = E_FAIL;
            break;
        }

        // convert to host order
        for (i = 0; i < cbExpNet; i++)
        {
            dwExp <<= 8;
            dwExp |= pbExpNet[i];
        }

		//Scott: new! replaces the following line.
		pbModNet = new BYTE[cchBase64ModulusLen];
		cbModNet = DecodeBase64((BYTE*)pszBase64Modulus, cchBase64ModulusLen, pbModNet, cchBase64ModulusLen);
		hr = cbModNet ? S_OK : E_FAIL;
        //  decode modulus
//        hr = DecodeBase64String(pszBase64Modulus,
//                                cchBase64ModulusLen,
//                                &pbModNet, &cbModNet);
        if (FAILED(hr))
        {
            break;
        }
        if (cbModNet > cbModulusLen)
        {
            hr = E_FAIL;
            break;
        }

        pbMod = new BYTE[cbModulusLen];
        if (0 == pbMod)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        // convert to host order by reversing
        for (i = 0; i < cbModNet; i++)
        {
            pbMod[i] = pbModNet[cbModNet - 1 - i];
        }

        // set leading zeroes
        for (; i < cbModulusLen - 1; i++)
        {
            pbMod[i] = 0;
        }

        *pdwExponent = dwExp;
        *ppbModulus = pbMod;
        *pcbModulus = cbModulusLen;

    }   while (FALSE);

    delete [] pbExpNet;
    delete [] pbModNet;
    if (FAILED(hr))
    {
        delete [] pbMod;
    }

    return(hr);
}   //  DecodeBase64RSAKey


/*
 *  function    :   PrintByteArray
 *  description :   helper, debug prints a byte array
 *  arguments   :   array, array size
 *  returns     :
 *  notes       :
 */
void
PrintByteArray
(
    const BYTE* pbArray,
    const DWORD cbArraySize
)
{
#ifdef  CRYPTO_DEBUG
    char    szText[MAX_PATH+1];
    char    sztmp[10];
    DWORD   i = 0, j = 0, k = cbArraySize/16;
    DWORD   rest = cbArraySize % 16;

    for (i = 0; i < k; i++)
    {
        StringCchPrintfA(szText, MAX_PATH, "%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X,%2.2X\n",
                        pbArray[i*16+0x0],
                        pbArray[i*16+0x1],
                        pbArray[i*16+0x2],
                        pbArray[i*16+0x3],
                        pbArray[i*16+0x4],
                        pbArray[i*16+0x5],
                        pbArray[i*16+0x6],
                        pbArray[i*16+0x7],
                        pbArray[i*16+0x8],
                        pbArray[i*16+0x9],
                        pbArray[i*16+0xa],
                        pbArray[i*16+0xb],
                        pbArray[i*16+0xc],
                        pbArray[i*16+0xd],
                        pbArray[i*16+0xe],
                        pbArray[i*16+0xf]);
        OutputDebugStringA(szText);
    }
    StringCchPrintfA(szText, MAX_PATH, "");
    for (i = 0; i < rest; i++)
    {
        StringCchPrintfA(sztmp, 10, "%2.2x,", pbArray[k*16 + i]);
        StringCchCatA(szText, MAX_PATH, sztmp);
    }
    OutputDebugStringA(szText);
    OutputDebugStringA("\n");
#endif  //  CRYPTO_DEBUG
}   //  PrintByteArray

