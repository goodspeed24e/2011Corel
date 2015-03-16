#ifndef _INC_CTRHANDSHAKOBJECT_H
#define _INC_CTRHANDSHAKOBJECT_H

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