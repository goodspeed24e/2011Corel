/*
 *  file        :   AuthenticationHelper
 *  description :   declaration of hte authentication helper
 *
 *
 */

#pragma once

#define XRML_TAG_MODULUS                ("Modulus")
#define XRML_TAG_EXPONENT               ("Exponent")
#define XRML_TAG_RSAKEYVALUE            ("RSAKeyValue")

#define RSA_MODULUS_BYTE_LENGTH  512


/*
 *  class       :   COPPCertHelper
 *  description :   certificate parsing helper function
 *  inherits    :   
 *  notes       :   
 */
class COPPCertHelper
{
public:
    //  ctors
    COPPCertHelper();
    ~COPPCertHelper();

    //  methods
    HRESULT     SetCertificate(const BYTE*  pbCert,
                                const DWORD cbCertLen);

    HRESULT     ExtractPublicKey(BYTE**     ppbKey,
                                DWORD*      pcbKeyLen,
                                DWORD*      pdwKeyExponent);

protected:

    HRESULT     FindElement(CHAR**          ppstrElement,
                            DWORD*          pcbElementLen,
                            const CHAR*     pstrTag);

    HRESULT     ResetCertificate();

private:
    BYTE*       m_pbCertificate;
    DWORD       m_cbCertificateLength;
};  //  class COPPcertHelper

//  other helper functions
HRESULT DecodeBase64RSAKey(const CHAR*  szBase64Modulus, 
                           const DWORD  cchBase64ModulusLen, 
                           const DWORD  cbModulusLen,
                           const CHAR*  szBase64Exponent, 
                           const DWORD  cchBase64ExponentLen, 
                           BYTE**       ppbBinaryRSAKey, 
                           DWORD*       pcbBinaryRSAKeyLen, 
                           DWORD*       pcbRSAExponent);

HRESULT DecodeBase64String(const CHAR*  pszBase64String,
                           const DWORD  cchBase64StringLen,
                           BYTE**       ppbBinaryNumber,
                           DWORD*       pcbBinaryNumberLen);
