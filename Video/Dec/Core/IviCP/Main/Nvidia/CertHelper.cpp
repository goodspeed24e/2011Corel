/*
 *  file        :   CertHelper
 *  description :   implementation of the certificate helper
 */
#pragma warning(disable:4312)

#include <windows.h>
#include <streams.h>
#include "strsafe.h"

#include "CertHelper.h"

/*
 *        class:    COPPCertHelper
 *  description:    helper class for processing XrML certificates
 */
/*
 *  constructor :
 *  description :
 *  parameters  :
 *  returns     :
 */
COPPCertHelper::COPPCertHelper
(): m_pbCertificate(NULL),
    m_cbCertificateLength(0)
{}  //  COPPCertHelper::COPPCertHelper

/*
 *  dtor        :
 *  description :
 */
COPPCertHelper::~COPPCertHelper
()
{
    delete [] m_pbCertificate;
    m_pbCertificate = 0;
    m_cbCertificateLength = 0;
}   //  COPPCertHelper::~COPPCertHelper

/*
 *  function    :
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
COPPCertHelper::ExtractPublicKey
(
    BYTE**          ppbKey,
    DWORD*          pcbKeyLength,
    DWORD*          pdwExponent
)
{
    HRESULT         hr = S_OK;
    DWORD           cbKey64Len = 0, cbExponentLen = 0, dwExponent = 0;
    BYTE*           pbExponent = 0;
    CHAR*           szKey64 = 0;
    CHAR*           szExponent64 = 0;
    DWORD           cbBinaryKeyLen = 0;
    BYTE*           pbBinaryKey = 0;
    DWORD           dwSkip = 0;
    DWORD           dwFlags = 0;
    const DWORD     RSA_MODULUS_LENGTH = 0x100;

    //  validation
    if ((0 == ppbKey) || (0 == pcbKeyLength) || (0 == pdwExponent))
        return E_INVALIDARG;
    
    if ((0 == m_pbCertificate) || (0 == m_cbCertificateLength))
        return E_UNEXPECTED;

    //
    do
    {
        //  find key
        if (FAILED(hr = FindElement(&szKey64, &cbKey64Len, XRML_TAG_MODULUS)))
            break;

        //  find exponent
        if (FAILED(hr = FindElement(&szExponent64, &cbExponentLen, XRML_TAG_EXPONENT)))
            break;

        //  Base64 decode the modulus and the exponent
        if (FAILED(hr = DecodeBase64RSAKey(szKey64, cbKey64Len/(sizeof CHAR), RSA_MODULUS_LENGTH,
                                        szExponent64, cbExponentLen, &pbBinaryKey, &cbBinaryKeyLen, &dwExponent)))
            break;

        //  prepare out params
        *ppbKey = pbBinaryKey;
        *pcbKeyLength = cbBinaryKeyLen;
        *pdwExponent = dwExponent;

    }   while (FALSE);

    //  cleanup
    delete [] szKey64;
    delete [] szExponent64;
    if (FAILED(hr))
    {
        delete [] pbBinaryKey;
    }

    return hr;
}   //  COPPCertHelper::ExtractPublicKey


/*
 *  function    :   FindElement
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
COPPCertHelper::FindElement
(
    CHAR**          ppstrElement,
    DWORD*          pcbElementLen,
    const CHAR*     pstrTag
)
{
    HRESULT     hr = S_OK;
    CHAR*       pbIdx = 0;
    DWORD       cbIdx = 0;
    CHAR*       pstrElement = 0;
    DWORD       cbElementLen = 0;
    CHAR*       pbBuf = 0;
    DWORD       cbBufLen = 0;
    CHAR        szTag[MAX_PATH+1] = "";
    DWORD       chLen = 0;
    
    if ((0 == ppstrElement) || (0 == pcbElementLen) || (0 == pstrTag))
        return E_INVALIDARG;

    do
    {
        //  prepare NULL terminated buffer
        cbBufLen = m_cbCertificateLength + sizeof (CHAR);
        pbBuf = new CHAR[cbBufLen / sizeof (CHAR)];
        if (0 == pbBuf)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //  copy certificate; NULL-terminate
		memcpy(pbBuf, m_pbCertificate, m_cbCertificateLength);
		pbBuf[m_cbCertificateLength] = 0;

        //  start parsing - first look for the leading tag
        StringCchPrintfA(szTag, MAX_PATH, "<%s>", pstrTag);
        pbIdx = strstr(pbBuf, szTag);
        if (0 == pbIdx)
        {
            hr = E_FAIL;
            break;
        }
        StringCchLengthA(szTag, MAX_PATH, (size_t*)&chLen);
        cbIdx = static_cast<DWORD>(pbIdx - pbBuf + chLen * sizeof (CHAR));
        pbIdx = pbBuf + cbIdx;

        //  look for the closing tag
        StringCchPrintfA(szTag, MAX_PATH, "</%s>", pstrTag);
        pbIdx = strstr(pbIdx, szTag);
        if (0 == pbIdx)
        {
            hr = E_FAIL;
            break;
        }
        chLen = static_cast<DWORD>((pbIdx - pbBuf - cbIdx)/sizeof CHAR);
        pbIdx = pbBuf + cbIdx;

        pstrElement = new CHAR[chLen + 1];
        if (0 == pstrElement)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //  copy
        CopyMemory(pstrElement, pbIdx, chLen * sizeof CHAR);
        pstrElement[chLen] = '\0';

    }   while (FALSE);

    //  cleanup
    ZeroMemory(pbBuf, cbBufLen);
    delete [] pbBuf;
    pbBuf = 0;

    if (FAILED(hr))
    {
        if (0 != pstrElement)
        {
            ZeroMemory(pstrElement, chLen * sizeof CHAR);
            delete [] pstrElement;
            pstrElement = 0;
        }
        chLen = 0;
    }

    //  and return
    *ppstrElement = pstrElement;
    *pcbElementLen = chLen * sizeof CHAR;

    return hr;
}   //  COPPCertHelper::FindElement

/*
 *  function    :   SetCertificate
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
COPPCertHelper::SetCertificate
(
    const BYTE*     pbCert,
    const DWORD     cbCertLen
)
{
    HRESULT         hr = S_OK;
    
    if ((0 == pbCert) || (0 == cbCertLen))
        return E_INVALIDARG;

    do
    {
        hr = ResetCertificate();

        m_pbCertificate = new BYTE[cbCertLen];
        if (0 == m_pbCertificate)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        CopyMemory(m_pbCertificate, pbCert, cbCertLen);
        m_cbCertificateLength = cbCertLen;

        hr = S_OK;
    }   while (FALSE);

    return hr;
}   //  COPPCertHelper::SetCertificate

/*
 *  function    :   ResetCertificate
 *  description :
 *  arguments   :
 *  returns     :
 *  notes       :
 */
HRESULT
COPPCertHelper::ResetCertificate
()
{
    HRESULT     hr = S_OK;

    if ((0 == m_pbCertificate) || (0 == m_cbCertificateLength))
        return E_UNEXPECTED;

    ZeroMemory(m_pbCertificate, m_cbCertificateLength);
    delete [] m_pbCertificate;
    m_pbCertificate = 0;
    m_cbCertificateLength = 0;

    return S_OK;
}   //  COPPCertHelper::ResetCertificate
