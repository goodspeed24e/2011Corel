#ifndef _INC_CTRHANDSHAKOBJECT_H
#define _INC_CTRHANDSHAKOBJECT_H

#include "IVITrWinDef.h"

NS_TRLIB_BEGIN

#ifdef __cplusplus
class CtrHandshakObject
{
public:
	CtrHandshakObject();
	~CtrHandshakObject();
	void fnSetObject(HANDLE hHandle);
private:
	HANDLE m_hMutex;
};
#endif

NS_TRLIB_END

#endif