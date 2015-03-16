#ifndef __MSDK_H__
#define __MSDK_H__

typedef void* (__cdecl * lpfn_GetFreeSurface)(void* pEncInst);
typedef void* (__cdecl * lpfn_UpdateSurface)(void* pEncInst, void* NewEncodingSurface);

typedef struct __MSDK_MEM_ALLOCATOR__
{
	void* pEncInst;
	lpfn_GetFreeSurface		MSDK_GetFreeSurface;
	lpfn_UpdateSurface		MSDK_UpdateSurface;
	
	DWORD reserved[29];
}MSDK_MEM_ALLOCATOR;

#endif
