#ifndef _DISPSVR_DIRECT_DRAW_HELPER_H_
#define _DISPSVR_DIRECT_DRAW_HELPER_H_

namespace DispSvr
{
    class CDirectDrawHelper
    {
    public:
        static HRESULT QueryVideoMemorySize(DWORD* pdwTotalMem, DWORD* pdwFreeMem);
    };
}

#endif  // _DISPSVR_DIRECT_DRAW_HELPER_H_