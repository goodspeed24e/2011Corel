#ifndef _IVI_TRUTIL_H
#define _IVI_TRUTIL_H

#include "IVITrWinDef.h"

#ifdef _WIN32
#include <stdlib.h>

NS_TRLIB_BEGIN

	///*!
	// [Internal function. Do not call it.]
	//
	// Encrypt string.\n
	//
	//-# szValue - [IN] the content needs to be encrypted by XOR with bytePattern.\n
	//-# nBuffSize -[IN] the size of sz_value.\n
	//-# bytePattern - [IN] a pattern to encrypt code by XOR.\n 
	//-# nEncryptType - [IN] 0, encrypt CLSID, szValue is clsid by StringfromCLSID(); 1, encrypt string\n
	// Return value - [bool] not use.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl trEncryptContent(char *szValue, int nBuffSize, BYTE bytePattern, int nEncryptType);

	///*!
	// [Internal function. Do not call it.]
	//
	// Encrypt string.\n
	//
	//-# wszValue - [IN] the content needs to be encrypted by XOR with bytePattern.\n
	//-# nBuffSize -[IN] the size of sz_value.\n
	//-# bytePattern - [IN] a pattern to encrypt code by XOR.\n 
	//-# nEncryptType - [IN] 0, encrypt CLSID, szValue is clsid by StringfromCLSID(); 1, encrypt string\n
	// Return value - [bool] not use.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl trEncryptContent(wchar_t *wszValue, int nBuffSize, BYTE bytePattern, int nEncryptType);

	///*!
	// [Internal function. Do not call it.]
	//
	// Decrypt string.\n
	//
	//-# szValue - [IN] the content needs to be decrypted by XOR with bytePattern.\n
	//-# bytePattern - [IN] a pattern to decrypt code by XOR.\n 
	//-# nDecryptType - [IN] 0, decrypt CLSID, szValue is clsid by StringfromCLSID(); 1, decrypt string\n
	// Return value - [bool] not use.\n
	//
	// Supported platform: All Windows\n
	//
	//*/
	bool __cdecl trDecryptContent(char *szValue, BYTE bytePattern, int nDecryptType);

	///*!
	// [Internal function. Do not call it.]
	//
	// Decrypt string.\n
	//
	//-# wszValue - [IN] the content needs to be decrypted by XOR with bytePattern.\n
	//-# bytePattern - [IN] a pattern to decrypt code by XOR.\n 
	//-# nDecryptType - [IN] 0, decrypt CLSID, szValue is clsid by StringfromCLSID(); 1, decrypt string\n
	// Return value - [bool] not use.\n
	//
	// Supported platform: All Windows\n
	//
	//*/
	bool __cdecl trDecryptContent(wchar_t *wszValue, BYTE bytePattern, int nDecryptType);

NS_TRLIB_END

	///*!
	// Encrypt string.\n
	//
	//-# sz_value - [IN] the content needs to be encrypted by XOR with bytePattern.\n
	//-# int_buffsize -[IN] the size of sz_value.\n
	//-# byte_pattern - [IN] a pattern to encrypt code by XOR.\n 
	//-# int_encrypttype - [IN] 0, encrypt CLSID, sz_Value is clsid by StringfromCLSID(); 1, encrypt string\n
	// Return value - None\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// the length of encrpyted content will be at least double size than the original.\n
	// As the following example, it should allocate 16 x 2 + 1 units for the encrypted content.\n
	// 16 is the length of "theSimpleContent", 1 is for terminated character.\n
	//
	// Usage:\n
	//
	//	char szSample[33]; 
	//	sprintf(szSample, "theSimpleContent");
	//	...
	//	iviTR_ENCRYPT_CONTENT_EX(szSample, 33, 22, 1);
	//	...
	// 
	//*/
	#define iviTR_ENCRYPT_CONTENT_EX(sz_value, int_buffsize, byte_pattern, int_encrypttype) \
	{\
		trEncryptContent(sz_value, int_buffsize, byte_pattern, int_encrypttype);\
	}

	///*!
	// Encrypt string.\n
	//
	//-# sz_value - [IN] the content needs to be encrypted by XOR with bytePattern.\n
	//-# byte_pattern - [IN] a pattern to encrypt code by XOR.\n 
	//-# int_encrypttype - [IN] 0, encrypt CLSID, sz_Value is clsid by StringfromCLSID(); 1, encrypt string\n
	// Return value - None\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// For backward compatible, please use iviTR_ENCRYPT_CONTENT_EX first.
	// 
	//*/
#if (_MSC_VER < 1400)
#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif
#endif
	#define iviTR_ENCRYPT_CONTENT(sz_value, byte_pattern, int_encrypttype)	\
		iviTR_ENCRYPT_CONTENT_EX(sz_value, _countof(sz_value), byte_pattern, int_encrypttype)

	///*!
	// Decrypt string.\n
	//
	//-# sz_value - [IN] the content needs to be decrypted by XOR with bytePattern.\n
	//-# byte_pattern - [IN] a pattern to decrypt code by XOR.\n 
	//-# int_decrypttype - [IN] 0, decrypt CLSID, sz_Value is clsid by StringfromCLSID(); 1, decrypt string\n
	// Return value - None\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// 
	//*/
	#define iviTR_DECRYPT_CONTENT(sz_value, byte_pattern, int_decrypttype) \
	{\
		trDecryptContent(sz_value, byte_pattern, int_decrypttype);\
	}

#elif defined(__linux__)
	#define iviTR_ENCRYPT_CONTENT_EX(sz_value, int_buffsize, byte_pattern, int_encrypttype)
	#define iviTR_ENCRYPT_CONTENT(sz_value, byte_pattern, int_encrypttype)
	#define iviTR_DECRYPT_CONTENT(sz_value, byte_pattern, int_decrypttype)
#endif // _WIN32

#ifdef __cplusplus

NS_TRLIB_BEGIN

	class CtrReverseStr
	{
	private:
		BYTE *m_pDupStr;

	public:
		CtrReverseStr(char *pszSrcData) : m_pDupStr(NULL)
		{
			m_pDupStr = (BYTE*)_strrev(_strdup(pszSrcData));
		}
		CtrReverseStr(wchar_t *pszSrcData) : m_pDupStr(NULL)
		{
			m_pDupStr = (BYTE*)_wcsrev(_wcsdup(pszSrcData));
		}
		virtual ~CtrReverseStr(void)
		{
			if (m_pDupStr) 
				free(m_pDupStr);
			m_pDupStr = NULL;
		}

		inline operator const wchar_t*() const throw()
		{
			return (m_pDupStr != NULL) ? (const wchar_t*)m_pDupStr : NULL;
		}
		inline operator wchar_t*() const throw()
		{
			return (m_pDupStr != NULL) ? (wchar_t*)m_pDupStr : NULL;
		}
		inline operator const char*() const 
		{
			return (m_pDupStr != NULL) ? (const char*)m_pDupStr : NULL;
		}
		inline operator char*() const 
		{
			return (m_pDupStr != NULL) ? (char*)m_pDupStr : NULL;
		}
	};

NS_TRLIB_END

///*!
// A constant reversed string.\n
// It can be reversed to normal string in runtime.\n
//
//-# sz_NORMAL - [IN] the content needs to be encrypted by XOR with bytePattern.\n
//-# sz_REVERSED - [IN] a pattern to encrypt code by XOR.\n 
// Return value - A constant normal string.\n
//
// Supported platform:		All Windows
// Performance overhead:	low
// Security level:			low
// Usage scope:				Macro scope
//
// Note:\n
// Please treat it character by character. 
// If your character is '\r', '\n' or '\t', that would be one character.
// 
// example1.
// Original syntax: printf("C:\\%s\\tr.txt", szBuf);
// Change to macro: printf(iviTR_CONST_REVERSE_STR("C:\\%s\\tr.txt\r\n", "\n\rtxt.rt\\s%\\:C"), szBuf);
//
// example2.
// Original syntax: strnicmp(szBuf, _T("BMW.dtcp"));
// Change to macro: strnicmp(szBuf, iviTR_CONST_REVERSE_STR(_T("BMW.dtcp"), _T("pctd.WMB")));
//
//*/
	#ifdef _WIN32
		#define iviTR_CONST_REVERSE_STR(sz_NORMAL, sz_REVERSED) CtrReverseStr(sz_REVERSED)
	#elif defined(__linux__)
		#define iviTR_CONST_REVERSE_STR(sz_NORMAL, sz_REVERSED) sz_NORMAL
	#endif
#else
	#define iviTR_CONST_REVERSE_STR(sz_NORMAL, sz_REVERSED) sz_NORMAL
#endif

#ifdef __cplusplus
	#define ENCRYPT_BYTE_PATTERN_FOR_CtrArrayOf 28

NS_TRLIB_BEGIN

    ///*!
    // this is a template class to secure sensitive array data
    //
    // Note1: This class is not thread-safe. Caller should handle synchronization issue outside this class.
    // Note2: the sizeof() C operator can not work as it is expected because sizeof(CtrArrayOf) doesn't equal to sizeof(the decrypted data) !
    // Please use CtrArrayOf.m_fnSizeof() instead if you want to get the total bytes of the decrypted data.
    //*/
    template <class Type> class CtrArrayOf
    {
    private:
	    ///*!
	    // the decrypted raw data array
	    //*/
        Type *m_pElementList;

	    ///*!
	    // the total element in 'm_pElementList'
	    //*/
	    int m_nElementCount;

	    ///*!
	    // the NULL-terminated original encrypted HEX string passed in the constructor
	    //*/
	    char *m_pszSrcEncData;

	    ///*!
	    // flag to indicate if 'm_pElementList' contains decrypted data
	    //*/
	    bool m_bIsDataDecrypted;

    protected:
	    ///*!
	    // scramble 'm_pElementList' to be garbage data
	    //
	    // Return value - true if successful, otherwise, false.
	    //*/
	    bool m_fnScrambleData(void)
	    {
		    int nSize = this->m_nElementCount * sizeof(Type);

		    // scramble the decrypted buffer although it will be freed
		    // (this can defend memory dump attack)
		    for (int ni = 0; ni < nSize; ni ++){
			    ((BYTE*)this->m_pElementList)[ni] ^= ni;
		    }

		    return true;
	    };

	    ///*!
	    // decrypt 'm_pszSrcEncData' to 'm_pElementList'
	    //
	    // Return value - true if successful, otherwise, false.
	    //*/
	    bool m_fnDecryptData(void)
	    {
		    // local variables need to be freed below
		    char *pszTmp = NULL;
		    // local variables need to be freed above

		    bool bRetValue = true;
		    int nSize;

		    if (!this->m_bIsDataDecrypted){ // not decrypted yet
			    nSize = strlen(this->m_pszSrcEncData);

			    pszTmp = new char[nSize + 1];

			    if (NULL == pszTmp){
                    #ifdef _DEBUG
				        __asm int 3
                    #endif

				    bRetValue = false;
				    goto err_out;
			    }

				#if (_MSC_VER >= 1300) && defined(_WIN32)
				    strcpy_s(pszTmp, nSize + 1, this->m_pszSrcEncData);
				#else
				    strcpy(pszTmp, this->m_pszSrcEncData);
				#endif

			    // decrypt back to HEX string
                iviTR_DECRYPT_CONTENT(pszTmp, ENCRYPT_BYTE_PATTERN_FOR_CtrArrayOf, 1);

			    DWORD dwData; // can not declare as BYTE only for below sscanf() use because sscanf() will treat dwData as 32-bit
			    BYTE *pData;
			    char *pszTmp2 = pszTmp;
			    int nIndex;


			    for (int ni = 0; ni < this->m_nElementCount; ni ++){
				    pData = (BYTE*)&this->m_pElementList[ni];

				    for (nIndex = sizeof(Type) - 1; nIndex >= 0; nIndex --){
					    if (1 != sscanf(pszTmp2, "%02X", &dwData)){ // convert back to HEX value
                            #ifdef _DEBUG
				                __asm int 3
                            #endif

    					    bRetValue = false;
						    goto err_out;
					    }

					    pData[nIndex] = ((BYTE*)&dwData)[0];
					    pszTmp2 += 2; // skip 2 HEX characters
				    }
			    }


			    if (NULL != pszTmp2[0]){
                    #ifdef _DEBUG
				        __asm int 3
                    #endif

    			    bRetValue = false;
				    goto err_out;
			    }

			    this->m_bIsDataDecrypted = true;
		    }

        err_out:
	        if (NULL != pszTmp){
			    delete [] pszTmp;
			    pszTmp = NULL;
		    }

	        return bRetValue;
	    };


    public:
        ///*!
        // constructor of CtrArrayOf template class
        //
        // -# pszSrcEncData - [IN] can not be NULL. It is the given data encrypted from iviTR_ENCRYPT_CONTENT(byte pattern = ENCRYPT_BYTE_PATTERN_FOR_CtrArrayOf, encrypt type = 1) with the source data in capital little-endian HEX string (like "001B2E").
        // For example, if the source data is DWORD array {15, 32, 256}, it should be extended to {0x0000000F, 0x00000020, 0x00000100} first. Then, transform to HEX string "0000000F" "00000020" "00000100" ! Finally,
        // use 'iviTR_ENCRYPT_CONTENT(byte pattern = ENCRYPT_BYTE_PATTERN_FOR_CtrArrayOf, encrypt type = 1)' to encrypt it as "2c2c2c2c2c2c2c5a" "2c2c2c2c2c2c2e2c" "2c2c2c2c2c2d2c2c" ! So, 'pszSrcEncData' should be "2c2c2c2c2c2c2c5a2c2c2c2c2c2c2e2c2c2c2c2c2c2d2c2c" !
        // If the source data is BYTE string, it should contain the NULL terminator. For example, if the source data is "This is a test !", it should be converted to HEX string as "5468697320697320612074657374202100" ! Then, 
        // use 'iviTR_ENCRYPT_CONTENT(byte pattern = ENCRYPT_BYTE_PATTERN_FOR_CtrArrayOf, encrypt type = 1)' to encrypt it as "29282a242a252b2f2e2c2a252b2f2e2c2a2d2e2c2b282a292b2f2b282e2c2e2d2c2c" ! So, 'pszSrcEncData' should be "29282a242a252b2f2e2c2a252b2f2e2c2a2d2e2c2b282a292b2f2b282e2c2e2d2c2c" !
        //
        // Return value - None
        //*/
	    CtrArrayOf(char *pszSrcEncData)
	    {
		    this->m_bIsDataDecrypted = false;
		    this->m_pszSrcEncData = NULL;
		    this->m_pElementList = NULL;

		    // total encrypted data bytes
		    int nSize = strlen(pszSrcEncData);

		    // calculate the total element after decrypting 'pszSrcEncData'
		    // strlen(pszSrcEncData) / 2 = the bytes used to represent the source HEX string
		    // (strlen(pszSrcEncData) / 2) / 2 = the bytes used to represent the source data
		    // (strlen(pszSrcEncData) / 4) / sizeof(Type) = the total element
		    this->m_nElementCount = nSize / (4 * sizeof(Type));

            this->m_pszSrcEncData = new char[nSize + 1];

		    if (NULL == this->m_pszSrcEncData){
                #ifdef _DEBUG
				    __asm int 3
                #endif

			    return;
		    }

            this->m_pElementList = new Type[this->m_nElementCount];

		    if (NULL == this->m_pElementList){
                #ifdef _DEBUG
				    __asm int 3
                #endif

                return;
		    }

		    // copy the given encrypted source data including NULL terminator
		    memcpy((void*)this->m_pszSrcEncData, pszSrcEncData, nSize + 1);
	    };

	    virtual ~CtrArrayOf(void)
	    {
		    if (NULL != this->m_pElementList){
			    this->m_fnScrambleData();
			    delete [] this->m_pElementList;
			    this->m_pElementList = NULL;
		    }

		    // the given encrypted HEX string is not important so don't need to scramble the buffer
		    if (NULL != this->m_pszSrcEncData){
			    delete [] this->m_pszSrcEncData;
			    this->m_pszSrcEncData = NULL;
		    }
	    };

    public:
        ///*!
        // Re-encrypt the data if it is already decrypted before. It will do nothing if the data is already encrypted.
        //
	    // Return value - true if successful, otherwise, false.
        //*/
	    bool m_fnEncryptData(void)
	    {
            bool bRetValue = true;

            if (this->m_bIsDataDecrypted){ // already decrypted before
                bRetValue = this->m_fnScrambleData();
                this->m_bIsDataDecrypted = false;
            }

            return bRetValue;
	    };

        ///*!
        // calculate the size in byte of the decrypted data
        //
        // Return value - None
        //*/
	    unsigned m_fnSizeOf(void)
	    {
		    return this->m_nElementCount * sizeof(Type);
	    };

        ///*!
        // opeator overloading on '[]'
        //
        // -# nIndex - [IN] the zero-based index to the decrypted data
        //
        // Return value - the decrypted data element
        //*/
	    Type& operator[](int nIndex)
	    {
		    this->m_fnDecryptData();
		    return this->m_pElementList[nIndex];
	    };

        ///*!
        // opeator overloading on '(TYPE*)' to cast the data type
        //
        // Return value - the decrypted data element list
        //*/
	    operator Type*()
	    {
		    this->m_fnDecryptData();
		    return this->m_pElementList;
	    };

        ///*!
        // opeator overloading on '&'
        //
        // Return value - the decrypted data element list
        //*/
	    Type* operator &()
	    {
		    this->m_fnDecryptData();
		    return this->m_pElementList;
	    };
        
    };

	class CtrEncryptContentObject
	{
	public:
		CtrEncryptContentObject();
		~CtrEncryptContentObject();
		void fnEncryptContent(char **ppszEncrypted, char *pszOriginal, BYTE byPattern);
		void fnEncryptContent(WCHAR **ppwszEncrypted, WCHAR *pwszOriginal, BYTE byPattern);
		void fnDecryptContent(char **ppszDecrypted, char *pszOriginal, BYTE byPattern);
		void fnDecryptContent(WCHAR **ppwszDecrypted, WCHAR *pwszOriginal, BYTE byPattern);
		void fnClear();
	private:
		BYTE *m_pbyteEncrypted;
	};

    ///*!
    // this is a template class to secure sensitive array data. And it is version 2 of string decryption.
    //
    // Note1: This class is not thread-safe. Caller should handle synchronization issue outside this class.
    // Note2: the sizeof() C operator can not work as it is expected because sizeof(CtrDecryptor) doesn't equal to sizeof(the decrypted data) !
    // Please use CtrDecryptor.m_fnSizeof() instead if you want to get the total bytes of the decrypted data.
	// sample :
	// CtrDecryptor<WCHAR> wszDecrypted(L"66163A665C3A4232ECA65C66305C313032313332343335343635373638373938", 22);
	// WCHAR *pwszTarget = &wszDecrypted;
	// ::MessageBoxW(NULL, pwszTarget, L"decryption", MB_OK);

    //*/
    template <class Type> class CtrDecryptor
    {
    private:
	    ///*!
	    // the decrypted raw data array
	    //*/
        Type *m_pElementList;

	    ///*!
	    // the total element in 'm_pElementList'
	    //*/
	    int m_nElementCount;

	    ///*!
	    // the NULL-terminated original encrypted HEX string passed in the constructor
	    //*/
	    Type *m_pSrcEncData;

	    ///*!
	    // flag to indicate if 'm_pElementList' contains decrypted data, it is valid while m_bIsEncryption is unset.
	    //*/
	    bool m_bIsDataDecrypted;

	    ///*!
	    // the pattern will be needed during protecting operation(encryption or decryption).
	    //*/
		BYTE m_bytePattern;

    protected:
	    ///*!
	    // decrypt 'm_pszSrcEncData' to 'm_pElementList'
	    //
	    // Return value - true if successful, otherwise, false.
	    //*/
	    bool m_fnDecryptData(void)
	    {
		    if (!this->m_bIsDataDecrypted)
			{
				Type *pTarget;
				CtrEncryptContentObject encObject;
				if (m_pSrcEncData)
					encObject.fnDecryptContent(&pTarget, m_pSrcEncData, m_bytePattern);

				int i = 0;
				if (m_pElementList)
				{
					while (*(pTarget + i) != 0x0000 && i < m_nElementCount)
					{
						m_pElementList[i] = *(pTarget + i);
						i++;
					}
					m_pElementList[i] = 0x0000;
					m_bIsDataDecrypted = true;
				}
			}

	        return true;
	    };

    public:
        ///*!
        // constructor of CtrDecryptor template class
        //
        // -# pSrcEncData, the unprotected content
		// -# bytePattern, the pattern will be needed during protecting operation.
        // Return value - None
        //*/
	    CtrDecryptor(Type *pSrcEncData, BYTE bytePattern = 28)
	    {
			this->m_bytePattern = bytePattern;
		    this->m_bIsDataDecrypted = false;
		    this->m_pSrcEncData = NULL;
		    this->m_pElementList = NULL;

			m_fnAttach(pSrcEncData, bytePattern);
	    };

	    virtual ~CtrDecryptor(void)
	    {
			m_fnDetach();
	    };

    public:
        ///*!
        // Detach member data : (m_pElementList & m_pSrcEncData)
        //
        // Return value - None
        //*/
	    void m_fnDetach(void)
	    {
		    if (NULL != this->m_pElementList){
			    delete [] this->m_pElementList;
			    this->m_pElementList = NULL;
		    }

		    // the given encrypted HEX string is not important so don't need to scramble the buffer
		    if (NULL != this->m_pSrcEncData){
			    delete [] this->m_pSrcEncData;
			    this->m_pSrcEncData = NULL;
		    }
			this->m_bytePattern = 0;
			this->m_bIsDataDecrypted = false;
	    };

        ///*!
        // Attach a new encrypted charactor string with new pattern.
		// Call m_fnDetach before calling m_fnAttach for keep it without any memory leak.
        //
        // -# pSrcEncData, the unprotected content
		// -# bytePattern, the pattern will be needed during protecting operation.
		// 
        // Return value - true / false
        //*/
		bool m_fnAttach(Type *pSrcEncData, BYTE bytePattern)
		{
		    // total encrypted data bytes
		    int nSize = 0;
			if (pSrcEncData) {
				while (*(pSrcEncData + nSize) != 0x0000)
					nSize++;
			}

		    // calculate the total element after decrypting 'pszSrcEncData'
		    // strlen(pszSrcEncData) / 2 = the bytes used to represent the source HEX string
		    // (strlen(pszSrcEncData) / 2) / 2 = the bytes used to represent the source data
		    // (strlen(pszSrcEncData) / 4) / sizeof(Type) = the total element
			this->m_nElementCount = nSize / 4;

            this->m_pSrcEncData = new Type[nSize + 1];

		    if (NULL == this->m_pSrcEncData){
                #ifdef _DEBUG
				    __asm int 3
                #endif

                return false;
		    }

            this->m_pElementList = new Type[this->m_nElementCount + 1];

		    if (NULL == this->m_pElementList){
                #ifdef _DEBUG
				    __asm int 3
                #endif

                return false;
		    }

		    // copy the given encrypted source data including NULL terminator
			int i = 0;
			for (; i < nSize; i++)
				m_pSrcEncData[i] = pSrcEncData[i];
			m_pSrcEncData[i] = 0x0000;

			this->m_bytePattern = bytePattern;
			this->m_bIsDataDecrypted = false;

			return true;
		};

        ///*!
        // calculate the size in byte of the decrypted data
        //
        // Return value - None
        //*/
	    unsigned m_fnSizeOf(void)
	    {
			return (m_bIsDataDecrypted) ? (this->m_nElementCount * sizeof(Type)) : 0;
	    };

        ///*!
        // opeator overloading on '[]'
        //
        // -# nIndex - [IN] the zero-based index to the decrypted data
        //
        // Return value - the decrypted data element
        //*/
	    Type& operator[](int nIndex)
	    {
			this->m_fnDecryptData();
			return (m_bIsDataDecrypted && (m_pElementList != NULL)) ? this->m_pElementList[nIndex] : 0;
	    };

        ///*!
        // opeator overloading on '(TYPE*)' to cast the data type
        //
        // Return value - the decrypted data element list
        //*/
	    operator Type*()
	    {
			this->m_fnDecryptData();
			return (m_bIsDataDecrypted) ? this->m_pElementList : NULL;
	    };

        ///*!
        // opeator overloading on '&'
        //
        // Return value - the decrypted data element list
        //*/
	    Type* operator &()
	    {
			this->m_fnDecryptData();
			return (m_bIsDataDecrypted) ? this->m_pElementList : NULL;
	    };
        
    };

    ///*!
    // this is a template class to secure sensitive array data. And it is version 2 of string encryption. 
    //
    // Note1: This class is not thread-safe. Caller should handle synchronization issue outside this class.
    // Note2: the sizeof() C operator can not work as it is expected because sizeof(CtrEncryptor) doesn't equal to sizeof(the decrypted data) !
    // Please use CtrEncryptor.m_fnSizeof() instead if you want to get the total bytes of the decrypted data.
	// sample :
	// CtrEncryptor<WCHAR> wszEncrypted(L"ด๚ธี...", 22);
	// WCHAR *pwszTarget = &wszEncrypted;
	// ::MessageBoxW(NULL, pwszTarget, L"encryption", MB_OK);

    //*/
    template <class Type> class CtrEncryptor
    {
    private:
	    ///*!
	    // the decrypted raw data array
	    //*/
        Type *m_pElementList;

	    ///*!
	    // the total element in 'm_pElementList'
	    //*/
	    int m_nElementCount;

	    ///*!
	    // the NULL-terminated original encrypted HEX string passed in the constructor
	    //*/
	    Type *m_pSrcEncData;

	    ///*!
	    // flag to indicate if 'm_pElementList' contains decrypted data, it is valid while m_bIsEncryption is set.
	    //*/
	    bool m_bIsDataEncrypted;

	    ///*!
	    // the pattern will be needed during protecting operation(encryption or decryption).
	    //*/
		BYTE m_bytePattern;

    protected:
	    ///*!
	    // encrypt 'm_pSrcEncData' to 'm_pElementList'
	    //
	    // Return value - true if successful, otherwise, false.
	    //*/
	    bool m_fnEncryptData(void)
	    {
		    if (!this->m_bIsDataEncrypted)
			{
				Type *pTarget;
				CtrEncryptContentObject encObject;
				if (m_pSrcEncData)
					encObject.fnEncryptContent(&pTarget, m_pSrcEncData, m_bytePattern);

				int i = 0;
				if (m_pElementList)
				{
					while (*(pTarget + i) != 0x0000 && i < m_nElementCount)
					{
						m_pElementList[i] = *(pTarget + i);
						i++;
					}
					m_pElementList[i] = 0x0000;
					m_bIsDataEncrypted = true;
				}
			}

	        return true;
	    };


    public:
        ///*!
        // constructor of CtrEncryptor template class
        //
        // -# pSrcEncData, the unprotected content
		// -# bytePattern, the pattern will be needed during protecting operation, the default is 28.
        // Return value - None
        //*/
	    CtrEncryptor(Type *pSrcEncData, BYTE bytePattern = 28)
	    {
			this->m_bytePattern = bytePattern;
			this->m_bIsDataEncrypted = false;
		    this->m_pSrcEncData = NULL;
		    this->m_pElementList = NULL;

			m_fnAttach(pSrcEncData, bytePattern);
	    };

	    virtual ~CtrEncryptor(void)
	    {
			m_fnDetach();
	    };

    public:
        ///*!
        // Detach member data : (m_pElementList & m_pSrcEncData)
        //
        // Return value - None
        //*/
	    void m_fnDetach(void)
	    {
		    if (NULL != this->m_pElementList){
			    delete [] this->m_pElementList;
			    this->m_pElementList = NULL;
		    }

		    // the given encrypted HEX string is not important so don't need to scramble the buffer
		    if (NULL != this->m_pSrcEncData){
			    delete [] this->m_pSrcEncData;
			    this->m_pSrcEncData = NULL;
		    }
			this->m_bytePattern = 0;
			this->m_bIsDataEncrypted = false;
	    };

        ///*!
        // Attach a new native(decrypter) charactor string with new pattern.
		// Call m_fnDetach before calling m_fnAttach for keep it without any memory leak.
        //
        // -# pSrcEncData, the unprotected content
		// -# bytePattern, the pattern will be needed during protecting operation.
		// 
        // Return value - true / false
        //*/
		bool m_fnAttach(Type *pSrcEncData, BYTE bytePattern)
		{
		    // total encrypted data bytes
		    int nSize = 0;
			if (pSrcEncData) {
				while (*(pSrcEncData + nSize) != 0x0000)
					nSize++;
			}

		    // calculate the total element after decrypting 'pszSrcEncData'
		    // strlen(pszSrcEncData) / 2 = the bytes used to represent the source HEX string
		    // (strlen(pszSrcEncData) / 2) / 2 = the bytes used to represent the source data
		    // (strlen(pszSrcEncData) / 4) / sizeof(Type) = the total element
			this->m_nElementCount = nSize * 4;

            this->m_pSrcEncData = new Type[nSize + 1];

		    if (NULL == this->m_pSrcEncData){
                #ifdef _DEBUG
				    __asm int 3
                #endif

			    return false;
		    }

            this->m_pElementList = new Type[this->m_nElementCount + 1];

		    if (NULL == this->m_pElementList){
                #ifdef _DEBUG
				    __asm int 3
                #endif

                return false;
		    }

		    // copy the given encrypted source data including NULL terminator
			int i = 0;
			for (; i < nSize; i++)
				m_pSrcEncData[i] = pSrcEncData[i];
			m_pSrcEncData[i] = 0x0000;

			this->m_bytePattern = bytePattern;
			this->m_bIsDataEncrypted = false;

			return true;
		};

        ///*!
        // calculate the size in byte of the decrypted data
        //
        // Return value - None
        //*/
	    unsigned m_fnSizeOf(void)
	    {
			return (m_bIsDataEncrypted) ? (this->m_nElementCount * sizeof(Type)) : 0;
	    };

        ///*!
        // opeator overloading on '[]'
        //
        // -# nIndex - [IN] the zero-based index to the decrypted data
        //
        // Return value - the decrypted data element
        //*/
	    Type& operator[](int nIndex)
	    {
			this->m_fnEncryptData();
			return (m_bIsDataEncrypted && (m_pElementList != NULL)) ? this->m_pElementList[nIndex] : 0;
	    };

        ///*!
        // opeator overloading on '(TYPE*)' to cast the data type
        //
        // Return value - the decrypted data element list
        //*/
	    operator Type*()
	    {
			this->m_fnEncryptData();
			return (m_bIsDataEncrypted) ? this->m_pElementList : NULL;
	    };

        ///*!
        // opeator overloading on '&'
        //
        // Return value - the decrypted data element list
        //*/
	    Type* operator &()
	    {
			this->m_fnEncryptData();
			return (m_bIsDataEncrypted) ? this->m_pElementList : NULL;
	    };
        
    };

  
    #ifdef _WIN32
    #ifndef __TCHAR_DEFINED
    #include <tchar.h>
    #endif
	typedef CtrDecryptor<TCHAR> CtrDecryptor_TCHAR;
	typedef CtrEncryptor<TCHAR> CtrEncryptor_TCHAR;
    #endif // _WIN32    

NS_TRLIB_END

#ifdef _WIN32
	///*!
	// pack CtrDecryptor to a macro to decrypt the encrypted string.\n
	//
	// -# name, a local variable of CtrDecryptor, to execut decrypting operation.\n
    // -# sz_encrypted, the encrpyted string.\n
	// -# int_pattern, the pattern will be needed during decrypting operation.\n
	// -# sz_original - [IN] the unencrypted string, assure the decrypted string is right even if TR is off.\n
	//
	// Return value - None\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// 
	// Usage:\n
	//
	//	iviTR_ALLOCATE_ENCRYPTED_TCHAR(testName, _T("6F21726F6972676969676E69616E6C61206C73207473727469726E69676E"), 33, _T("original string"));	
	//	...
	//  testName should be "original string", to do something...
	//	...
	//
	//*/
	#define iviTR_ALLOCATE_ENCRYPTED_TCHAR(name, sz_encrypted, int_pattern, sz_original)	\
		CtrDecryptor<TCHAR> name(sz_encrypted, int_pattern)


	///*!
	// Re-encrypt TCHAR string come from iviTR_ALLOCATE_ENCRYPTED_TCHAR.\n
	//
	// -# name, a local variable of CtrDecryptor, to execut decrypting operation.\n
	// -# int_pattern, the pattern will be needed during encrypt operation.\n
	//
	// Return value - None\n
	//
	// Supported platform:		All Windows
	// Performance overhead:	low
	// Security level:			Medium
	// Usage scope:				Macro scope
	//
	// Note:\n
	// 
	// Usage:\n
	//
	//	iviTR_ALLOCATE_ENCRYPTED_TCHAR(testName, _T("6F21726F6972676969676E69616E6C61206C73207473727469726E69676E"), 33, _T("original string"));
	//	...
	//  testName should be "original string", to do something...
	//  iviTR_RE_ENCRYPT_ALLOCATED_TCHAR(testName, 55);
	//	...
	//
	//*/
	#define iviTR_RE_ENCRYPT_ALLOCATED_TCHAR(name, int_pattern) \
	{ \
		TCHAR *pszEncrypted_##name;	\
		CtrEncryptContentObject __Object_##name;	\
		__Object_##name.fnEncryptContent(&pszEncrypted_##name, name, int_pattern);	\
		name.m_fnDetach();	\
		name.m_fnAttach(pszEncrypted_##name, int_pattern);	\
	}

#elif defined(__linux__)
	#define iviTR_ALLOCATE_ENCRYPTED_TCHAR(name, sz_encrypted, int_pattern, sz_original)	\
		TCHAR name[] = sz_original
	#define iviTR_RE_ENCRYPT_ALLOCATED_TCHAR(name, int_pattern) {}
#endif // _WIN32

#else // __cplusplus
	#define iviTR_ALLOCATE_ENCRYPTED_TCHAR(name, sz_encrypted, int_pattern, sz_original)	\
		TCHAR name[] = sz_original
	#define iviTR_RE_ENCRYPT_ALLOCATED_TCHAR(name, int_pattern) {}
#endif // __cplusplus


#endif // _IVI_TRUTIL_H