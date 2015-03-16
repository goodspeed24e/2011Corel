// VC1line21.h: interface for the CVC1line21 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINE21_H__1530E27C_A03A_4469_9BA8_896E83132CB8__INCLUDED_)
#define AFX_LINE21_H__1530E27C_A03A_4469_9BA8_896E83132CB8__INCLUDED_

#define LINE21BUF_SIZE 256

#include <windows.h>

#ifdef USE_FASTCALL
#define __fastcall
#endif

class CATSCDec  
{
public:

	static CATSCDec* __fastcall CreateCATSCDec();
	static HRESULT __fastcall destroyInstance();

	INT GetCCData( long *plCCLength, unsigned char **pplCCBuffer, int bCC2);
	void Reset();
	INT DecodeLine21Data (LPVOID  pBuffer, DWORD    dwBufferLen);
	CATSCDec();
	~CATSCDec();

private:
	static UINT m_uInstanceCount;
	static CATSCDec* m_pCATSCDec;

	unsigned char  m_cc1buf[LINE21BUF_SIZE]; // cc data of 1st field or frame
	unsigned int   m_cc1len;
};

#endif // !defined(AFX_LINE21_H__1530E27C_A03A_4469_9BA8_896E83132CB8__INCLUDED_)
