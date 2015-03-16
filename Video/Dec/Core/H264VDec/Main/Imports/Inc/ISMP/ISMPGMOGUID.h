#ifndef _CISMPGMOGUID_H
#define _CISMPGMOGUID_H

#include <windows.h>

// {163977EB-22FA-4fdb-99DC-AF3E16FC3176}
static const GUID IID_ISMPGMO = 
{ 0x163977eb, 0x22fa, 0x4fdb, { 0x99, 0xdc, 0xaf, 0x3e, 0x16, 0xfc, 0x31, 0x76 } };

class ISMPGMO: public IUnknown
{
public:
	virtual STDMETHODIMP SPConnect(void *pData) = 0;  
};
#endif