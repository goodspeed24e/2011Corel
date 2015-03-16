#ifndef _HIJACKDETECT_DD_H_
#define _HIJACKDETECT_DD_H_

#define DD_IMAGESIZE	299008

#define DD_OFFSET_ADDREF	1
#define DD_OFFSET_RELEASE	2
#define DD_OFFSET_FLIP		11

INT GetDetFuncNumDD();
HRESULT LoadSurfaceVTableDD(LPDWORD pVTableDetect, LPDWORD pSurface);
HRESULT LoadSurfaceVTableOffsetDD(LPDWORD pVTableOffset);

#endif // _UTIL_HIJACKDETECT_DD_H_