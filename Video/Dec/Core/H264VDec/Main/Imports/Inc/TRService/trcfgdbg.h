//#include	"../LIBGPI/gpi.h"

// DEBUG EVALUATION release
// config: DP on, TR off, TB on
// overwrite previous config settings
//
// WE DISABLE CSS - this allows no protection to be put on - no keys can be found.
// WE ENABLE TIMEBOMB.

#undef TR_DEBUG_PRINTF
#undef TR_ENABLE
#undef TR_ENABLE_CSS
#undef TR_OPTIONS

#define TR_DEBUG_PRINTF
//#define TR_OPTIONS	(GPI_OPTION_DVD|GPI_OPTION_VCD|GPI_OPTION_SVCD|GPI_OPTION_AC3|GPI_OPTION_NOTIMEOUT|GPI_OPTION_NOEXPIRE)
