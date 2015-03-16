//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2007 InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _I_VDO_PARAMETER_H_
#define _I_VDO_PARAMETER_H_

#include "VdoParamType.h"

// {02750C2F-C6D5-4c50-BE14-D42DDA9A1B2C}
EXTERN_GUID(IID_IVdoParameter, 
						0x2750c2f, 0xc6d5, 0x4c50, 0xbe, 0x14, 0xd4, 0x2d, 0xda, 0x9a, 0x1b, 0x2c);

class IVdoParameter : public IUnknown
{
public:
	virtual HRESULT GetParameter(void* parameter, VdoParamType pType = VDO_PARAM_UNKOWN) const = 0;
	virtual HRESULT SetParameter(void* parameter, VdoParamType pType = VDO_PARAM_UNKOWN)= 0;	
};
	
#endif //_I_VDO_PARAMETER_H_
