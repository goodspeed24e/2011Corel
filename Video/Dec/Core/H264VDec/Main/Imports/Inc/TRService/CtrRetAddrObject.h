//=============================================================================
//  THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
//  ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//  ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE
//  COMPANY.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2006  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
// Copyright (C) 1991 - 1999 Rational Software Corporation

#if defined (_MSC_VER) && (_MSC_VER >= 1000)
#pragma once
#endif
#ifndef _INC_CTRRETADDROBJECT_452DF0D1008D_INCLUDED
#define _INC_CTRRETADDROBJECT_452DF0D1008D_INCLUDED

interface ItrDATAManagerBase;

///*!
// [Internal class. Do not use it.]
//A class object that keeps current thread id and current 
//function return address and sends them as a trRADATA 
//package to TRDATA Manager for saving in its destructor.
//*/
//##ModelId=452DF0D1008D
class CtrRetAddrObject 
{
public:
	///*!
	//The base interface pointer to TRDATA Mananger.
	//*/
	//##ModelId=45308EF7007E
	ItrDATAManagerBase* m_pItrDATAManagerBase;

	//##ModelId=452DF23503A3
	CtrRetAddrObject(DWORD& dwRetAddr);

	//##ModelId=452DF2990059
	virtual ~CtrRetAddrObject();

private:

	///*!
	//A thread identifier.
	//*/
	//##ModelId=452DF0F003DA
	DWORD m_dwThreadId;

	///*!
	//A memory address location where the next instruction is 
	//in and carried out after the (callee) function's 
	//execution.  Such address is within the caller function.
	//*/
	//##ModelId=452DF11C03CC
	DWORD m_dwRetAddr;

};

#endif /* _INC_CTRRETADDROBJECT_452DF0D1008D_INCLUDED */
