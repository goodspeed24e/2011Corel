#ifndef _DXVA_ERROR_H_
#define _DXVA_ERROR_H_

#include <ddraw.h>
#define checkDDError(errCode) ((errCode==DDERR_WASSTILLDRAWING) || (errCode==E_PENDING))
#endif
