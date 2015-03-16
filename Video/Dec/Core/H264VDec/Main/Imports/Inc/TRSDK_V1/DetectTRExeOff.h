#ifndef __DETECT_TREXE_OFF_H_
#define __DETECT_TREXE_OFF_H_

// detect TRExe applied macro -- turn off
#define iviTR_CHECK_TREXE_APPLIED()
#define TR_IS_TREXE_APPLIED(pbTR_ENABLE) \
    { \
        (*pbTR_ENABLE) = false; \
    }

#endif // __DETECT_TREXE_OFF_H_
