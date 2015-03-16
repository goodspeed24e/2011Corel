#undef TR_DEBUG_PRINTF
#undef TR_ENABLE
#undef TR_ENABLE_CSS

#if !WINDVD_CUSTOMER // Not defined when built from IDE; defined when build from commandline
	//#pragma message ("WINDVD_CUSTOMER is not defined")
	#define TR_DEBUG_PRINTF  
#else // External (release)
	//#pragma message ("WINDVD_CUSTOMER is defined")
	#if !defined(_DEBUG)
	#ifndef USE_OVIA
		#define TR_ENABLE
	#endif
	#endif	// !defined(_DEBUG)
#endif	//  !WINDVD_CUSTOMER 

#ifndef NOCSS
#define TR_ENABLE_CSS
#endif	//NOCSS
