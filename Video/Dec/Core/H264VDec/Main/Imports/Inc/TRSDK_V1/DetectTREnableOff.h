#ifndef __DETECT_TR_ENABLE_OFF_H_
#define __DETECT_TR_ENABLE_OFF_H_

// detect TREnable applied macro -- turn off
#define iviTR_CHECK_ENGINE_TR_ENABLE_APPLIED()
#define iviTR_CHECK_UI_TR_ENABLE_APPLIED()

#define iviTR_IS_TR_ENABLE_APPLIED_FOR_ENGINE(pbTR_ENABLE, pbTR_ENABLE_NEWMACROS) \
    { \
        (*pbTR_ENABLE) = (*pbTR_ENABLE_NEWMACROS) = false; \
    }

// call this macro to check if TR_ENABLE/TR_ENABLE_NEWMACROS is applied for UI part
#define iviTR_IS_TR_ENABLE_APPLIED_FOR_UI(pbTR_ENABLE, pbTR_ENABLE_NEWMACROS) \
    { \
        (*pbTR_ENABLE) = (*pbTR_ENABLE_NEWMACROS) = false; \
    }

#endif // __DETECT_TR_ENABLE_OFF_H_