// detect TRExe applied macro -- turn off
#define iviTR_CHECK_TREXE_APPLIED()
#define TR_IS_TREXE_APPLIED(pbTR_ENABLE) \
    { \
        (*pbTR_ENABLE) = false; \
    }
