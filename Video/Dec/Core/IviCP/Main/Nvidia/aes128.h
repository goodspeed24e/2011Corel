#pragma once

/**
 * An encryption library based on AES-128
 * block cipher to support CTR modes.
 *
 */

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#define AES_DW                  4
typedef struct _AES128_Helper   AES128_Helper;

class AES128{
private:
    AES128_Helper *m_pHelper;

public:
    AES128();
    ~AES128();

    void GenerateKey(DWORD pKey[AES_DW]);
    void UpdateIV(DWORD pIV[AES_DW]);
    void EncryptCTR64(const BYTE* pt, BYTE* ct, const DWORD dwByteCount);
	void EncryptECB(const BYTE* pt, BYTE* ct, const DWORD dwByteCount);
};