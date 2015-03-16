#ifndef _IVI_TRUTIL_H
#define _IVI_TRUTIL_H

#ifdef _WIN32

	///*!
	// [Internal function. Do not call it.]
	//
	// Encrypt string.\n
	//
	//-# szValue - [IN] the content needs to be encrypted by XOR with bytePattern.\n
	//-# bytePattern - [IN] a pattern to encrypt code by XOR.\n 
	//-# nEncryptType - [IN] 0, encrypt CLSID, szValue is clsid by StringfromCLSID(); 1, encrypt string\n
	// Return value - [bool] not use.\n
	// Supported platform: All Windows\n
	//*/
	bool __cdecl trEncryptContent(char *szValue, BYTE bytePattern, int nEncryptType);

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
	// 
	//*/
	#define iviTR_ENCRYPT_CONTENT(sz_value, byte_pattern, int_encrypttype) \
	{\
		trEncryptContent(sz_value, byte_pattern, int_encrypttype);\
	}

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
	#define iviTR_ENCRYPT_CONTENT(sz_value, byte_pattern, int_encrypttype)
	#define iviTR_DECRYPT_CONTENT(sz_value, byte_pattern, int_decrypttype)
#endif // _WIN32

    #define ENCRYPT_BYTE_PATTERN_FOR_CtrArrayOf 28

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

#endif