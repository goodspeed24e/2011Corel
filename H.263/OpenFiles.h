
#include "stdafx.h"
#include <stdio.h>

#define T_bmp 1
#define T_yuv 2
#define T_raw 3
#define T_sif 4

BYTE * OpenImageFile(CString szFilePathName, int *width, int *height, int nType);

CString GetNextFileName(CString filename,BOOL flag);

BYTE * OpenSIF(CString fileName, int *width, int *height);

#define WIDTHBYTES(i)    ((i+31)/32*4)
BYTE * OpenBMP(CString fileName, int *width, int *height);

BYTE * OpenRAW(CString fileName, int *width, int *height);


